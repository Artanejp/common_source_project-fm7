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

#include "../../statesub.h"

void CMOS::decl_state()
{
	enter_decl_state(STATE_VERSION);

#ifdef _FMRCARD
	DECL_STATE_ENTRY_2D_ARRAY(cmos, 4, 0x800);
#else
	DECL_STATE_ENTRY_2D_ARRAY(cmos, 1, 0x800);
#endif
	DECL_STATE_ENTRY_BOOL(modified);
	DECL_STATE_ENTRY_UINT8(bank);

	leave_decl_state();
}

void CMOS::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}

//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(cmos, sizeof(cmos), 1);
//	state_fio->FputBool(modified);
//	state_fio->FputUint8(bank);
}

bool CMOS::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}

//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(cmos, sizeof(cmos), 1);
//	modified = state_fio->FgetBool();
//	bank = state_fio->FgetUint8();
	return true;
}

