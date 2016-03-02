/*
	SHARP MZ-80A Emulator 'EmuZ-80A'

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	Modify : Hideki Suga
	Date   : 2014.12.30 -

	[ MZ-80AIF ]
*/

#include "mz80aif.h"
#include "../mb8877.h"

void MZ80AIF::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xdc:
		// drive reg
		d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 3);
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x80);
		break;
	case 0xdd:
		// side reg
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 1);
		break;
	}
}

