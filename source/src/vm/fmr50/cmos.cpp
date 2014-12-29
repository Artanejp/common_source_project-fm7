/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

	[ cmos ]
*/

#include "cmos.h"
#include "../../fileio.h"

void CMOS::initialize()
{
	// load cmos image
	memset(cmos, 0, sizeof(cmos));
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

void CMOS::reset()
{
	bank = 0;
}

void CMOS::write_io8(uint32 addr, uint32 data)
{
	switch(addr) {
	case 0x90:
		bank = data & 3;
		break;
	default:
		if(!(addr & 1)) {
			if(cmos[bank][(addr >> 1) & 0x7ff] != data) {
				cmos[bank][(addr >> 1) & 0x7ff] = data;
				modified = true;
			}
		}
		break;
	}
}

uint32 CMOS::read_io8(uint32 addr)
{
	if(!(addr & 1)) {
		return cmos[bank][(addr >> 1) & 0x7ff];
	}
	return 0xff;
}

