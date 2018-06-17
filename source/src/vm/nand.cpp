/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ nand gate ]
*/

#include "nand.h"

void NAND::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(data & mask) {
		bits_in |= id;
	} else {
		bits_in &= ~id;
	}
	bool next = (bits_mask != bits_in);
	if(prev != next || first) {
		write_signals(&outputs, next ? 0xffffffff : 0);
		prev = next;
		first = false;
	}
}

#define STATE_VERSION	1

#include "../statesub.h"

void NAND::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT32(bits_mask);
	DECL_STATE_ENTRY_UINT32(bits_in);
	DECL_STATE_ENTRY_BOOL(prev);
	DECL_STATE_ENTRY_BOOL(first);

	leave_decl_state();
}
void NAND::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint32(bits_mask);
//	state_fio->FputUint32(bits_in);
//	state_fio->FputBool(prev);
//	state_fio->FputBool(first);
}

bool NAND::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
	
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	bits_mask = state_fio->FgetUint32();
//	bits_in = state_fio->FgetUint32();
//	prev = state_fio->FgetBool();
//	first = state_fio->FgetBool();
	return true;
}

