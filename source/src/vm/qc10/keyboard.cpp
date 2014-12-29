/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.13 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../z80sio.h"

void KEYBOARD::initialize()
{
	for(int i = 0; i < 8; i++) {
		led[i] = false;
	}
	repeat = enable = true;
	key_stat = emu->key_buffer();
}

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	// rec command
	process_cmd(data & 0xff);
}

void KEYBOARD::key_down(int code)
{
	if(enable) {
		if(key_stat[0x10]) {
			if(code == 0x0d) code = 0x83;	// SHIFT + ENTER
			if(code == 0x70) code = 0x84;	// SHIFT + F1
			if(code == 0x71) code = 0x85;	// SHIFT + F2
			if(code == 0x72) code = 0x86;	// SHIFT + F3
			if(code == 0x73) code = 0x87;	// SHIFT + F4
		}
		if(code = key_map[code]) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, code, 0xff);
		}
	}
}

void KEYBOARD::key_up(int code)
{
	if(enable) {
		// key break
		if(code == 0x10) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x86, 0xff);	// shift break
		} else if(code == 0x11) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8a, 0xff);	// ctrl break
		} else if(code == 0x12) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8c, 0xff);	// graph break
		}
	}
}

void KEYBOARD::process_cmd(uint8 val)
{
	switch(val & 0xe0) {
	case 0x00:
		// repeat starting time set:
		break;
	case 0x20:
		// repeat interval set
		break;
	case 0xa0:
		// repeat control
		repeat = ((val & 1) != 0);
		break;
	case 0x40:
		// key_led control
		led[(val >> 1) & 7] = ((val & 1) != 0);
		break;
	case 0x60:
		// key_led status read
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, 1, 1);
		for(int i = 0; i < 8; i++) {
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0xc0 | (i << 1) | (led[i] ? 1: 0), 0xff);
		}
		break;
	case 0x80:
		// key sw status read
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, 1, 1);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x80, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x82, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x84, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x86 | (key_stat[0x10] ? 1: 0), 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x88, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8a | (key_stat[0x11] ? 1: 0), 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8c | (key_stat[0x12] ? 1: 0), 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0x8e, 0xff);
		break;
	case 0xc0:
		// keyboard enable
		enable = ((val & 1) != 0);
		break;
	case 0xe0:
		// reset
		for(int i = 0; i < 8; i++) {
			led[i] = false;
		}
		repeat = enable = true;
		// diagnosis
		if(!(val & 1)) {
			d_sio->write_signal(SIG_Z80SIO_CLEAR_CH0, 1, 1);
			d_sio->write_signal(SIG_Z80SIO_RECV_CH0, 0, 0xff);
		}
		break;
	}
}
