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
	full_auto = 0;
	joy_stat = emu->joy_buffer();
	register_frame_event(this);
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
	if(full_auto & 2) {
		if(joy_stat[num] & 0x40) val &= ~0x20;
		if(joy_stat[num] & 0x80) val &= ~0x10;
	}
	return val;
}

void JOYSTICK::event_frame()
{
	// synch to vsync
	full_auto = (full_auto + 1) & 3;
}

