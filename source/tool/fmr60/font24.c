/*
	Dump FMR-60 FONT ROM
*/

#include <stdio.h>
#include <stdlib.h>
/*
#include <farstr.h>
#include <dos.h>
*/

void main()
{
	FILE* fp;
	int l, h, r, v0, v1;
	
	fp = fopen("ANK24.ROM", "wb");
	outp(0x400, 0);
	for(l = 0; l < 256; l++) {
		outpw(0x406, l);
		for(r = 0; r < 24; r++) {
			v0 = inpw(0x580 + 4 * r);
			fputc(v0 & 0xff, fp);
			v0 >>= 8;
			fputc(v0 & 0xff, fp);
		}
	}
	outp(0x400, 1);
	fclose(fp);
	
	fp = fopen("KANJI24.ROM", "wb");
	for(h = 128; h < 256; h++) {
		outp(0x400, 0);
		for(l = 0; l < 256; l++) {
			outpw(0x406, l | (h << 8));
			for(r = 0; r < 24; r++) {
				v0 = inpw(0x580 + 4 * r);
				v1 = inp(0x582 + 4 * r);
				fputc(v0 & 0xff, fp);
				v0 >>= 8;
				fputc(v0 & 0xff, fp);
				fputc(v1 & 0xff, fp);
			}
		}
		outp(0x400, 1);
		printf("%d / 128\n", h -127);
	}
	fclose(fp);
}
