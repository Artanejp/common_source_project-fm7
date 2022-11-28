/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.14-

	[ memory ]
*/

#include "memory.h"
#include "../i8080.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 10, eb = (e) >> 10; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x400 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x400 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	memset(mon, 0xff, sizeof(mon));
	memset(prom1, 0xff, sizeof(prom1));
	memset(prom2, 0xff, sizeof(prom2));
	memset(ram, 0, sizeof(ram));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MON.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(mon, sizeof(mon), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("PROM1.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(prom1, sizeof(prom1), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("PROM2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(prom2, sizeof(prom2), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x07ff, wdmy, mon );
	SET_BANK(0x0800, 0x0bff, wdmy, prom1);
	SET_BANK(0x0c00, 0x0fff, wdmy, prom2);
	SET_BANK(0x1000, 0x7fff, wdmy, rdmy);
	SET_BANK(0x8000, 0x87ff, ram,  ram );
	SET_BANK(0x8800, 0xffff, wdmy, rdmy);
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 10][addr & 0x3ff];
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	if((config.dipswitch & 1) && d_cpu->read_signal(SIG_I8080_INTE)) {
		d_cpu->write_signal(SIG_I8080_INTR, 1, 1);
	}
	*wait = 0;
	return read_data8(addr);
}

void MEMORY::load_binary(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::save_binary(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
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
	return true;
}

