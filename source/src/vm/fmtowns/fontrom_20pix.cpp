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
	memset(font_kanji20, sizeof(font_kanji20), 0xff);
	if(fio->Fopen(create_local_path(_T("FMT_F20.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_kanji20, sizeof(font_kanji20), 1);
		fio->Fclose();
	}
	delete fio;
}

uint32_t FONT_ROM_20PIX::read_data8(uint32_t addr)
{
	if((addr >= 0xc2180000) && (addr < 0xc2200000)) {
		return (uint32_t)(font_kanji20[addr & 0x7ffff]);
	}
	return 0xff;
}

}
