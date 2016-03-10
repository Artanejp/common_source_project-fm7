/*
	Dump N5200 IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	void far *ptr;
	void *buf = malloc(0x8000);
	
	/* F0000-FFFFF: IPL */
	fp = fopen("IPL.ROM", "wb");
	ptr = MK_FP(0xf000, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	ptr = MK_FP(0xf800, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	fclose(fp);
}
