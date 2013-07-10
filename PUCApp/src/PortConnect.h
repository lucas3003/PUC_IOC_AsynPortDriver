#ifndef PORT_CONNECT_H_
#define PORT_CONNECT_H_

#include <asynPortDriver.h>
#include <asynOctetSyncIO.h>
#include <Command.cpp>

#define P_AddressString "ADDRESS"
#define P_IdString      "ID"
#define P_ValueString   "VALUE"
#define P_TypeString    "TYPE"
#define P_SizeString    "SIZE"
#define P_OffsetString  "OFFSET"


class PortConnect : public asynPortDriver
{
public:
    PortConnect(const char *portName, const char *serialName);
    
    virtual asynStatus writeFloat64(asynUser* pasynUser, epicsFloat64 value);
    virtual asynStatus readFloat64(asynUser* pasynUser, epicsFloat64* value);
    
    virtual asynStatus writeFloat64Array(asynUser* pasynUser, epicsFloat64* value, size_t nElements);
    
protected:
	int P_Address;
	int P_Id;
	int P_Value;
	int P_Type;
	int P_Size;
	int P_Offset;

private:
	Command com;
	asynUser* user;
};

#endif
