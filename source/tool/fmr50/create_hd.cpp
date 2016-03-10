/*
	hdd image creater for eFMR-50
	
	Author : Takeda.Toshiya
	Date   : 2004.09.10 -
*/

#include <stdio.h>

// variable scope of 'for' loop (for microsoft visual c++ 6.0)
#define for if(0);else for
// type definition
typedef unsigned char uint8_t;

#define HDD_SIZE (40*1024*1024)

void main()
{
	FILE* fp = fopen("SCSI0.DAT", "wb");
	for(int i = 0; i < HDD_SIZE; i++)
		fputc(0, fp);
	fclose(fp);
}

