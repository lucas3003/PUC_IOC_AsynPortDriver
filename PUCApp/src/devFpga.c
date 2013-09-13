/* 
 * Support for CAEN A2620 Power Supplies
 * 
 * Author: W. Eric Norum
 * "2011/02/18 23:15:06 (UTC)"
 *
 * This code was inspired by the driver produced by Giulio Gaio
 * <giulio.gaio@elettra.trieste.it> but takes a considerably different
 * approach and requires neither the sequencer nor the IMCA library.
 *
 * Compile-time options:
 *   Uncomment the "#define ENABLE_TIMING_TESTS 1" line to enable code
 *   that measures the time taken to process a transaction.
 */

/************************************************************************\
* Copyright (c) 2011 Lawrence Berkeley National Laboratory, Accelerator
* Technology Group, Engineering Division
* This code is distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution.
\*************************************************************************/

#define MAX 16
#define KBUF_SIZE (4096)
#define UBUF_SIZE (4096)
#define BRAM_SIZE  0x4000
#define DMA_WRITE_CMD 0xc0000000
#define DMA_READ_CMD 0x80000000
#define IRQ_ON_CMD 0x40000000
#define IRQ_OFF_CMD 0x00000000

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "asynDriver.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <asynOctet.h>
#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <errlog.h>
#include <iocsh.h>
#include "asynOctetSyncIO.h"
#include "asynFloat32Array.h"
#include "asynInt32.h"
#include "asynFloat64.h"
//#include "asynFloat32ArraySyncIO.h"
#include "asynCommonSyncIO.h"
#include "asynStandardInterfaces.h"
//#include "drvAsynIPPort.h"
#include "devFpga.h"
#include <stdbool.h>
#include <epicsExport.h>
#include "unionConversion.h"
#include "FpgaPcieRecordParams.h"
#include "readwriteDMA.h"
#include "/usr/include/pciDriver/driver/pciDriver.h"
#include "/usr/include/pciDriver/lib/pciDriver.h"//TODO: small include

#define KBUF_SIZE (4096)
#define UBUF_SIZE (4096)

typedef enum dstatus
{
	READING_KM,
	WRITTING_KM,
	IDLE
} dma_status;

typedef struct DataAcquisitionDirectory
{
	char dir_str[100];
	int n_chars;
	bool pini;
} DataAcquisitionDirectory;

/*
 * Interposed layer private storage
 */
typedef struct FrontendPvt {
	asynUser      *pasynUser;      /* To perform lower-interface I/O */

	asynInterface  asynCommon;     /* Our interfaces */
	asynInterface  asynInt32;
	asynInterface  asynFloat64;
	asynInterface  asynDrvUser;
	asynInterface  asynOctet;
	asynInterface  Float32Array;
	asynInterface  asynOctetSyncIO;

	unsigned long commandCount;
	unsigned long setpointUpdateCount;
	unsigned long retryCount;
	unsigned long noReplyCount;
	unsigned long badReplyCount;

	char *serverAddress;
	char *portName;

	pd_device_t *pci_handle;
	pd_kmem_t *kmem_handle;
	pd_umem_t *umem_handle;
	void *kernel_memory;
	void **bar;
	dma_status status;
	
	int seed;

	DataAcquisitionDirectory *directory;

} FpgaPciePvt;

static void intTask (FpgaPciePvt *ppvt);

/*
 * asynCommon methods
 */
static void
report(void *pvt, FILE *fp, int details)
{
    FpgaPciePvt *ppvt = (FpgaPciePvt *)pvt;

    if (details >= 1) {
#ifdef ENABLE_TIMING_TESTS
        fprintf(fp, "Transaction time avg:%.3g max:%.3g\n", ppvt->transAvg, ppvt->transMax);
        if (details >= 2) {
            ppvt->transMax = 0;
            ppvt->transAvg = 0;
        }
#endif
        fprintf(fp, "         Command count: %lu\n", ppvt->commandCount);
        fprintf(fp, " Setpoint update count: %lu\n", ppvt->setpointUpdateCount);
        fprintf(fp, "           Retry count: %lu\n", ppvt->retryCount);
        fprintf(fp, "        No reply count: %lu\n", ppvt->noReplyCount);
        fprintf(fp, "       Bad reply count: %lu\n", ppvt->badReplyCount);
    }
}

