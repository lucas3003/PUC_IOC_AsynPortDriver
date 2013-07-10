#include <Command.h>
#include <stdlib.h>

int Command :: checkSize(int size)
{
	if(size < 128) return size;
	
	int result;
	
	result = ((size-2)/128)-1;
	result = result | 0x80; //Set first bit 1
	
	return result;
}

unsigned int Command :: valueToBytes(double value)
{			
	unsigned int result = (unsigned int) ((value+10)*262143)/20.0;	
	return result;
}


char * Command :: writeVariable(int address, int size, int id, double value, int * bytesToWrite)
{
	char * result;
	int i;
	unsigned int check = 0;
	
	printf("Value = %f\n", value);
	
	/*
	   Packet: Destination (1 byte), origin (1 byte), command (1 byte), size (1 byte), 
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
	result[size+5] = 0x100 - (check & 0xFF);
	
	printf("Write: %d | %d | %d | %d | %d | %u | %u\n", result[0], result[1], result[2], result[3], result[4], copyBytes, result[size+5] & 0xFF);
	
	return result;	
}
