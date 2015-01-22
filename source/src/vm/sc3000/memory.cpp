/*
	SEGA SC-3000 Emulator 'eSC-3000'

	Author : Takeda.Toshiya
	Date   : 2010.08.17-

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
	memset(cart, 0xff, sizeof(cart));
	memset(ipl, 0xff, sizeof(ipl));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("SF7000.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, ram + 0x0000, ipl);
	SET_BANK(0x2000, 0x3fff, ram + 0x2000, rdmy);
	SET_BANK(0x4000, 0xffff, ram + 0x4000, ram + 0x4000);
	
	inserted = false;
	ram_selected = false;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	// from PIO-P6
	ram_selected = ((data & mask) != 0);
	
	if(ram_selected) {
		SET_BANK(0x0000, 0x3fff, ram, ram);
	} else if(inserted) {
		SET_BANK(0x0000, 0x3fff, wdmy, cart);
	} else {
		SET_BANK(0x0000, 0x1fff, ram + 0x0000, ipl);
		SET_BANK(0x2000, 0x3fff, ram + 0x2000, rdmy);
	}
}

void MEMORY::open_cart(_TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(cart, 0xff, sizeof(cart));
		fio->Fread(cart, sizeof(cart), 1);
		fio->Fclose();
		inserted = true;
		ram_selected = false;
		
		// set memory map
		SET_BANK(0x0000, 0x7fff, wdmy, cart);
	}
	delete fio;
}

void MEMORY::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	inserted = false;
	ram_selected = false;
	
	// set memory map
	SET_BANK(0x0000, 0x1fff, ram + 0x0000, ipl);
	SET_BANK(0x2000, 0x3fff, ram + 0x2000, rdmy);
	SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
}

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->FputBool(inserted);
	state_fio->FputBool(ram_selected);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	inserted = state_fio->FgetBool();
	ram_selected = state_fio->FgetBool();
	
	// post process
	if(inserted) {
		SET_BANK(0x0000, 0x7fff, wdmy, cart);
	} else {
		SET_BANK(0x0000, 0x1fff, ram + 0x0000, ipl);
		SET_BANK(0x2000, 0x3fff, ram + 0x2000, rdmy);
		SET_BANK(0x4000, 0x7fff, ram + 0x4000, ram + 0x4000);
	}
	if(ram_selected) {
		SET_BANK(0x0000, 0x3fff, ram, ram);
	}
	return true;
}