static asynStatus
drvUserCreate(void *drvPvt, asynUser *pasynUser,const char *drvInfo, const char **pptypeName, size_t *psize)
{
	//int offset;
	char *pstring = NULL;
	/* We are passed a string that identifies this command.
	* Set dataType and/or pasynUser->reason based on this string */

	/*for (i=0; i<FRONTEND_N_PARAMS; i++) {
		if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
			pasynManager->getAddr(pasynUser, &offset);
			if ((offset < 0) ) {
				asynPrint(pPlc->pasynUserTrace, ASYN_TRACE_ERROR,"%s::drvUserCreate port %s invalid memory request %d, max=%d\n",driver, pPlc->portName, offset, pPlc->modbusLength);
                		return asynError;
            		}
		pasynUser->reason = modbusDataCommand;
		if (pptypeName) *pptypeName = epicsStrDup(MODBUS_DATA_STRING);
		if (psize) *psize = sizeof(MODBUS_DATA_STRING);
		asynPrint(pasynUser, ASYN_TRACE_FLOW,"%s::drvUserCreate, port %s data type=%s\n", driver, pPlc->portName, pstring);
            return asynSuccess;
		}
	}*/

	return FpgaPcieparamProcess(pasynUser,pstring,drvInfo,pptypeName,psize);
}
static asynStatus drvUserGetType(void *drvPvt, asynUser *pasynUser, const char **pptypeName, size_t *psize)
{
	int command = pasynUser->reason;
	if (pptypeName)
		*pptypeName = epicsStrDup(FpgaPcieParam[command].paramString);
	if(psize) *psize = sizeof(command);
	return asynSuccess;
}
static asynStatus drvUserDestroy(void *drvPvt, asynUser *pasynUser){
	return asynSuccess;
}
static asynDrvUser drvUser = { drvUserCreate,drvUserGetType, drvUserDestroy };


static asynStatus
connect(void *pvt, asynUser *pasynUser)
{
    return pasynManager->exceptionConnect(pasynUser);
}

