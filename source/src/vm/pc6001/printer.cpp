/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : Takeda.Toshiya
	Date   : 2014.05.24-

	[ printer ]
*/

#include "printer.h"

void PRINTER::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PRINTER_OUT) {
		emu->printer_out(data);
	} else if(id == SIG_PRINTER_STB) {
		emu->printer_strobe((data & mask) == 0);
	}
}

