/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2010.09.02 -

	[ psg*2 ]
*/

#include "psg.h"

void PSG::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xe9:
		d_psg_l->write_io8(0, data);
		d_psg_r->write_io8(0, data);
		break;
	}
}

