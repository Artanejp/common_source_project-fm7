/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[fonts]
*/

#include "../../fileio.h"
#include "./fontroms.h"

namespace FMTOWNS {
	
void FONT_ROMS::initialize()
{
	memset(font_kanji16, sizeof(font_kanji16), 0xff);
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_FNT.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_kanji16, sizeof(font_kanji16), 1);
		fio->Fclose();
	}

	delete fio;
}

uint32_t FONT_ROMS::read_memory_mapped_io8(uint32_t addr)
{
	if((addr >= 0xc2100000) && (addr < 0xc2140000)) {
		return (uint32_t)(font_kanji16[addr & 0x3ffff]);
	} else if((addr >= 0x000ca000) && (addr < 0x000ca800)) {
		return (uint32_t)(font_kanji16[0x1e800 + (addr & 0x7ff)]);
	} else if((addr >= 0x000cb000) && (addr < 0x000cc000)) {
		return (uint32_t)(font_kanji16[0x1f000 + (addr & 0x7ff)]);
	}
	return 0xff;
}

}
