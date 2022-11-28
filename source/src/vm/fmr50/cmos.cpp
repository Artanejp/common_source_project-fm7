/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

	[ cmos ]
*/

#include "cmos.h"

void CMOS::initialize()
{
	// load cmos image
	memset(cmos, 0, sizeof(cmos));
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

void CMOS::reset()
{
	bank = 0;
}

void CMOS::write_io8(uint32_t addr, uint32_t data)
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

uint32_t CMOS::read_io8(uint32_t addr)
{
	if(!(addr & 1)) {
		return cmos[bank][(addr >> 1) & 0x7ff];
	}
	return 0xff;
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
	state_fio->StateArray(&cmos[0][0], sizeof(cmos), 1);
	state_fio->StateValue(modified);
	state_fio->StateValue(bank);
	return true;
}

