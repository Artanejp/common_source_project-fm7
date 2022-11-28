/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'

	Author : Takeda.Toshiya
	Date   : 2015.12.14-

	[ memory ]
*/

// AUTO/STEPスイッチのために実装中のソース
// 実際に使用するかは未定

#include "memory.h"
#include "../i8080.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 9, eb = (e) >> 9; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x200 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x200 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	boot_mode = -1;
	
	memset(mon, 0xff, sizeof(mon));
	memset(bsmon, 0xff, sizeof(bsmon));
	memset(ext, 0xff, sizeof(ext));
	
	static const uint8_t top[3] = {0xc3, 0x00, 0xf0};
	static const uint8_t rst[3] = {0xc3, 0xdd, 0x83};
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("TK80.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(mon, sizeof(mon), 1);
		fio->Fclose();
	} else {
		// default
		memcpy(mon, top, sizeof(top));
		memcpy(mon + 0x38, rst, sizeof(rst));
	}
	if(fio->Fopen(create_local_path(_T("BSMON.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(bsmon, sizeof(bsmon), 1);
		fio->Fclose();
		// patch
		memcpy(mon + 0x38, rst, sizeof(rst));
	}
	if(fio->Fopen(create_local_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x0000, 0x07ff, wdmy, mon  );
	SET_BANK(0x0800, 0x0bff, wdmy, rdmy );
	SET_BANK(0x0c00, 0x7bff, wdmy, ext  );
	SET_BANK(0x7c00, 0x7dff, wdmy, rdmy ); // mmio
	SET_BANK(0x7e00, 0x7fff, vram, vram );
	SET_BANK(0x8000, 0xcfff, ram,  ram  );
	SET_BANK(0xd000, 0xefff, wdmy, basic);
	SET_BANK(0xf000, 0xffff, wdmy, bsmon);
}

void MEMORY::reset()
{
	// load basic rom
	if(boot_mode != config.boot_mode) {
		memset(basic, 0xff, sizeof(basic));
		FILEIO* fio = new FILEIO();
		if(config.boot_mode == 0) {
			if(fio->Fopen(create_local_path(_T("LV1BASIC.ROM")), FILEIO_READ_BINARY)) {
				fio->Fread(basic + 0x1000, 0x1000, 1);
				fio->Fclose();
			}
		} else {
			if(fio->Fopen(create_local_path(_T("LV2BASIC.ROM")), FILEIO_READ_BINARY)) {
				fio->Fread(basic, sizeof(basic), 1);
				fio->Fclose();
			}
		}
		delete fio;
		boot_mode = config.boot_mode;
		
		memset(ram, 0, sizeof(ram));
		memset(vram, 0x20, sizeof(vram));
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	switch(addr) {
	case 0x7df8:
	case 0x7df9:
		d_sio->write_io8(addr, data);
		break;
	case 0x7dfc:
	case 0x7dfd:
	case 0x7dfe:
	case 0x7dff:
		d_pio->write_io8(addr, data);
		break;
	}
	wbank[addr >> 9][addr & 0x1ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	switch(addr) {
	case 0x7df8:
	case 0x7df9:
		return d_sio->read_io8(addr, data);
	case 0x7dfc:
	case 0x7dfd:
	case 0x7dfe:
		return d_pio->read_io8(addr, data);
	}
	return rbank[addr >> 9][addr & 0x1ff];
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
	state_fio->StateArray(vram, sizeof(vram), 1);
	return true;
}

