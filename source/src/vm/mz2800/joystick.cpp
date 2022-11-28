/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ joystick ]
*/

#include "joystick.h"

void JOYSTICK::initialize()
{
	mode = 0xf;
	joy_stat = emu->get_joy_buffer();
}

void JOYSTICK::write_io8(uint32_t addr, uint32_t data)
{
	mode = data;
}

uint32_t JOYSTICK::read_io8(uint32_t addr)
{
	uint32_t val = 0x3f;
	int num = (mode & 0x40) ? 1 : 0;
	bool dir = true;
	
	// trigger mask
	if(num) {
		if(!(mode & 0x04)) val &= ~0x20;
		if(!(mode & 0x08)) val &= ~0x10;
		dir = ((mode & 0x20) == 0);
	} else {
		if(!(mode & 0x01)) val &= ~0x20;
		if(!(mode & 0x02)) val &= ~0x10;
		dir = ((mode & 0x10) == 0);
	}
	
	// direction
	if(dir) {
		if(joy_stat[num] & 0x08) val &= ~0x08;
		if(joy_stat[num] & 0x04) val &= ~0x04;
		if(joy_stat[num] & 0x02) val &= ~0x02;
		if(joy_stat[num] & 0x01) val &= ~0x01;
	}
	
	// trigger
	if(joy_stat[num] & 0x10) val &= ~0x20;
	if(joy_stat[num] & 0x20) val &= ~0x10;
	return val;
}

#define STATE_VERSION	2

bool JOYSTICK::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(mode);
	return true;
}

