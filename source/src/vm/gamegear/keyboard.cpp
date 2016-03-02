/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

static const uint8_t key_map[8][12] = {
	{ 0x31, 0x51, 0x41, 0x5a, 0x15, 0xbc, 0x4b, 0x49, 0x38, 0x00, 0x00, 0x00 },
	{ 0x32, 0x57, 0x53, 0x58, 0x20, 0xbe, 0x4c, 0x4f, 0x39, 0x00, 0x00, 0x00 },
	{ 0x33, 0x45, 0x44, 0x43, 0x24, 0xbf, 0xbb, 0x50, 0x30, 0x00, 0x00, 0x00 },
	{ 0x34, 0x52, 0x46, 0x56, 0x2e, 0xe2, 0xba, 0xc0, 0xbd, 0x00, 0x00, 0x00 },
	{ 0x35, 0x54, 0x47, 0x42, 0x00, 0x28, 0xdd, 0xdb, 0xde, 0x00, 0x00, 0x00 },
	{ 0x36, 0x59, 0x48, 0x4e, 0x00, 0x25, 0x0d, 0x00, 0xdc, 0x00, 0x00, 0x29 },
	{ 0x37, 0x55, 0x4a, 0x4d, 0x00, 0x27, 0x26, 0x00, 0x13, 0x12, 0x11, 0x10 },
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x81, 0x82, 0x84, 0x88, 0x90, 0xa0 }
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	joy_stat = emu->get_joy_buffer();
	
	column = 0;
	break_pressed = false;
	start_pressed = false;
	// register event to update the key status
	register_frame_event(this);
}

void KEYBOARD::reset()
{
	sk1100 = false;
}

void KEYBOARD::event_frame()
{
	bool new_pressed = (key_stat[VK_F9] != 0);
	if(new_pressed && !break_pressed) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
	break_pressed = new_pressed;
	
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
	uint32_t data = 0;
	
	if( sk1100 && column < 7) {
		// keyboard
		for (int i = 0; i < 12; i++) {
			if (key_stat[key_map[column][i]]) {
				data |= (1 << i);
			}
#ifdef SDL
			if (key_stat[key_map[column][i]] >0x87) key_stat[key_map[column][i]]++;
			if (key_stat[key_map[column][i]] >0x8f) {
				key_stat[key_map[column][i]]=0;
			}
#endif
		}
	} else {
		// joystick
		for(int i = 0; i < 12; i++) {
			uint8_t map = key_map[7][i];
			uint8_t stat = (map & 0x80) ? joy_stat[1] : joy_stat[0];
			if(stat & (map & 0x3f)) {
				data |= (1 << i);
			}
		}
//		if (joy_stat[0] & 0x40) {
		if (joy_stat[0] == 0x40) {
			start_pressed=true;
		} else {
			start_pressed=false;
		}
	}
	d_pio->write_signal(SIG_I8255_PORT_A, ~data, 0xff);
	data >>= 8;
	d_pio->write_signal(SIG_I8255_PORT_B, ~data, 0x0f);
}

bool KEYBOARD::is_start()
{
	return start_pressed;
}
