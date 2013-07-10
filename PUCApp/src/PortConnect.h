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


class PortConnect : public asynPortDriver
{
public:
    PortConnect(const char *portName, const char *serialName);
    virtual asynStatus writeFloat64(asynUser* pasynUser, epicsFloat64 value);
    
protected:
	int P_Address;
	int P_Id;
	int P_Value;
	int P_Type;
	int P_Size;

private:

	Command com;
	asynUser* user;
};

#endif
