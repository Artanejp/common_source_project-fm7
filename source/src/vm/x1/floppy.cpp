/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ floppy ]
*/

#include "floppy.h"
#include "../mb8877.h"
#ifdef _X1TURBO_FEATURE
#include "../disk.h"
#endif

#define EVENT_SPINDLE_UP	0
#define EVENT_MOTOR_OFF		1

void FLOPPY::reset()
{
	register_id = -1;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xffc:
		if(!(prev & 0x80) && (data & 0x80)) {
			// L -> H
			if(register_id != -1) {
				cancel_event(this, register_id);
				register_id = -1;
			}
			if(!motor_on) {
				motor_on = true;
				register_event(this, EVENT_SPINDLE_UP, 560000, false, &register_id);
			}
		} else if((prev & 0x80) && !(data & 0x80)) {
			// H -> L
			if(register_id != -1) {
				cancel_event(this, register_id);
				register_id = -1;
			}
			register_event(this, EVENT_MOTOR_OFF, 1500000, false, &register_id);
		}
		// FIXME: drvsel is active while motor is on ???
		d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 0x03);
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 0x10);
		prev = data;
		break;
	}
}

#ifdef _X1TURBO_FEATURE
uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xffc:	// FM
//		for(int drv = 0; drv < 4; drv++) {
//			d_fdc->set_drive_mfm(drv, false);
//		}
		return 0xff;
	case 0xffd:	// MFM
//		for(int drv = 0; drv < 4; drv++) {
//			d_fdc->set_drive_mfm(drv, true);
//		}
		return 0xff;
	case 0xffe:	// 2HD
		for(int drv = 0; drv < 4; drv++) {
			d_fdc->set_drive_type(drv, DRIVE_TYPE_2HD);
//			d_fdc->set_drive_rpm(drv, 360);
		}
		return 0xff;
	case 0xfff:	// 2D/2DD
		for(int drv = 0; drv < 4; drv++) {
			if(d_fdc->get_media_type(drv) == MEDIA_TYPE_2DD) {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
			} else {
				d_fdc->set_drive_type(drv, DRIVE_TYPE_2D);
			}
//			d_fdc->set_drive_rpm(drv, 300);
		}
		return 0xff;
	}
	return 0xff;
}
#endif

void FLOPPY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_SPINDLE_UP) {
		d_fdc->write_signal(SIG_MB8877_MOTOR, 1, 1); // READY
	} else if(event_id == EVENT_MOTOR_OFF) {
		d_fdc->write_signal(SIG_MB8877_MOTOR, 0, 0); // notREADY
		motor_on = false;
	}
	register_id = -1;
}

#define STATE_VERSION	3

bool FLOPPY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(prev);
	state_fio->StateValue(motor_on);
	state_fio->StateValue(register_id);
	return true;
}

