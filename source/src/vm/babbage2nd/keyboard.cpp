/*
	Gijutsu-Hyoron-Sha Babbage-2nd Emulator 'eBabbage-2nd'

	Author : Takeda.Toshiya
	Date   : 2009.12.26 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../z80pio.h"

void KEYBOARD::key_down(int code)
{
	if(0x30 <= code && code <= 0x39) {
		code -= 0x30;
		d_pio->write_signal(SIG_Z80PIO_PORT_A, code, 0x1f);
	} else if(0x41 <= code && code <= 0x46) {
		code = (code - 0x41) + 0x0a;
		d_pio->write_signal(SIG_Z80PIO_PORT_A, code, 0x1f);
	} else if(0x70 <= code && code <= 0x73) {
		code = (0x73 - code) + 0x10;
		d_pio->write_signal(SIG_Z80PIO_PORT_A, code, 0x1f);
	} else if(code == 0x74) {
		vm->reset();
	}
}

