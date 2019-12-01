/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ SYSTEM rom & RAM area 0x000f8000 - 0x000fffff]
	* MEMORY :
	*   0x000f8000 - 0x000fffff : RAM / DICTIONARY (BANKED)
	*   0xfffc0000 - 0xffffffff : SYSTEM ROM
	* I/O : 
	*   0x0480                         : F8 BANK
*/

#include "./towns_common.h"
#include "./towns_sysrom.h"
#include "../../fileio.h"

namespace FMTOWNS {
void SYSROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0x00, sizeof(ram));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_SYS.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}

	ram_wait_val = 6;
	rom_wait_val = 6;
}


void SYSROM::reset()
{
	select_f8_rom = true;
	select_f8_dictram = false;
}

void SYSROM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0480:
		select_f8_dictram = ((data & 0x02) != 0);
		select_f8_rom = ((data & 0x01) != 0);
		break;
	}
}
	
uint32_t SYSROM::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0480:
		return (0x00 | ((select_f8_dictram) ? 0x02 : 0x00) | ((select_f8_rom) ? 0x01 : 0x00));
		break;
	}
	return 0xff;
}
	
uint32_t SYSROM::read_data8w(uint32_t addr, int* wait)
{
	uint8_t n_data = 0xff;
	if(addr < 0xfffc0000) { // Banked (from MSDOS/i86 compatible mode) 
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			if(select_f8_rom) { // BOOT ROM
				n_data = rom[(addr & 0x7fff) + 0x38000];
				if(wait != NULL) {
					*wait = rom_wait_val;
				}
			} else { // RAM
				if((select_f8_dictram) && (addr < 0x000fa000)) {
					// OK?
					if(d_dict != NULL) {
						n_data = d_dict->read_data8w(0xc21400000 + (addr & 0x1fff), wait);
						return n_data;
					}
				} else {
					n_data = ram[(addr & 0x7fff) + 0x8000];
				}
				if(wait != NULL) {
					*wait = ram_wait_val;
				}
			}
			return n_data;
		} else {
			if(wait != NULL) {
				*wait = 0;
			}
		}
	} else {
		n_data = rom[addr & 0x3ffff];
		if(wait != NULL) {
			*wait = rom_wait_val;
		}
	}
	return (uint32_t)n_data;
}
	
uint32_t SYSROM::read_data16w(uint32_t addr, int* wait)
{
	pair16_t nd;
	int dummy;
	nd.w = 0x00;
	// OK?
	nd.b.l = read_data8w((addr & 0xfffffffe) + 0, &dummy);
	nd.b.h = read_data8w((addr & 0xfffffffe) + 1, wait);
	return nd.w;
}

uint32_t SYSROM::read_data32w(uint32_t addr, int* wait)
{
	pair32_t nd;
	int dummy;
	nd.d = 0x00;
	// OK?
	nd.b.l  = read_data8w((addr & 0xfffffffc) + 0, &dummy);
	nd.b.h  = read_data8w((addr & 0xfffffffc) + 1, &dummy);
	nd.b.h2 = read_data8w((addr & 0xfffffffc) + 2, &dummy);
	nd.b.h3 = read_data8w((addr & 0xfffffffc) + 3, wait);
	return nb.d;
}
	
void SYSROM::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	if(addr < 0xfffc0000) {
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			if(select_f8_rom) {
				if(wait != NULL) {
					*wait = rom_wait_val;
				}
				return;
			} else {
				// RAM
				if((select_f8_dictram) && (addr < 0x000fa000)) { // OK?
					if(d_dict != NULL) {
						d_dict->write_data8w(0xc21400000 + (addr & 0x1fff), data, wait);
						return;
					}
				} else {
					ram[addr & 0x7fff] = data;
				}
				if(wait != NULL) {
					*wait = ram_wait_val;
				}
			}
			return;
		}
		if(wait != NULL) {
			*wait = 0;
		}
		return;
	}
	// ADDR >= 0xfffc0000
	if(wait != NULL) {
		*wait = rom_wait_val;
	}
	return;
}


void SYSROM::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	pair16_t nd;
	int dummy;
	nd.w = (uint16_t)data;
	// OK?
	write_data8w((addr & 0xfffffffe) + 0, nb.b.l, &dummy);
	write_data8w((addr & 0xfffffffe) + 1, nb.b.h, wait);
}

void SYSROM::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	pair32_t nd;
	int dummy;
	nd.d = data;
	write_data8w((addr & 0xfffffffc) + 0, nb.b.l, &dummy);
	write_data8w((addr & 0xfffffffc) + 1, nb.b.h, &dummy);
	write_data8w((addr & 0xfffffffc) + 2, nb.b.h2, &dummy);
	write_data8w((addr & 0xfffffffc) + 3, nb.b.h3, wait);
}
	
void SYSROM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_FMTOWNS_SYSROMSEL:
		select_f8_rom = ((data & mask) == 0);
		break;
	case SIG_FMTOWNS_F8_DICTRAM:
		select_f8_dictram = ((data & mask) != 0);
		break;
	case SIG_FMTOWNS_RAM_WAIT:
		ram_wait_val = data;
		break;
	case SIG_FMTOWNS_ROM_WAIT:
		rom_wait_val = data;
		break;
	}
}

uint32_t SYSROM::read_signal(int ch)
{
	switch(ch) {
	case SIG_FMTOWNS_SYSROMSEL:
		return ((select_f8_rom) ? 0xffffffff : 0x00000000);
		break;
	case SIG_FMTOWNS_F8_DICTRAM:
		return ((select_f8_dictram) ? 0xffffffff : 0x00000000);
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

bool SYSROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(select_f8_rom);
	state_fio->StateValue(select_f8_dictram);
	state_fio->StateValue(rom_wait_val);
	state_fio->StateValue(ram_wait_val);
	
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}	
}
