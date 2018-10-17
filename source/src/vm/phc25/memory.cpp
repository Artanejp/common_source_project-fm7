/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

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
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
#ifdef _MAP1010
	SET_BANK(0x0000, 0x5fff, wdmy, rom );
	SET_BANK(0x6000, 0x77ff, vram, vram);
	SET_BANK(0x7800, 0x7fff, wdmy, rdmy);
	SET_BANK(0x8000, 0xffff, ram,  ram );
#else
	SET_BANK(0x0000, 0x5fff, wdmy, rom );
	SET_BANK(0x6000, 0x77ff, vram, vram);
	SET_BANK(0x7800, 0xbfff, wdmy, rdmy);
	SET_BANK(0xc000, 0xffff, ram,  ram );
#endif
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
#ifdef _MAP1010
	if(0x7800 <= addr && addr < 0x8000) {
		// memory mapped i/o
		return;
	}
#endif
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
#ifdef _MAP1010
	if(0x7800 <= addr && addr < 0x7860) {
		return d_kbd->read_io8(addr);
	} else if(0x7860 <= addr && addr < 0x8000) {
		// memory mapped i/o
		return 0xff;
	}
#endif
	return rbank[addr >> 11][addr & 0x7ff];
}

#define STATE_VERSION	1

#include "../../statesub.h"

void MEMORY::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_1D_ARRAY(ram, sizeof(ram));
	DECL_STATE_ENTRY_1D_ARRAY(vram, sizeof(vram));
	
	leave_decl_state();
}

void MEMORY::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(ram, sizeof(ram), 1);
//	state_fio->Fwrite(vram, sizeof(vram), 1);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(ram, sizeof(ram), 1);
//	state_fio->Fread(vram, sizeof(vram), 1);
	return true;
}

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateBuffer(ram, sizeof(ram), 1);
	state_fio->StateBuffer(vram, sizeof(vram), 1);
	return true;
}
