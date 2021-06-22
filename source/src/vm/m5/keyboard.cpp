/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ keyboard ]
*/

#include "keyboard.h"

namespace M5 {
	
static const int key_map[7][8] = {
	// back-space (0x08): reset/halt key
	// Column0 From MAME 0.208: src/mame/drivers/m5.cpp 20191105 K.O.
	// Column0 : Ctrl(LCONTROL), TAB, LSHIFT, RSHIFT, UNUSED, UNUSED, SPACE, ENTER
	{VK_CONTROL, 0x09, VK_LSHIFT, VK_RSHIFT, 0x00, 0x00, 0x20, 0x0d},
	{0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
	{0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49},
	{0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b},
	{0x5a, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0xbc},
	{0x39, 0x30, 0xbd, 0xde, 0xbe, 0xbf, 0xe2, 0xdc},
	{0x4f, 0x50, 0xc0, 0xdb, 0x4c, 0xbb, 0xba, 0xdd}
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
//	joy_stat = emu->get_joy_buffer();
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint32_t val = 0;
	
	switch(addr & 0xff) {
	case 0x30:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
/*	case 0x38:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:*/
		for(int i = 0; i < 8; i++) {
			val |= key_stat[key_map[addr & 0x7][i]] ? (1 << i) : 0;
		}
		return val;
	case 0x31:
//	case 0x39:
		for(int i = 0; i < 8; i++) {
			val |= key_stat[key_map[1][i]] ? (1 << i) : 0;
		}
		joy_stat = emu->get_joy_buffer();
		val |= (joy_stat[0] & 0x10) ? 0x01 : 0;
		val |= (joy_stat[0] & 0x20) ? 0x02 : 0;
		val |= (joy_stat[1] & 0x10) ? 0x10 : 0;
		val |= (joy_stat[1] & 0x20) ? 0x20 : 0;
		emu->release_joy_buffer(joy_stat);
		return val;
	case 0x37:
//	case 0x3f:
		joy_stat = emu->get_joy_buffer();
		val |= (joy_stat[0] & 0x08) ? 0x01 : 0;
		val |= (joy_stat[0] & 0x01) ? 0x02 : 0;
		val |= (joy_stat[0] & 0x04) ? 0x04 : 0;
		val |= (joy_stat[0] & 0x02) ? 0x08 : 0;
		val |= (joy_stat[1] & 0x08) ? 0x10 : 0;
		val |= (joy_stat[1] & 0x01) ? 0x20 : 0;
		val |= (joy_stat[1] & 0x04) ? 0x40 : 0;
		val |= (joy_stat[1] & 0x02) ? 0x80 : 0;
		emu->release_joy_buffer(joy_stat);
		return val;
	}
	return 0xff;
}

}
