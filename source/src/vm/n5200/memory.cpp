/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2009.06.03-

	[ memory ]
*/

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(exram, 0, sizeof(exram));
	memset(vram, 0, sizeof(vram));
	memset(tvram, 0, sizeof(tvram));
	memset(backup, 0, sizeof(backup));
	memset(ipl, 0xff, sizeof(ipl));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
		for(int i = 0xa8e; i < 0xafc; i++) {
			ipl[i] = 0x90;
		}
	}
	delete fio;
}

void MEMORY::release()
{
	// save ram image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("TVRAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(tvram, sizeof(tvram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	SET_BANK(0x000000, 0xffffff, wdmy, rdmy);
	SET_BANK(0x000000, 0x0bffff, ram, ram);
	SET_BANK(0x0c0000, 0x0dffff, vram, vram);	// ???
	SET_BANK(0x0e0000, 0x0e77ff, tvram, tvram);
	SET_BANK(0x0e7800, 0x0effff, backup, backup);
	SET_BANK(0x0f0000, 0x0fffff, wdmy, ipl);
	SET_BANK(0x100000, 0x1fffff, exram, exram);
	SET_BANK(0xff0000, 0xffffff, wdmy, ipl);
	
	protect = true;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffffff;
	if(0xe7800 <= addr && addr < 0xf0000 && protect) {
		return;
	}
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x74:
		protect = ((data & 1) != 0);
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	return 0xff;
}

