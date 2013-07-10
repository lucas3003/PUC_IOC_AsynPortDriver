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
	int    checkSize(int size);
	unsigned int valueToBytes(double value);
	
public:    
    char * writeVariable(int address, int size, int id, double value, int * bytesToWrite);    
};

#endif