static asynStatus
disconnect(void *pvt, asynUser *pasynUser)
{
    return pasynManager->exceptionDisconnect(pasynUser);
}
static asynCommon commonMethods = { report, connect, disconnect };
/*
static asynStatus IOconnect(const char *port, int addr,
                         asynUser **ppasynUser, const char *drvInfo){
	return asynSuccess;
}
static asynStatus IOdisconnect(asynUser *pasynUser){
	return asynSuccess;
}
static asynStatus IOopenSocket(const char *server, int port, char **portName){
	return asynSuccess;
}*/
/*asynStatus IOwrite(asynUser *pasynUser,char const *buffer, size_t buffer_len,double timeout,size_t *nbytesTransfered){
	return asynSuccess;
}
static asynOctetSyncIO OctetSyncIOMethods = {IOconnect,IOdisconnect,IOopenSocket,IOwrite};*/
/*
*asynOctet methods
*/
static asynStatus
 octetWrite(void *pvt, asynUser *pasynUser, const char *data,
	size_t numchars, size_t *nBytesTransfered)
{
	FpgaPciePvt *ppvt = (FpgaPciePvt*)pvt;
	asynStatus status = asynError;
	FpgaPcieParam_t command;
	command = pasynUser->reason;
	size_t wrote;
	/*if ((numchars < 2) || (data[numchars-1] != '\0')) {
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize,
                                      "Value is not a null-terminated string");
		return asynError;
	}*/
	uint32_t count;
	if(command == DAdirectory){
		if(ppvt->directory->pini == true){
			ppvt->directory->pini = false;
			strncpy(data,ppvt->directory->dir_str,ppvt->directory->n_chars);
			*nBytesTransfered = ppvt->directory->n_chars;
			status = asynSuccess;
		}
		else{
			char *dest;
			ppvt->directory->pini = false;
   			memset(ppvt->directory->dir_str,'\0',100);
			dest = strncpy(ppvt->directory->dir_str,data,(int)numchars);
			if (dest != NULL){
				*nBytesTransfered = ppvt->directory->n_chars = (int)numchars;
				status = asynError;
			}
			else{
				*nBytesTransfered = ppvt->directory->n_chars = -1;
				status = asynSuccess;
			}
		}
	}
	pasynOctetSyncIO->write(pasynUser,NULL,0,0,NULL);//TODO:pog!why?!

	FILE *file_dir;
	file_dir = fopen (ppvt->directory->dir_str,"w");
	if (file_dir == NULL){
		printf("file %s  not found!\n",ppvt->directory->dir_str);
		return asynError;
	}
        printf("9\n");
	fclose(file_dir);
	//iocshCmd(data);TODO:remember this line
	return status;

}
static asynOctet octetMethods = { octetWrite };
/*
static asynStatus
readArray(asynUser *pasynUser, epicsFloat32 *pvalue,size_t nelem,size_t *nIn,double timeout)
{
	int i;
        printf("4\n");
	for(i=0;i<nelem;i++)
		pvalue[i] = (epicsFloat32)sin(i);
        printf("3\n");
	*nIn = nelem;
	return asynSuccess;
}
static asynFloat32ArraySyncIO asynFloat32ArraySyncIOMethods = { readArray };

*Float 32 array
*/
static asynStatus float32ArrayWrite(void *drvPvt, asynUser *pasynUser,
epicsFloat32 *value, size_t nelements){
	return asynSuccess;
}
static asynStatus
float32ArrayRead(void *pvt,asynUser *pasynUser, epicsFloat32 *value, size_t nelements,size_t *nIn){
        printf("nelements %d\n",(int)nelements);
	int command = pasynUser->reason;
	FpgaPciePvt *ppvt = (FpgaPciePvt*)pvt;
	if(command == Waveform){
		pasynOctetSyncIO->read(pasynUser,NULL,0,0,NULL,NULL);//TODO:pog!why?!
		int seed = ppvt->seed;
	        printf("nelements %d\n",(int)nelements);
		asynStatus status = asynError;
		int i=0;
		value[i] = sin(seed);
		for(i=0;i<nelements;i++)
			value[i] = value[i]  + (epicsFloat32)sin(i+seed);
		if ((i+seed < 6280))
			ppvt->seed = i + seed;
		else
			ppvt->seed = 0;
		*nIn = nelements;
	}
	return asynSuccess;
}

static asynFloat32Array asynFloat32ArrayMethods  = { NULL,float32ArrayRead,NULL,NULL };	 

/*
 * asynInt32 methods
 */
static asynStatus
int32Write(void *pvt, asynUser *pasynUser, epicsInt32 value)
{
	FpgaPciePvt *ppvt = (FpgaPciePvt*)pvt;
	asynStatus status = asynError;
	int bar0size = pd_getBARsize(ppvt->pci_handle,0);
	int bar1size = pd_getBARsize(ppvt->pci_handle,1);
	int bar2size = pd_getBARsize(ppvt->pci_handle,2);
	printf("Writting registers space (single access)\n");
	unsigned int buf[UBUF_SIZE];
	FpgaPcieParam_t command;
	command = pasynUser->reason ;
	int i = 0;
	switch (command){
		case bar0:
			printf("can`t write bar0\n");
			status = asynSuccess;
			break;
		case bar1:
			/*writting sdram*/
			for(i=0;i<MAX;i++){
				buf[i] = ~value;
			}
			memcpy( (void*)bar1, (void*)buf, MAX*sizeof(uint32_t) );
			status = asynSuccess;
			for(i=0;i<MAX;i++) {
				if (buf[i] != value)
					printf("error copying\n");
				status = asynError;
			}
			break;
		default:
			printf("test record loading\n");
			break;
	}
	return status;

}
	
