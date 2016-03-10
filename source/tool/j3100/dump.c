/*
	Dump J-3100 IPL/KANJI ROM
*/

#include <stdio.h>
#include <stdlib.h>
#include <farstr.h>
#include <dos.h>

void main()
{
	FILE* fp;
	void far *ptr_l;
	void far *ptr_h;
	int i;
	void *buf = malloc(0x8000);
	
	/* F0000-FFFFF: IPL */
	fp = fopen("IPL.ROM", "wb");
	
	ptr_l = MK_FP(0xf000, 0);
	ptr_h = MK_FP(0xf800, 0);
	
	far_memcpy(buf, ptr_l, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	far_memcpy(buf, ptr_h, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	
	fclose(fp);
	
	/* E0000-EFFFF: KANJI */
	fp = fopen("KANJI.ROM", "wb");
	
	ptr_l = MK_FP(0xe000, 0);
	ptr_h = MK_FP(0xe800, 0);
	
	for(i = 0; i < 16; i++) {
		char far *p = (char far *)ptr_l;
		*p = i | 0x80;
		
		far_memcpy(buf, ptr_l, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		far_memcpy(buf, ptr_h, 0x8000);
		fwrite(buf, 0x8000, 1, fp);
		
		*p = 0;
	}
	
	fclose(fp);
}
