/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20-

	[ not gate ]
*/

#include "not.h"

void NOT::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool next = ((data & mask) == 0);
	if(prev != next || first) {
		write_signals(&outputs, next ? 0xffffffff : 0);
		prev = next;
		first = false;
	}
}

#define STATE_VERSION	1

bool NOT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(prev);
	state_fio->StateValue(first);
	return true;
}

