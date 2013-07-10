#include <Command.h>
#include <stdlib.h>
#include <math.h>

int Command :: checkSize(int size)
{
	if(size < 128) return size;
	
	int result;
	
	result = ((size-2)/128)-1;
	result = result | 0x80; //Set first bit 1
	
	return result;
}

int Command :: checkSize(char size)
{
	int more, less;	
	
	more = (size >> 7) & 0x1;
	less = size & 0x7F;
	
	if(more) return 128*(less+1)+2;	 
	
	return less;	
}

//Default: 18 bits
unsigned int Command :: valueToBytes(double value)
{			
	unsigned int result = (unsigned int) ((value+10)*262143)/20.0;	
	return result;
}

//Default: 18 bits
float Command :: bytesToValue(unsigned int bytes)
{
	float result = ((20*bytes)/262143.0)-10;
	return result;
}

unsigned int Command :: valueToBytes(double value, int bits)
{
	unsigned int result = (unsigned int) ((value+10)*(pow(2, (double)bits)-1))/20.0;
	return result;
}

float Command :: bytesToValue (unsigned int bytes, int bits)
{
	float result = ((20*bytes)/(pow(2, (double) bits)-1)) -10;
	return result;
}

double Command :: readingVariable(char * header, char * payload)
{
	//Header: 4 bytes
	//Payload: size+1 bytes	
	
	//TODO: Check command and checksum
	
	int i;
	unsigned int bytes = 0;
	int size = checkSize((char) header[3]);
	
	for(i = 0; i < size; i++)
	{
		bytes = bytes << 8;
		bytes += payload[i];
	}
	
	printf("Size = %d\n", size);
	printf("Bytes = %u\n", bytes);
	
	return bytesToValue(bytes & 0x3FFFF);
	
}

char * Command :: readVariable(int address, int id, int * bytesToWrite)
{
	char * result;
	int i;
	printf("Read id = %d\n", id);
	unsigned int check = 0;
	
	
	/*
	    Packet: Address (1 byte), origin (1 byte), command (1 byte), size (1 byte), 
	    payload (1 byte), checksum (1 byte)	 
	*/
	
	result = (char *) malloc(6*sizeof(char));
	*bytesToWrite = 6*sizeof(char);
	
	result[0] = address & 0xFF;
	result[1] = 0;
	result[2] = READ_VARIABLE;
	result[3] = 1;
	result[4] = id & 0xFF;
	
	for(i = 0; i <= 4; i++) check += result[i];
	
	//Checksum
	result[5] = 0x100 - (check & 0xFF);
	
	printf("Send: %d | %d | %d | %d | %d | %u\n", result[0], result[1], result[2], result[3], result[4], result[5] & 0xFF);	
	
	return result;
}

char * Command :: writeCurveBlock(int address, int size, int id, int offset, epicsFloat64 * values, size_t nElements, int * bytesToWrite)
{
	/*
	   Packet: Address (1 byte), origin (1 byte), command (1 byte), size(1 byte),
	   payload (size bytes), checksum (1 byte)	
	*/
	
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
}

char * Command :: writeVariable(int address, int size, int id, double value, int * bytesToWrite)
{
	char * result;
	int i;
	unsigned int check = 0;
	
	printf("Write value = %f\n", value);
	
	/*
	   Packet: Address (1 byte), origin (1 byte), command (1 byte), size (1 byte), 
	   payload (size bytes), checksum (1 byte)	  
	*/
	
	result = (char *) malloc ((5+size)*sizeof(char));
	*bytesToWrite = (5+size)*sizeof(char);
	result[0] = address & 0xFF;
	result[1] = 0;
	result[2] = WRITE_VARIABLE;
	result[3] = checkSize(size);
	result[4] = id & 0xFF;
	
	unsigned int bytes = valueToBytes(value);
	
	unsigned int copyBytes = bytes;
	
	for(i = size+3; i > 4; i--)
	{	
		result[i] = bytes & 255;
		bytes = bytes >> 8;
	}
	
	for(i = 0; i <= size+3; i++) check += result[i];
	
	//Checksum
	result[size+4] = 0x100 - (check & 0xFF);
	
	printf("Send: %d | %d | %d | %d | %d | %u | %u\n", result[0], result[1], result[2], result[3], result[4], copyBytes, result[size+4] & 0xFF);
	
	return result;	
}
