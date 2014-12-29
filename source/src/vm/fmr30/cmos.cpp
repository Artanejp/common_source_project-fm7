/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ cmos ]
*/

#include "cmos.h"
#include "../../fileio.h"

void CMOS::initialize()
{
	// load cmos image
	memset(cmos, 0xff, sizeof(cmos));
	modified = false;
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("CMOS.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(cmos, sizeof(cmos), 1);
		fio->Fclose();
	}
	delete fio;
}

void CMOS::release()
{
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(emu->bios_path(_T("CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(cmos, sizeof(cmos), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void CMOS::write_io8(uint32 addr, uint32 data)
{
	if(cmos[addr & 0x1fff] != data) {
		cmos[addr & 0x1fff] = data;
		modified = true;
	}
}

uint32 CMOS::read_io8(uint32 addr)
{
	return cmos[addr & 0x1fff];
}

