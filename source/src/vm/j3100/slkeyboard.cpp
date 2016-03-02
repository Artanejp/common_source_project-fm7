/*
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ keyboard ]
*/

#include "slkeyboard.h"
#include "keycode.h"
#include "../i8259.h"
#include "../../fifo.h"

void KEYBOARD::initialize()
{
	key_buf = new FIFO(8);
	register_frame_event(this);
}

void KEYBOARD::release()
{
	key_buf->release();
	delete key_buf;
}

void KEYBOARD::reset()
{
	key_buf->clear();
	key_code = 0;
	key_ctrl = 0x48;	// ???
	key_read = true;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x61:
		// bit7: H->L = reset flipflop ???
		if(data == 0xa1) {
			// reset keyboard ???
			key_buf->clear();
			key_read = true;
			register_event(this, 0, 2000000, false, NULL);
		}
		key_ctrl = data;
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint32_t val = 0;

	switch(addr) {
	case 0x60:
		key_read = true;
		d_pic->write_signal(SIG_I8259_IR1 | SIG_I8259_CHIP0, 0, 0);
		val = key_code;
		key_code = 0;
		return val;
	case 0x61:
		return key_ctrl;
	}
	return 0xff;
}

void KEYBOARD::event_frame()
{
	if(!key_buf->empty() && key_read) {
		key_code = key_buf->read();
		key_read = false;
		d_pic->write_signal(SIG_I8259_IR1 | SIG_I8259_CHIP0, 1, 1);
	}
}

void KEYBOARD::event_callback(int event_id, int err)
{
	// initialized ???
	key_buf->write(42 | 0x80);
	key_buf->write(42);
}

void KEYBOARD::key_down(int code)
{
	if(key_table[code]) {
		key_buf->write(key_table[code] | 0x80);
	}
}

void KEYBOARD::key_up(int code)
{
	if(key_table[code]) {
		key_buf->write(key_table[code]);
	}
}