static asynStatus
int32Read(void *pvt, asynUser *pasynUser, epicsInt32 *value)
{
	FpgaPciePvt *ppvt = (FpgaPciePvt*)pvt;
	asynStatus status = asynError;
	printf("Writting registers space (single access)\n");
	FpgaPcieParam_t command;
	command = pasynUser->reason ;
	int i = 0;
	epicsInt32 *registerValue;
	switch (command){
		case bar0:
			/*reading registers space-test*/
			registerValue = (epicsInt32*)ppvt->bar[0];
			for(i=0;i<MAX;i++){
								
				value[0] = value[0]+registerValue[i];
			}
			value[0] = value[0]/MAX;
			status = asynSuccess;
			break;
		case bar1:
			/*Reading  SDRAM space (single access)-test*/
			registerValue = (epicsInt32*)ppvt->bar[1];
			for(i=0;i<MAX;i++)
				value[0] = value[0]+registerValue[i];
			value[0] = value[0]/MAX;
			status = asynSuccess;
			break;
		case dmakm:
			/*Reading DMA test kernel memory-test*/
			if(value[0]){
				uint32_t *registerValue0 = (uint32_t*)ppvt->bar[0];
				uint32_t *registerValue1 = (uint32_t*)ppvt->bar[1];
				ppvt->status = READING_KM;
				DMAKernelMemoryRead(registerValue0, registerValue1, (uint64_t)0, ppvt->kernel_memory, BRAM_SIZE,ppvt->kmem_handle,1);
			}
			value[0] = 0;
			break;
		default:
			printf("test\n");
			break;					
	}

	return status;
				
}

static asynInt32 int32Methods = { int32Write, int32Read };

/*
 * asynFloat64 methods
 */
static asynStatus
float64Write(void *pvt, asynUser *pasynUser, epicsFloat64 value)
{
	return asynSuccess;
}

static asynStatus
float64Read(void *pvt, asynUser *pasynUser, epicsFloat64 *value)
{
	return asynSuccess;
}

static asynFloat64 float64Methods = { float64Write, float64Read };

static void intTask (FpgaPciePvt *ppvt){
	FpgaPciePvt *pvt = (FpgaPciePvt*)ppvt;
	int i = 0;
	int test_len = 2;
	epicsInt32 *registerValue32;
	while (1){
		pd_waitForInterrupt(pvt->pci_handle,0);
		printf("Interrupt!\n");
		if (ppvt->status == WRITTING_KM){
			if (ppvt->bar[1] != 0) {
			registerValue32 = pvt->bar[1];
			// Print contents of the SDRAM
				printf("Checking SDRAM (single access)\n");
				for(i=0;i<(test_len >> 2);i++)
					printf("bar1[i] = %d",registerValue32[i]);
			}
			else if (pvt->bar[2] != 0) {
				registerValue32 = pvt->bar[2];
				// Print contents of the BRAM
				printf("Checking WB BRAM (single access)\n");
				for(i=0;i<(test_len >> 3);i++)
					printf("bar2[i] = %d", registerValue32[i]);
			}
			ppvt->status = IDLE;
		}
		else if(ppvt->status == READING_KM){
			// Get Buffer contents
			registerValue32 = pvt->kernel_memory;
			printf("Get Buffer content after DMA\n");
			for(i=0;i<(2 >> 2);i++)//only testing
				printf("kernel_memory[i] = %d\n",registerValue32[i]);
		}
			ppvt->status = IDLE;
	}
	return;
}

