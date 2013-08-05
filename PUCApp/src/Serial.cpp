#include <Serial.h>

Serial :: Serial()
{
	isConnected = false;
	timeout = 5000;
}

bool Serial :: connect(const char * serialName)
{
	try
	{
	 	asynStatus status = pasynOctetSyncIO->connect(serialName, 0, &user, NULL);
	   
		if(status == asynSuccess) return true; 
		else return false;   

		pasynOctetSyncIO->flush(user);

		isConnected = true;
		return true;
	}
	catch(...)
	{
		return false;
	}

}

int Serial :: checkSize(char size)
{
	int more, less;	
	
	more = (size >> 7) & 0x1;
	less = size & 0x7F;
	
	if(more) return 128*(less+1)+2;	 
	
	return less;
}

bool Serial :: send (char address, char * packet, int bytes)
{
	char * result;
	int i;
	unsigned int check = 0;
	size_t wrote;

	try
	{
		result = (char *) malloc((bytes+3)*sizeof(char));
		result[0] = address & 0xFF;
		result[1] = 0;

		for(i = 2; i < bytes+2; i++) result[i] = packet[i-2];
		for(i = 0; i < bytes+2; i++) check += result[i];

		result[bytes+2] = 0x100 - (check & 0xFF);
		printf("Serial::send : result[%d] = %d\n", bytes+2, result[bytes+2]&0xFF);

		asynStatus status = pasynOctetSyncIO->write(user, result, bytes+3, timeout, &wrote);

		if(status != asynSuccess) return false;

		return true;
	}
	catch(...)
	{
		return false;
	}
}

char * Serial :: receive()
{
	char *address, *header, *payload, *checksum, *result;
	int size;
	size_t bytesRead;
	int eomReason;
	asynStatus status;


	try
	{
		address = (char *) malloc (2*sizeof(char));
		header = (char *) malloc (2*sizeof(char));

		status = pasynOctetSyncIO->read(user, address, 2, timeout, &bytesRead, &eomReason);
		if((status != asynSuccess) || bytesRead != 2) return "";

		//TODO : Verify address

		status = pasynOctetSyncIO->read(user, header, 2, timeout, &bytesRead, &eomReason);
		if((status != asynSuccess) || bytesRead != 2) return "";

		size = checkSize((char) header[1]);
		printf("Serial: size = %d\n", size);
		printf("Serial: header[0] = %u\n", header[0]&0xFF);
		printf("Serial: header[1] = %d\n", header[1]);

		if(size > 0)
		{
			payload = (char *) malloc(size*sizeof(char));

			status = pasynOctetSyncIO->read(user, payload, size, timeout, &bytesRead, &eomReason);
			if((status != asynSuccess) || bytesRead != size) return "";			
		}


		checksum = (char *) malloc(1*sizeof(char));

		status = pasynOctetSyncIO->read(user, checksum, 1, timeout, &bytesRead, &eomReason);
		if((status != asynSuccess) || bytesRead != 1) return "";

		//TODO : Verify checksum

		printf("Serial: checksum = %d\n", checksum[0] & 0xFF);

		result = (char *) malloc((3+size)*sizeof(char));
		memcpy(result,     header,  2);

		if(size > 0)
		{
			memcpy(result + 2, payload, size);			
		}

		result[2+size] = '\0';

		return result;
	}
	catch(...)
	{
		return "";
	}
}



