/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ floppy ]
*/

#include "floppy.h"
#include "../disk.h"
#include "../i8259.h"
#include "../mb8877.h"

void FLOPPY::initialize()
{
	fdcr = 6;
	fdsl = 0;
	fdst = 1;
	drvsel = 0;
	irq = false;
	changed[0] = changed[1] = changed[2] = changed[3] = false;
}

void FLOPPY::reset()
{
	for(int i = 0; i < MAX_DRIVE; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
		d_fdc->set_drive_mfm (i, true);
	}
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x34:
		// drive control register
		fdcr = data;
		update_intr();
		for(int i = 0; i < 4; i++) {
			d_fdc->set_drive_mfm(i, ((data & 2) != 0));
		}
//		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x18);
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 4);
		break;
	case 0x35:
		// drive select register
		fdsl = data;
		if(drvsel != (data & 3)) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, drvsel = data & 3, 3);
			fdst = changed[drvsel] ? 1 : 0;
			changed[drvsel] = false;
		}
		if((drvsel & 2) ? (data & 0x80) : (data & 0x40)) {
			d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2DD);
		} else {
			d_fdc->set_drive_type(drvsel, DRIVE_TYPE_2HD);
		}
		d_fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
		break;
	case 0x36:
		// echo clear
		if(data & 1) {
			fdst &= ~1;
		}
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x34:
		return fdcr;
	case 0x35:
		{
			uint32_t value = fdsl & 0x3f;
			if(d_fdc->get_media_type(drvsel) == MEDIA_TYPE_2D || d_fdc->get_media_type(drvsel) == MEDIA_TYPE_2DD) {
				drvsel |= (drvsel & 2) ? 0x80 : 0x40;
			}
			return value;
		}
	case 0x36:
		return fdst;
	}
	return 0xff;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_FLOPPY_IRQ) {
		irq = ((data & mask) != 0);
		update_intr();
	}
}

void FLOPPY::update_intr()
{
	d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR1, (irq && (fdcr & 1)) ? 1 : 0, 1);
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
	state_fio->StateValue(fdcr);
	state_fio->StateValue(fdsl);
	state_fio->StateValue(fdst);
	state_fio->StateValue(drvsel);
	state_fio->StateValue(irq);
	state_fio->StateArray(changed, sizeof(changed), 1);
	return true;
}

