/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ joystick ]
*/

#include "joystick.h"

void JOYSTICK::initialize()
{
	mode = 0xf;
	joy_stat = emu->joy_buffer();
}

void JOYSTICK::write_io8(uint32 addr, uint32 data)
{
	mode = data;
}

uint32 JOYSTICK::read_io8(uint32 addr)
{
	uint32 val = 0x3f;
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

void JOYSTICK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint32(mode);
}

bool JOYSTICK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	mode = state_fio->FgetUint32();
	return true;
}

