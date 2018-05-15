/*
	Dump PC-98RL BIOS/KANJI ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	void far *src;
	void *tmp = malloc(0x1000);
	
	outp(0x53d, 0x46);
	outp(0x43f, 0xc0);
	
	/* SASI */
	fp = fopen("SASI.ROM", "wb");
	src = MK_FP(0xd700, 0);
	far_memcpy(tmp, src, 0x1000);
	fwrite(tmp, 0x1000, 1, fp);
	fclose(fp);
}
