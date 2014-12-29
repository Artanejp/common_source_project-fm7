/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

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

