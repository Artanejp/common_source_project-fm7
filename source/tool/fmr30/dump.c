/*
	Dump FMR-30 IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <farstr.h>
#include <dos.h>
#include <memory.h>
#include <machine.h>

void main()
{
	FILE* fp;
	void *tmp;
	void far *ipl;
	
	outp(0xd1, 2);
	
	tmp = malloc(0x8000);
	fp = fopen("FMR30IPL.ROM", "wb");
	ipl = MK_FP(0xf000, 0);
	far_memcpy(tmp, ipl, 0x8000);
	fwrite(tmp, 0x8000, 1, fp);
	ipl = MK_FP(0xf800, 0);
	far_memcpy(tmp, ipl, 0x8000);
	fwrite(tmp, 0x8000, 1, fp);
	fclose(fp);
}
