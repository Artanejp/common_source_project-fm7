/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2016.01.05-

	[ printer ]
*/

#include "printer.h"

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x7fff) {
	case 0x1fe:
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 0x80);
		d_prn->write_signal(SIG_PRINTER_RESET, data, 0x40);
		break;
	case 0x1ff:
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	switch(addr & 0x7fff) {
	case 0x1fe:
		return 0xf2 | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 1 : 0);
	}
	return 0xff;
}

