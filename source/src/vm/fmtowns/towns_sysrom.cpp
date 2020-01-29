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
		select_f8_rom = ((data & 0x02) == 0);
		select_f8_dictram = ((data & 0x01) != 0);
//		out_debug_log(_T("F8 ROM %s, F8 DICTRAM %s"), (select_f8_rom) ? _T("ON") : _T("OFF")
//					  ,(select_f8_dictram) ? _T("ON") : _T("OFF"));
		break;
	}
}
	
uint32_t SYSROM::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0480:
		return (0x00 | ((select_f8_dictram) ? 0x01 : 0x00) | ((select_f8_rom) ? 0x00 : 0x02));
		break;
	}
	return 0xff;
}
	
uint32_t SYSROM::read_memory_mapped_io8(uint32_t addr)
{
	uint8_t n_data = 0xff;
	if(addr < 0xfffc0000) { // Banked (from MSDOS/i86 compatible mode) 
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			if(select_f8_rom) { // BOOT ROM
				n_data = rom[(addr & 0x7fff) + 0x38000];
			} else { // RAM
				if((select_f8_dictram) && (addr < 0x000fa000)) {
					// OK?
					if(d_dict != NULL) {
						n_data = d_dict->read_memory_mapped_io8(0xc21400000 + (addr & 0x1fff));
						return n_data;
					}
				} else {
					n_data = ram[addr & 0x7fff];
				}
			}
		} else if((addr >= 0x000f0000) && (addr < 0x000f8000)) {
//			n_data = ram[addr & 0x7fff];
		}
	} else {
		n_data = rom[addr & 0x3ffff];
	}
	return (uint32_t)n_data;
}
	
uint32_t SYSROM::read_memory_mapped_io16(uint32_t addr)
{
	pair16_t nd;
	int dummy;
	nd.w = 0x00;
	// OK?
	nd.b.l = read_memory_mapped_io8((addr & 0xfffffffe) + 0);
	nd.b.h = read_memory_mapped_io8((addr & 0xfffffffe) + 1);
	return nd.w;
}

uint32_t SYSROM::read_memory_mapped_io32(uint32_t addr)
{
	pair32_t nd;
	nd.d = 0x00;
	// OK?
	nd.b.l  = read_memory_mapped_io8((addr & 0xfffffffc) + 0);
	nd.b.h  = read_memory_mapped_io8((addr & 0xfffffffc) + 1);
	nd.b.h2 = read_memory_mapped_io8((addr & 0xfffffffc) + 2);
	nd.b.h3 = read_memory_mapped_io8((addr & 0xfffffffc) + 3);
	return nd.d;
}
	
void SYSROM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if(addr < 0xfffc0000) {
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			// page 000F8xxxx : enable to write at all condition.
			if(select_f8_rom) {
				ram[addr & 0x7fff] = data;
				return;
			} else
			{
				// RAM
				if((select_f8_dictram) && (addr < 0x000fa000)) { // OK?
					if(d_dict != NULL) {
						d_dict->write_memory_mapped_io8(0xc21400000 + (addr & 0x1fff), data);
						return;
					}
				} else {
					ram[addr & 0x7fff] = data;
				}
			}
		}  else if((addr >= 0x000f0000) && (addr < 0x000f8000)) {
			//ram[addr & 0x7fff] = data;
		}
		return;
	}
	// ADDR >= 0xfffc0000
	return;
}


void SYSROM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	pair16_t nd;
	nd.w = (uint16_t)data;
	// OK?
	write_memory_mapped_io8((addr & 0xfffffffe) + 0, nd.b.l);
	write_memory_mapped_io8((addr & 0xfffffffe) + 1, nd.b.h);
}

void SYSROM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	pair32_t nd;
	nd.d = data;
	write_memory_mapped_io8((addr & 0xfffffffc) + 0, nd.b.l);
	write_memory_mapped_io8((addr & 0xfffffffc) + 1, nd.b.h);
	write_memory_mapped_io8((addr & 0xfffffffc) + 2, nd.b.h2);
	write_memory_mapped_io8((addr & 0xfffffffc) + 3, nd.b.h3);
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
	
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}	
}
