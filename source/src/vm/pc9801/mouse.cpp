/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2011.12.27-

	[ mouse ]
*/

#include "mouse.h"
#include "../i8255.h"
#include "../i8259.h"

#define EVENT_TIMER	0

namespace PC9801 {

static const int freq_table[4] = {120, 60, 30, 15};

void MOUSE::initialize()
{
//	status = emu->get_mouse_buffer();
	status = NULL;
	
	ctrlreg = 0xff;
	freq = cur_freq = 0;
	
	register_frame_event(this);
	register_event(this, EVENT_TIMER, 1000000.0 / freq_table[freq], true, &register_id);
}

void MOUSE::reset()
{
	dx = dy = 0;
	lx = ly = -1;
	if(register_id >= 0) {
		cancel_event(this, register_id);
	}
	register_event(this, EVENT_TIMER, 1000000.0 / 120.0, true, &register_id);
	cur_freq = 0;
	freq = 0;
}

#if !defined(SUPPORT_HIRESO)
void MOUSE::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xbfdb:
		if((data & 0xfc) == 0) {
			freq = data;
		}
		break;
	}
}

uint32_t MOUSE::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xbfdb:
		return freq;
	}
	return 0xff;
}
#endif

void MOUSE::event_callback(int event_id, int err)
{
	if(!(ctrlreg & 0x10)) {
		#if !defined(SUPPORT_HIRESO)
			d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR5, 1, 1);
		#else
			d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR6, 1, 1);
		#endif
	}
	if(cur_freq != (freq & 3)) {
		cancel_event(this, register_id);
		register_event(this, EVENT_TIMER, 1000000.0 / freq_table[freq & 3], true, &register_id);
		cur_freq = freq & 3;
	}
}

void MOUSE::event_frame()
{
	status = emu->get_mouse_buffer();
	int x = status[0];
	int y = status[1];
	
	if(x > 32) {
		x = 32;
	} else if(x < -32) {
		x = -32;
	}
	if(y > 32) {
		y = 32;
	} else if(y < -32) {
		y = -32;
	}
	dx += x;
	dy += y;
	
	if(dx > 127) {
		dx = 127;
	} else if(dx < -128) {
		dx = -128;
	}
	if(dy > 127) {
		dy = 127;
	} else if(dy < -128) {
		dy = -128;
	}
	emu->release_mouse_buffer(status);
	update_mouse();
}

void MOUSE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(!(ctrlreg & 0x80) && (data & 0x80)) {
		lx = dx;
		ly = dy;
		dx = dy = 0;
	}
	ctrlreg = data & mask;
	update_mouse();
}

void MOUSE::update_mouse()
{
	int val = 0;
	
	int32_t _button = emu->get_mouse_button();
	if(!(_button & 1)) val |= 0x80;	// left
	if(!(_button & 2)) val |= 0x20;	// right
	if(!(_button & 4)) val |= 0x40;	// center
	
	switch(ctrlreg & 0xe0) {
	case 0x00: val |= (dx >> 0) & 0x0f; break;
	case 0x20: val |= (dx >> 4) & 0x0f; break;
	case 0x40: val |= (dy >> 0) & 0x0f; break;
	case 0x60: val |= (dy >> 4) & 0x0f; break;
	case 0x80: val |= (lx >> 0) & 0x0f; break;
	case 0xa0: val |= (lx >> 4) & 0x0f; break;
	case 0xc0: val |= (ly >> 0) & 0x0f; break;
	case 0xe0: val |= (ly >> 4) & 0x0f; break;
	}
	d_pio->write_signal(SIG_I8255_PORT_A, val, 0xff);
}

#define STATE_VERSION	2

bool MOUSE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ctrlreg);
	state_fio->StateValue(freq);
	state_fio->StateValue(cur_freq);
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(lx);
	state_fio->StateValue(ly);
	state_fio->StateValue(register_id);
	return true;
}

}
