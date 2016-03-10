/*
	Dump FMR-50 IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	void *tmp;
	void far *ipl;
	
	tmp = malloc(0x4000);
	fp = fopen("FMR50IPL.ROM", "wb");
	ipl = MK_FP(0xfc00, 0);
	far_memcpy(tmp, ipl, 0x4000);
	fwrite(tmp, 0x4000, 1, fp);
	fclose(fp);
	
	fp = fopen("MACHINE.ID", "wb");
	fputc(inp(0x30), fp);
	fputc(inp(0x31), fp);
	fclose(fp);
}
