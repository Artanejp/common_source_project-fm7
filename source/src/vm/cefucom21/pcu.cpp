/*
	Hino Electronics CEFUCOM-21 Emulator 'eCEFUCOM-21'

	Author : Takeda.Toshiya
	Date   : 2019.03.28-

	[ PCU-I (peripheral control unit ???) ]
*/

#include "pcu.h"
#include "../z80pio.h"

void PCU::initialize()
{
}

void PCU::reset()
{
}

void PCU::write_io8(uint32_t addr, uint32_t data)
{
}

uint32_t PCU::read_io8(uint32_t addr)
{
	return 0xff;
}

void PCU::event_frame()
{
}

void PCU::write_signal(int id, uint32_t data, uint32_t mask)
{
}

#define STATE_VERSION	1

bool PCU::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return true;
}

