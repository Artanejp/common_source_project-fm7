/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ nand gate ]
*/

#include "nand.h"
#include "../fileio.h"

void NAND::write_signal(int id, uint32 data, uint32 mask)
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

void NAND::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint32(bits_mask);
	state_fio->FputUint32(bits_in);
	state_fio->FputBool(prev);
	state_fio->FputBool(first);
}

bool NAND::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	bits_mask = state_fio->FgetUint32();
	bits_in = state_fio->FgetUint32();
	prev = state_fio->FgetBool();
	first = state_fio->FgetBool();
	return true;
}

