/*
	DUMP PC-98HA 1MB MEMORY CARD
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
	
	/* MEMORY CARD */
	fp = fopen("MEMCARD.BIN", "wb");
	src = MK_FP(0xdc00, 0);
	for(page = 0; page < 64; page++) {
		printf("page=%d/64\n", page);
		outp(0x1e8e, 0x82);
		outp(0xe8e, page);
		far_memcpy(tmp, src, 0x4000);
		fwrite(tmp, 0x4000, 1, fp);
	}
	fclose(fp);
}
