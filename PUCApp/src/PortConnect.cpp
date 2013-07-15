#include <iocsh.h>
#include <epicsExport.h>
#include <stdio.h>
#include <string.h>
#include "PortConnect.h"

PortConnect::PortConnect(const char* portName, const char * serialName) : asynPortDriver(portName, 0, 6, asynInt32Mask | asynOctetMask | asynFloat64Mask | asynFloat64ArrayMask | asynEnumMask | asynDrvUserMask, 0, 0, 1, 0, 0)
{   
   printf("Constructor\n");
   
   createParam(P_AddressString, asynParamInt32, &P_Address);
   createParam(P_IdString, asynParamInt32, &P_Id);
   createParam(P_ValueString, asynParamFloat64, &P_Value);
   createParam(P_TypeString, asynParamInt32, &P_Type);
   createParam(P_SizeString, asynParamInt32, &P_Size);
   createParam(P_OffsetString, asynParamInt32, &P_Offset);
   
   asynStatus status = pasynOctetSyncIO->connect(serialName, 0, &user, NULL);
   
   if(status == asynSuccess) printf("Success: Connect to port\n");   
   else printf("Error: Connect to port");   

   timeout = 5000;
   
   pasynOctetSyncIO->flush(user);   
}

//Override method from AsynPortDriver
asynStatus PortConnect :: readFloat64(asynUser* pasynUser, epicsFloat64* value)
{
	printf("Read float64\n");
	asynStatus status = asynError;
	
	int type = 0, size = 0;
	double val = 0;
	
	getIntegerParam(P_Type, &type);	
	getIntegerParam(P_Size, &size);
	getDoubleParam(P_Value, &val);
	
	printf("Type: %d\n", type);
	printf("Size: %d\n", size);
	
	if(type) 
	{
		printf("Write record\n");
		*value = (epicsFloat64) val;
		return asynSuccess;
	}
	
	if(pasynUser->reason == P_Value)
	{		
		size_t wrote;
		
		printf("Sending request to read\n");
		
		int address = 0, id = 0;
		
		getIntegerParam(P_Address, &address);
		getIntegerParam(P_Id,      &id);

		printf("Address: %d, Id: %d\n", address, id);
		
		int bytesToWrite;		
		char * write = com.readVariable(address, id, &bytesToWrite);
		
		if (write[0] == 0) return asynSuccess;
		
		status = pasynOctetSyncIO->write(user, write, bytesToWrite, timeout, &wrote);
		
		if(status != asynSuccess) return status;
		
		//Read response from PUC
		//First, read the header, and after read the payload and checksum
		
		char * header;		
		char * payload;
		int size;
		
		size_t bytesRead;
		int eomReason;
		
		header = (char *) malloc(4*sizeof(char));
		
		printf("Reading\n");
		status = pasynOctetSyncIO->read(user, header, 4, timeout, &bytesRead, &eomReason);		
		if(status != asynSuccess) return status;
				
		size = com.checkSize(header[3]);
		payload = (char *) malloc((size+1)*sizeof(char));
		
		status = pasynOctetSyncIO->read(user, payload, size+1, timeout, &bytesRead, &eomReason);
		if(status != asynSuccess) return status;
				
		*value = com.readingVariable(header, payload);

		//Check the checksum
		unsigned int check = 0;
		char check2;
		int i;

		for(i = 0; i < 4; i++)
		{
			check += header[i];
		}

		for(i = 0; i < size; i++)
		{
			check += payload[i];
		}		
		
		
		check2 = (check & 0xFF);
		printf("Debug: check2 = %d\n",check2);
		printf("Debug: checksum = %d\n", payload[size]);

		check2 += (payload[size] & 0xFF);
		
		if(check2) printf("Invalid checksum\n");
		else printf("Checksum correct\n");
	}	
	
	return status;
	
}

