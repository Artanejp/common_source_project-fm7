/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20-

	[ not gate ]
*/

#include "not.h"
#include "../fileio.h"

void NOT::write_signal(int id, uint32 data, uint32 mask)
{
	bool next = ((data & mask) == 0);
	if(prev != next || first) {
		write_signals(&outputs, next ? 0xffffffff : 0);
		prev = next;
		first = false;
	}
}

#define STATE_VERSION	1

void NOT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(prev);
	state_fio->FputBool(first);
}

bool NOT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	prev = state_fio->FgetBool();
	first = state_fio->FgetBool();
	return true;
}

