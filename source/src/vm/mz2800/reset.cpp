/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ reset ]
*/

#include "reset.h"

void RESET::initialize()
{
	prev = 0xff;
}

void RESET::write_signal(int id, uint32_t data, uint32_t mask)
{
	// from i8255 port c
	if(!(prev & 2) && (data & 2)) {
		static_cast<VM *>(vm)->cpu_reset();
	}
	if(!(prev & 8) && (data & 8)) {
		vm->reset();
	}
	prev = data & mask;
}

#define STATE_VERSION	1

bool RESET::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(prev);
	return true;
}

