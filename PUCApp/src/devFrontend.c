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

#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsString.h>
#include <epicsThread.h>
#include <epicsTime.h>
#include <errlog.h>
#include <iocsh.h>
//#include "Command.h"
#include "asynDriver.h"
#include "asynOctetSyncIO.h"
#include "asynInt32.h"
#include "asynFloat64.h"
#include "asynCommonSyncIO.h"
#include "asynStandardInterfaces.h"
#include "drvAsynIPPort.h"
#include "devFrontend.h"
#include <epicsExport.h>

/*
 * Bits in configure 'flags' argument
 */
#define FLAG_SHOULD_INHIBIT_READBACK    0x1

/*
 * asynFloat64 subaddresses
 */
#define A_READ_SETPOINT_CURRENT          0
#define A_READ_READBACK_CURRENT          1
#define A_WRITE_SETPOINT_CURRENT         100

/*
 * asynInt32 subaddresses
 */
#define A_READ_POWER_ON             1
#define A_READ_GENERIC_FAULT        2
#define A_READ_DC_UNDER_V           3
#define A_READ_MOSFET_OVER_T        4
#define A_READ_SHUNT_OVER_T         5
#define A_READ_INTERLOCK            6
#define A_READ_INPUT_OVER_I         7
#define A_READ_CROWBAR              8
#define A_READ_SLEW_MODE            9
#define A_READ_FORCE_READBACK       99
#define A_WRITE_POWER_ON            100
#define A_WRITE_RESET               101
#define A_WRITE_SLEW_MODE           102

/*
 * Control byte bit assignmentss
 */
#define A26X_CONTROL_RESERVED      0x1    /* Must be set in all commands */
#define A26X_CONTROL_ENABLE_SLEW   0x10   /* Enable slew-rate limiting */
#define A26X_CONTROL_RESET_ERRORS  0x20   /* Reset latched errors */
#define A26X_CONTROL_POWER_ON      0x40   /* Turn supply on */
#define A26X_CONTROL_READBACK_ONLY 0x80   /* Perform readback only */

/*
 * Status byte bit assignmentss
 */
#define A26X_STATUS_POWER_ON      0x1    /* Supply is on */
#define A26X_STATUS_GENERIC_FAULT 0x2    /* Generic fault (hw error) */
#define A26X_STATUS_DC_UNDER_V    0x4    /* DC undervoltage */
#define A26X_STATUS_MOSFET_OVER_T 0x8    /* MOSFET overtemp */
#define A26X_STATUS_SHUNT_OVER_T  0x10   /* Shunt overtemp */
#define A26X_STATUS_INTERLOCK     0x20   /* Interlock chain broken */
#define A26X_STATUS_INPUT_OVER_I  0x40   /* Input overcurrent */
#define A26X_STATUS_CROWBAR       0x80   /* Crowbar (short circuit protect) */

/*
 * Supply slew rate (Amps/sec)
 */
#define A26X_SLEW_RATE  20

/*
 * Conditional-compile control
 */
/* #define ENABLE_TIMING_TESTS 1 */

/*
 * Readback values
 */


/** Number of asyn parameters (asyn commands) this driver supports. */
#define FRONTEND_N_PARAMS 6

/** Specific asyn commands for this support module. These will be used and
 * managed by the parameter library (part of areaDetector). */
typedef enum FrontendParam_t {
	//mjpg_quality      /** JPEG quality (int32 read/write) */ = NDPluginDriverLastParam,
	t_setpoint         /** Temperature setpoint*/,
	t_sensor1   /** Temeperature sensor 1*/,	
	t_sensor2 /** Temperature sensor 2*/,	
	t_sensor3 /** Temperature sensor 3*/,	
	t_sensor4    /** Temeperature sensor 4*/,	
	c1_switchstate      /** Switch state*/,			
    FrontendLastParam
} FrontendParam_t;

typedef struct{
	FrontendParam_t paramEnum;
	char *paramString;
}FrontendParamStruct;

static FrontendParamStruct FrontendParam[FRONTEND_N_PARAMS] = {
	{t_setpoint,      "T_SetPoint"},
	{t_sensor1,         "T_Sensor1"},
	{t_sensor2,  "T_Sensor2"},
	{t_sensor3,   "T_Sensor3"},
	{t_sensor4, "T_Sensor4"},
	{c1_switchstate, "S_State"},
};

typedef struct a26xReadback {
    int    status;
    double setpointCurrent;
    double readbackCurrent;
} a26xReadback;

