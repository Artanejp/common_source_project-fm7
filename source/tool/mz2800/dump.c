/*
	Dump MZ-2800 IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <farstr.h>
#include <dos.h>
#include <memory.h>

void main()
{
	FILE* fp;
	void *tmp;
	void far *ipl;
	char bank;
	
	tmp = malloc(0x8000);
	fp = fopen("MZ28IPL.ROM", "wb");
	
	printf("dump ff0000-ff7fff\n");
	bank = inp8(0x8c);
	outp8(0x8c, 0x3f);
	ipl = MK_FP(0xb000, 0);
	far_memcpy(tmp, ipl, 0x8000);
	outp8(0x8c, bank);
	fwrite(tmp, 0x8000, 1, fp);
	
	printf("dump ff8000-ffffff\n");
	bank = inp8(0x8c);
	outp8(0x8c, 0x3f);
	ipl = MK_FP(0xb800, 0);
	far_memcpy(tmp, ipl, 0x8000);
	outp8(0x8c, bank);
	fwrite(tmp, 0x8000, 1, fp);
	
	printf("complete\n");
	fclose(fp);
}
