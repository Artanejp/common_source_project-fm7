/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ cmos ]
*/

#include "cmos.h"

void CMOS::initialize()
{
	// load cmos image
	memset(cmos, 0xff, sizeof(cmos));
	modified = false;
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("CMOS.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(cmos, sizeof(cmos), 1);
		fio->Fclose();
	}
	delete fio;
}

void CMOS::release()
{
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(cmos, sizeof(cmos), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void CMOS::write_io8(uint32_t addr, uint32_t data)
{
	if(cmos[addr & 0x1fff] != data) {
		cmos[addr & 0x1fff] = data;
		modified = true;
	}
}

uint32_t CMOS::read_io8(uint32_t addr)
{
	return cmos[addr & 0x1fff];
}

#define STATE_VERSION	1

bool CMOS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(cmos, sizeof(cmos), 1);
	state_fio->StateValue(modified);
	return true;
}

