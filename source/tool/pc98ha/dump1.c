/*
	DUMP PC-98HA IPL/DICTIONARY/KANJI ROM
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
	
	/* IPL */
	fp = fopen("IPL.ROM", "wb");
	src = MK_FP(0xf000, 0);
	far_memcpy(tmp, src, 0x8000);
	fwrite(tmp, 0x8000, 1, fp);
	src = MK_FP(0xf800, 0);
	far_memcpy(tmp, src, 0x8000);
	fwrite(tmp, 0x8000, 1, fp);
	fclose(fp);
	
	/* DICTIONARY */
	fp = fopen("DICT.ROM", "wb");
	src = MK_FP(0xd400, 0);
	for(page = 0; page < 48; page++) {
		printf("page=%d/48\n", page);
		outp(0x4c10, page);
		far_memcpy(tmp, src, 0x4000);
		fwrite(tmp, 0x4000, 1, fp);
	}
	fclose(fp);
	
	/* KANJI */
	fp = fopen("KANJI.ROM", "wb");
	src = MK_FP(0xd800, 0);
	for(page = 0; page < 16; page++) {
		printf("page=%d/16\n", page);
		outp(0x8c10, page);
		far_memcpy(tmp, src, 0x4000);
		fwrite(tmp, 0x4000, 1, fp);
	}
	fclose(fp);
}
