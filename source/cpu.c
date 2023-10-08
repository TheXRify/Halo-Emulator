#include "cpu.h"

#include "halo_defs.h"

#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <windows.h>

enum {
	LDA, LDB, LDD, LDE,
	LIA, LIB, LID, LIE,
	ADD, SUB, DIV, MUL,
	XOR, OR, AND,
	JMP, JZ, JNZ, JC, JNC,
	SB, GB,
	HLT,
	EB, DB,
	DEF,
	SP, DP,
	INC, DEC,
	STATE
};

static halo_t halo = {0};

inline void initComponent(int step, int num, short *arr);

void haloInit(void) {
	initComponent(1, DP + 1, halo.reg);
	assert(halo.reg != NULL);
	
	for(int i = 0; i < MEMORY_LIMIT; i++) {
		halo.mem[i] = 0;
	}
	
	initComponent(1, 2, halo.bus);
	assert(halo.bus != NULL);
	halo.bus[0] = -1;
	halo.bus[1] = -1;
	
	initComponent(1, 6, halo.lines);
	assert(halo.lines != NULL);
	
	halo.state = READ;
	halo.pc = PROGRAM_MEMORY_OFFSET;
	halo.mem_ptr = VARIABLE_MEMORY_OFFSET;
	halo.running = 0;
}

void haloSetBus(unsigned char i, short val) {
	halo.bus[i] = val;
}

short haloGetBus(unsigned char i) {
	return halo.bus[i];
}

void initComponent(int step, int num, short *arr) {
	for (int i = 0; i < num; i += step) {
		arr[i] = 0;
	}
}

void haloLoadProgram(char *program, int size) {
	memcpy(halo.mem, program, size);
}

inline void setFlag(void);
int processOpcode(char opcode, int *step);

void haloBeginProcess(void) {
	if(!halo.running)
		halo.running = 1;
	
	char opcode = 0;
	
	printf("%dHz - %dms\n", CLOCK_SPEED_HZ, CLOCK_SPEED_MS);
	printf("\n");
	
	/*printf("================== PROGRAM MEMORY =================\n\n");
	for(int i = 0; i < 0x1000; i++) {
		if(i % 15 == 0)
			printf("\n");
		
		printf("%02x ", halo.mem[i]);
	}
	printf("\n");*/
	
	while(halo.running) {
		int step = 1;
		opcode = halo.mem[halo.pc];
		
		if(processOpcode(opcode, &step) != 0) {
			fprintf(stderr, "Opcode, 0x%04x, not defined\n", opcode);
			halo.running = 0;
			break;
		}
		
		halo.pc += step;
		
		if(halo.lines[IO] == 1) {
			// TODO: implement keypresses through EXPR1
			if(halo.bus[1] != -1) {
				if((halo.bus[1] > 31 && halo.bus[1] < 127) || halo.bus[1] == '\n') {
					printf("%c", halo.bus[1]);
				} else if(halo.bus[1] == '\0') {
					halo.bus[1] = -1;
				} else {
					printf("%d", halo.bus[1]);
				}
				
				halo.bus[1] = -1;
			}			
		}
		
		Sleep(1/CLOCK_SPEED_HZ);
	}
	
	/*for(int i = VARIABLE_MEMORY_OFFSET; i < VARIABLE_MEMORY_OFFSET + 100; i++) {
		if(i % 15 == 0)
			printf("\n");
		
		printf("%02x ", halo.mem[i]);
	}
	printf("\n");*/
	
	printf("\n=====END PROCESS=====\n");
}

// LDA, LDB, LDD, LDE
static inline void LDX(int reg) {
	short left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
	
	if(halo.state == FETCH) {
		halo.reg[reg] = halo.mem[halo.reg[left]];
		halo.state = READ;
	} else {
		halo.reg[reg] = halo.mem[left];
	}
}

// LIA, LIB, LID, LIE
static inline void LIX(int reg) {
	short left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
	
	if(halo.state == FETCH) {
		halo.reg[reg] = halo.reg[left];
		halo.state = READ;
	} else {
		halo.reg[reg] = left;
	}
}

