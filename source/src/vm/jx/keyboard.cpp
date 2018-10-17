/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.10-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../../fifo.h"

static const int key_table[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
	0x2a, 0x1d, 0x38, 0x46, 0x3a, 0x6f, 0x00, 0x00, 0x00, 0x6b, 0x00, 0x01, 0x6d, 0x6c, 0x00, 0x00,
	0x39, 0x4f, 0x5e, 0x54, 0x47, 0x4b, 0x48, 0x4d, 0x50, 0x00, 0x00, 0x00, 0x00, 0x52, 0x53, 0x00,
	0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x70, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x4e, 0x00, 0x4a, 0x71, 0x7c,
	0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x6e, 0x6f, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x27, 0x33, 0x0c, 0x34, 0x35,
	0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x6A, 0x29, 0x0d, 0x00,
	0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define EVENT_SEND	0
#define EVENT_TIMER	1

void KEYBOARD::initialize()
{
	key_buf = new FIFO(8);
	
#ifdef KEYBOARD_HACK
	register_event(this, EVENT_SEND, 440 * 21, true, NULL);
#else
	register_event(this, EVENT_SEND, 220, true, NULL);
#endif
#ifndef TIMER_HACK
	register_event_by_clock(this, EVENT_TIMER, 4, true, NULL);
#endif
}

void KEYBOARD::release()
{
	key_buf->release();
	delete key_buf;
}

void KEYBOARD::reset()
{
	key_buf->clear();
#ifndef KEYBOARD_HACK
	send_count = 0;
#endif
	key_latched = false;
	
	d_pio->write_signal(SIG_I8255_PORT_C, 0x00, 0x01); // clear keyboard latched
	d_pio->write_signal(SIG_I8255_PORT_C, 0x00, 0x40); // set stop bit
	
	nmi_reg = 0;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xa0:
		nmi_reg = data;
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xa0:
		d_pio->write_signal(SIG_I8255_PORT_C, 0x00, 0x01);
		key_latched = false;
		return nmi_reg; // ???
#ifdef KEYBOARD_HACK
	case 0xa1:
		if(!key_buf->empty()) {
			return key_buf->read();
		}
		break;
#endif
	}
	return 0xff;
}

void KEYBOARD::event_callback(int id, int err)
{
	if(id == EVENT_SEND) {
#ifdef KEYBOARD_HACK
		if(!key_buf->empty()) {
			// rising edge
			if(nmi_reg & 0x80) {
				d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
			}
			d_pio->write_signal(SIG_I8255_PORT_C, 0x01, 0x01);
			key_latched = true;
		}
#else
		if(send_count == 0 && !key_buf->empty()) {
			int code = key_buf->read();
			int parity = 1;
			uint64_t bit = 1;
			
			for(int i = 0; i < 8; i++) {
				if(code & (1 << i)) {
					parity++;
				}
			}
			send_data = 0;
			
			#define SET_HALF_BIT(v) { \
				if(v) { \
					send_data |= bit; \
				} \
				bit <<= 1; \
			}
			#define SET_BIT(v) { \
				if(v) { \
					SET_HALF_BIT(1); \
					SET_HALF_BIT(0); \
				} else { \
					SET_HALF_BIT(0); \
					SET_HALF_BIT(1); \
				} \
			}
			
			SET_BIT(1);	// start bit
			for(int i = 0; i < 8; i++) {
				SET_BIT(code & (1 << i));
			}
			SET_BIT(parity & 1);
			send_count = 42;
		}
		if(send_count) {
			if(send_count == 42 && !key_latched) {
				// rising edge
				if(nmi_reg & 0x80) {
					d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
				}
				d_pio->write_signal(SIG_I8255_PORT_C, 0x01, 0x01);
				key_latched = true;
			}
			d_pio->write_signal(SIG_I8255_PORT_C, (send_data & 1) ? 0x40 : 0, 0x40);
			send_data >>= 1;
			send_count--;
		}
#endif
#ifndef TIMER_HACK
	} else if(id == EVENT_TIMER) {
		if(!(nmi_reg & 0x20)) {
			d_pit->write_signal(SIG_I8253_CLOCK_1, 1, 1);
			d_pit->write_signal(SIG_I8253_CLOCK_1, 0, 0);
		}
#endif
	}
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
#ifndef TIMER_HACK
	if(id == SIG_KEYBOARD_TIMER) {
		if(nmi_reg & 0x20) {
			d_pit->write_signal(SIG_I8253_CLOCK_1, data, mask);
		}
	}
#endif
}

void KEYBOARD::key_down(int code)
{
	if(key_table[code]) {
		key_buf->write(key_table[code]);
	}
}

void KEYBOARD::key_up(int code)
{
	if(key_table[code]) {
		key_buf->write(key_table[code] | 0x80);
	}
}

#define STATE_VERSION	1

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!key_buf->process_state(state_fio, loading)) {
		return false;
	}
#ifndef KEYBOARD_HACK
	state_fio->StateUint64(send_data);
	state_fio->StateInt32(send_count);
#endif
	state_fio->StateBool(key_latched);
	state_fio->StateUint8(nmi_reg);
	return true;
}

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!key_buf->process_state(state_fio, loading)) {
		return false;
	}
#ifndef KEYBOARD_HACK
	state_fio->StateUint64(send_data);
	state_fio->StateInt32(send_count);
#endif
	state_fio->StateBool(key_latched);
	state_fio->StateUint8(nmi_reg);
	return true;
}
