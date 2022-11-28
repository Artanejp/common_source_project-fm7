/*
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2011.05.16-

	[ floppy ]
*/

#include "floppy.h"
#include "../mb8877.h"

#define EVENT_MOTOR_ON	0
#define EVENT_MOTOR_OFF	1

void FLOPPY::initialize()
{
	prev_dc = 0;
	motor_on = false;
	
	// always motor on (temporary)
//	d_fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
}

void FLOPPY::reset()
{
	register_id = -1;
	irq_enabled = false;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xdc:
		// drive reg
		if(data & 4) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 3);
		}
		// motor on/off
		if(!(prev_dc & 0x80) && (data & 0x80)) {
			// L -> H
			if(register_id != -1) {
				cancel_event(this, register_id);
				register_id = -1;
			}
			if(!motor_on) {
				register_event(this, EVENT_MOTOR_ON, 560000, false, &register_id);
			}
		} else if((prev_dc & 0x80) && !(data & 0x80)) {
			// H -> L
			if(register_id != -1) {
				cancel_event(this, register_id);
				register_id = -1;
			}
			if(motor_on) {
				register_event(this, EVENT_MOTOR_OFF, 1500000, false, &register_id);
			}
		}
		prev_dc = data;
		break;
	case 0xdd:
		// side reg
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 1);
		break;
	case 0xde:
		// ???
		break;
	case 0xdf:
		// irq enable
		irq_enabled = ((data & 1) != 0);
		break;
	}
}

void FLOPPY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_MOTOR_ON) {
		d_fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
		motor_on = true;
	} else if(event_id == EVENT_MOTOR_OFF) {
		d_fdc->write_signal(SIG_MB8877_MOTOR, 0, 0);
		motor_on = false;
	}
	register_id = -1;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_FLOPPY_DRQ) {
		if(irq_enabled && (data & mask) != 0) {
			d_cpu->set_intr_line(true, true, 0);
		}
	}
}

#define STATE_VERSION	1

bool FLOPPY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(prev_dc);
	state_fio->StateValue(register_id);
	state_fio->StateValue(motor_on);
	state_fio->StateValue(irq_enabled);
	return true;
}

