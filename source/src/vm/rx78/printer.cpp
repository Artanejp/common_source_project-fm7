/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ printer ]
*/

#include "printer.h"

void PRINTER::initialize()
{
	busy = strobe = false;
}

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xe2:
		strobe = ((data & 0x80) != 0);
		break;
	case 0xe3:
		out = data;
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xe2:
		return busy ? 1 : 0;
	}
	return 0xff;
}

void PRINTER::write_signal(int id, uint32_t data, uint32_t mask)
{
//	if(id == SIG_PRINTER_BUSY) {
//		busy = ((data & mask) != 0);
//	}
}

#define STATE_VERSION	1

#include "../../statesub.h"

void PRINTER::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_BOOL(strobe);
	DECL_STATE_ENTRY_BOOL(busy);
	DECL_STATE_ENTRY_UINT8(out);
	
	leave_decl_state();
}

void PRINTER::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputBool(strobe);
//	state_fio->FputBool(busy);
//	state_fio->FputUint8(out);
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
//	strobe = state_fio->FgetBool();
//	busy = state_fio->FgetBool();
//	out = state_fio->FgetUint8();
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
	state_fio->StateBool(strobe);
	state_fio->StateBool(busy);
	state_fio->StateUint8(out);
	return true;
}
