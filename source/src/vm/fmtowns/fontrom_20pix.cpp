/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.11 -

	[20pixels fonts]
*/

#include "../../fileio.h"
#include "./fontrom_20pix.h"

namespace FMTOWNS {

void FONT_ROM_20PIX::initialize()
{
	FILEIO* fio = new FILEIO();
	memset(rom, sizeof(rom), 0xff);
	if(fio->Fopen(create_local_path(_T("FMT_F20.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
}

uint32_t FONT_ROM_20PIX::read_memory_mapped_io8(uint32_t addr)
{
	if(addr < 0x80000) {
		return (uint32_t)(rom[addr & 0x7ffff]);
	}
	return 0xff;
}

uint32_t FONT_ROM_20PIX::read_memory_mapped_io16(uint32_t addr)
{
	if(addr < (0x80000 - 1)) {
		pair16_t nd;
		nd.read_2bytes_le_from(&(rom[addr]));
		return nd.w;
	}
	return 0xffff;
}

uint32_t FONT_ROM_20PIX::read_memory_mapped_io32(uint32_t addr)
{
	if(addr < (0x80000 - 3)) {
		pair32_t nd;
		nd.read_4bytes_le_from(&(rom[addr]));
		return nd.d;
	}
	return 0xffffffff;
}

uint32_t FONT_ROM_20PIX::read_dma_data8w(uint32_t addr, int* wait)
{
	uint32_t val = read_memory_mapped_io8(addr); // OK?
	__LIKELY_IF(wait != NULL) {
		+wait = 0; // Discard WAIT VALUE(s) for DMA transfer.
	}
	return val;
}

uint32_t FONT_ROM_20PIX::read_dma_data16w(uint32_t addr, int* wait)
{
	uint32_t val = read_memory_mapped_io16(addr); // OK?
	__LIKELY_IF(wait != NULL) {
		+wait = 0; // Discard WAIT VALUE(s) for DMA transfer.
	}
	return val;
}

uint32_t FONT_ROM_20PIX::read_dma_data32w(uint32_t addr, int* wait)
{
	uint32_t val = read_memory_mapped_io32(addr); // OK?
	__LIKELY_IF(wait != NULL) {
		+wait = 0; // Discard WAIT VALUE(s) for DMA transfer.
	}
	return val;
}


}
