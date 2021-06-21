/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2021.06.16 -
    History : 2021.06.16 Initial
	[ Towns Mouse]

*/

#include "./mouse.h"
#include "./joystick.h"

namespace FMTOWNS {

#define EVENT_MOUSE_TIMEOUT		1
#define EVENT_MOUSE_SAMPLING	2
#define EVENT_MOUSE_RESET		3
	
void MOUSE::initialize()
{
	phase = 0;
	strobe = false;
	mouse_state = NULL;
	mouse_connected = false;
	port_num = 0;
	
	dx = dy = 0;
	lx = ly = 0;
	event_timeout = -1;
	event_sampling = -1;

	register_frame_event(this);
}

void MOUSE::release()
{
}

void MOUSE::event_pre_frame()
{
	event_callback(EVENT_MOUSE_SAMPLING, 0);
}
	
void MOUSE::reset()
{
	// Values around mouse aren't initialized on reset.
//	mouse_state = emu->get_mouse_buffer();
//	update_config(); // Update MOUSE PORT.
	if(mouse_connected) {
		uint32_t com_d = SIG_JOYPORT_TYPE_MOUSE;
		if(port_num != 0) {
			com_d |= SIG_JOYPORT_CH1;
		}
		if(strobe) {
			com_d |= SIG_JOYPORT_COM;
		}
		if(d_joyport != nullptr) {
			d_joyport->write_signal(com_d, 0xffffffff, 0xffffffff);
		}
	}
}

void MOUSE::update_strobe()
{

	bool _bak = strobe;
	uint8_t _mask = ((port_num & 1) != 0) ? 0x20 : 0x10;
	strobe = ((mouse_mask & _mask) != 0) ? true : false;
	
	uint32_t com_d = SIG_JOYPORT_TYPE_MOUSE;
	if(port_num != 0) {
		com_d |= SIG_JOYPORT_CH1;
	}
	if(strobe) {
		com_d |= SIG_JOYPORT_COM;
	}
	if((_bak != strobe)/* && (flag)*/) {
		if(phase == 0) {
			if(strobe) {
				// Sample data from MOUSE to registers.
				lx = -dx;
				ly = -dy;
				dx = 0;
				dy = 0;
				// From FM Towns Technical book, Sec.1-7, P.241.
				// (Ta + Tj * 3 + Ti) <= 920.0uS 
				//force_register_event(this, EVENT_MOUSE_TIMEOUT, 920.0, false, event_timeout);
				force_register_event(this, EVENT_MOUSE_TIMEOUT, 2000.0, false, event_timeout);
				phase = 2; // SYNC from MAME 0.225. 20201126 K.O
			}
			if(d_joyport != nullptr) {
				d_joyport->write_signal(com_d, 0xffffffff, 0xffffffff);
			}
			return;
		}
		phase++;
		if(d_joyport != nullptr) {
			d_joyport->write_signal(com_d, 0xffffffff, 0xffffffff);
		}
	}
}


uint32_t MOUSE::update_mouse()
{
	uint32_t mouse_data = 0x0f;
	switch(phase) {
//	case 1: // SYNC
//		mouse_data = 0x0f;
//		break;
	case 2: // X_HIGH
		mouse_data = (lx >> 0) & 0x0f;
		break;
	case 3: // X_LOW
		mouse_data = (lx >> 4) & 0x0f;
		break;
	case 4: // Y_HIGH
		mouse_data = (ly >> 0) & 0x0f;
		break;
	case 5: // Y_LOW
		mouse_data = (ly >> 4) & 0x0f;
		// From FM Towns Technical book, Sec.1-7, P.241.
		// Ti(min)
		force_register_event(this, EVENT_MOUSE_TIMEOUT, 150.0, false, event_timeout);
		break;
	}

//	out_debug_log(_T("READ MOUSE DATA=%01X PHASE=%d STROBE=%d"), mouse_data, mouse_phase, (mouse_strobe) ? 1 : 0);
	return mouse_data;
}

uint32_t MOUSE::read_signal(int ch)
{
	switch(ch) {
	case SIG_MOUSE_DATA:
		if(mouse_connected) {
			uint8_t trig = (mouse_mask >> (port_num << 1)) & 0x03;
			uint8_t rval = 0xf0;
			
			rval |= (update_mouse() & 0x0f);
			mouse_state = emu->get_mouse_buffer();
			if((trig & 0x01) == 0) {
				rval &= ~0x10; // Button LEFT
			}
			if((trig & 0x02) == 0) {
				rval &= ~0x20; // Button RIGHT
			}
			if(mouse_state != NULL) {
				uint32_t stat = mouse_state[2];
				if((stat & 0x01) == 0) {
					rval &= ~0x10; // Button LEFT
				}
				if((stat & 0x02) == 0) {
					rval &= ~0x20; // Button RIGHT
				}
			}
			if(!(strobe)) { // COM
				rval = rval & ~0x40;
			}
			return rval;
		}
		break;
	case SIG_MOUSE_ENABLE:
		return ((mouse_connected) ? 0xffffffff : 0);
		break;
	case SIG_MOUSE_NUM:
		return port_num & 1;
		break;
	}
	return 0xffffffff;
}
	
void MOUSE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_MOUSE_ENABLE:
		{
			bool _bak = mouse_connected;
			mouse_connected = (((data & mask) & 0xfffffffe) != 0) ? true : false;
			port_num = (data & mask) & 0x01;
			if(_bak != mouse_connected) {
				if(!(_bak)) { // off -> on
					phase = 0;
					dx = dy = 0;
					lx = ly = 0;
					strobe = false;
					clear_event(this, event_timeout);
//					force_register_event(this, EVENT_MOUSE_SAMPLING,
//										 4.0e3, true, event_sampling);
				} else { // on -> off
					clear_event(this, event_timeout);
//					clear_event(this, event_sampling);
				}
			}
		}
		break;
	case SIG_MOUSE_STROBE:
		mouse_mask = data;
		if(mouse_connected) {
			update_strobe();
		}
		break;
	}
}

void MOUSE::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_MOUSE_RESET:
		event_timeout = -1;
		phase = 0;
		break;
	case EVENT_MOUSE_TIMEOUT:
		phase = 0;
		event_timeout = -1;
		dx = dy = 0;
		lx = ly = 0;
		break;
	case EVENT_MOUSE_SAMPLING:
		mouse_state = emu->get_mouse_buffer();
		if(mouse_state != NULL) {
			dx += mouse_state[0];
			dy += mouse_state[1];
			if(dx < -127) {
				dx += 128;
			} else if(dx > 127) {
				dx -= 128;
			}
			if(dy < -127) {
				dy += 128;
			} else if(dy > 127) {
				dy -= 128;
			}
		}
		break;
	}
}
	
#define STATE_VERSION 1

bool MOUSE::process_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	
	state_fio->StateValue(mouse_connected);
	state_fio->StateValue(port_num);
	
	state_fio->StateValue(phase);
	state_fio->StateValue(strobe);
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(lx);
	state_fio->StateValue(ly);
	
	state_fio->StateValue(mouse_mask);
	state_fio->StateValue(event_timeout);
	state_fio->StateValue(event_sampling);

	return true;
}
}
