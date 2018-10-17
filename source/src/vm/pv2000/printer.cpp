/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ printer ]
*/

#include "printer.h"

void PRINTER::initialize()
{
	busy = false;
}

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x80:
		// data
		out = data;
		break;
	case 0xa0:
		// ctrl #0 ?
		ctrl0 = data;
		break;
	case 0xb0:
		// ctrl #1 ?
		ctrl1 = data;
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	// bit7 = busy
	return busy ? 0xff : 0x7f;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void PRINTER::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT8(out);
	DECL_STATE_ENTRY_UINT8(ctrl0);
	DECL_STATE_ENTRY_UINT8(ctrl1);
	DECL_STATE_ENTRY_BOOL(busy);

	leave_decl_state();
}

void PRINTER::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint8(out);
//	state_fio->FputUint8(ctrl0);
//	state_fio->FputUint8(ctrl1);
//	state_fio->FputBool(busy);
}

bool PRINTER::load_state(FILEIO* state_fio)
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
//	out = state_fio->FgetUint8();
//	ctrl0 = state_fio->FgetUint8();
//	ctrl1 = state_fio->FgetUint8();
//	busy = state_fio->FgetBool();
	return true;
}

bool PRINTER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(out);
	state_fio->StateUint8(ctrl0);
	state_fio->StateUint8(ctrl1);
	state_fio->StateBool(busy);
	return true;
}
