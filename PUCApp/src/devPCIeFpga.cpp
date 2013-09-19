#include <iocsh.h>
#include <epicsExport.h>
#include <stdio.h>
#include <string.h>
#include "devPCIeFpga.h"

devPCIeFpga::devPCIeFpga(const char* portName, const char * serialName) : asynPortDriver(portName, 0, 6, asynInt32Mask | asynOctetMask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask, 0, 0, 1, 0, 0)
{   
   printf("Constructor\n");

   createParam(SDRAM_Acquire_value, asynParamInt32, &SDRAM_Acquire);

   asynStatus status = pasynOctetSyncIO->connect(serialName, 0, &user, NULL);
   
   if(status == asynSuccess) printf("Success: Connect to port\n");   
   else printf("Error: Connect to port");   

   timeout = 5000;
   
   //pasynOctetSyncIO->flush(user);   
}

static void convert_octet_int32(char *answer_octet, epicsInt32 *answer_int32){
	unsigned_int_32_value aux;
	int i;
	for(i=0;i<4;i++){
		aux.vvalue[i] = answer_octet[i];
	}
	*answer_int32 = aux.ui32value;
	return;
}
static void convert_int32_octet(epicsInt32 *ask_int32,char *ask_octet){
	unsigned_int_32_value aux;
	int i;
	aux.ui32value = *ask_int32;
	for(i=0;i<4;i++){
		ask_octet[i] = aux.vvalue[i];
	}
	return;
}	

asynStatus devPCIeFpga :: readInt32(asynUser* pasynUser, epicsInt32* value)
{
	printf("Read Int32");
	asynStatus status = asynError;
	
	int function = pasynUser->reason;
	int channel;
	const char *paramName;
	const char* functionName = "readInt32";
	size_t bytesRead;
	int eomReason;
	getAddress(pasynUser, &channel);

	/* Set the parameter in the parameter library. */
	//setIntegerParam(channel, function, value);

	/* Fetch the parameter string name for possible use in debugging */
	getParamName(function, &paramName);
	
	printf("Sending request to read: %d\n",pasynUser->reason);
	
	if (function == SDRAM_Acquire) {
		/*setting parameters for driver operation*/
		/*reading four bytes, for one epicsInt32*/
                pasynUser->reason = bar2;
		char *answer_octet = (char*)malloc(sizeof(char)*4);
		epicsInt32 answer_int32;
		epicsInt32 *addr = (epicsInt32*)malloc(sizeof(epicsInt32));
		*addr = 0;
                pasynUser->drvUser = addr;
		pasynOctetSyncIO->read(pasynUser, answer_octet, sizeof(epicsInt32), 5000, &bytesRead, &eomReason);
		convert_octet_int32(answer_octet,&answer_int32);
		*value = answer_int32;
		free(addr);
		free(answer_octet);
	}
		
	return status;
	
}
asynStatus devPCIeFpga :: writeInt32(asynUser* pasynUser, epicsInt32 value)
{	
	printf("Write Int32");
	asynStatus status = asynError;
	
	int function = pasynUser->reason;
	int channel;
	const char *paramName;
	const char* functionName = "writeInt32";
	size_t bytesWrite;
	
	getAddress(pasynUser, &channel);

	/* Set the parameter in the parameter library. */
	setIntegerParam(channel, function, value);

	/* Fetch the parameter string name for possible use in debugging */
	getParamName(function, &paramName);
	
	printf("Sending request to write: %d\n",pasynUser->reason);
	
	if (function == SDRAM_Acquire) {
		/*setting parameters for driver operation*/
		/*reading four bytes, for one epicsInt32*/
                pasynUser->reason = bar2;
		char *ask_octet = (char*)malloc(sizeof(char)*4);
		convert_int32_octet(&value,ask_octet);
		epicsInt32 *addr = (epicsInt32*)malloc(sizeof(epicsInt32));
		*addr = 0;
                pasynUser->drvUser = addr;
		pasynOctetSyncIO->write(pasynUser, ask_octet, sizeof(epicsInt32), 5000, &bytesWrite);
		free(addr);
		free(ask_octet);
	}
		
	return status;
	
}

extern "C"{

int devPCIeFPGAConfigure(const char * portName, const char * serialName)
{
   new devPCIeFpga(portName, serialName);
   return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = {"portName",   iocshArgString};
static const iocshArg initArg1 = {"serialName", iocshArgString};

static const iocshArg * const initArgs [] = {&initArg0, &initArg1};

static const iocshFuncDef initFuncDef = {"devPCIeFPGAConfigure", 2, initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
   devPCIeFPGAConfigure(args[0].sval, args[1].sval);
}

void devPCIeFPGARegister(void)
{
   iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(devPCIeFPGARegister);
}
