/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[MSDOS ROM]
*/

#include "./towns_common.h"
#include "./msdosrom.h"

namespace FMTOWNS {
	
void MSDOSROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	FILEIO *fio;
	if(fio->Fopen(create_local_path(_T("FMT_DOS.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
}

uint32_t MSDOSROM::read_data8(uint32_t addr)	
{
	uint8_t d = 0xff;
	if((addr >= 0xc2000000) && (addr < 0xc2080000)) {
		d = rom[addr & 0x7ffff];
	}
	return (uint32_t)d;
}


}
