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
	return rom[addr & 0x3ffff];
}

uint32_t SYSROM::read_memory_mapped_io16(uint32_t addr)
{
	addr &= 0x3ffff;
	__UNLIKELY_IF(addr == 0x3ffff) return 0xffff; // BUS fault.
	pair16_t nd;
	nd.read_2bytes_le_from(&(rom[addr]));
	return nd.w;
}

uint32_t SYSROM::read_memory_mapped_io32(uint32_t addr)
{
	addr &= 0x3ffff;
	__UNLIKELY_IF(addr > 0x3fffc) return 0xffffffff; // BUS fault.
	pair32_t nd;
	nd.read_4bytes_le_from(&(rom[addr]));
	return nd.d;
}

void SYSROM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	// ADDR >= 0xfffc0000
	return;
}


void SYSROM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	// NOP
}

void SYSROM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	// NOP
}


}
