/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ dictionary rom/ram & cmos & RAM area 0x000d0000 - 0x000dffff]
	* MEMORY :
	*   0x000d0000 - 0x000d7fff : DICTIONARY ROM (BANKED)
	*   0x000d8000 - 0x000d9fff : DICTIONARY RAM / GAIJI RAM
	*   0x000da000 - 0x000dffff : RESERVED
	*   0xc2080000 - 0xc20fffff : DICTIONARY ROM (NOT BANKED)
	*   0xc2140000 - 0xc2141fff : DICTIONARY RAM
	* I/O : 
	*   0x0484                         : DICTIONARY BANK (for 0xd0000 - 0xd7ffff)
	*   0x3000 - 0x3ffe (even address) : DICTIONARY RAM
*/

#include "./towns_common.h"
#include "./towns_dictionary.h"
#include "../../fileio.h"

namespace FMTOWNS {
void DICTIONARY::initialize()
{
	memset(dict_rom, 0xff, sizeof(dict_rom));
	memset(dict_ram, 0x00, sizeof(dict_ram));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}

	cmos_dirty = true;
	if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_READ_BINARY)) {
		if(fio->Fread(dict_ram, sizeof(dict_ram), 1) == 1) {
			cmos_dirty = false;
		}
		fio->Fclose();
	}
	delete fio;

	ram_wait_val = 6; // OK?
	rom_wait_val = 6; // OK?
}

void DICTIONARY::release()
{
	if(cmos_dirty) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(dict_ram, sizeof(dict_ram), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void DICTIONARY::reset()
{
	dict_bank = 0;
}

uint32_t DICTIONARY::read_data8w(uint32_t addr, int* wait)
{
	uint8_t n_data = 0xff;
	// 0xd0000 - 0xdffff : primary  is VRAM, secondary is DICTIONARY.
	if((addr < 0x000e0000) && (addr >= 0x000d0000)) {
		if(addr < 0xd8000) {
			if(wait != NULL) {
				*wait = rom_wait_val;
			}
			return dict_rom[(((uint32_t)dict_bank) << 15) | (addr & 0x7fff)];
		} else if(addr < 0xda000) {
			if(wait != NULL) {
				*wait = ram_wait_val;
			}
			return dict_ram[addr & 0x1fff];
		} else {
			if(wait != NULL) {
				*wait = 0;
			}
		}			
	} else if((addr >= 0xc20800000) && (addr < 0xc2100000)) {
		n_data = dict_rom[addr & 0x7ffff];
	} else if((addr >= 0xc21400000) && (addr < 0xc2142000)) {
		n_data = dict_ram[addr & 0x1fff];
	}
	return n_data;
}

void DICTIONARY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	if((addr < 0x000e0000) && (addr >= 0x000d0000)) {
		if(addr < 0xd8000) {
			if(wait != NULL) {
				*wait = rom_wait_val;
			}
			return;
		} else if(addr < 0xda000) {
			if(wait != NULL) {
				*wait = ram_wait_val;
			}
			dict_ram[addr & 0x1fff] = data;
			return;
		}
		// ToDo: address >= 0xda000
		if(wait != NULL) {
			*wait = ram_wait_val;
		}
		return;
	} else if((addr >= 0xc20800000) && (addr < 0xc2100000)) {
		if(wait != NULL) {
			*wait = rom_wait_val;
		}
		return;
	} else if((addr >= 0xc21400000) && (addr < 0xc2142000)) {
		dict_ram[addr & 0x1fff] = data;
		if(wait != NULL) {
			*wait = ram_wait_val;
		}
		return;
	}
	if(wait != NULL) {
		*wait = 0;
	}
}

uint32_t DICTIONARY::read_data16w(uint32_t addr, int* wait)
{
	pair16_t n;
	addr = addr & 0xfffffffe;
	if((addr >= 0xd8000) && (addr <= 0xe0000)) { // DICTRAM
		int tmpwait;
		int waitsum = 0;
		n.b.l = (uint8_t)read_data8w(addr + 0, &tmpwait);
		waitsum += tmpwait;
		n.b.h = (uint8_t)read_data8w(addr + 1, &tmpwait);
		waitsum += tmpwait;
		if(wait != NULL) {
			*wait = waitsum;
		}
		return n.w;
	}
	int dummy;
	n.b.l = (uint8_t)read_data8w(addr + 0, &dummy);
	n.b.h = (uint8_t)read_data8w(addr + 1, wait);
	return (uint32_t)(n.u16);
}

uint32_t DICTIONARY::read_data32w(uint32_t addr, int* wait)
{
	pair32_t n;
	addr = addr & 0xfffffffc;
	if((addr >= 0xd8000) && (addr <= 0xe0000)) { // DICTRAM
		int tmpwait;
		int waitsum = 0;
		n.b.l = (uint8_t)read_data8w(addr + 0, &tmpwait);
		waitsum += tmpwait;
		n.b.h = (uint8_t)read_data8w(addr + 1, &tmpwait);
		waitsum += tmpwait;
		n.b.h2 = (uint8_t)read_data8w(addr + 2, &tmpwait);
		waitsum += tmpwait;
		n.b.h3 = (uint8_t)read_data8w(addr + 3, &tmpwait);
		waitsum += tmpwait;
		if(wait != NULL) {
			*wait = waitsum;
		}
		return n.d;
	}
	int dummy;
	n.b.l  = (uint8_t)read_data8w(addr + 0, &dummy);
	n.b.h  = (uint8_t)read_data8w(addr + 1, &dummy);
	n.b.h2 = (uint8_t)read_data8w(addr + 2, &dummy);
	n.b.h3 = (uint8_t)read_data8w(addr + 3, wait);
	return n.d;
}

void DICTIONARY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	pair16_t n;
	addr = addr & 0xfffffffe;
	n.w = (uint16_t)data;
	if((addr >= 0xd8000) && (addr <= 0xe0000)) { // DICTRAM
		int tmpwait;
		int waitsum = 0;
		write_data8w(addr + 0, n.b.l, &tmpwait);
		waitsum += tmpwait;
		write_data8w(addr + 1, n.b.h, &tmpwait);
		waitsum += tmpwait;
		if(wait != NULL) {
			*wait = waitsum;
		}
		return;
	}		
	int dummy;
	write_data8w(addr + 0, n.b.l, &dummy);
	write_data8w(addr + 1, n.b.h, wait);
}

