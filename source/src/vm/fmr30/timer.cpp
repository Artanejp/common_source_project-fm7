/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ timer ]
*/

#include "timer.h"
#include "../i8259.h"

void TIMER::initialize()
{
	ctrl = status = 0;
}

void TIMER::write_io8(uint32 addr, uint32 data)
{
	switch(addr) {
	case 0x42:
		ctrl = data;
		update_intr();
		break;
	}
}

uint32 TIMER::read_io8(uint32 addr)
{
	switch(addr) {
	case 0x42:
		return ctrl;
	case 0x43:
		return status;
	}
	return 0xff;
}

void TIMER::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_TIMER_CH0) {
		if(data & mask) {
			status |= 1;
		} else {
			status &= ~1;
		}
		update_intr();
	} else if(id == SIG_TIMER_CH1) {
		if(data & mask) {
			status |= 2;
		} else {
			status &= ~2;
		}
		update_intr();
	}
}

void TIMER::update_intr()
{
	d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR0, (ctrl & status & 3) ? 1 : 0, 1);
}

