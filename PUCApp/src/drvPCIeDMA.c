/**********************************************************************
* Asyn device support using TCP stream or UDP datagram port           *
**********************************************************************/       
/***********************************************************************
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory, and the Regents of the University of
* California, as Operator of Los Alamos National Laboratory, and
* Berliner Elektronenspeicherring-Gesellschaft m.b.H. (BESSY).
* asynDriver is distributed subject to a Software License Agreement
* found in file LICENSE that is included with this distribution.
***********************************************************************/

/*
 * drvAsynIPPort.c,v 1.35 2007/02/05 17:25:39 norume Exp
 */

/* Previous versions of drvAsynIPPort.c (1.29 and earlier, asyn R4-5 and earlier)
 * attempted to allow 2 things:
 * 1) Use an EPICS timer to time-out an I/O operation.
 * 2) Periodically check (every 5 seconds) during a long I/O operation to see if
 *    the operation should be cancelled.
 *
 * Item 1) above was not really implemented because there is no portable robust was
 * to abort a pend I/O operation.  So the timer set a flag which was checked after
 * the poll() was complete to see if the timeout had occured.  This was not robust,
 * because there were competing timers (timeout timer and poll) which could fire in
 * the wrong order.
 *
 * Item 2) was not implemented, because asyn has no mechanism to issue a cancel
 * request to a driver which is blocked on an I/O operation.
 *
 * Since neither of these mechanisms was working as designed, the driver has been 
 * re-written to simplify it.  If one or both of these are to be implemented in the future
 * the code as of version 1.29 should be used as the starting point.
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <osiUnistd.h>
#include <osiSock.h>
#include <cantProceed.h>
#include <errlog.h>
#include <iocsh.h>
#include <epicsAssert.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <osiUnistd.h>

#include <epicsExport.h>
#include "asynDriver.h"
#include "asynOctet.h"
#include "unionConversion.h"
#include "readwriteDMA.h"

#include "/usr/include/pciDriver/driver/pciDriver.h"
#include "/usr/include/pciDriver/lib/pciDriver.h"

#define BRAM_SIZE  0x4000

#if defined(__rtems__)
# define USE_SOCKTIMEOUT
#else
# define USE_POLL
# if defined(vxWorks) || defined(_WIN32)
#  define FAKE_POLL
# else
#  include <sys/poll.h>
# endif
#endif

/*
 * This structure holds the hardware-specific information for a single
 * asyn link.  There is one for each IP socket.
 */
typedef struct {
    asynUser          *pasynUser;        /* Not currently used */
    char              *DeviceName;
    char              *portName;
    pd_device_t	      *pci_handle;
    void              *kernel_memory;
    pd_kmem_t         *kmem_handle;
    pd_umem_t         *umem_handle;
    void              **bar;
    int               broadcastFlag;
    int               fd;
    unsigned long     nRead;
    unsigned long     nWritten;
    osiSockAddr       farAddr;
    asynInterface     common;
    asynInterface     octet;

} pcie_dma_pvt;


/*
 * Clean up a pcieController
 */
static void
pcie_dma_pvt_Cleanup(pcie_dma_pvt *pvt)
{
    if (pvt) {
        if (pvt->pci_handle != NULL)
            pd_close(pvt->pci_handle);
        if (pvt->bar != NULL)//TODO:unmapping bars
        //TODO: unmap kernel memory
            //for(i=0;i<6;i++){
                  
        free(pvt->portName);
        free(pvt->DeviceName);
        free(pvt);
    }
}

/*Beginning of asynCommon methods*/
/*
 * Report link parameters
 */
static void
report(void *drvPvt, FILE *fp, int details)
{
    pcie_dma_pvt *pvt = (pcie_dma_pvt *)drvPvt;

    assert(pvt);
    fprintf(fp, "Port %s: %sonnected\n",
        pvt->portName,
        pvt->DeviceName >= 0 ? "C" : "Disc");
    if (details >= 1) {
        fprintf(fp, "                    device name: %s\n", pvt->DeviceName);
        //fprintf(fp, "    Characters written: %lu\n", tty->nWritten);
        //fprintf(fp, "       Characters read: %lu\n", tty->nRead);
    }
}