/*
 * Interposed layer private storage
 */
typedef struct FrontendPvt {
    asynUser      *pasynUser;      /* To perform lower-interface I/O */

    asynInterface  asynCommon;     /* Our interfaces */
    asynInterface  asynInt32;
    asynInterface  asynFloat64;
    asynInterface  asynDrvUser;

    a26xReadback   readback;       /* Most recent readback values */
    int            slewMode;       /* 0 or A26X_CONTROL_ENABLE_SLEW */

    unsigned long  commandCount;    /* Statistics */
    unsigned long  setpointUpdateCount;
    unsigned long  retryCount;
    unsigned long  noReplyCount;
    unsigned long  badReplyCount;

    sllp_client_t *sllp;
    struct sllp_vars_list *vars;

#ifdef ENABLE_TIMING_TESTS
    double         transMax;
    double         transAvg;
#endif

} FrontendPvt;

/*
 * Send command and get reply
 */
uint8_t lastCommand;         
int sendCommandtest(uint8_t *data, uint32_t *count)
{
        if(*count < 256 && *count > 0)
        {
                int i;
                printf("Count = %d\n", *count);
 
                for(i = 0; i < *count; i++)
                {
                        printf("data[%d] = %02X\n", i, data[i]);
                }
 
                lastCommand = data[0];         
        }
 
        return EXIT_SUCCESS;
}
int receiveCommandtest(uint8_t *data, uint32_t *count)
{
        printf("RECEIVE BEGIN\n");
        uint8_t packet[5];
        //bool flag = false;
 
        //Simula uma variavel de leitura de tamanho 3 e uma variavel de escrita de tamanho 3
       
        if(lastCommand == 0x02)
        {
                //flag = true;
                printf("Comando de listar variaveis\n");
                packet[0] = 0x03;
                packet[1] = 0x01;
                packet[2] = 0x03;
                //packet[3] = 0x83;
                *count = 3;            
        }
 
        //Read simulation
        else if(lastCommand == 0x10)
        {
                //flag = true;
                printf("Comando de ler variavel\n");
                packet[0] = 0x11;
                packet[1] = 0x03;
                packet[2] = 0x02;
                packet[3] = 0xFF;
                packet[4] = 0x0A;
                *count = 5;
        }
 
        else if (lastCommand == 0x04)
        {
                //flag = true;
                printf("Comando de listar grupos de variaveis\n");
                packet[0] = 0x05;
                packet[1] = 0x00;
                *count = 2;
        }
 
        else if(lastCommand == 0x08)
        {
                //flag = true;
                printf("Comando de listar curvas\n");
 
                packet[0] = 0x09;
                packet[1] = 0x00;
                *count = 2;
        }
 
        //if(flag) memcpy(data, packet, *count);
        memcpy(data, packet, *count);
        printf("RECEIVE END\n");
 
        return EXIT_SUCCESS;
}
/*
 * asynCommon methods
 */
