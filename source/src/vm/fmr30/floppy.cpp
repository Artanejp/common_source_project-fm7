/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ floppy ]
*/

#include "floppy.h"
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

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
	case 0x34:
		// drive control register
		fdcr = data;
		update_intr();
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

uint32 FLOPPY::read_io8(uint32 addr)
{
	switch(addr & 0xffff) {
	case 0x34:
		return fdcr;
	case 0x35:
		return fdsl;
	case 0x36:
		return fdst;
	}
	return 0xff;
}

void FLOPPY::write_signal(int id, uint32 data, uint32 mask)
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

