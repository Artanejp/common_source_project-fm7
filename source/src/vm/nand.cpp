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

bool NAND::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(bits_mask);
	state_fio->StateValue(bits_in);
	state_fio->StateValue(prev);
	state_fio->StateValue(first);
	return true;
}

