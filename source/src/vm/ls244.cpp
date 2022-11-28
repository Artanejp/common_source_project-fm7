/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ 74LS244 / 74LS245 ]
*/

#include "ls244.h"

void LS244::initialize()
{
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

bool LS244::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(din);
	return true;
}

