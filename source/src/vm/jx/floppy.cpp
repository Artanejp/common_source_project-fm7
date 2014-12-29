/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.09-

	[ diskette i/f ]
*/

#include "floppy.h"
#include "../i8259.h"
#include "../upd765a.h"

void FLOPPY::reset()
{
	prev = 0;
	register_id = -1;
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	switch(addr) {
	case 0xf2:
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 1);	// drive enable
		if(data & 0x20) {
			// WatchDog Timer is enabled
			if((prev & 0x40) && !(data & 0x40)) {
				if(register_id != -1) {
					cancel_event(this, register_id);
				}
				register_event_by_clock(this, 0, 3 * CPU_CLOCKS, false, &register_id);
			}
		} else {
			if(register_id != -1) {
				cancel_event(this, register_id);
				register_id = -1;
			}
		}
		if((prev & 0x80) && !(data & 0x80)) {
			d_fdc->reset();
		}
		prev = data;
		break;
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
	// WatchDog Timer
	emu->out_debug_log("WatchDog Timer\n");
	d_pic->write_signal(SIG_I8259_IR6, 1, 1);
	register_id = -1;
}

