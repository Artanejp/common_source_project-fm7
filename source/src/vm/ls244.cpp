/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ 74LS244 / 74LS245 ]
*/

#include "ls244.h"

void LS244::initialize()
{
	DEVICE::initialize();
	din = 0xff;
}

void LS244::write_io8(uint32_t addr, uint32_t data)
{
	write_signals(&outputs, data);
}

uint32_t LS244::read_io8(uint32_t addr)
{
	return din;
}

void LS244::write_signal(int id, uint32_t data, uint32_t mask)
{
	din = (din & ~mask) | (data & mask);
}

#define STATE_VERSION	1

void LS244::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(din);
}

bool LS244::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	din = state_fio->FgetUint8();
	return true;
}

