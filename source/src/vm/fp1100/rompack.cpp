/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ rom pack ]
*/

#include "rompack.h"

void ROMPACK::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("ROMPACK.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
}

uint32_t ROMPACK::read_io8(uint32_t addr)
{
	if(addr < 0x8000) {
		return rom[addr];
	} else if(0xff00 <= addr && addr < 0xff80) {
		return 0x00; // device id
	}
	return 0xff;
}