//Request and read a curve from PUC.
//Override method from AsynPortDriver
asynStatus PortConnect :: readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
{
	printf("readFloat64Array\n");
	asynStatus status;

	int offset = 0, address = 0, id = 0, size = 0;
	int bytesToWrite;
	size_t wrote;

	getIntegerParam(P_Address, &address);
	getIntegerParam(P_Size,    &size);
	getIntegerParam(P_Id,      &id);
	getIntegerParam(P_Offset,  &offset);

	char * write = com.readCurve(address, size, id, offset, &bytesToWrite);
	
	printf("Bytes to write: %d\n", bytesToWrite);

	status = pasynOctetSyncIO->write(user, write, bytesToWrite, timeout, &wrote);

	//Reading response from PUC
	char * bufferRead;
	bufferRead = (char *) malloc(16391*sizeof(char));
	
	size_t bytesRead;
	int eomReason;

	status = pasynOctetSyncIO->read(user, bufferRead, 16391, timeout, &bytesRead, &eomReason);
	printf("Bytes read: %li\n", bytesRead);
	printf("eomReason: %d\n", eomReason);
	if(status != asynSuccess) return status;

	memcpy(value, com.readingCurve(bufferRead), 8192*sizeof(double));

	printf("Debug 2 values for test:\n");
	printf("value[100] = %f\n", value[100]);
  	printf("value[1000] = %f\n", value[1000]);

	*nIn = nElements; //Correct this

	fflush(stdout);
	return status;
}

//Override method from AsynPortDriver
//Write a curve to PUC
asynStatus PortConnect :: writeFloat64Array(asynUser* pasynUser, epicsFloat64* value, size_t nElements)
{
	asynStatus status = asynError;
	printf("writeFloat64Array\n");
	size_t wrote;
	
	if(pasynUser->reason == P_Value)
	{
		int offset = 0, address = 0, id=0, size=0;
		int bytesToWrite;
		
		getIntegerParam(P_Address, &address);
		getIntegerParam(P_Size,    &size);
		getIntegerParam(P_Id,      &id);
		getIntegerParam(P_Offset, &offset);
		
		char * write = com.writeCurveBlock(address, size, id, offset, value, nElements, &bytesToWrite);
		
		printf("Bytes to write: %d\n", bytesToWrite);

		//////////////////Verify command
		status = pasynOctetSyncIO->write(user, write, bytesToWrite, timeout, &wrote);		
		//////////////////
		
		//Read response from PUC
		char * bufferRead;
		bufferRead = (char *) malloc(5*sizeof(char));
		
		size_t bytesRead;
		int eomReason;
		
		status = pasynOctetSyncIO->read(user, bufferRead, 5, timeout, &bytesRead, &eomReason);
		
		if(status != asynSuccess) return status;
		
		printf("Bytes read: %lu \n", bytesRead);
		printf("%u | %u | %u | %u | %u \n", bufferRead[0]&0xFF, bufferRead[1]&0xFF, bufferRead[2]&0xFF, bufferRead[3]&0xFF , bufferRead[4]&0xFF);			
		printf("Write: %d, Wrote: %li \n", bytesToWrite, wrote);		
		
		//TODO: Verify checksum and the response	
		
	}
	
	fflush(stdout);	
	return status;
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
		size_t wrote;

		printf("Data writing\n");	
		printf("Value = %f\n", value);
				
		int address = 0, id=0, size=0;
		
		getIntegerParam(P_Address, &address);
		getIntegerParam(P_Size,    &size);
		getIntegerParam(P_Id,      &id);
				
		int bytesToWrite;
		char * write = com.writeVariable(address, size, id, (double) value, &bytesToWrite);
		
		pasynOctetSyncIO->flush(user);
		status = pasynOctetSyncIO->write(user, write, bytesToWrite, timeout, &wrote);
		
		if(status != asynSuccess) return status;
		
		//Read response from PUC		
		char * bufferRead;
		bufferRead = (char *) malloc(5*sizeof(char));
		
		size_t bytesRead;
		int eomReason;
		
		status = pasynOctetSyncIO->read(user, bufferRead, 5, timeout, &bytesRead, &eomReason);
		
		if(status != asynSuccess) return status;
	
		//TODO: Check correctly the response
		if(!((bufferRead[2]&0xFF) == 0xE0)) printf("Write fail, response: %u\n", bufferRead[2]&0xFF);

		printf("Bytes read: %li\n", bytesRead);	
		printf("Read on third byte: %u\n", bufferRead[2]&0xFF);			
		printf("Bytes write: %d, Bytes wrote: %li \n", bytesToWrite, wrote);
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
