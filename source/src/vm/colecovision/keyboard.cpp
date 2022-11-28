/*
	COLECO ColecoVision Emulator 'yaCOLECOVISION'

	Author : tanam
	Date   : 2016.08.14-

	[ keyboard ]
*/

#include "keyboard.h"

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	joy_stat = emu->get_joy_buffer();
	// register event to update the key status
//	register_frame_event(this); // is this needed?
}

void KEYBOARD::event_frame()
{
	if (joy_stat[0] & 0x04 || joy_stat[0] & 0x08) {
		d_cpu->write_signal(SIG_CPU_IRQ, 1, 1);
	}
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	if ((addr & 0x000000ff)==0x80) {
		tenkey=true;
	}
	if ((addr & 0x000000ff)==0xc0) {
		tenkey=false;
	}
	return;
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	// Controller 1
	if ((addr & 0x000000ff)==0xfc) {
		uint32_t button=0x70;//0xf0;
		if (!tenkey) {
			uint32_t joystick=0x7f;//0xff;
			if (joy_stat[0] & 0x01) joystick &= 0xfe;	// U
			if (joy_stat[0] & 0x02) joystick &= 0xfb;	// D
			if (joy_stat[0] & 0x04) joystick &= 0xf7;	// L
			if (joy_stat[0] & 0x08) joystick &= 0xfd;	// R
			if (joy_stat[0] & 0x20) joystick &= 0xbf;	// F1
			return joystick;
		}
		if (joy_stat[0] & 0x10)
			button &= 0xbf;         // F2
		if ((key_stat[0x31] & 0x80) || (key_stat[0x61] & 0x80))
			return (button | 0x0d); // 1
		if ((key_stat[0x32] & 0x80) || (key_stat[0x62] & 0x80))
			return (button | 0x07); // 2
		if ((key_stat[0x33] & 0x80) || (key_stat[0x63] & 0x80))
			return (button | 0x0c); // 3
		if ((key_stat[0x34] & 0x80) || (key_stat[0x64] & 0x80))
			return (button | 0x02); // 4
		if ((key_stat[0x35] & 0x80) || (key_stat[0x65] & 0x80))
			return (button | 0x03); // 5
		if ((key_stat[0x36] & 0x80) || (key_stat[0x66] & 0x80))
			return (button | 0x0e); // 6
		if ((key_stat[0x37] & 0x80) || (key_stat[0x67] & 0x80))
			return (button | 0x05); // 7
		if ((key_stat[0x38] & 0x80) || (key_stat[0x68] & 0x80))
			return (button | 0x01); // 8
		if ((key_stat[0x39] & 0x80) || (key_stat[0x69] & 0x80))
			return (button | 0x0b); // 9
		if ((key_stat[0x30] & 0x80) || (key_stat[0x60] & 0x80))
			return (button | 0x0a); // 0
		if (key_stat[0xbd] & 0x80)
			return (button | 0x09); // * '-'
		if (key_stat[0xde] & 0x80)
			return (button | 0x06); // # '^'
		if (key_stat[0x43] & 0x80)
			return (button | 0x08); // F3 'c'
		if (key_stat[0x56] & 0x80)
			return (button | 0x04); // F4 'v'
		return (button | 0x0f);
	}
	// Controller 2
	if ((addr & 0x000000ff)==0xff) {
		uint32_t button=0x70;//0xf0;
		if (!tenkey) {
			uint32_t joystick=0x7f;//0xff;
			if (joy_stat[1] & 0x01) joystick &= 0xfe;	// U
			if (joy_stat[1] & 0x02) joystick &= 0xfb;	// D
			if (joy_stat[1] & 0x04) joystick &= 0xf7;	// L
			if (joy_stat[1] & 0x08) joystick &= 0xfd;	// R
			if (joy_stat[1] & 0x20) joystick &= 0xbf;	// F1
			return joystick;
		}
		if (joy_stat[1] & 0x10)
			button &= 0xbf;         // F2
		if (key_stat[0x51] & 0x80)
			return (button | 0x0d); // 1 'q'
		if (key_stat[0x57] & 0x80)
			return (button | 0x07); // 2 'w'
		if (key_stat[0x45] & 0x80)
			return (button | 0x0c); // 3 'e'
		if (key_stat[0x52] & 0x80)
			return (button | 0x02); // 4 'r'
		if (key_stat[0x54] & 0x80)
			return (button | 0x03); // 5 't'
		if (key_stat[0x59] & 0x80)
			return (button | 0x0e); // 6 'y'
		if (key_stat[0x55] & 0x80)
			return (button | 0x05); // 7 'u'
		if (key_stat[0x49] & 0x80)
			return (button | 0x01); // 8 'i'
		if (key_stat[0x4f] & 0x80)
			return (button | 0x0b); // 9 'o'
		if (key_stat[0x50] & 0x80)
			return (button | 0x0a); // 0 'p'
		if (key_stat[0xc0] & 0x80)
			return (button | 0x09); // * '@'
		if (key_stat[0xdb] & 0x80)
			return (button | 0x06); // # '['
		if (key_stat[0xbc] & 0x80)
			return (button | 0x08); // F3 ','
		if (key_stat[0xbe] & 0x80)
			return (button | 0x04); // F4 '.'
		return (button | 0x0f);
	}
	return 0x0ff;
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
	state_fio->StateValue(tenkey);
	return true;
}

