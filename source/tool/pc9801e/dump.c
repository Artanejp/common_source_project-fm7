/*
	Dump PC-9801E/F/M IPL ROM
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
	
	/* CC000-CDFFF: SOUND */
	fp = fopen("SOUND.ROM", "wb");
	
	ptr = MK_FP(0xcc00, 0);
	far_memcpy(buf, ptr, 0x2000);
	fwrite(buf, 0x2000, 1, fp);
	
	fclose(fp);
	
	/* D6000-D6FFF: 2DDIF */
	fp = fopen("2DDIF.ROM", "wb");
	
	ptr = MK_FP(0xd600, 0);
	far_memcpy(buf, ptr, 0x1000);
	fwrite(buf, 0x1000, 1, fp);
	
	fclose(fp);
	
	/* D7000-D7FFF: 2HDIF */
	fp = fopen("2HDIF.ROM", "wb");
	
	ptr = MK_FP(0xd700, 0);
	far_memcpy(buf, ptr, 0x1000);
	fwrite(buf, 0x1000, 1, fp);
	
	fclose(fp);
	
	/* E8000-FFFFF: IPL */
	fp = fopen("IPL.ROM", "wb");
	
	ptr = MK_FP(0xe800, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	
	ptr = MK_FP(0xf000, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	
	ptr = MK_FP(0xf800, 0);
	far_memcpy(buf, ptr, 0x8000);
	fwrite(buf, 0x8000, 1, fp);
	
	fclose(fp);
}
