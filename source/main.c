#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#include "cpu.h"

int main(int argc, char **argv) {
	argc--;
	argv++;
	
	haloInit();
	
	FILE *fd = fopen(*argv, "r");
	
	char program[0x4000] = {};
	
	fread(program, 1, 0x4000, fd);
	fclose(fd);
	
	haloLoadProgram(program, 0x4000);
	haloBeginProcess();
}