int processOpcode(char opcode, int *step) {
	short left = 0;
	short right = 0;
	
	switch(opcode) {
		case LDA:
		{
			LDX(RA);
			(*step) += 2;
			return 0;
		};
		case LDB:
		{
			LDX(RB);
			(*step) += 2;
			return 0;
		};
		case LDD: 
		{
			LDX(RD);
			(*step) += 2;
			return 0;
		};
		case LDE:
		{
			LDX(RE);
			(*step) += 2;
			return 0;
		};
		case LIA:
		{
			LIX(RA);
			(*step) += 2;
			return 0;
		};
		case LIB:
		{
			LIX(RB);
			(*step) += 2;
			return 0;
		};
		case LID:
		{
			LIX(RD);
			(*step) += 2;
			return 0;
		};
		case LIE:
		{
			LIX(RE);
			(*step) += 2;
			return 0;
		};
		case ADD:
		{
			halo.reg[RT] = halo.reg[RA] + halo.reg[RB];
			setFlag();
			return 0;
		};
		case SUB:
		{
			halo.reg[RT] = halo.reg[RA] - halo.reg[RB];
			setFlag();
			return 0;
		};
		case MUL:
		{
			halo.reg[RT] = halo.reg[RA] * halo.reg[RB];
			setFlag();
			return 0;
		};
		case DIV:
		{
			halo.reg[RT] = halo.reg[RA] / halo.reg[RB];
			setFlag();
			return 0;
		};
		case XOR:
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			right = ((halo.mem[halo.pc + 3] << 8) | halo.mem[halo.pc + 4]);
			
			if(halo.state == FETCH) {
				halo.reg[RT] = halo.reg[left] ^ halo.reg[right]; // halo.mem[right]
				halo.state = READ;
			} else {
				halo.reg[RD] = left ^ right;
			}
			
			setFlag();
			(*step) += 4;
			return 0;
		};
		case OR: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			right = ((halo.mem[halo.pc + 3] << 8) | halo.mem[halo.pc + 4]);
			
			if(halo.state == FETCH) {
				halo.reg[RT] = halo.reg[left] | halo.reg[right];
				halo.state = READ;
			} else {
				halo.reg[RT] = left | right;
			}
			
			setFlag();
			(*step) += 4;
			return 0;
		};
		case AND: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			right = ((halo.mem[halo.pc + 3] << 8) | halo.mem[halo.pc + 4]);
			
			if(halo.state == FETCH) {
				halo.reg[RT] = halo.reg[left] & halo.reg[right];
				halo.state = READ;
			} else {
				halo.reg[RT] = left & right;
			}
			
			setFlag();
			(*step) += 4;
			return 0;
		};
		case JMP: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			halo.pc = left + PROGRAM_MEMORY_OFFSET;
			(*step) = 0;
			return 0;
		};
		case JZ:
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.reg[RF] == ZERO) {
				halo.pc = left + PROGRAM_MEMORY_OFFSET;
				(*step) = 0;
				return 0;
			}
			
			(*step) += 2;
			return 0;
		};
		case JNZ: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.reg[RF] != ZERO) {
				halo.pc = left + PROGRAM_MEMORY_OFFSET;
				(*step) = 0;
				return 0;
			}
			
			(*step) += 2;
			return 0;
		};
		case JC: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.reg[RF] == CARRY) {
				halo.pc = left + PROGRAM_MEMORY_OFFSET;
				(*step) = 0;
				return 0;
			}
			
			(*step) += 2;
			return 0;
		};
		case JNC:
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.reg[RF] != CARRY) {
				halo.pc = left + PROGRAM_MEMORY_OFFSET;
				(*step) = 0;
				return 0;
			}
			
			(*step) += 2;
			return 0;
		};
		case SB:
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.state == FETCH) {
				halo.bus[HOUT] = halo.reg[left];
				halo.state = READ;
			} else {
				halo.bus[HOUT] = left;
			}
			
			(*step) += 2;
			return 0;
		};
		case GB:
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			halo.reg[left] = halo.bus[HIN];
			halo.bus[HIN] = -1;
			halo.state = READ;
			
			(*step) += 2;
			return 0;
		};
		case HLT:
		{
			halo.running = 0;
			return 0;
		};
		case EB:
		{
			halo.lines[halo.reg[RE]] = 1;
			return 0;
		};
		case DB:
		{
			halo.lines[halo.reg[RE]] = 0;
			return 0;
		};
		case DEF: // TODO: prototyping
		{
			int i = 0;
			for(; i < 0xff; i++) {
				int pc = halo.pc + i + 1;
				if((unsigned)(halo.mem[pc]) >= (unsigned)(0xff))
					break;
				
				
				char temp = halo.mem[pc];
				halo.mem[halo.mem_ptr] = temp;
				halo.mem_ptr ++;
				(*step) += 1;
			}
			(*step) += 1;
			
			return 0;
		};
		case SP: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.state == FETCH) {
				halo.reg[RSP] = halo.reg[left];
				halo.state = READ;
			} else {
				halo.reg[RSP] = left;
			}
			
			(*step) += 2;
			return 0;
		};
		case DP: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.state == FETCH) {
				halo.reg[RDP] = halo.reg[left];
				halo.state = READ;
			} else {
				halo.reg[RDP] = left;
			}
			
			(*step) += 2;
			return 0;
		};
		case INC: 
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.state == FETCH) {
				halo.reg[left] += 1;
				halo.state = READ;
			} else {
				// TODO: prototype later
			}
			
			(*step) += 2;
			return 0;
		};
		case DEC:
		{
			left = ((halo.mem[halo.pc + 1] << 8) | halo.mem[halo.pc + 2]);
			
			if(halo.state == FETCH) {
				halo.reg[left] -= 1;
				halo.state = READ;
			} else {
				// TODO: prototype later
			}
			
			(*step) += 2;
			return 0;
		};
		case STATE:
		{
			halo.state = FETCH;
			return 0;
		};
		default:
		{
			return -1;
		}
	}
}

void setFlag(void) {
	if(halo.reg[RT] > 0xFFFF) {
		halo.reg[RC] = halo.reg[RD] - 0xFFFF;
		halo.reg[RF] = CARRY;
	}
	
	if(halo.reg[RT] < -0xFFFF) {
		halo.reg[RC] = 0xFFFF + halo.reg[RD];
		halo.reg[RF] = CARRY;
	}
	
	if(halo.reg[RT] == 0)
		halo.reg[RF] = ZERO;
}
