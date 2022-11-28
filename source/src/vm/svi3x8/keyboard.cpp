/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/keyboard.cpp

	modified by tanam
	Date   : 2018.12.09-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

static const uint8_t key_map[11][8] = {
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0xbb, 0xba, 0xbc, 0xde, 0xbe, 0xbf,
	0xbd, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0xdb, 0xdc, 0xdd, 0x08, 0x26, 
	0x10, 0x11, 0xa4, 0xa5, 0x1b, 0x75, 0x0d, 0x25,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x24, 0x2d, 0x28,
	0x20, 0x09, 0x2e, 0x14, 0x76, 0x77, 0x00, 0x27,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6b, 0x6d, 0x6a, 0x6f, 0x6e, 0x6c
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	column = 0;
//	break_pressed = false;
	
	// register event to update the key status
	register_frame_event(this);
}

void KEYBOARD::event_frame()
{
//	bool new_pressed = (key_stat[0x13] != 0);
//	if(new_pressed && !break_pressed) {
//		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
//	}
//	break_pressed = new_pressed;
	
	update_keyboard();
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(column != (data & mask)) {
		column = data & mask;
		update_keyboard();
	}
}

void KEYBOARD::update_keyboard()
{
	uint8_t val = 0;
	
	if(column < 11) {
		for(int i = 0; i < 8; i++) {
			if(key_stat[key_map[column][i]] != 0) {
				val |= 1 << i;
			}
		}
	}
	d_pio->write_signal(SIG_I8255_PORT_B, ~val, 0xff);
}

#define STATE_VERSION	2

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(column);
//	state_fio->StateValue(break_pressed);
	return true;
}