/*
 * Create a link
 */
static asynStatus
connectIt(void *drvPvt, asynUser *pasynUser)
{
     pcie_dma_pvt *pvt = (pcie_dma_pvt *)drvPvt; 

    
     if (pvt->pci_handle == NULL || pvt->kernel_memory == NULL ||
         pvt->kmem_handle == NULL || pvt->umem_handle == NULL )
	     /*initialize pci handle*/ 
	     pcie_dma_pvt_Cleanup(pvt);
             pvt->pci_handle = callocMustSucceed(1,sizeof(pd_device_t),"pd_device_t"); 
	     if (pd_open(atoi(pvt->DeviceName), pvt->pci_handle) != 0) {
		  printf("failed to open device\ntry another device id\n");
		  return -1;
	     }
	     else
	     {
		  printf("device opened\n");
	     }
	     /*allocating bars*/

	    void **bar = callocMustSucceed(6, sizeof(void*), "bar");
	    int i = 0;
	    printf("Allocating bars\n");
	    for (i=0;i<=4;i=i+2){
		  bar[i] = pd_mapBAR(pvt->pci_handle, i);
		  if (bar[i] == NULL){
		        printf("failed mapping bar%d\n",i);
		        pcie_dma_pvt_Cleanup(pvt);
		        return -1;
		  }
	    }

	    pvt->bar = bar;

	    /*allocating kernel memory*/

	    pvt->kmem_handle = callocMustSucceed(1, sizeof(pd_kmem_t), "pd_kmem_t");
	    pvt->kernel_memory = pd_allocKernelMemory(pvt->pci_handle, BRAM_SIZE, pvt->kmem_handle);
	    if(pvt->kernel_memory == NULL){
		  printf("Failed allocating kernel memory\n");
		  pcie_dma_pvt_Cleanup(pvt);
		  return -1;
	    }

	    /*allocating user memory*/
	    void *mem = NULL;
	    posix_memalign( (void**)&mem,16,BRAM_SIZE );//TODO:compilation warning!!implicit declaration
	    if (mem == NULL){
		  printf("memory aligned failed\n");
		  pcie_dma_pvt_Cleanup(pvt);
		  return -1;
	    }
	    pvt->umem_handle = callocMustSucceed(1, sizeof(pd_umem_t), "pd_umem_t");
	    pd_mapUserMemory(pvt->pci_handle, mem, BRAM_SIZE, pvt->umem_handle);//TODO:check return value
	    free(mem);
    
     return asynSuccess;
}

static asynStatus
disconnect(void *drvPvt, asynUser *pasynUser)
{
    pcie_dma_pvt *pvt = (pcie_dma_pvt *)drvPvt;
    
    assert(pvt);
    asynPrint(pasynUser, ASYN_TRACE_FLOW,
                                    "%s disconnect\n", pvt->DeviceName);
    pcie_dma_pvt_Cleanup(pvt);
    return asynSuccess;
}

/*Beginning of asynOctet methods*/
/*
 * Write to the TCP port
 */
