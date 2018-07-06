/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ kanji rom pac 2 ]
*/

#include "kanjipac2.h"

void KANJIPAC2::initialize(int id)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
}

void KANJIPAC2::reset()
{
	ptr = 0;
}

void KANJIPAC2::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x18:
		ptr = (ptr & 0x1ff00) | data;
		break;
	case 0x19:
		ptr = (ptr & 0x100ff) | (data << 8);
		break;
	case 0x1a:
		ptr = (ptr & 0x0ffff) | ((data & 0x01) << 16);
		break;
	}
}

uint32_t KANJIPAC2::read_io8(uint32_t addr)
{
	return rom[ptr & 0x1ffff];
}

#define STATE_VERSION	1

#include "../../statesub.h"

void KANJIPAC2::decl_state()
{
	state_entry = new csp_state_utils(STATE_VERSION, 0, (const _TCHAR *)_T("PAC2SLOT::KANJIPAC2"), NULL);

	DECL_STATE_ENTRY_UINT32(ptr);
	
	//leave_decl_state();
}

void KANJIPAC2::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
	
//	state_fio->FputUint32(ptr);
}

bool KANJIPAC2::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	ptr = state_fio->FgetUint32();
	return true;
}

