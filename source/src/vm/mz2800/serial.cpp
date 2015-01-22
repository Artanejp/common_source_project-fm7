/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2015.01.17 -

	[ serial ]
*/

#include "serial.h"
#include "../z80sio.h"
#include "../../fileio.h"

void SERIAL::reset()
{
	addr_a0 = true;
}

void SERIAL::write_io8(uint32 addr, uint32 data)
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

uint32 SERIAL::read_io8(uint32 addr)
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

void SERIAL::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(addr_a0);
}

bool SERIAL::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	addr_a0 = state_fio->FgetBool();
	return true;
}

