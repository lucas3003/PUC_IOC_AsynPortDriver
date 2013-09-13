#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64)
typedef union DV {
	uint8_t vvalue[8];
	double dvalue;
} double_value;

typedef union FV {
	uint8_t vvalue[8];
	float fvalue;
} float_value;

typedef union SIV {
	uint8_t vvalue[2];
	short int sivalue;
} short_int_value;

typedef union UI8V {
	uint8_t vvalue[1];
	uint8_t ui8value;
} unsigned_int_8_value;

typedef union UI32V {
	uint8_t vvalue[1];
	uint32_t ui32value;
} unsigned_int_32_value;
#endif
