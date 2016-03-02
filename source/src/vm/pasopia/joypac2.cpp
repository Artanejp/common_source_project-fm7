/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ joystick ]
*/

#include "joypac2.h"

void JOYPAC2::initialize(int id)
{
	joy = emu->get_joy_buffer();
}

void JOYPAC2::write_io8(uint32_t addr, uint32_t data)
{
	
}

uint32_t JOYPAC2::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr & 0xff) {
	case 0x19:
		if(joy[1] & 0x01) val &= ~0x01;
		if(joy[1] & 0x02) val &= ~0x02;
		if(joy[1] & 0x04) val &= ~0x04;
		if(joy[1] & 0x08) val &= ~0x08;
		if(joy[1] & 0x10) val &= ~0x10;
		if(joy[1] & 0x20) val &= ~0x20;
		return val;
	case 0x1a:
		if(joy[0] & 0x01) val &= ~0x01;
		if(joy[0] & 0x02) val &= ~0x02;
		if(joy[0] & 0x04) val &= ~0x04;
		if(joy[0] & 0x08) val &= ~0x08;
		if(joy[0] & 0x10) val &= ~0x10;
		if(joy[0] & 0x20) val &= ~0x20;
		return val;
	}
	return 0xff;
}

