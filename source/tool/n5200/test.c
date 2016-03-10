/*
	Dump PC-9801E/F/M IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>
#include <memory.h>

void wait()
{
	int i, j = 0;
	for(i = 0; i < 32000; i++)
		j += i;
}

void main()
{
	int i,tmp[10];
	di();
	wait();
	outp(0x43, 0x00);
	wait();
	outp(0x43, 0x00);
	wait();
	outp(0x43, 0x00);
	wait();
	outp(0x43, 0x40);
	wait();
	outp(0x43, 0x5f);
	wait();
	outp(0x43, 0x3a);
	wait();
	outp(0x43, 0x32);
	wait();
	outp(0x43, 0x16);
	wait();
	for(i = 0; i < 4;) {
		int tmp2 = inp(0x43);
		if(tmp2 & 2) {
			outp(0x43, 0x16);
			tmp[i++] = inp(0x41);
		}
	}
	for(i = 0; i < 4; i++)
		printf("%2x\n", tmp[i]);
	outp(0x41, 0x00);
	wait();
	outp(0x43, 0x16);
	wait();
	for(i = 0; i < 4;) {
		int tmp2 = inp(0x43);
		if(tmp2 & 2) {
			outp(0x43, 0x16);
			tmp[i++] = inp(0x41);
		}
	}
	for(i = 0; i < 4; i++)
		printf("%2x\n", tmp[i]);
	outp(0x64, 0x00);
	ei();
}
