#include <Sllp.h>
#include <stdlib.h>
#include <math.h>

int Sllp :: checkSize(int size)
{
	if(size < 128) return size;
	
	int result;
	
	result = ((size-2)/128)-1;
	result = result | 0x80; //Set first bit 1
	
	return result;
}

int Sllp :: checkSize(char size)
{
	int more, less;	
	
	more = (size >> 7) & 0x1;
	less = size & 0x7F;
	
	if(more) return 128*(less+1)+2;	 
	
	return less;
}

unsigned int Sllp :: valueToBytes(double value, int bits)
{
	unsigned int result = (unsigned int) ((value+10)*(pow(2, (double)bits)-1))/20.0;
	return result;
}

float Sllp :: bytesToValue (unsigned int bytes, int bits)
{
	float result = ((20*bytes)/(pow(2, (double) bits)-1)) -10;
	return result;
}

double Sllp :: readingVariable(char * header, char * payload,int simple)
{
	//Header: 4 bytes
	//Payload: size+1 bytes	
	
	//TODO: Check command and checksum
	
	int i;
	unsigned int bytes = 0;
	int size = 0;
	if(!simple)
		size = checkSize((char) header[3]);
	else
		size = checkSize((char) header[1]);
	union {
               	unsigned char c[8];
               	double f;
       	} u;
	for(i = 0; i < size; i++)
	{
		u.c[i] =  payload[i];
	}
	
	printf("Size = %d\n", size);
	printf("Bytes = %u\n", bytes);
	return u.f;
	
}

/*double * Sllp :: readingCurve(char * packet)
{
	
	   Payload: Byte 4 to byte 16389
	   Points by offset: 8192
        
	printf("Reading curve\n");

	double * result;
	int i, j, iresult=0;
	unsigned int temp = 0;

	result = (double*) malloc(8192*sizeof(double));

	for(i = 4; i < 16390; i+=2)
	{
		temp = 0;

		for(j = 0; j < 2; j++)
 		{
			temp = temp << 8;
			temp += packet[i+j];			
		}

		result[iresult++] = bytesToValue(temp & 0xFFFF, 16);
	}

	return result;
}*/

char * Sllp :: readVariable(int id, int * bytesToWrite)
{
	/*
		Packet: Command (1 byte), size (1 byte), payload (size bytes)
	*/

	char * result;
	int i;
	printf("Read id = %d\n", id);
	unsigned int check = 0;

	result[0] = READ_VARIABLE;
	result[1] = 1;
	result[2] = id & 0xFF;

	printf("Send: %d | %d | %d \n", result[0], result[1], result[2] & 0xFF);

	fflush(stdout);
	return result;
}

/*char * Sllp :: readCurve(int address, int size, int id, int offset, int * bytesToWrite)
{
	
	   Packet: Address (1 byte), origin (1 byte), command (1 byte), size (1 byte), payload (2 bytes),
	   checksum (1 byte)
	

	char * result;
	unsigned int check = 0;
	int i;
	result = (char *) malloc (7*sizeof(char));
	*bytesToWrite = 7*sizeof(char);

	result[0] = address & 0xFF;
 	result[1] = 0;
	result[2] = TRANSMIT_BLOCK_CURVE;
	result[3] = checkSize(size);
	result[4] = id & 0xFF;
	result[5] = offset & 0xFF;

	for(i = 0; i <= 5; i++) check += result[i];

	result[6] = 0x100 - (check & 0xFF);

	printf("Send: %d | %d | %d | %d | %d | %d | %u\n", result[0], result[1], result[2], result[3], result[4], result[5], result[6]&0xFF);

	fflush(stdout);
	return result;	
}*/

/*char * Sllp :: writeCurveBlock(int address, int size, int id, int offset, epicsFloat64 * values, size_t nElements, int * bytesToWrite)
{
	
	   Packet: Address (1 byte), origin (1 byte), command (1 byte), size (1 byte),
	   payload (size bytes), checksum (1 byte)	
	
	
	char * result;
	size_t i, ibuf;
	unsigned int bytes;
	
	
	result = (char *) malloc ((5+size)*sizeof(char));
	*bytesToWrite = (5+size)*sizeof(char);
	
	result[0] = address & 0xFF;
	result[1] = 0;
	result[2] = BLOCK_CURVE;
	result[3] = checkSize(size);
	result[4] = id & 0xFF;
	result[5] = offset & 0xFF;
	ibuf = 6;
	
	//8192 point by offset. Each value have 2 bytes. Total payload = (points*2)+offset+id -> Total payload = 8192*2+1+1
	//Total payload = 16386
	//If the number of points is smallest than 8192, the last bytes will byte 0
	
	for(i = 0; i < nElements; i++)
	{
		
		bytes = valueToBytes(values[i], 16) & 0xFFFF;
		result[ibuf++] = (bytes >> 8) & 0xFF;
		result[ibuf++] = bytes & 0xFF;
	}
	
	if(nElements < 8192)
		for(i = ibuf; i < 16390; i++) result[i] = 0;		
	
	//Checksum 0	
	result[16390] = 0;	
	
	//16391 bytes
	return result;
}*/

bool Sllp :: checkReturn(char* packet)
{
	if((packet[0] & 0xFF) == OK_COMMAND) return true;

	return false;
}

char * Sllp :: writeVariable(int id, int bits, double value, int * bytesToWrite)
{
	char * result;
	int i;
	unsigned int check = 0;
	
	printf("Write value = %f\n", value);

	int size = bits/8;
	if((bits/8.0) > (bits/8)) size++;
	size++;

	result = (char *) malloc ((2+size)*sizeof(char));
	*bytesToWrite = (2+size)*sizeof(char);

	result[0] = WRITE_VARIABLE;
	result[1] = checkSize(size);
	result[2] = id & 0xFF;

	unsigned int bytes = valueToBytes(value, bits);
	unsigned int copyBytes = bytes;

	for(i = size+1; i>2; i--)
	{
		result[i] = bytes & 255;
		bytes = bytes >> 8;
	}

	printf("Send:  %d | %d | %u | %u | %u\n", result[0], result[1], result[2]&0xFF, copyBytes, result[size+2] & 0xFF);
	return result;
}
