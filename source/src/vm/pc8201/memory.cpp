/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.03.31-

	[ memory ]
*/

#include "memory.h"
#include "cmt.h"
#include "../datarec.h"
#include "../upd1990a.h"

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
	memset(ipl, 0xff, sizeof(ipl));
	memset(ext, 0xff, sizeof(ext));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom / ram images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("RAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::release()
{
	// save ram image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	sio = bank = 0;
	update_bank();
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

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xf0) {
	case 0x90:
		// system control
		if((sio & 8) != (data & 8)) {
			d_cmt->write_signal(SIG_CMT_REMOTE, data, 8);
			d_drec->write_signal(SIG_DATAREC_REMOTE, data, 8);
		}
		if((sio & 0x10) != (data & 0x10)) {
			d_rtc->write_signal(SIG_UPD1990A_STB, data, 0x10);
		}
		sio = data;
		break;
	case 0xa0:
		// bank control
		bank = data;
		update_bank();
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	// $A0: bank status
	return (sio & 0xc0) | (bank & 0xf);
}

void MEMORY::update_bank()
{
	switch(bank & 3) {
	case 0:
		SET_BANK(0x0000, 0x7fff, wdmy, ipl);
		break;
	case 1:
		SET_BANK(0x0000, 0x7fff, wdmy, ext);
		break;
	case 2:
		SET_BANK(0x0000, 0x7fff, ram + 0x08000, ram + 0x08000);
		break;
	case 3:
		SET_BANK(0x0000, 0x7fff, ram + 0x10000, ram + 0x10000);
		break;
	}
	switch((bank >> 2) & 3) {
	case 0:
		SET_BANK(0x8000, 0xffff, ram + 0x00000, ram + 0x00000);
		break;
	case 1:
		SET_BANK(0x8000, 0xffff, wdmy, rdmy);
		break;
	case 2:
		SET_BANK(0x8000, 0xffff, ram + 0x08000, ram + 0x08000);
		break;
	case 3:
		SET_BANK(0x8000, 0xffff, ram + 0x10000, ram + 0x10000);
		break;
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
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(sio);
	state_fio->StateValue(bank);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

