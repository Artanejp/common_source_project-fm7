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

void TIMER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x42:
		ctrl = data;
		update_intr();
		break;
	}
}

uint32_t TIMER::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x42:
		return ctrl;
	case 0x43:
		return status;
	}
	return 0xff;
}

void TIMER::write_signal(int id, uint32_t data, uint32_t mask)
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

#define STATE_VERSION	1

bool TIMER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ctrl);
	state_fio->StateValue(status);
	return true;
}

