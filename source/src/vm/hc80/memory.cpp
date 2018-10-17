/*
	EPSON HC-80 Emulator 'eHC-80'

	Author : Takeda.Toshiya
	Date   : 2008.03.14 -

	[ memory ]
*/

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 13, eb = (e) >> 13; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x2000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x2000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// initialize memory
	memset(ram, 0, sizeof(ram));
	memset(sys, 0xff, sizeof(sys));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load backuped ram / rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("DRAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("SYS.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(sys, sizeof(sys), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::release()
{
	// save battery backuped ram
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("DRAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	set_bank(0);
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[(addr >> 13) & 7][addr & 0x1fff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[(addr >> 13) & 7][addr & 0x1fff];
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	set_bank(data);
}

void MEMORY::set_bank(uint32_t val)
{
	if(val & 1) {
		SET_BANK(0x0000, 0xffff, ram, ram);
	} else {
		SET_BANK(0x0000, 0x7fff, wdmy, sys);
		SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
	}
	bank = val;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void MEMORY::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_1D_ARRAY(ram, sizeof(ram));
	DECL_STATE_ENTRY_UINT8(bank);
	
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
//	state_fio->FputUint8(bank);
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
//	bank = state_fio->FgetUint8();
	
	// post process
	set_bank(bank);
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
	state_fio->StateUint8(bank);
	
	// post process
	if(loading) {
		set_bank(bank);
	}
	return true;
}
