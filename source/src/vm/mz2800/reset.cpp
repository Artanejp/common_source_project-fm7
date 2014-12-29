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

void RESET::write_signal(int id, uint32 data, uint32 mask)
{
	// from i8255 port c
	if(!(prev & 2) && (data & 2)) {
		vm->cpu_reset();
	}
	if(!(prev & 8) && (data & 8)) {
		vm->reset();
	}
	prev = data & mask;
}

