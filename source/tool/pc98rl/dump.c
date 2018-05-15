/*
	Dump PC-98RL BIOS/KANJI ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <farstr.h>
#include <dos.h>
#include <memory.h>
#include <machine.h>

void *buf;

void kanji(unsigned char first, unsigned char second)
{
	union REGS in_reg, out_reg;
	struct SREGS seg_reg;
	
	seg_reg.ds  = FP_SEG((void far *)buf);
	in_reg.x.bx = FP_OFF((void far *)buf);
	in_reg.h.dh = first;
	in_reg.h.dl = second;
	in_reg.h.ah = 0x1f;
	int86x(0x18, &in_reg, &out_reg, &seg_reg);
}

void main()
{
	FILE* fp;
	int first, second;
	
	buf = malloc(0x8000);
	
	fp = fopen("FONT24.ROM", "wb");
	for(second = 0x00; second <= 0xff; second++) {
		kanji(0, (unsigned char)second);
		fwrite(buf, 48, 1, fp);
	}
#if 1
	for(first = 0x21; first <= 0x7c; first++) {
		for(second = 0x21; second <= 0x7e; second++) {
			kanji((unsigned char)first, (unsigned char)second);
			fwrite(buf, 72, 1, fp);
		}
	}
#else
	for(first = 0x21; first <= 0x27; first++) {
		for(second = 0x21; second <= 0x7e; second++) {
			kanji((unsigned char)first, (unsigned char)second);
			fwrite(buf, 72, 1, fp);
		}
	}
	for(first = 0x30; first <= 0x73; first++) {
		for(second = 0x21; second <= 0x7e; second++) {
			kanji((unsigned char)first, (unsigned char)second);
			fwrite(buf, 72, 1, fp);
		}
	}
	for(first = 0x79; first <= 0x7c; first++) {
		for(second = 0x21; second <= 0x7e; second++) {
			kanji((unsigned char)first, (unsigned char)second);
			fwrite(buf, 72, 1, fp);
		}
	}
#endif
	fclose(fp);
	
	fp = fopen("BIOS.ROM", "wb");
	far_memcpy(buf, MK_FP(0xf000, 0), 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	far_memcpy(buf, MK_FP(0xf800, 0), 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	fclose(fp);
}

/*
0	00-ff	48	12288
21-73	21-7e	72	561744
79-7c	21-7e	72	27072
*/
