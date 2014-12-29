/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ floppy ]
*/

#include "floppy.h"
#include "../i8259.h"
#include "../mb8877.h"

void FLOPPY::initialize()
{
	drvreg = drvsel = 0;
	irq = irqmsk = false;
	changed[0] = changed[1] = changed[2] = changed[3] = false;
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	int nextdrv = drvsel;
	
	switch(addr & 0xffff) {
	case 0x208:
		// drive control register
		irqmsk = ((data & 1) != 0);
		update_intr();
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x10);
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 4);
		break;
	case 0x20c:
		// drive select register
		if(data & 1) {
			nextdrv = 0;
		} else if(data & 2) {
			nextdrv = 1;
		} else if(data & 4) {
			nextdrv = 2;
		} else if(data & 8) {
			nextdrv = 3;
		}
		if(drvsel != nextdrv) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, drvsel = nextdrv, 3);
		}
		drvreg = data;
		break;
	}
}

uint32 FLOPPY::read_io8(uint32 addr)
{
	switch(addr & 0xffff) {
	case 0x208:
		if(changed[drvsel]) {
			changed[drvsel] = false;
			return d_fdc->fdc_status() | 0xe1;	// fdd*2
		}
//		return d_fdc->fdc_status() | 0x60;	// fdd*1
		return d_fdc->fdc_status() | 0xe0;	// fdd*2
	case 0x20c:
		return drvreg;
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
	d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR6, irq && irqmsk ? 1 : 0, 1);
}

