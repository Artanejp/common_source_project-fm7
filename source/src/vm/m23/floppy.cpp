/*
	SORD M23 Emulator 'Emu23'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ fdc control ]
*/

#include "floppy.h"
#include "../mb8877.h"

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xb5:
	case 0xc5:
	case 0xcd:
		// bit7 : ??? (BASICÇ≈ëÄçÏÇµÇƒÇ¢ÇÈ 0x10,0x50,0xd0)
		// bit6 : density (0=double, 1=single)
		// bit5 : side
		// bit4 : 1=ready ????
		// bit3 : 1=drive #4
		// bit2 : 1=drive #3
		// bit1 : 1=drive #2
		// bit0 : 1=drive #1
		for(int drv = 0; drv < MAX_DRIVE; drv++) {
			if(data & (1 << drv)) {
				d_fdc->write_signal(SIG_MB8877_DRIVEREG, drv, 3);
			}
			d_fdc->set_drive_mfm(drv, ((data & 0x40) == 0));
		}
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 0x20);
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x10); // READY
		break;
	}
}