void DICTIONARY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	pair32_t n;
	addr = addr & 0xfffffffc;
	n.d = data;
	if((addr >= 0xd8000) && (addr <= 0xe0000)) { // DICTRAM
		int tmpwait;
		int waitsum = 0;
		write_data8w(addr + 0, n.b.l, &tmpwait);
		waitsum += tmpwait;
		write_data8w(addr + 1, n.b.h, &tmpwait);
		waitsum += tmpwait;
		write_data8w(addr + 2, n.b.h2, &tmpwait);
		waitsum += tmpwait;
		write_data8w(addr + 3, n.b.h3, &tmpwait);
		waitsum += tmpwait;
		if(wait != NULL) {
			*wait = waitsum;
		}
		return;
	}
	int dummy;
	write_data8w(addr + 0, n.b.l, &dummy);
	write_data8w(addr + 1, n.b.h, &dummy);
	write_data8w(addr + 2, n.b.h2, &dummy);
	write_data8w(addr + 3, n.b.h3, wait);
}


void DICTIONARY::write_io8(uint32_t addr, uint32_t data)
{
	if(addr == 0x0484) {
		dict_bank = data & 0x0f;
	} else if((addr >= 0x3000) && (addr < 0x4000)) {
		if((addr & 0x0001) == 0) { // OK?
			uint32_t naddr = (addr >> 1) & 0x7ff;
			cmos_dirty = true;
			dict_ram[naddr] = (uint8_t)data;
		}
	}
}

uint32_t DICTIONARY::read_io8(uint32_t addr)
{
	uint32_t data;
	if(addr == 0x0484) {
		data = dict_bank & 0x0f;
	} else if((addr >= 0x3000) && (addr < 0x4000)) {
		if((addr & 0x0001) == 0) { // OK?
			uint32_t naddr = (addr >> 1) & 0x7ff;
			data = dict_ram[naddr];
		} else {
			data = 0xff;
		}
	} else {
		data = 0xff;
	}
	return data;
}

void DICTIONARY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_FMTOWNS_DICT_BANK:
		dict_bank = (uint8_t)(data & 0x0f);
		break;
	case SIG_FMTOWNS_RAM_WAIT:
		ram_wait_val = data;
		break;
	case SIG_FMTOWNS_ROM_WAIT:
		rom_wait_val = data;
		break;
	}
}

uint32_t DICTIONARY::read_signal(int ch)
{
	switch(ch) {
	case SIG_FMTOWNS_DICT_BANK:
		return (uint32_t)(dict_bank & 0x0f);
		break;
	case SIG_FMTOWNS_RAM_WAIT:
		return ram_wait_val;
		break;
	case SIG_FMTOWNS_ROM_WAIT:
		return rom_wait_val;
		break;
	}
	return 0x00;
}
	
#define STATE_VERSION	1

bool DICTIONARY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(dict_bank);
	state_fio->StateArray(dict_ram, sizeof(dict_ram), 1);
	state_fio->StateValue(ram_wait_val);
	state_fio->StateValue(rom_wait_val);
	
	if(loading) {
		cmos_dirty = true;
	}/* else {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(dict_ram, sizeof(dict_ram), 1);
			fio->Fclose();
		}
		delete fio;
		cmos_dirty = false;
	} */
	return true;
}	
}