static asynStatus writeRaw(void *drvPvt, asynUser *pasynUser,
    const char *data, size_t numchars,size_t *nbytesTransfered)
{
    pcie_dma_pvt *pvt = (pcie_dma_pvt *)drvPvt;
    //int thisWrite;
    asynStatus status = asynSuccess;
    int writePollmsec;

    assert(pvt);
    asynPrint(pasynUser, ASYN_TRACE_FLOW,
              "%s write.\n", pvt->DeviceName);
    asynPrintIO(pasynUser, ASYN_TRACEIO_DRIVER, data, numchars,
                "%s write %d\n", pvt->DeviceName, (int)numchars);
    if (pvt->pci_handle == NULL) {
        epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,
                      "%s disconnected:", pvt->DeviceName);
        return asynError;
    }
    if (numchars == 0) {
        *nbytesTransfered = 0;
        return asynSuccess;
    }
    writePollmsec = pasynUser->timeout * 1000.0;
    if (writePollmsec == 0) writePollmsec = 1;
    if (writePollmsec < 0) writePollmsec = -1;

    char *bar = (char *)malloc(sizeof(char)*4);
    char *address_str = (char *)malloc(sizeof(char)*4);
    char *message = (char *)malloc(sizeof(char)*(numchars));
    int address;
    int i = 0;
    int j = 0;
    int nchars = 0;

    for(i=0;i<4;i++)
        bar[i] = data[i];
    for(i=0;i<4;i++)
        address_str[i] = data[i+4];
    for(i=0;i<numchars;i++)
        message[i] = data[8+i];  

    address = atoi(address_str);

    printf("writting %s @ %s. message %s\n",bar,address_str,message);

    if (strcmp(bar,"BAR2")==0){
         uint32_t *sdram = (uint32_t*)pvt->bar[2] + address;
         unsigned_int_32_value auxi32;
         while(1){
                   for(j=0;j < 4;j++){
                        auxi32.vvalue[j] = message[i*4+j];
                        if (nchars == numchars)
                             break;
                        nchars++;
                   }
                   if (nchars == numchars)
                        break;
                   sdram[i] = auxi32.ui32value;
                   i++;
         }
    }
    else if (strcmp(bar,"BAR0")){
    }
    free(bar);
    free(address_str);
    free(message);
    *nbytesTransfered = nchars;
    return status;
}

/*
 * Read from the pcie interface
 * the data string must be filled with register+address
 * data MUST be dinamically allocated!
 */
static asynStatus readRaw(void *drvPvt, asynUser *pasynUser,
    char *data, size_t maxchars,size_t *nbytesTransfered,int *gotEom)
{
    pcie_dma_pvt *pvt = (pcie_dma_pvt *)drvPvt;
//    int thisRead;
    int readPollmsec;
    asynStatus status = asynSuccess;

    assert(pvt);//TODO:is necessary?
    asynPrint(pasynUser, ASYN_TRACE_FLOW,
              "%s read.\n", pvt->DeviceName);
    if (pvt->pci_handle == NULL) {
        epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,
                      "%s disconnected:", pvt->DeviceName);
        return asynError;
    }
    if (maxchars <= 0) {
        epicsSnprintf(pasynUser->errorMessage,pasynUser->errorMessageSize,
                      "%s maxchars %d. Why <=0?\n",pvt->DeviceName,(int)maxchars);
        return asynError;
    }
    readPollmsec = pasynUser->timeout * 1000.0;
    if (readPollmsec == 0) readPollmsec = 1;
    if (readPollmsec < 0) readPollmsec = -1;
    /*setting hardware blocking*/

    if (gotEom) *gotEom = 0;
/**-->**/
    
    int i = 0;
    int j = 0;
    char *bar = (char*)malloc(sizeof(char)*4);
    char *address_str = (char*)malloc(sizeof(char)*4);
    char *answer = (char*)malloc(sizeof(char)*maxchars);

    int address;
    int nchars = 0;

    for(i=0;i<4;i++)
        bar[i] = data[i];
    for(i=0;i<4;i++)
        address_str[i] = data[i+4];

    address = atoi(address_str);
    
    printf("reading %s @ %s %d  \n",bar,address_str,address);
    
    if (strcmp(bar,"BAR2") == 0){
         
         uint32_t *sdram = (uint32_t*)pvt->bar[2] + address;
         unsigned_int_32_value auxi32;
         while(1){
                   auxi32.ui32value = sdram[i];
                   for(j=0;j < 4;j++){
                        printf("%c\n",auxi32.vvalue[j]);
                        answer[i*4+j] = auxi32.vvalue[j];
                        if (nchars == maxchars)
                             break;
                        nchars++;
                   }
                   if (nchars == maxchars)
                        break;
                   i++;
         }
    }
    if (strcmp(bar, "DMAr") == 0){

         uint32_t *sdram = (uint32_t*)pvt->bar[2] + address;
         unsigned_int_32_value auxi32;

    }
    else if (strcmp(bar,"BAR0")){
    }
    free(data);
    data = answer;
    free(bar);
    free(address_str);
    
    //TODO:check correctness

    return status;
}

/*
 * Flush pending input
 */
