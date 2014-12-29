/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2013.12.31-

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

