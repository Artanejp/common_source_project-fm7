/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ printer ]
*/

#include "printer.h"

void PRINTER::initialize()
{
	busy = strobe = false;
}

void PRINTER::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xe2:
		strobe = ((data & 0x80) != 0);
		break;
	case 0xe3:
		out = data;
		break;
	}
}

uint32 PRINTER::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xe2:
		return busy ? 1 : 0;
	}
	return 0xff;
}

void PRINTER::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PRINTER_BUSY) {
		busy = ((data & mask) != 0);
	}
}

