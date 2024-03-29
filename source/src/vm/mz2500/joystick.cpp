/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ joystick ]
*/

#include "joystick.h"

namespace MZ2500 {

void JOYSTICK::initialize()
{
	mode = 0xf;
//	joy_stat = emu->get_joy_buffer();
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
	joy_stat = emu->get_joy_buffer();
	if(dir) {
		if(joy_stat[num] & 0x08) val &= ~0x08;
		if(joy_stat[num] & 0x04) val &= ~0x04;
		if(joy_stat[num] & 0x02) val &= ~0x02;
		if(joy_stat[num] & 0x01) val &= ~0x01;
	}
	
	// trigger
	if(joy_stat[num] & 0x10) val &= ~0x20;
	if(joy_stat[num] & 0x20) val &= ~0x10;
	emu->release_joy_buffer(joy_stat);
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

}
