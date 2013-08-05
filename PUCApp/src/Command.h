#ifndef COMMAND_H
#define COMMAND_H

enum COMMANDS
{
   READ_VARIABLE        =0x10,
   READING_VARIABLE     =0x11,
   WRITE_VARIABLE       =0x20,
   TRANSMIT_BLOCK_CURVE	=0x40,
   BLOCK_CURVE          =0x41,
   OK_COMMAND           =0xE0
};

class Command
{
private:		
	unsigned int valueToBytes(double value);
	unsigned int valueToBytes(double value, int bits);
	float bytesToValue(unsigned int bytes);
	float bytesToValue (unsigned int bytes, int bits);
	
	int          checkSize(int size);	
	
public:    
	int    checkSize(char size);
    char * writeVariable(int address, int size, int id, double value, int * bytesToWrite,int simple);    
    char * readVariable(int address, int id, int * bytesToWrite,int simple);
    double readingVariable(char * header, char * payload,int simple);
    double* readingCurve(char * packet);
    char * readCurve(int address, int size, int id, int offset, int * bytesToWrite);
    char * writeCurveBlock(int address, int size, int id, int offset, epicsFloat64 * values, size_t nElements, int * bytesToWrite);
    
};

#endif
