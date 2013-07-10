#include <iocsh.h>
#include <epicsExport.h>
#include <stdio.h>
#include "PortConnect.h"

PortConnect::PortConnect(const char* portName, const char * serialName) : asynPortDriver(portName, 0, 5, asynInt32Mask | asynOctetMask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask, 0, 0, 1, 0, 0)
{
   //TODO: get and set parameters.
   
   printf("Constructor\n");
   
   createParam(P_AddressString, asynParamInt32, &P_Address);
   createParam(P_IdString, asynParamInt32, &P_Id);
   createParam(P_ValueString, asynParamFloat64, &P_Value);
   createParam(P_TypeString, asynParamInt32, &P_Type);
   createParam(P_SizeString, asynParamInt32, &P_Size);
   
   asynStatus status = pasynOctetSyncIO->connect(serialName, 0, &user, NULL);
   
   if(status == asynSuccess) printf("Success: Connect to port\n");   
   else printf("Error: Connect to port");   
   
   pasynOctetSyncIO->flush(user);   
}

//Override method from AsynPortDriver
asynStatus PortConnect :: writeFloat64(asynUser* pasynUser, epicsFloat64 value)
{	
	asynStatus status = asynError;
	
	int type = 0;
	getIntegerParam(P_Type,    &type);
	
	if(!type) return status;
	
	//User can modify only the value
	if(pasynUser->reason == P_Value)
	{	
		//Change this variable to global
		size_t wrote;

		printf("Data writing\n");	
		printf("Value = %f\n", value);
				
		int address = 0, id=0, size=0;
		
		getIntegerParam(P_Address, &address);
		getIntegerParam(P_Size,    &size);
		getIntegerParam(P_Id,      &id);
		
		int bytesToWrite;
		char * write = com.writeVariable(address, size, id, (double) value, &bytesToWrite);
		
		//pasynOctetSyncIO->flush(user);
		status = pasynOctetSyncIO->write(user, write, bytesToWrite, 5000, &wrote);
		
		if(status != asynSuccess) return status;
		
		//Read response from PUC
		//char * bufferRead;
		//bufferRead = (char *) malloc(5*sizeof(char));
		
		//size_t bytesRead;
		//int eomReason;
		
		//status = pasynOctetSyncIO->read(user, bufferRead, 5, 5000, &bytesRead, &eomReason);
		
		//if(status != asynSuccess) return status;
		
		printf("Write: %d, Wrote: %li \n", bytesToWrite, wrote);	
		//printf("Read: %d", bufferRead[2]);	
	}

	return status;	
}

extern "C"{

int portConnectConfigure(const char * portName, const char * serialName)
{
   new PortConnect(portName, serialName);
   return(asynSuccess);
}

/* EPICS iocsh shell commands */

static const iocshArg initArg0 = {"portName",   iocshArgString};
static const iocshArg initArg1 = {"serialName", iocshArgString};

static const iocshArg * const initArgs [] = {&initArg0, &initArg1};

static const iocshFuncDef initFuncDef = {"portConnectConfigure", 2, initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
   portConnectConfigure(args[0].sval, args[1].sval);
}

void portConnectRegister(void)
{
   iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(portConnectRegister);
}
