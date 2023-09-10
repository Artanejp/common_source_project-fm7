/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[MSDOS ROM]
*/
#include "../../fileio.h"
#include "./msdosrom.h"

namespace FMTOWNS {

void MSDOSROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	FILEIO *fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_DOS.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
}

uint32_t MSDOSROM::read_dma_data8w(uint32_t addr, int* wait)
{
	uint32_t val = read_memory_mapped_io8(addr); // OK?
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard WAIT VALUE(s) for DMA transfer.
	}
	return val;
}

uint32_t MSDOSROM::read_dma_data16w(uint32_t addr, int* wait)
{
	uint32_t val = read_memory_mapped_io16(addr); // OK?
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard WAIT VALUE(s) for DMA transfer.
	}
	return val;
}

uint32_t MSDOSROM::read_dma_data32w(uint32_t addr, int* wait)
{
	uint32_t val = read_memory_mapped_io32(addr); // OK?
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard WAIT VALUE(s) for DMA transfer.
	}
	return val;
}

uint32_t MSDOSROM::read_memory_mapped_io8(uint32_t addr)
{
	__LIKELY_IF(addr < 0x80000) {
		return rom[addr];
	}
	return 0xff;
}

uint32_t MSDOSROM::read_memory_mapped_io16(uint32_t addr)
{
	__LIKELY_IF(addr < (0x80000 - 1)) {
		pair16_t nd;
		nd.read_2bytes_le_from(&(rom[addr]));
		return nd.w;
	}
	return 0xffff;
}

uint32_t MSDOSROM::read_memory_mapped_io32(uint32_t addr)
{
	__LIKELY_IF(addr < (0x80000 - 3)) {
		pair32_t nd;
		nd.read_4bytes_le_from(&(rom[addr]));
		return nd.d;
	}
	return 0xffffffff;
}


}
