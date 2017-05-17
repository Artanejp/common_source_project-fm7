/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.13-

	[ floppy i/f ]
*/

#include "floppy.h"
#include "../mb8877.h"

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xf024:
		if(data & 0x08) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, 0, 3);
		} else if(data & 0x04) {
			d_fdc->write_signal(SIG_MB8877_DRIVEREG, 1, 3);
		}
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x80);
		break;
	}
}

