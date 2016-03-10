/*
	Dump MZ-6500 IPL ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	char *buf;
	void far *src;
	unsigned char bank, i;
	int j;
	
	buf = malloc(0x8000);
	
	fp = fopen("MZ65IPL.ROM", "wb");
	src = MK_FP(0xfc00, 0);
	far_memcpy(buf, src, 0x4000);
	for(j = 0; j < 0x4000; j++)
		fputc(buf[j], fp);
	fclose(fp);
	
	fp = fopen("MZ65DIC.ROM", "wb");
	for(i = 5; i >= 4; i--) {
		bank = inp(0x230);
		bank &= 0x1f;
		bank |= i << 5;
		outp(0x230, bank);
		
		src = MK_FP(0xa000, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		src = MK_FP(0xa800, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		src = MK_FP(0xb000, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		src = MK_FP(0xb800, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
	}
	fclose(fp);
	
	fp = fopen("MZ65KNJ.ROM", "wb");
	for(i = 7; i >= 6; i--) {
		bank = inp(0x230);
		bank &= 0x1f;
		bank |= i << 5;
		outp(0x230, bank);
		
		src = MK_FP(0xa000, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		src = MK_FP(0xa800, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		src = MK_FP(0xb000, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		src = MK_FP(0xb800, 0);
		far_memcpy(buf, src, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
	}
	fclose(fp);
}
