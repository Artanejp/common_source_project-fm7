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

void PRINTER::write_io8(uint32_t addr, uint32_t data)
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

uint32_t PRINTER::read_io8(uint32_t addr)
{
	// bit7 = busy
	return busy ? 0xff : 0x7f;
}

#define STATE_VERSION	1

bool PRINTER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(out);
	state_fio->StateValue(ctrl0);
	state_fio->StateValue(ctrl1);
	state_fio->StateValue(busy);
	return true;
}

