/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2015.01.17 -

	[ serial ]
*/

#include "serial.h"
#include "../z80sio.h"

void SERIAL::reset()
{
	addr_a0 = true;
}

void SERIAL::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
		if(addr_a0) {
			d_sio->write_io8(addr, data);
		}
		break;
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
		if(!addr_a0) {
			d_sio->write_io8(addr, data);
		}
		break;
	case 0xcd:
		addr_a0 = ((data & 0x80) == 0);
		d_sio->set_tx_clock(0, (4000000.0 / 13.0) / (1 << ((data >> 3) & 7)));
		d_sio->set_rx_clock(0, (4000000.0 / 13.0) / (1 << ((data >> 3) & 7)));
		d_sio->set_tx_clock(1, (4000000.0 / 13.0) / (1 << ((data >> 0) & 7)));
		d_sio->set_rx_clock(1, (4000000.0 / 13.0) / (1 << ((data >> 0) & 7)));
		break;
	}
}

uint32_t SERIAL::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
		if(addr_a0) {
			return d_sio->read_io8(addr);
		}
		break;
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
		if(!addr_a0) {
			return d_sio->read_io8(addr);
		}
		break;
	}
	return 0xff;
}

#define STATE_VERSION	1

bool SERIAL::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(addr_a0);
	return true;
}

