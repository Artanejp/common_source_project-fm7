/*
	EPSON HC-40 Emulator 'eHC-40'

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

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
	memset(basic, 0xff, sizeof(basic));
	memset(util, 0xff, sizeof(util));
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
	if(fio->Fopen(create_local_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("UTIL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(util, sizeof(util), 1);
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
	SET_BANK(0x0000, 0xffff, ram, ram);
	
	switch(val & 0xf0) {
	// bank 0
	case 0x00:
		SET_BANK(0x0000, 0x7fff, wdmy, sys);
		break;
	// bank 1
	case 0x40:
		break;
	// bank 2
	case 0x80:
		SET_BANK(0xc000, 0xdfff, wdmy, basic);
		break;
	case 0x90:
		SET_BANK(0xa000, 0xbfff, wdmy, basic + 0x2000);
		SET_BANK(0xc000, 0xdfff, wdmy, basic);
		break;
	case 0xa0:
		SET_BANK(0x6000, 0x7fff, wdmy, basic + 0x6000);
		SET_BANK(0x8000, 0xdfff, wdmy, basic);
		break;
	// bank 3
	case 0xc0:
		SET_BANK(0xc000, 0xdfff, wdmy, util);
		break;
	case 0xd0:
		SET_BANK(0xa000, 0xbfff, wdmy, util + 0x2000);
		SET_BANK(0xc000, 0xdfff, wdmy, util);
		break;
	case 0xe0:
		SET_BANK(0x6000, 0x7fff, wdmy, util + 0x6000);
		SET_BANK(0x8000, 0xdfff, wdmy, util);
		break;
	}
	bank = val;
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
	state_fio->StateValue(bank);
	
	// post process
	if(loading) {
		set_bank(bank);
	}
	return true;
}

