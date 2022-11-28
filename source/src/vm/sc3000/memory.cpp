/*
	SEGA SC-3000 Emulator 'eSC-3000'

	Author : Takeda.Toshiya
	Date   : 2010.08.17-

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
//	memset(cart, 0xff, sizeof(cart));
	memset(ipl, 0xff, sizeof(ipl));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("SF7000.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	close_cart();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr >= 0xfffd) {
		if(bank[addr - 0xfffd] != 0xff) {
			bank[addr - 0xfffd] = data;
			update_bank();
		}
	}
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_SEL) {
		// from PIO-P6
		ram_selected = ((data & mask) != 0);
		update_bank();
	}
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(cart, 0xff, sizeof(cart));
		fio->Fread(cart, sizeof(cart), 1);
		if(fio->Ftell() > 0x8000) {
			// ホーム麻雀(32KB+16KB) or ロレッタの肖像(128KB)
			bank[0] = 0;
			bank[1] = 1;
			bank[2] = 2;
		} else {
			bank[0] = bank[1] = bank[2] = 0xff;
		}
		fio->Fclose();
		inserted = true;
		ram_selected = false;
		
		// set memory map
		update_bank();
	}
	delete fio;
}

void MEMORY::close_cart()
{
	memset(cart, 0xff, sizeof(cart));
	inserted = false;
	ram_selected = false;
	bank[0] = bank[1] = bank[2] = 0xff;
	
	// set memory map
	update_bank();
}

void MEMORY::update_bank()
{
	if(!inserted) {
		SET_BANK(0x0000, 0x1fff, ram + 0x0000, ipl);
		SET_BANK(0x2000, 0x3fff, ram + 0x2000, rdmy);
		SET_BANK(0x4000, 0xffff, ram + 0x4000, ram + 0x4000);
	} else {
		if(bank[0] == 0xff) {
			SET_BANK(0x0000, 0x3fff, wdmy, cart + 0x0000);
		} else {
			SET_BANK(0x0000, 0x3fff, wdmy, cart + 0x4000 * (bank[0] & 7));
		}
		if(bank[1] == 0xff) {
			SET_BANK(0x4000, 0x7fff, wdmy, cart + 0x4000);
		} else {
			SET_BANK(0x4000, 0x7fff, wdmy, cart + 0x4000 * (bank[1] & 7));
		}
		if(bank[2] == 0xff) {
			SET_BANK(0x8000, 0xbfff, ram + 0x8000, ram + 0x8000);
		} else {
			SET_BANK(0x8000, 0xbfff, wdmy, cart + 0x4000 * (bank[2] & 7));
		}
		SET_BANK(0xc000, 0xffff, ram + 0xc000, ram + 0xc000);
	}
	if(ram_selected) {
		SET_BANK(0x0000, 0x3fff, ram, ram);
	}
}

#define STATE_VERSION	2

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
	state_fio->StateValue(ram_selected);
	state_fio->StateArray(bank, sizeof(bank), 1);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

