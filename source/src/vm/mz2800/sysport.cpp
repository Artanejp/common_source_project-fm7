/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ system poty ]
*/

#include "sysport.h"
#include "../i8253.h"

void SYSPORT::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0x7fff) {
	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
		// input gate signal to i8253 ch0 and ch1
		d_pit->write_signal(SIG_I8253_GATE_0, 1, 1);
		d_pit->write_signal(SIG_I8253_GATE_1, 1, 1);
		break;
	}
}

uint32 SYSPORT::read_io8(uint32 addr)
{
	switch(addr & 0x7fff) {
	case 0xbe:
		// z80sio ack
		return d_sio->intr_ack();
	}
	return 0xff;
}

