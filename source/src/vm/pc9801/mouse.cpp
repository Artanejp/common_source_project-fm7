/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2011.12.27-

	[ mouse ]
*/

#include "mouse.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../../fileio.h"

#define EVENT_TIMER	0

static const int freq_table[4] = {120, 60, 30, 15};

void MOUSE::initialize()
{
	status = emu->mouse_buffer();
	
	ctrlreg = 0xff;
	freq = cur_freq = 0;
	
	register_frame_event(this);
	register_event(this, EVENT_TIMER, 1000000.0 / freq_table[freq], true, &register_id);
}

void MOUSE::reset()
{
	dx = dy = 0;
	lx = ly = -1;
}

void MOUSE::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
	case 0xbfdb:
		// this port is not available on PC-9801/E/F/M
		freq = data & 3;
		break;
	}
}

void MOUSE::event_callback(int event_id, int err)
{
	if(!(ctrlreg & 0x10)) {
		d_pic->write_signal(SIG_I8259_CHIP1 | SIG_I8259_IR5, 1, 1);
	}
	if(cur_freq != freq) {
		cancel_event(this, register_id);
		register_event(this, EVENT_TIMER, 1000000.0 / freq_table[freq] + err, true, &register_id);
		cur_freq = freq;
	}
}

void MOUSE::event_frame()
{
	dx += status[0];
	if(dx > 64) {
		dx = 64;
	} else if(dx < -64) {
		dx = -64;
	}
	dy += status[1];
	if(dy > 64) {
		dy = 64;
	} else if(dy < -64) {
		dy = -64;
	}
	update_mouse();
}

void MOUSE::write_signal(int id, uint32 data, uint32 mask)
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
	
	if(!(status[2] & 1)) val |= 0x80;	// left
	if(!(status[2] & 2)) val |= 0x20;	// right
	if(!(status[2] & 4)) val |= 0x40;	// center
	
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

#define STATE_VERSION	1

void MOUSE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(ctrlreg);
	state_fio->FputInt32(freq);
	state_fio->FputInt32(cur_freq);
	state_fio->FputInt32(dx);
	state_fio->FputInt32(dy);
	state_fio->FputInt32(lx);
	state_fio->FputInt32(ly);
	state_fio->FputInt32(register_id);
}

bool MOUSE::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	ctrlreg = state_fio->FgetInt32();
	freq = state_fio->FgetInt32();
	cur_freq = state_fio->FgetInt32();
	dx = state_fio->FgetInt32();
	dy = state_fio->FgetInt32();
	lx = state_fio->FgetInt32();
	ly = state_fio->FgetInt32();
	register_id = state_fio->FgetInt32();
	return true;
}

