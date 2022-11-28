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

void PRINTER::write_io8(uint32_t addr, uint32_t data)
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

uint32_t PRINTER::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xe2:
		return busy ? 1 : 0;
	}
	return 0xff;
}

void PRINTER::write_signal(int id, uint32_t data, uint32_t mask)
{
//	if(id == SIG_PRINTER_BUSY) {
//		busy = ((data & mask) != 0);
//	}
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
	state_fio->StateValue(strobe);
	state_fio->StateValue(busy);
	state_fio->StateValue(out);
	return true;
}

