/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2014.12.26-

	[ kanji/dictionary rom ]
*/

#include "kanji.h"

void KANJI::initialize()
{
	memset(kanji, 0xff, sizeof(kanji));
	memset(dic, 0xff, sizeof(dic));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("MZ1R23.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("DICT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("MZ1R24.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
	}
	delete fio;
}

void KANJI::reset()
{
	control_reg = kanji_addr = dic_addr = 0;
}

void KANJI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xb8:
		control_reg = data;
		break;
	case 0xb9:
		dic_addr = (addr & 0xff00) | data;
		kanji_addr = dic_addr << 5;
		break;
	}
}

uint32_t KANJI::read_io8(uint32_t addr)
{
	uint32_t val;
	
	switch(addr & 0xff) {
	case 0xb9:
		if(control_reg & 0x80) {
			val = kanji[(kanji_addr++) & 0x1ffff];
		} else {
			val = dic[((control_reg & 3) << 16) | ((dic_addr++) & 0xffff)];
		}
		if(control_reg & 0x40) {
			val = ((val & 0x80) >> 7) | ((val & 0x40) >> 5) | ((val & 0x20) >> 3) | ((val & 0x10) >> 1) | ((val & 8) << 1) | ((val & 4) << 3) | ((val & 2) << 5) | ((val & 1) << 7);
		}
		return val;
	}
	return 0xff;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void KANJI::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT32(control_reg);
	DECL_STATE_ENTRY_UINT32(kanji_addr);
	DECL_STATE_ENTRY_UINT32(dic_addr);
	
	leave_decl_state();
}

void KANJI::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint32(control_reg);
//	state_fio->FputUint32(kanji_addr);
//	state_fio->FputUint32(dic_addr);
}

bool KANJI::load_state(FILEIO* state_fio)
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
//	control_reg = state_fio->FgetUint32();
//	kanji_addr = state_fio->FgetUint32();
//	dic_addr = state_fio->FgetUint32();
	return true;
}

bool KANJI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint32(control_reg);
	state_fio->StateUint32(kanji_addr);
	state_fio->StateUint32(dic_addr);
	return true;
}
