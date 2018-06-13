/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.09-

	[ floppy ]
*/

#include "floppy.h"
#include "../mb8877.h"

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x3fd8:
//		if((data & 0x0c) == 0x0c) {
		d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 0x03);
		d_fdc->write_signal(SIG_MB8877_SIDEREG,  data, 0x40);
		// bit7: set when track > 59
//		}
		break;
	}
}

