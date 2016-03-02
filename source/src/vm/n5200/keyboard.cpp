/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8251.h"

static const int key_table[256] = {
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x0e,0x0f,  -1,  -1,  -1,0x1c,  -1,  -1,
	0x70,0x74,0x73,0x60,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x00,0x35,0x51,  -1,  -1,
	0x34,0x36,0x37,0x3f,0x3e,0x3b,0x3a,0x3c,0x3d,  -1,  -1,  -1,  -1,0x38,0x39,  -1,
	0x0a,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,  -1,  -1,  -1,  -1,  -1,  -1,
	0x0a,0x1d,0x2d,0x2b,0x1f,0x12,0x20,0x21,0x22,0x17,0x23,0x24,0x25,0x2f,0x2e,0x18,
	0x19,0x10,0x13,0x1e,0x14,0x16,0x2c,0x11,0x2a,0x15,0x29,  -1,  -1,  -1,  -1,  -1,
	0x4e,0x4a,0x4b,0x4c,0x46,0x47,0x48,0x42,0x43,0x44,0x45,0x49,  -1,0x40,0x50,0x41,
	0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x60,0x61,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x27,0x26,0x30,0x0b,0x31,0x32,
	0x1a,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x1b,0x0d,0x28,0x0c,  -1,
	  -1,  -1,0x33,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1
};

void KEYBOARD::initialize()
{
	memset(flag, 0, sizeof(flag));
}

void KEYBOARD::reset()
{
	kana = caps = rst = false;
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_KEYBOARD_RST) {
		bool next = ((data & mask) != 0);
		if(rst && !next) {
			// keyboard reset
			d_sio->write_signal(SIG_I8251_RECV, 0xa0, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, 0x81, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, 0xc0, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, 0xe0, 0xff);
		}
		rst = next;
	} else if(id == SIG_KEYBOARD_RECV) {
		// receive command
		data &= mask;
		if(data == 0) {
			// keyboard reset
			d_sio->write_signal(SIG_I8251_RECV, 0xa0, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, 0x81, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, 0xc0, 0xff);
			d_sio->write_signal(SIG_I8251_RECV, 0xe0, 0xff);
		}
	}
}

void KEYBOARD::key_down(int code)
{
	if(code == 0x14) {
		caps = !caps;
		d_sio->write_signal(SIG_I8251_RECV, 0x71 | (caps ? 0 : 0x80), 0xff);
	} else if(code == 0x15) {
		kana = !kana;
		d_sio->write_signal(SIG_I8251_RECV, 0x72 | (kana ? 0 : 0x80), 0xff);
	} else if((code = key_table[code & 0xff]) != -1) {
		if(flag[code]) {
			d_sio->write_signal(SIG_I8251_RECV, code | 0x80, 0xff);
		}
		d_sio->write_signal(SIG_I8251_RECV, code, 0xff);
//		if(!flag[code]) {
//			d_sio->write_signal(SIG_I8251_RECV, code, 0xff);
//		}
		flag[code] = 1;
	}
}

void KEYBOARD::key_up(int code)
{
	if((code = key_table[code & 0xff]) != -1) {
		if(flag[code]) {
			d_sio->write_signal(SIG_I8251_RECV, code | 0x80, 0xff);
		}
		flag[code] = 0;
	}
}

