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
	memset(font_data, 0xff, sizeof(font_data));
	if(fio->Fopen(create_local_path(_T("FMT_F20.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_data, sizeof(font_data), 1);
		fio->Fclose();
	}
	delete fio;
}

uint32_t FONT_ROM_20PIX::read_memory_mapped_io8(uint32_t addr)
{
	if(addr < 0x80000) {
		return (uint32_t)(font_data[addr & 0x7ffff]);
	}
	return 0xff;
}



}
