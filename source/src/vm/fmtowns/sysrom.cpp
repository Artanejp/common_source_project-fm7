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
#include "./sysrom.h"
#include "./dictionary.h"
#include "../../fileio.h"

namespace FMTOWNS {
void SYSROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_SYS.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}

}


void SYSROM::reset()
{
}
	
uint32_t SYSROM::read_memory_mapped_io8(uint32_t addr)
{
	uint8_t n_data = 0xff;
	if(addr < 0xfffc0000) { // Banked (from MSDOS/i86 compatible mode)
		if((addr >= 0x000f8000) && (addr < 0x00100000)) {
			n_data = rom[(addr & 0x7fff) + 0x38000];
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
	nd.b.l = read_memory_mapped_io8(addr + 0);
	nd.b.h = read_memory_mapped_io8(addr + 1);
	return nd.w;
}

uint32_t SYSROM::read_memory_mapped_io32(uint32_t addr)
{
	pair32_t nd;
	nd.d = 0x00;
	// OK?
	nd.b.l  = read_memory_mapped_io8(addr + 0);
	nd.b.h  = read_memory_mapped_io8(addr + 1);
	nd.b.h2 = read_memory_mapped_io8(addr + 2);
	nd.b.h3 = read_memory_mapped_io8(addr + 3);
	return nd.d;
}
	
void SYSROM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	// ADDR >= 0xfffc0000
	return;
}


void SYSROM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	pair16_t nd;
	nd.w = (uint16_t)data;
	// OK?
	write_memory_mapped_io8(addr + 0, nd.b.l);
	write_memory_mapped_io8(addr + 1, nd.b.h);
}

void SYSROM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	pair32_t nd;
	nd.d = data;
	write_memory_mapped_io8(addr + 0, nd.b.l);
	write_memory_mapped_io8(addr + 1, nd.b.h);
	write_memory_mapped_io8(addr + 2, nd.b.h2);
	write_memory_mapped_io8(addr + 3, nd.b.h3);
}
	

}
