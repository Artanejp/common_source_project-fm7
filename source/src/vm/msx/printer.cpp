/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : src/vm/mz2500/printer.cpp

	modified by umaiboux
	Date   : 2016.03.xx-

	[ printer ]
*/

#include "printer.h"

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 1) {
	case 0:
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 0x01);
		break;
	case 1:
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	switch(addr & 1) {
	case 0:
		if (d_prn != this) {
			return 0xfd | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 2 : 0);
		}
	}
	return 0xff;
}