static void
report(void *pvt, FILE *fp, int details)
{
    FrontendPvt *ppvt = (FrontendPvt *)pvt;

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
	int i;
	//int offset;
	char *pstring;
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

	for (i=0; i<FRONTEND_N_PARAMS; i++) {
		pstring = FrontendParam[i].paramString;
		if (epicsStrCaseCmp(drvInfo, pstring) == 0) {
			pasynUser->reason = FrontendParam[i].paramEnum;
			if (pptypeName) *pptypeName = epicsStrDup(pstring);
			if (psize) *psize = sizeof(FrontendParam[i].paramEnum);
			asynPrint(pasynUser, ASYN_TRACE_FLOW,"drvUserCreate, command=%s\n", pstring);
            		return asynSuccess;
		}
	}
	asynPrint(pasynUser, ASYN_TRACE_ERROR,"drvUserCreate, unknown command=%s\n",drvInfo);
	return asynError;
}
static asynStatus drvUserGetType(void *drvPvt, asynUser *pasynUser, const char **pptypeName, size_t *psize)
{
	int command = pasynUser->reason;
	if (pptypeName)
		*pptypeName = epicsStrDup(FrontendParam[command].paramString);
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
 * asynInt32 methods
 */
static asynStatus
int32Write(void *pvt, asynUser *pasynUser, epicsInt32 value)
{
	FrontendPvt *ppvt = (FrontendPvt *)pvt;
	asynStatus status = asynError;
	
	//User can modify only the value
	size_t wrote;
	printf("Data writing Int32\n");	
		
	int bytesToWrite;
	
	//TODO:USE THE PROTOCOL!
	//TODO:USE nobts!
	char *result;
	result = (char *)malloc((3+1)*sizeof(char));
	result[0] = 0x20;//WRITE_VARIABLE;
	result[1] = 1+1;
	result[2] = pasynUser->reason;

	union {
		unsigned char c[4];
		epicsInt32 f;
	} u;
	u.f = value;
	printf("epicsInt32Value: %d\n",u.f);
	
	int i = 0;
	for(i = 0;i < 1; i++){
		result[3+i] = u.c[i];
	}
	
	for(i = 2;i < 7; i++)
		printf("message %d\n",result[i]);
	pasynOctetSyncIO->flush(ppvt->pasynUser);
	status = pasynOctetSyncIO->write(ppvt->pasynUser, result, (3+1)*sizeof(char), 5000, &wrote);
		
	if(status != asynSuccess) return status;
		
	//Read response from PUC		
	//char * bufferRead;
	//bufferRead = (char *) malloc(5*sizeof(char));
		
	//size_t bytesRead;
	//int eomReason;
		
	//status = pasynOctetSyncIO->read(user, bufferRead, 5, 5000, &bytesRead, &eomReason);
		
	//if(status != asynSuccess) return status;
		
	//printf("Read: %d\n", bufferRead[2]);			
	printf("Write: %d, Wrote: %li \n", bytesToWrite, wrote);	

	return status;
}

static asynStatus
int32Read(void *pvt, asynUser *pasynUser, epicsInt32 *value)
{
	FrontendPvt *ppvt = (FrontendPvt *)pvt;
	printf("Read Int32");
	asynStatus status = asynError;
	
	size_t wrote;
	
	printf("Sending request to read: %d\n",pasynUser->reason);
	
	int bytesToWrite;
	char *result;
	result = (char *) malloc(3*sizeof(char));
	bytesToWrite = 3*sizeof(char);
		
	result[0] = 0x10;//READ_VARIABLE
	result[1] = 1;
	result[2] = (pasynUser->reason) & 0xFF;
	fflush(stdout);
		
	status = pasynOctetSyncIO->write(ppvt->pasynUser, result, bytesToWrite, 5000, &wrote);
	
	if(status != asynSuccess) return status;
		
	//Read response from PUC
	//First, read the header, and after read the payload and checksum
		
	char * header;		
	char * payload;
	int size;
		
	size_t bytesRead;
	int eomReason;
		
	header = (char *) malloc(2*sizeof(char));
		
	printf("Reading\n");
	status = pasynOctetSyncIO->read(ppvt->pasynUser, header, 2, 5000, &bytesRead, &eomReason);		
	if(status != asynSuccess) return status;
				
	size = header[1];
	payload = (char *) malloc((size+1)*sizeof(char));
		
	status = pasynOctetSyncIO->read(ppvt->pasynUser, payload, size, 5000, &bytesRead, &eomReason);
	if(status != asynSuccess) return status;
	//TODO:NO THE BEST WAY TO DO THIS CONVERSION!!Use the protocol!	
	union{
		unsigned char c[4];
		epicsInt32 inp_i;
	}u;
	u.inp_i = 0;
	int i;
	for(i=0;i<size;i++){
		u.c[i] = payload[i];
	}
	*value = u.inp_i;
	//*value = com.readingVariable(header, payload,simple);
	
		
	return status;
}

static asynInt32 int32Methods = { int32Write, int32Read };

/*
 * asynFloat64 methods
 */
static asynStatus
float64Write(void *pvt, asynUser *pasynUser, epicsFloat64 value)
{
	FrontendPvt *ppvt = (FrontendPvt *)pvt;
	unsigned int raw;
	uint8_t* send;
	
	float val = (float) value;
	raw = (unsigned int) ((val+10)*262143)/20.0;

	send = (uint8_t*) malloc(3*sizeof(char));

	int i;
	for(i = 0; i < 2; i++)
	{
		send[i] = raw & 255;
		raw = raw >> 8;
	}

	struct sllp_var * var = &ppvt->vars->list[pasynUser->reason];

	if(sllp_write_var(sllp, var, send))
	{
		return asynError;
	}

	return asynSuccess;
}

static asynStatus
float64Read(void *pvt, asynUser *pasynUser, epicsFloat64 *value)
{
	FrontendPvt *ppvt = (FrontendPvt *)pvt;

	uint8_t *val;
	unsigned int raw = 0;
	val = (uint8_t*) malloc(3*sizeof(char));

	struct sllp_var * var = &ppvt->vars->list[pasynUser->reason];
	

	if(sllp_read_var(ppvt->sllp, var, val))
	{
		return asynError;
	}

	int i;

	for(i=0; i<3; i++)
	{
		raw = raw << 8;
		raw += val[i];				
	}

	//18 bits
	float result = ((20*raw)/262143.0)-10;

	*value = (epicsFloat64) result;

	return asynSuccess;
}

static asynFloat64 float64Methods = { float64Write, float64Read };

epicsShareFunc int 
//devFrontendConfigure(const char *portName, const char *hostInfo, int flags, int priority)
devFrontendConfigure(const char *portName, const char *hostInfo, int priority)
{
    FrontendPvt *ppvt;
    char *lowerName, *host;
    asynStatus status;

    /*
     * Create our private data area
     */
    ppvt = callocMustSucceed(1, sizeof(FrontendPvt), "devFrontendConfigure");
    if (priority == 0) priority = epicsThreadPriorityMedium;

    ppvt->sllp = sllp_client_new(sendCommandtest, receiveCommandtest);

	if(!ppvt->sllp)
	{
		printf("SLLP fail\n");
		return -1;
	}

	sllp_client_init(ppvt->sllp);
	sllp_get_vars_list(ppvt->sllp, &ppvt->vars);

	printf("SLLP initialized\n");


    /*
     * Create the port that we'll use for I/O.
     * Configure it with our priority, autoconnect, no process EOS.
     * We have to create this port since we are multi-address and the
     * IP port is single-address.
     */
    lowerName = callocMustSucceed(1, strlen(portName)+5, "devFrontendConfigure");
    sprintf(lowerName, "%s_TCP", portName);
    host = callocMustSucceed(1, strlen(hostInfo)+5, "devFrontendConfigure");
    sprintf(host, "%s TCP", hostInfo);
    drvAsynIPPortConfigure(lowerName, host, priority, 0, 1);
    status = pasynOctetSyncIO->connect(lowerName, -1, &ppvt->pasynUser, NULL);
    if (status != asynSuccess) {
        printf("Can't connect to \"%s\"\n", lowerName);
        return -1;
    }
    free(host);
    free(lowerName);

    /*
     * Create our port
     */
    status = pasynManager->registerPort(portName,
                                        ASYN_CANBLOCK,
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
    ppvt->asynCommon.interfaceType = asynCommonType;
    ppvt->asynCommon.pinterface  = &commonMethods;
    ppvt->asynCommon.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynCommon);
    if (status != asynSuccess) {
        printf("Can't register asynCommon support.\n");
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
    
    ppvt->asynDrvUser.interfaceType = asynDrvUserType;
    ppvt->asynDrvUser.pinterface = &drvUser;
    ppvt->asynDrvUser.drvPvt = ppvt;
    status = pasynManager->registerInterface(portName, &ppvt->asynDrvUser);
    if (status != asynSuccess) {
        printf("Can't register asynFloat64 support.\n");
        return -1;
    }
    return 0;
}

/*
 * IOC shell command
 */
static const iocshArg devFrontendConfigureArg0 = { "port name",iocshArgString};
static const iocshArg devFrontendConfigureArg1 = { "host:port",iocshArgString};
static const iocshArg devFrontendConfigureArg2 = { "flags",iocshArgInt};
//static const iocshArg devFrontendConfigureArg3 = { "priority",iocshArgInt};
static const iocshArg *devFrontendConfigureArgs[] = {
                    &devFrontendConfigureArg0, &devFrontendConfigureArg1,
                    &devFrontendConfigureArg2/*, &devFrontendConfigureArg3*/ };
static const iocshFuncDef devFrontendConfigureFuncDef =
                      {"devFrontendConfigure",3,devFrontendConfigureArgs};
static void devFrontendConfigureCallFunc(const iocshArgBuf *args)
{
    devFrontendConfigure(args[0].sval, args[1].sval, args[2].ival);
}

static void
devFrontendConfigure_RegisterCommands(void)
{
    iocshRegister(&devFrontendConfigureFuncDef,devFrontendConfigureCallFunc);
}
epicsExportRegistrar(devFrontendConfigure_RegisterCommands);
