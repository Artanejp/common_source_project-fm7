/*
	DUMP PC-98LT ROM DRIVE
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	void far *src;
	void *tmp = malloc(0x8000);
	char page;
	
	/* ROM DRIVE */
	fp = fopen("ROMDRV.ROM", "wb");
	for(page = 0; page < 8; page++) {
		printf("page=%d/8\n", page);
		outp(0xcc10, page);
		src = MK_FP(0xe000, 0);
		far_memcpy(tmp, src, 0x8000);
		fwrite(tmp, 0x8000, 1, fp);
		src = MK_FP(0xe800, 0);
		far_memcpy(tmp, src, 0x8000);
		fwrite(tmp, 0x8000, 1, fp);
	}
	fclose(fp);
}
