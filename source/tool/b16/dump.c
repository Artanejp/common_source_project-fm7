/*
	Dump B16 IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	unsigned char bank;
	unsigned int i;
	void far *ptr;
	void *buf = malloc(0x8000);
	
	/* F0000-FFFFF: IPL */
	printf("IPL ROM\n");
	fp = fopen("IPL.ROM", "wb");
	ptr = MK_FP(0xf000, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	ptr = MK_FP(0xf800, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	fclose(fp);
	
	/* KANJI ROM */
	fp = fopen("KANJI.ROM", "wb");
	outp(0xf4, 0);
	for(bank = 0; bank < 4; bank++) {
		outp(0x84, bank);
		printf("KANJI ROM %d/16\n", bank * 4 + 1);
		for(i = 0; i < 0x8000; i += 2) {
			ptr = MK_FP(0xc000, i);
retry1:
			far_memcpy(buf, ptr, 1);
			if(!(inp(0x80) & 0x10)) goto retry1;
			fwrite(buf, 1, 1, fp);
		}
		printf("KANJI ROM %d/16\n", bank * 4 + 2);
		for(i = 0; i < 0x8000; i += 2) {
			ptr = MK_FP(0xc800, i);
retry2:
			far_memcpy(buf, ptr, 1);
			if(!(inp(0x80) & 0x10)) goto retry2;
			fwrite(buf, 1, 1, fp);
		}
		printf("KANJI ROM %d/16\n", bank * 4 + 3);
		for(i = 0; i < 0x8000; i += 2) {
			ptr = MK_FP(0xd000, i);
retry3:
			far_memcpy(buf, ptr, 1);
			if(!(inp(0x80) & 0x10)) goto retry3;
			fwrite(buf, 1, 1, fp);
		}
		printf("KANJI ROM %d/16\n", bank * 4 + 4);
		for(i = 0; i < 0x8000; i += 2) {
			ptr = MK_FP(0xd800, i);
retry4:
			far_memcpy(buf, ptr, 1);
			if(!(inp(0x80) & 0x10)) goto retry4;
			fwrite(buf, 1, 1, fp);
		}
	}
	fclose(fp);
	
	/* DIC ROM */
	fp = fopen("DIC.ROM", "wb");
	for(bank = 0x88; bank <= 0x8d; bank++) {
		printf("DIC ROM %d/6\n", bank - 0x88 + 1);
		outp(0xf4, bank);
		ptr = MK_FP(0xc000, 0);
		far_memcpy(buf, ptr, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		ptr = MK_FP(0xc800, 0);
		far_memcpy(buf, ptr, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		ptr = MK_FP(0xd000, 0);
		far_memcpy(buf, ptr, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		ptr = MK_FP(0xd800, 0);
		far_memcpy(buf, ptr, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
	}
	fclose(fp);
}
