/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.14 -

	[ kanji rom ]
*/

#include "kanji.h"

void KANJI::initialize()
{
	// init image
	memset(kanji, 0xff, sizeof(kanji));
	
	// load kanji image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	delete fio;
	
	ptr = 0;
	strobe = false;
}

void KANJI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x80:
		ptr = (ptr & 0xff00) | data;
		break;
	case 0x81:
		ptr = (ptr & 0x00ff) | (data << 8);
		break;
	case 0x84:
		strobe = true;
		break;
	case 0x86:
		strobe = false;
		break;
	}
}

uint32_t KANJI::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x80:
		return kanji[(ptr << 1) | 0];
	case 0x81:
		return kanji[(ptr << 1) | 1];
	}
	return 0xff;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void KANJI::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT16(ptr);
	DECL_STATE_ENTRY_BOOL(strobe);
	
	leave_decl_state();
}
	
void KANJI::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint16(ptr);
//	state_fio->FputBool(strobe);
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
//	ptr = state_fio->FgetUint16();
//	strobe = state_fio->FgetBool();
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
	state_fio->StateUint16(ptr);
	state_fio->StateBool(strobe);
	return true;
}
