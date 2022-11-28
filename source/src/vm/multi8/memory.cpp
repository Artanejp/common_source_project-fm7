/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ memory ]
*/

#include "memory.h"
#include "../i8255.h"

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
	memset(rom, 0xff, sizeof(rom));
	memset(fdc, 0xff, sizeof(fdc));
	memset(ram0, 0, sizeof(ram0));
	memset(ram1, 0, sizeof(ram1));
	memset(vram, 0, sizeof(vram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FDC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(fdc, sizeof(fdc), 1);
		fio->Fclose();
		
		// 8255 Port A, bit1 = 0 (fdc rom exists)
		d_pio->write_signal(SIG_I8255_PORT_A, 0, 2);
	} else {
		// 8255 Port A, bit1 = 1 (fdc rom does not exist)
		d_pio->write_signal(SIG_I8255_PORT_A, 2, 2);
	}
	delete fio;
}

void MEMORY::reset()
{
	map1 = 0xf;
	map2 = 0;
	update_map();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if((addr & 0xc000) == 0x8000 && (map1 & 0x10)) {
		uint32_t ptr = addr & 0x3fff;
		// select vram
		if(!(map1 & 1)) {
			vram[0x0000 | ptr] = data;
		}
		if(!(map1 & 2)) {
			vram[0x4000 | ptr] = data;
		}
		if(!(map1 & 4)) {
			vram[0x8000 | ptr] = data;
		}
		if(!(map1 & 8)) {
			vram[0xc000 | ptr] = data;
		}
		return;
	}
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if((addr & 0xc000) == 0x8000 && (map1 & 0x10)) {
		uint32_t ptr = addr & 0x3fff;
		// select vram
		uint32_t val = 0xff;
		if(!(map1 & 1)) {
			val &= vram[0x0000 | ptr];
		}
		if(!(map1 & 2)) {
			val &= vram[0x4000 | ptr];
		}
		if(!(map1 & 4)) {
			val &= vram[0x8000 | ptr];
		}
		if(!(map1 & 8)) {
			val &= vram[0xc000 | ptr];
		}
		return val;
	}
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	map2 = data;
	update_map();
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_I8255_C) {
		map1 = data & mask;
		update_map();
	}
}

void MEMORY::update_map()
{
	if(map1 & 0x20) {
		SET_BANK(0x0000, 0x7fff, ram0, ram0);
		SET_BANK(0x8000, 0xffff, ram1, ram1);
	} else {
		SET_BANK(0x0000, 0x7fff, wdmy, rom);
		if(map2 & 1) {
			SET_BANK(0x6000, 0x6fff, wdmy, fdc);
		}
		SET_BANK(0x8000, 0xffff, ram1, ram1);
	}
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram0, sizeof(ram0), 1);
	state_fio->StateArray(ram1, sizeof(ram1), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(map1);
	state_fio->StateValue(map2);
	
	// post process
	if(loading) {
		update_map();
	}
	return true;
}

