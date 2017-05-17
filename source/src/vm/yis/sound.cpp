/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.05.07-

	[ sound i/f ]
*/

#include "sound.h"
#include "../beep.h"

void SOUND::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xf031:
		d_beep->write_signal(SIG_BEEP_ON, data, 1);
		break;
	case 0xf032:
		if(data > 1) {
			d_beep->set_frequency(49152 / data);
			d_beep->write_signal(SIG_BEEP_MUTE, 0, 0);
		} else {
			d_beep->write_signal(SIG_BEEP_MUTE, 1, 1);
		}
		break;
	}
}