static asynStatus
flushIt(void *drvPvt,asynUser *pasynUser)
{
    pcie_dma_pvt *pvt = (pcie_dma_pvt *)drvPvt;
    return asynSuccess;
}

/*
 * asynCommon methods
 */
static const struct asynCommon drvPcieDMACommon = {
    report,
    connectIt,
    disconnect
};

/*
 * Configure and register an IP socket from a hostInfo string
 */
/*
 * Port name: port being connected
 * hostinfo: device name
 */
int
drvPcieDMAConfigure(const char *portName,
                     const char *hostInfo,
                     unsigned int priority,
                     int noAutoConnect,
                     int noProcessEos)
{
    pcie_dma_pvt *pvt;
    asynInterface *pasynInterface;
    asynStatus status;
    int nbytes;
    asynOctet *pasynOctet;
    static int firstTime = 1;

    /*
     * Check arguments
     */
    if (portName == NULL) {
        printf("Port name missing.\n");
        return -1;
    }
    if (hostInfo == NULL) {
        printf("Device host information missing.\n");
        return -1;
    }

    /*
     * Perform some one-time-only initializations
     */
    if (firstTime) {
        firstTime = 0;
    }

    /*
     * Create a driver
     */
    nbytes = sizeof(*pvt) + sizeof(asynOctet);
    pvt = (pcie_dma_pvt *)callocMustSucceed(1, nbytes,
          "drvPcieDMAConfigure()");
    pasynOctet = (asynOctet *)(pvt+1);
    
    pvt->DeviceName = epicsStrDup(hostInfo);
    pvt->portName = epicsStrDup(portName);

    /*
     * Parse configuration parameters
     */

    /*TODO: check device name format*/

    /*check known device/host*/
    char temp[50];
    sprintf( temp, "/dev/fpga%s", hostInfo);
    FILE* filep = fopen(temp, "r");
    if (filep)
    {
       printf("\nfile descriptor found\n");
       fclose(filep);
    }
    else
    {
       printf("\nfile descriptor was not found!\n");
       pcie_dma_pvt_Cleanup(pvt);
       return -1;
    }

     /*initialize pci handle*/
     pvt->pci_handle = callocMustSucceed(1,sizeof(pd_device_t),"pd_device_t"); 
     if (pd_open(atoi(pvt->DeviceName), pvt->pci_handle) != 0) {
          printf("failed to open device fpga%d\ntry another device id\n",atoi(pvt->DeviceName));
          return -1;
     }
     else
     {
          printf("device opened\n");
     }
     /*allocating bars*/

    void **bar = callocMustSucceed(6, sizeof(void*), "bar");
    int i = 0;
    printf("Allocating bars\n");
    for (i=0;i<=4;i=i+2){
          bar[i] = pd_mapBAR(pvt->pci_handle, i);
          if (bar[i] == NULL){
                printf("failed mapping bar%d\n",i);
                pcie_dma_pvt_Cleanup(pvt);
                return -1;
          }
    }

    pvt->bar = bar;

    /*allocating kernel memory*/

    pvt->kmem_handle = callocMustSucceed(1, sizeof(pd_kmem_t), "pd_kmem_t");
    pvt->kernel_memory = pd_allocKernelMemory(pvt->pci_handle, BRAM_SIZE, pvt->kmem_handle);
    if(pvt->kernel_memory == NULL){
          printf("Failed allocating kernel memory\n");
          pcie_dma_pvt_Cleanup(pvt);
          return -1;
    }

    /*allocating user memory*/
    void *mem = NULL;
    posix_memalign( (void**)&mem,16,BRAM_SIZE );//TODO:compilation warning!!implicit declaration
    if (mem == NULL){
          printf("memory aligned failed\n");
          pcie_dma_pvt_Cleanup(pvt);
          return -1;
    }
    pvt->umem_handle = callocMustSucceed(1, sizeof(pd_umem_t), "pd_umem_t");
    pd_mapUserMemory(pvt->pci_handle, mem, BRAM_SIZE, pvt->umem_handle);//TODO:check return value
    free(mem);

    /*
    *  Link with higher level routines
    */
    pasynInterface = (asynInterface *)callocMustSucceed(2, sizeof *pasynInterface, "drvPcieDMAConfigure");
    pvt->common.interfaceType = asynCommonType;
    pvt->common.pinterface  = (void *)&drvPcieDMACommon;
    pvt->common.drvPvt = pvt;
    if (pasynManager->registerPort(pvt->portName,
                                   ASYN_CANBLOCK,
                                   !noAutoConnect,
                                   priority,
                                   0) != asynSuccess) {
        printf("drvPcieDMAConfigure: Can't register myself.\n");
        pcie_dma_pvt_Cleanup(pvt);
        return -1;
    }
    status = pasynManager->registerInterface(pvt->portName,&pvt->common);
    if(status != asynSuccess) {
        printf("drvPcieDMAConfigure: Can't register common.\n");
        pcie_dma_pvt_Cleanup(pvt);
        return -1;
    }
    pasynOctet->read = readRaw;//why not readRaw = readRaw
    pasynOctet->write = writeRaw;
    pasynOctet->flush = flushIt;
    pvt->octet.interfaceType = asynOctetType;
    pvt->octet.pinterface  = pasynOctet;
    pvt->octet.drvPvt = pvt;
    status = pasynOctetBase->initialize(pvt->portName,&pvt->octet,
        (noProcessEos ? 0 : 1), (noProcessEos ? 0 : 1), 1);
    if(status != asynSuccess) {
        printf("drvAsynIPPortConfigure: pasynOctetBase->initialize failed.\n");
        pcie_dma_pvt_Cleanup(pvt);
        return -1;
    }
    pvt->pasynUser = pasynManager->createAsynUser(0,0);
    status = pasynManager->connectDevice(pvt->pasynUser,pvt->portName,-1);
    if(status != asynSuccess) {
        printf("connectDevice failed %s\n",pvt->pasynUser->errorMessage);
        pcie_dma_pvt_Cleanup(pvt);
        return -1;
    }
    int n=1;
    int goteom=1;
    char *data = "BAR20000HELLO WORLD" ;
    char *answer = (char*)malloc(sizeof(char)*8);
    answer[0]='B';
    answer[1]='A';
    answer[2]='R';
    answer[3]='2';
    answer[4]='0';
    answer[5]='0';
    answer[6]='0';
    answer[7]='0';
    printf("write test\n");
    pasynOctet->write(pvt,pvt->pasynUser,data,11,&n);
    printf("read test\n");
    pasynOctet->read(pvt,pvt->pasynUser,answer,10,&n,&goteom);


    return 0;
}

