/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.13-

	[ memory ]
*/

#include "memory.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(bios, 0xff, sizeof(bios));
	memset(basic, 0xff, sizeof(basic));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("BIOS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bios, sizeof(bios), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	SET_BANK(0x0000, 0xbfff, ram, ram);
	SET_BANK(0xc000, 0xefff, ram + 0xc000, bios);
	SET_BANK(0xf000, 0xffff, ram + 0xf000, basic);
	amask = 0xc000;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr = (addr & 0xffff) | amask;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr = (addr & 0xffff) | amask;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	// $00: system control
	switch(data) {
	case 0:
		amask = 0xc000;
		break;
	case 1:
		amask = 0;
		break;
	case 2:
		SET_BANK(0xc000, 0xefff, ram + 0xc000, bios);
		SET_BANK(0xf000, 0xffff, ram + 0xf000, basic);
		break;
	case 3:
		SET_BANK(0xc000, 0xffff, ram + 0xc000, ram + 0xc000);
		break;
	}
}

