#ifndef PORT_CONNECT_H_
#define PORT_CONNECT_H_

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>
#include <Command.cpp>

#define P_TemperatureSetPoint "T_SetPoint"
#define P_TemperatureSensor1      "T_Sensor1"
#define P_TemperatureSensor2   "T_Sensor2"
#define P_TemperatureSensor3    "T_Sensor3"
#define P_TemperatureSensor4    "T_Sensor4"
#define P_SwitchState  "S_State"


class PortConnect : public asynPortDriver
{
public:
    PortConnect(const char *portName, const char *serialName);
    
    virtual asynStatus writeFloat64(asynUser* pasynUser, epicsFloat64 value);
    virtual asynStatus readFloat64(asynUser* pasynUser, epicsFloat64* value);
    
    //virtual asynStatus writeFloat64Array(asynUser* pasynUser, epicsFloat64* value, size_t nElements);
    //virtual asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn);
    
protected:
	int P_TemperatureSP;
	int P_TemperatureS1;
	int P_TemperatureS2;
	int P_TemperatureS3;
	int P_TemperatureS4;
	int P_SState;

private:
	Command com;
	asynUser* user;
	int timeout;
};

#endif