/*
 * IOC shell command registration
 */
static const iocshArg drvPcieDMAConfigureArg0 = { "port name",iocshArgString};
static const iocshArg drvPcieDMAConfigureArg1 = { "host:port [protocol]",iocshArgString};
static const iocshArg drvPcieDMAConfigureArg2 = { "priority",iocshArgInt};
static const iocshArg drvPcieDMAConfigureArg3 = { "disable auto-connect",iocshArgInt};
static const iocshArg drvPcieDMAConfigureArg4 = { "noProcessEos",iocshArgInt};
static const iocshArg *drvPcieDMAConfigureArgs[] = {
    &drvPcieDMAConfigureArg0, &drvPcieDMAConfigureArg1,
    &drvPcieDMAConfigureArg2, &drvPcieDMAConfigureArg3,
    &drvPcieDMAConfigureArg4};
static const iocshFuncDef drvPcieDMAConfigureFuncDef =
                      {"drvPcieDMAConfigure",5,drvPcieDMAConfigureArgs};
static void drvPcieDMAConfigureCallFunc(const iocshArgBuf *args)
{
    drvPcieDMAConfigure(args[0].sval, args[1].sval, args[2].ival,
                           args[3].ival, args[4].ival);
}

/*
 * This routine is called before multitasking has started, so there's
 * no race condition in the test/set of firstTime.
 */
static void
drvPcieDMARegisterCommands(void)
{
    static int firstTime = 1;
    if (firstTime) {
        iocshRegister(&drvPcieDMAConfigureFuncDef,drvPcieDMAConfigureCallFunc);
        firstTime = 0;
    }
}
epicsExportRegistrar(drvPcieDMARegisterCommands);

