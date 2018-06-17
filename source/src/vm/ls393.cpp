/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ 74LS393 ]
*/

#include "ls393.h"

void LS393::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool signal = ((data & mask) != 0);
	if(prev_in && !signal) {
		uint32_t prev_count = count++;
		for(int i = 0; i < 8; i++) {
			if(outputs[i].count) {
				int bit = 1 << i;
				if((prev_count & bit) != (count & bit)) {
					uint32_t val = (count & bit) ? 0xffffffff : 0;
					write_signals(&outputs[i], val);
				}
			}
		}
	}
	prev_in = signal;
}

#define STATE_VERSION	1

#include "../statesub.h"

void LS393::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT32(count);
	DECL_STATE_ENTRY_BOOL(prev_in);

	leave_decl_state();
}
	
void LS393::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint32(count);
//	state_fio->FputBool(prev_in);
}

bool LS393::load_state(FILEIO* state_fio)
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
//	count = state_fio->FgetUint32();
//	prev_in = state_fio->FgetBool();
	return true;
}

