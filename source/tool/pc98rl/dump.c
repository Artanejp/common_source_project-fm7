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

void kanji(unsigned char hi, unsigned char lo)
{
	union REGS in_reg, out_reg;
	struct SREGS seg_reg;
	
	seg_reg.ds  = FP_SEG((void far *)buf);
	in_reg.x.bx = FP_OFF((void far *)buf);
	in_reg.h.dh = hi;
	in_reg.h.dl = lo;
	in_reg.h.ah = 0x1f;
	int86x(0x18, &in_reg, &out_reg, &seg_reg);
}

void main()
{
	FILE* fp;
	int hi, lo;
	
	buf = malloc(0x8000);
	
	fp = fopen("KANJI.ROM", "wb");
	for(lo = 0x00; lo <= 0xff; lo++) {
		kanji(0, (unsigned char)lo);
		fwrite(buf, 48, 1, fp);
	}
	for(hi = 0x21; hi <= 0x27; hi++) {
		for(lo = 0x21; lo <= 0x7e; lo++) {
			kanji((unsigned char)hi, (unsigned char)lo);
			fwrite(buf, 72, 1, fp);
		}
	}
	for(hi = 0x30; hi <= 0x73; hi++) {
		for(lo = 0x21; lo <= 0x7e; lo++) {
			kanji((unsigned char)hi, (unsigned char)lo);
			fwrite(buf, 72, 1, fp);
		}
	}
	for(hi = 0x79; hi <= 0x7c; hi++) {
		for(lo = 0x21; lo <= 0x7e; lo++) {
			kanji((unsigned char)hi, (unsigned char)lo);
			fwrite(buf, 72, 1, fp);
		}
	}
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
