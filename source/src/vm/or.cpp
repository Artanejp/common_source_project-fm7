/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ or gate ]
*/

#include "or.h"

void OR::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(data & mask) {
		bits_in |= id;
	} else {
		bits_in &= ~id;
	}
	bool next = (bits_in != 0);
	if(prev != next || first) {
		write_signals(&outputs, next ? 0xffffffff : 0);
		prev = next;
		first = false;
	}
}

#define STATE_VERSION	1

bool OR::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateUint32(bits_in);
	state_fio->StateBool(prev);
	state_fio->StateBool(first);
 	return true;
}
