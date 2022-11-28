/*
	COLECO ColecoVision Emulator 'yaCOLECOVISION'

	Author : tanam
	Date   : 2016.08.14-

	[ memory ]
*/

#include "memory.h"

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
	memset(cart, 0xff, sizeof(cart));
	memset(ipl, 0xff, sizeof(ipl));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("COLECO.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	SET_BANK(0x2000, 0x5fff, wdmy, rdmy);
	SET_BANK(0x6000, 0x7fff, ram,  ram);
	SET_BANK(0x8000, 0xffff, wdmy, cart);
	
	inserted = false;
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(cart, 0xff, sizeof(cart));
		fio->Fread(cart, sizeof(cart), 1);
		fio->Fclose();
		inserted = true;
		
		// set memory map
		SET_BANK(0x8000, 0xffff, wdmy, cart);
	}
	delete fio;
}

void MEMORY::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	inserted = false;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	SET_BANK(0x2000, 0x5fff, wdmy, rdmy);
	SET_BANK(0x6000, 0x7fff, ram,  ram);
	SET_BANK(0x8000, 0xffff, wdmy, cart);
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
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(inserted);
	
	// post process
	if(loading) {
		if(inserted) {
			SET_BANK(0x8000, 0xffff, wdmy, cart);
		} else {
			SET_BANK(0x0000, 0x1fff, wdmy, ipl);
			SET_BANK(0x2000, 0x5fff, wdmy, rdmy);
			SET_BANK(0x6000, 0x7fff, ram,  ram);
			SET_BANK(0x8000, 0xffff, wdmy, cart);
		}
	}
	return true;
}

