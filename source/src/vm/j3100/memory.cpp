/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

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
	memset(vram, 0, sizeof(vram));
	memset(ems, 0, sizeof(ems));
	memset(kanji, 0xff, sizeof(kanji));
	memset(ipl, 0xff, sizeof(ipl));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x000000, 0x09ffff, ram, ram);
	SET_BANK(0x0a0000, 0x0b7fff, wdmy, rdmy);
	SET_BANK(0x0b8000, 0x0bffff, vram, vram);
	SET_BANK(0x0c0000, 0x0effff, wdmy, rdmy);
	SET_BANK(0x0f0000, 0x0fffff, wdmy, ipl);
	SET_BANK(0x100000, 0xfeffff, wdmy, rdmy);
	SET_BANK(0xff0000, 0xffffff, wdmy, ipl);
	
	kanji_bank = 0;
	memset(ems_page, 0, sizeof(ems_page));
}

void MEMORY::reset()
{
	
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	if((addr & 0xff0000) == 0x0e0000) {
		if(kanji_bank != (data & 0x8f)) {
			if(data & 0x80) {
				SET_BANK(0xe0000, 0xeffff, wdmy, kanji + 0x10000 * (data & 0x0f));
			} else {
				SET_BANK(0xe0000, 0xeffff, wdmy, rdmy);
			}
			kanji_bank = data & 0x8f;
		}
		return;
	}
	addr &= 0xffffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

static const int sets[8] = {
	0, 1, -1, -1, -1, 2, 3, -1
};

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	int set = sets[(addr >> 4) & 7];
	int page = (addr >> 14) & 3;
	
	switch(addr & 0xffff) {
	case 0x208: case 0x4208: case 0x8208: case 0xc208:
	case 0x218: case 0x4218: case 0x8218: case 0xc218:
	case 0x258: case 0x4258: case 0x8258: case 0xc258:
	case 0x268: case 0x4268: case 0x8268: case 0xc268:
		if(set != -1 && ems_page[set][page] != data) {
			ems_page[set][page] = data;
			update_ems(page);
		}
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	int set = sets[(addr >> 4) & 7];
	int page = (addr >> 14) & 3;
	
	switch(addr & 0xffff) {
	case 0x208: case 0x4208: case 0x8208: case 0xc208:
	case 0x218: case 0x4218: case 0x8218: case 0xc218:
	case 0x258: case 0x4258: case 0x8258: case 0xc258:
	case 0x268: case 0x4268: case 0x8268: case 0xc268:
		if(set != -1) {
			return ems_page[set][page];
		}
		break;
	}
	return 0xff;
}

void MEMORY::update_ems(int page)
{
	uint32_t start_addr = 0xd0000 + 0x4000 * page;
	uint32_t end_addr = start_addr + 0x3fff;
	
	for(int set = 0; set < 4; set++) {
		if(ems_page[set][page] & 0x80) {
			int ofs = set * 128 + (ems_page[set][page] & 127);
			SET_BANK(start_addr, end_addr, ems + 0x4000 * ofs, ems + 0x4000 * ofs);
			return;
		}
	}
	SET_BANK(start_addr, end_addr, wdmy, rdmy);
}

