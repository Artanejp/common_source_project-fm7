/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ kanji rom ]
*/

#include "kanji.h"
#include "../i8255.h"

void KANJI::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		
		// 8255 Port A, bit6 = 0 (kanji rom exists)
		d_pio->write_signal(SIG_I8255_PORT_A, 0, 0x40);
	} else {
		// 8255 Port A, bit6 = 1 (kanji rom does not exist)
		d_pio->write_signal(SIG_I8255_PORT_A, 0x40, 0x40);
	}
	delete fio;
}

void KANJI::reset()
{
	ptr = 0;
}

void KANJI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x40:
		ptr = (ptr & 0xff00) | data;
		break;
	case 0x41:
		ptr = (ptr & 0x00ff) | (data << 8);
		break;
	}
}

uint32_t KANJI::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x40:
		return rom[(ptr << 1) | 0];
	case 0x41:
		return rom[(ptr << 1) | 1];
	}
	return 0xff;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void KANJI::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT32(ptr);

	leave_decl_state();
}

void KANJI::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
	
//	state_fio->FputUint32(ptr);
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
//	ptr = state_fio->FgetUint32();
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
	state_fio->StateUint32(ptr);
	return true;
}
