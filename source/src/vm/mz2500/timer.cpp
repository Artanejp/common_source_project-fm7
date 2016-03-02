/*
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.03 -

	[ timer ]
*/

#include "timer.h"
#include "../i8253.h"

void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
		// input gate signal H->L->H to i8253 ch0 and ch1
		d_pit->write_signal(SIG_I8253_GATE_0, 1, 1);
		d_pit->write_signal(SIG_I8253_GATE_1, 1, 1);
		d_pit->write_signal(SIG_I8253_GATE_0, 0, 1);
		d_pit->write_signal(SIG_I8253_GATE_1, 0, 1);
		d_pit->write_signal(SIG_I8253_GATE_0, 1, 1);
		d_pit->write_signal(SIG_I8253_GATE_1, 1, 1);
		break;
	}
}

