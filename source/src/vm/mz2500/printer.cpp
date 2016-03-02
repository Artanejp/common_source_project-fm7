/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2015.12.28-

	[ printer ]
*/

#include "printer.h"

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xfe:
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 0x80);
		d_prn->write_signal(SIG_PRINTER_RESET, data, 0x40);
		break;
	case 0xff:
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xfe:
		return 0xf2 | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 1 : 0);
	}
	return 0xff;
}