epicsShareFunc int 
devFpgaPcieConfigure(const char *portName, const char *hostInfo, int priority, int device)
{
    FpgaPciePvt *ppvt;
    asynStatus status;

    #ifdef DEBUG
    printf("Configuration initiated\n");
    #endif
    /*
     * Create our private data area
     */
    ppvt = callocMustSucceed(1, sizeof(FpgaPciePvt), "devFpgaPcieConfigure");
    if (priority == 0) priority = epicsThreadPriorityMedium;
    ppvt->seed = 0;

    /*
     * Create the port that we'll use for I/O.
     * Configure it with our priority, autoconnect, no process EOS.
     * We have to create this port since we are multi-address and the
     * IP port is single-address.
     */

    ppvt->portName = epicsStrDup(portName);
    /*
     * Create our port
     */
    status = pasynManager->registerPort(ppvt->portName,
                                        ASYN_CANBLOCK,/*ASYN_MULTIDEVICE ? */
                                        1,         /*  autoconnect */
                                        priority,  /* priority (for now) */
                                        0);        /* default stack size */
    if (status != asynSuccess) {
        printf("Can't register port %s", portName);
        return -1;
    }
  
    /*
     * Advertise our interfaces
     */
    ppvt->Float32Array.interfaceType = asynFloat32ArrayType;
    ppvt->Float32Array.pinterface = (void *)&asynFloat32ArrayMethods;
    ppvt->Float32Array.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->Float32Array);
    if (status != asynSuccess) {
        printf("Can't register asynCommon support.\n");
        return -1;
    }
    ppvt->asynOctet.interfaceType = asynOctetType;
    ppvt->asynOctet.pinterface = &octetMethods;
    ppvt->asynOctet.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynOctet);
    if (status != asynSuccess) {
        printf("Can't register asynCommon support.\n");
        return -1;
    }
    /*ppvt->asynOctetSyncIO.interfaceType = asynOctetSyncIO;
    ppvt->asynOctetSyncIO.pinterface = &OctetSyncIOMethods;
    ppvt->asynOctet.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynOctetSyncIO);
    if (status != asynSuccess) {
        printf("Can't register asynCommon support.\n");
        return -1;
    }*/
    ppvt->asynCommon.interfaceType = asynCommonType;
    ppvt->asynCommon.pinterface  = &commonMethods;
    ppvt->asynCommon.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynCommon);
    if (status != asynSuccess) {
        printf("Can't register asynCommon support.\n");
        return -1;
    }
    ppvt->asynDrvUser.interfaceType = asynDrvUserType;
    ppvt->asynDrvUser.pinterface = &drvUser;
    ppvt->asynDrvUser.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynDrvUser);
    if (status != asynSuccess) {
        printf("Can't register asynFloat64 support.\n");
        return -1;
    }
    ppvt->asynInt32.interfaceType = asynInt32Type;
    ppvt->asynInt32.pinterface = &int32Methods;
    ppvt->asynInt32.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynInt32);
    if (status != asynSuccess) {
        printf("Can't register asynInt32 support.\n");
        return -1;
    }
    ppvt->asynFloat64.interfaceType = asynFloat64Type;
    ppvt->asynFloat64.pinterface = &float64Methods;
    ppvt->asynFloat64.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynFloat64);
    if (status != asynSuccess) {
        printf("Can't register asynFloat64 support.\n");
        return -1;
    }
    
    #ifdef DEBUG
    printf("Configuration succeeded\n");
    #endif
   
   /*Hardware configuration*/
   /*setting default directory*/
   ppvt->directory = callocMustSucceed(1, sizeof(DataAcquisitionDirectory), "DataAcquisitionDirectory");
   memset(ppvt->directory->dir_str,'\0',100);
   char *home_dir = getenv("HOME");//get home directory
   if(home_dir == NULL){
      printf("Getting Home directory:error\n");
      return asynError;
   }
   strcpy(ppvt->directory->dir_str,home_dir);
   ppvt->directory->n_chars = strlen(home_dir);
   ppvt->directory->pini = true;
   printf("%s\n",home_dir);
   /*allocating pci_handle*/


   #ifndef octettest
   char temp[50];
   ppvt->pci_handle = callocMustSucceed(1, sizeof(pd_device_t), "pci_handle");
   int ret;
   printf ("Trying device : %d\n", device);
   sprintf( temp, "/dev/fpga%d", device );
   ret = pd_open(device, ppvt->pci_handle);//TODO:temp or device?
   if (ret != 0) {
      printf( "failed to open device\ntry another device id\n" );
      return asynError;
   }
   else{
       printf("device found\n");
       //printf("Handle: %x\n", handle);TODO:handle?
   }
   /*allocating bars*/
   void **bar = callocMustSucceed(6, sizeof(void*), "bar");
   int i = 0;
   printf("Allocating bars\n");
   for (i=0;i<6;i++){
       bar[i] = pd_mapBAR(ppvt->pci_handle, i);
       if (bar[i] == NULL){
          printf("failed mapping bar%d\n",i);   
          return asynError;
       }
   }
   ppvt->bar = bar;
   /*allocating kernel memory*/
   ppvt->kmem_handle = callocMustSucceed(1, sizeof(pd_kmem_t), "pd_kmem_t");
   ppvt->kernel_memory = pd_allocKernelMemory(ppvt->pci_handle, BRAM_SIZE, ppvt->kmem_handle);
   if(ppvt->kernel_memory == NULL){
       printf("Failed allocating kernel memory\n");
       return asynError;
   }
   /*allocating user memory*/
   void *mem;
   posix_memalign( (void**)&mem,16,BRAM_SIZE );//TODO:compilation warning!!implicit declaration
   if (mem == NULL){
      printf("memory aligned failed\n");
      return asynError;
   }
   ppvt->umem_handle = callocMustSucceed(1, sizeof(pd_umem_t), "pd_umem_t");
   ret = pd_mapUserMemory(ppvt->pci_handle, mem, BRAM_SIZE, ppvt->umem_handle);
   free(mem);

   /*dma configuration*/
   unsigned int dmaCmd;
   unsigned int dmaSize=8;//TODO:check it!
   //turn interrupt on
   unsigned int *aux = (unsigned int *)bar[1];
   unsigned int physicalAddress = (unsigned int)ppvt->kmem_handle->pa;
   dmaCmd = IRQ_ON_CMD + dmaSize;
   aux[0] = dmaCmd;
   aux[1] = physicalAddress;//TODO:check:Get Physical Address?!
   aux[2] = 0;

   if (epicsThreadCreate("FpgaPcieTask",
                         epicsThreadPriorityHigh,
                         epicsThreadGetStackSize(epicsThreadStackMedium),
                         (EPICSTHREADFUNC)intTask,
                         ppvt) == NULL)
   	errlogPrintf("FpgaPcie epicsThreadCreate failure\n");
