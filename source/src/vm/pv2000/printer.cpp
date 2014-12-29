/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ printer ]
*/

#include "printer.h"

void PRINTER::initialize()
{
	busy = false;
}

void PRINTER::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x80:
		// data
		out = data;
		break;
	case 0xa0:
		// ctrl #0 ?
		ctrl0 = data;
		break;
	case 0xb0:
		// ctrl #1 ?
		ctrl1 = data;
		break;
	}
}

uint32 PRINTER::read_io8(uint32 addr)
{
	// bit7 = busy
	return busy ? 0xff : 0x7f;
}

