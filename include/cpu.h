/* date = September 29th 2023 4:05 am */

#ifndef CPU_H
#define CPU_H

#include "halo_defs.h"

typedef struct halo {
	short reg[9];
	short pc;
	short mem_ptr;
	char mem[MEMORY_LIMIT];
	
	/*
[0] = input
[1] = output
*/
	short bus[2];
	short lines[6];
	
	unsigned char state;
	unsigned char running;
} halo_t;

enum flags {
	CARRY, EQUAL, ZERO, NOT
};

enum states {
	FETCH, READ
};

void haloInit(void);
void haloSetBus(unsigned char i, short val);
short haloGetBus(unsigned char i);

void haloLoadProgram(char *program, int size);
void haloBeginProcess(void);

#endif //CPU_H