#endif
    return 0;
}

/*
 * IOC shell command
 */
/*
epicsShareFunc int 
devFpgaPcieConfigure(const char *portName, const char *hostInfo, int priority, int device)*/
static const iocshArg devFpgaPcieConfigureArg0 = { "port name",iocshArgString};
static const iocshArg devFpgaPcieConfigureArg1 = { "host:port",iocshArgString};
static const iocshArg devFpgaPcieConfigureArg2 = { "priority",iocshArgInt};
static const iocshArg devFpgaPcieConfigureArg3 = { "device",iocshArgInt};
//static const iocshArg devFrontendConfigureArg3 = { "priority",iocshArgInt};
static const iocshArg *devFpgaPcieConfigureArgs[4] = {
                    &devFpgaPcieConfigureArg0, &devFpgaPcieConfigureArg1,
                    &devFpgaPcieConfigureArg2, &devFpgaPcieConfigureArg3 };
static const iocshFuncDef devFpgaPcieConfigureFuncDef =
                      {"devFpgaPcieConfigure",4,devFpgaPcieConfigureArgs};
static void devFpgaPcieConfigureCallFunc(const iocshArgBuf *args)
{
    devFpgaPcieConfigure(args[0].sval, args[1].sval, args[2].ival,args[3].ival);
}

static void
devFpgaPcieConfigure_RegisterCommands(void)
{
    iocshRegister(&devFpgaPcieConfigureFuncDef,devFpgaPcieConfigureCallFunc);
}
epicsExportRegistrar(devFpgaPcieConfigure_RegisterCommands);
