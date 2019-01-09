/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ system rom & RAM area 0x000f0000 - 0x000fffff]
*/

#include "./towns_common.h"
#include "./towns_sysrom.h"
#include "../../fileio.h"

namespace FMTOWNS {
void SYSROM::initialize()
{
	cmos_dirty = false;
	
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0x00, sizeof(ram));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_SYS.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	
	wait_val = 3;
	map_dos = true;
}


void SYSROM::reset()
{
	//wait_val = 6; // OK?
	//map_dos = true;
}

uint32_t SYSROM::read_data8(uint32_t addr)
{
	uint8_t n_data = 0xff;
	if(addr < 0xfffc0000) { // Banked (from MSDOS/i86 compatible mode) 
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			if(map_dos) {
				n_data = rom[(addr & 0x7fff) + 0x38000];
			} else {
				n_data = ram[addr & 0xffff];
			}
		} else if((addr >= 0x000f0000) && (addr < 0x00100000)) {
				n_data = ram[addr & 0xffff];
		}
	} else {
		n_data = rom[addr & 0x3ffff];
	}
	return (uint32_t)n_data;
}

void SYSROM::write_data8(uint32_t addr, uint32_t data)
{
	if(addr < 0xfffc0000) {
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			if(!(map_dos)) {
				ram[addr & 0xffff] = (uint8_t)data;
			}
		} else if((addr >= 0x000f0000) && (addr < 0x00100000)) {
			ram[addr & 0xffff] = (uint8_t)data;
		}			
	}
}

void SYSROM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_FMTOWNS_SYSROMSEL:
		map_dos = ((data & mask) == 0);
		break;
	case SIG_FMTOWNS_SET_MEMWAIT:
		wait_val = (int)data;
		break;
	}
}

uint32_t SYSROM::read_signal(int ch)
{
	switch(ch) {
	case SIG_FMTOWNS_SYSROMSEL:
		return ((map_dos) ? 0x00 : 0x02);
		break;
	case SIG_FMTOWNS_SET_MEMWAIT:
		return (uint32_t)wait_val;
		break;
	}
	return 0x00;
}
	
uint32_t SYSROM::read_data8w(uint32_t addr, int *wait)
{
	if(wait != NULL) *wait = wait_val;
	return read_data8(addr, data);
}

void SYSROM::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	if(wait != NULL) *wait = wait_val;
	write_data8(addr, data);
}

#define STATE_VERSION	1

bool SYSROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(wait_val);
	state_fio->StateValue(map_dos);
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}	
}
