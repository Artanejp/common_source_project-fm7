/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ keyboard ]
*/

#include "keyboard.h"
#include "keycode.h"
#include "../i8259.h"
#include "../../fifo.h"

#define STATUS_OBF	1
#define STATUS_IBF	2

void KEYBOARD::initialize()
{
	recv_buf = new FIFO(32);
	cmd_param = new FIFO(32);
	
	register_frame_event(this);
}

void KEYBOARD::release()
{
	delete recv_buf;
	delete cmd_param;
}

void KEYBOARD::reset()
{
	recv_buf->clear();
	recv_data = 0;
	
	kbic_cmd = kbsc_cmd = -1;
	cmd_param->clear();
	cmd_byte = 0x00;
	kbic_status = 0x12;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x60:
		kbic_cmd = data;
		process_cmd();
		break;
	case 0x64:
		if(kbic_cmd != -1 || kbsc_cmd != -1) {
			cmd_param->write(data);
		} else {
			kbsc_cmd = data;
		}
		process_cmd();
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x60:
		kbic_status &= ~STATUS_OBF;
		d_pic->write_signal(SIG_I8259_IR1 | SIG_I8259_CHIP0, 0, 0);
		return recv_data;
	case 0x64:
		return kbic_status;
	}
	return 0xff;
}

void KEYBOARD::event_frame()
{
	if(!recv_buf->empty() && !(kbic_status & STATUS_OBF)) {
		recv_data = recv_buf->read();
		kbic_status |= STATUS_OBF;
		d_pic->write_signal(SIG_I8259_IR1 | SIG_I8259_CHIP0, 1, 1);
	}
}

void KEYBOARD::process_cmd()
{
	if(kbic_cmd != -1) {
		switch(kbic_cmd) {
		case 0x20:	// read command byte register
			recv_buf->write(cmd_byte);
			kbic_cmd = -1;
			break;
		case 0x60:	// write command byte register
			if(cmd_param->count() == 1) {
				cmd_byte = cmd_param->read();
				kbic_cmd = -1;
			}
			break;
		case 0xaa:	// self test
			recv_buf->write(0x55);
			kbic_cmd = -1;
			break;
		case 0xab:	// test k/b interface
			recv_buf->write(0x00);
			kbic_cmd = -1;
			break;
		case 0xac:	// diagnostics
			for(int i = 0; i < 16; i++) {
				recv_buf->write(0x00);
			}
			kbic_cmd = -1;
			break;
		case 0xad:	// disable keyboad
			cmd_byte |= 0x10;
			kbic_cmd = -1;
			break;
		case 0xae:	// enable keyboad
			cmd_byte &= ~0x10;
			kbic_cmd = -1;
			break;
		}
	} else {
		
	}
}

void KEYBOARD::key_down(int code)
{
	if(key_table[code]) {
		recv_buf->write(key_table[code] | 0x80);
	}
}

void KEYBOARD::key_up(int code)
{
	if(key_table[code]) {
		recv_buf->write(key_table[code]);
	}
}

