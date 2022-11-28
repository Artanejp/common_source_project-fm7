/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.14 -

	[ i/o controller ]
*/

#include "ioctrl.h"
#include "../beep.h"
#include "../i8259.h"
#include "../pcm1bit.h"
#include "../upd765a.h"
#include "../../fifo.h"

static const int key_table[256] = {
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x18,0x12,  -1,  -1,  -1,0x38,  -1,  -1,
	  -1,0x04,0x05,0x09,  -1,  -1,  -1,  -1,  -1,0x10,  -1,0x11,  -1,  -1,  -1,  -1,
	0x4A,  -1,  -1,0x5B,0x5A,0x15,0x13,0x16,0x14,  -1,  -1,  -1,  -1,0x17,  -1,  -1,
	0x27,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x26,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,0x31,0x45,0x43,0x33,0x23,0x34,0x35,0x36,0x30,0x3F,0x40,0x3B,0x47,0x46,0x2B,
	0x29,0x21,0x2C,0x32,0x2D,0x2F,0x44,0x22,0x3A,0x2E,0x39,  -1,  -1,  -1,  -1,  -1,
	0x4B,0x4F,0x50,0x51,0x52,0x53,0x54,0x56,0x57,0x58,0x59,0x55,  -1,0x5C,0x4D,0x5D,
	0x0B,0x0C,0x0D,0x0E,0x0F,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x3D,0x3C,0x48,0x28,0x41,0x42,
	0x2A,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,0x37,0x25,0x3E,0x24,  -1,
	  -1,  -1,0x49,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
	  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1
};

#define EVENT_KEY	0
#define EVENT_600HZ	1
#define EVENT_100HZ	2
#define EVENT_50HZ	3
#define EVENT_10HZ	4

void IOCTRL::initialize()
{
	// init keyboard
	key_stat = emu->get_key_buffer();
	mouse_stat = emu->get_mouse_buffer();
	key_buf = new FIFO(64);
	caps = kana = false;
	
	// timer
	ts = 0;
	
	// register event
	register_event_by_clock(this, EVENT_600HZ, CPU_CLOCKS / 600, true, NULL);
	register_event_by_clock(this, EVENT_100HZ, CPU_CLOCKS / 100, true, NULL);
	register_event_by_clock(this, EVENT_50HZ, CPU_CLOCKS / 50, true, NULL);
	register_event_by_clock(this, EVENT_10HZ, CPU_CLOCKS / 10, true, NULL);
}

void IOCTRL::release()
{
	key_buf->release();
	delete key_buf;
}

void IOCTRL::reset()
{
	key_val = key_mouse = 0;
	key_prev = -1;
	key_res = false;
	key_done = true;
	key_buf->clear();
	key_buf->write(0x1f0);
	key_buf->write(0x100);
	register_id = -1;
	update_key();
}

void IOCTRL::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x3f0) {
	case 0x22:
		ts = (data >> 3) & 3;
		d_beep->write_signal(SIG_BEEP_ON, ~data, 0x40);		// tone (2400hz)
		d_pcm->write_signal(SIG_PCM1BIT_ON, data, 0x40);	// direct
		d_pcm->write_signal(SIG_PCM1BIT_SIGNAL, data, 0x80);	// signal
		break;
	case 0x24:
		// tc/vfo
		d_fdc->write_signal(SIG_UPD765A_TC, data, 0x40);
		break;
	}
}

uint32_t IOCTRL::read_io8(uint32_t addr)
{
	switch(addr & 0x3ff) {
	case 0x20:
		key_done = true;
		update_key();
		return key_val;
	case 0x22:
		// bit1: 0 = Color Mode, 1 = Monochrome Mode
		// bit2: 1 = Double FDD, 0 = Single FDD
		// bit3: 1 = 2D, 0 = 2DD
		// bit4: 1 = KD, 0 = MD
		// bit5: 1 = Horizontal Monitor, 0 = Virtical Monitor
		{
			uint32_t value = key_mouse | 0x05;
			if(!config.drive_type) {
				value |= 0x08; // 2D
			}
			if(!config.monitor_type) {
				value |= 0x20; // Horizontal Monitor
			}
			return value;
		}
	}
	return 0xff;
}

void IOCTRL::event_callback(int event_id, int err)
{
	if(event_id == EVENT_KEY) {
		if(!key_buf->empty()) {
			key_val = key_buf->read();
			key_mouse = (key_val & 0x100) ? 0x10 : 0;
			key_val &= 0xff;
			key_done = false;
			d_pic->write_signal(SIG_I8259_IR3, 1, 1);
		}
		register_id = -1;
	} else if(event_id == EVENT_600HZ) {
		if(ts == 0) {
			d_pic->write_signal(SIG_I8259_IR2, 1, 1);
		}
	} else if(event_id == EVENT_100HZ) {
		if(ts == 1) {
			d_pic->write_signal(SIG_I8259_IR2, 1, 1);
		}
	} else if(event_id == EVENT_50HZ) {
		if(ts == 2) {
			d_pic->write_signal(SIG_I8259_IR2, 1, 1);
		}
		// mouse
		if(key_buf->empty()) {
			uint8_t val = 0;
			if(!(mouse_stat[2] & 1)) val |= 1;
			if(!(mouse_stat[2] & 2)) val |= 2;
			if(caps) val |= 0x10;
			if(kana) val |= 0x20;
			if(key_stat[0xa0]) val |= 0x40;	// lshift
			if(key_stat[0xa1]) val |= 0x80;	// rshift
			if(mouse_stat[0] || mouse_stat[1]) {
				key_buf->write(val | 4);
				key_buf->write(mouse_stat[0] & 0xff);
				key_buf->write(mouse_stat[1] & 0xff);
				update_key();
				key_prev = val;
			} else if(key_prev != val) {
				key_buf->write(val);
				update_key();
				key_prev = val;
			}
		}
	} else if(event_id == EVENT_10HZ) {
		if(ts == 3) {
			d_pic->write_signal(SIG_I8259_IR2, 1, 1);
		}
	}
}

void IOCTRL::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool next = ((data & mask) != 0);
	if(!key_res && next) {
		// reset
		caps = kana = false;
		key_buf->clear();
		key_buf->write(0x1f0);	// dummy
		key_buf->write(0x1f0);	// init code
		key_buf->write(0x100);	// error code
		key_done = true;
		update_key();
	}
	key_res = next;
}

void IOCTRL::key_down(int code)
{
	if(code == 0x14) {
		caps = !caps;
	} else if(code == 0x15) {
		kana = !kana;
	} else if((code = key_table[code & 0xff]) != -1) {
		code |= 0x80;
		key_buf->write(code | 0x100);
		update_key();
	}
}

void IOCTRL::key_up(int code)
{
	if((code = key_table[code & 0xff]) != -1) {
		code &= ~0x80;
		key_buf->write(code | 0x100);
		update_key();
	}
}

void IOCTRL::update_key()
{
	if(key_done && !key_buf->empty()) {
		if(register_id == -1) {
			register_event(this, EVENT_KEY, 1000, false, &register_id);
		}
	}
}

#define STATE_VERSION	1

bool IOCTRL::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(caps);
	state_fio->StateValue(kana);
	if(!key_buf->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(key_val);
	state_fio->StateValue(key_mouse);
	state_fio->StateValue(key_prev);
	state_fio->StateValue(key_res);
	state_fio->StateValue(key_done);
	state_fio->StateValue(register_id);
	state_fio->StateValue(ts);
	return true;
}

