#ifndef SERIAL_H
#define SERIAL_H

#include <asynOctetSyncIO.h>
	
class Serial
{

private:
	asynUser * user;
	int timeout;


public:
	Serial();
	bool connect(const char * serialName);
	bool send(char address, char * packet, int bytes);
	char * receive();
	bool isConnected;
	int checkSize(char size);


};


#endif
