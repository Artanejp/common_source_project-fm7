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

	mouse_state = NULL;
	mouse_connected = false;
	port_num = 0;
	
	dx = dy = 0;
	lx = ly = 0;
	event_timeout = -1;
	event_sampling = -1;
	axisdata = 0x0f;
	strobe = false;
	trig_a = false;
	trig_b = false;
	
}

void MOUSE::release()
{
}

	
void MOUSE::update_strobe(uint8_t data, bool force)
{
	bool _bak = strobe;
	uint8_t _mask = ((port_num & 1) != 0) ? 0x20 : 0x10;
	int com_d = SIG_JOYPORT_TYPE_MOUSE;
	strobe = ((data & 0x01) != 0) ? true : false;
	com_d |= ((port_num & 1) << 24);
	if((_bak != strobe) || (force)) {
		if(phase == 0) {
			if(strobe) {
				// Sample data from MOUSE to registers.
				sample_mouse_xy(); // Sample next value.
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
		} else {
			phase++;
		}
		if(d_joyport != nullptr) {
			d_joyport->write_signal(com_d | SIG_JOYPORT_COM, (strobe) ? 0xffffffff : 0x00000000, 0xffffffff);
		}
	}
}


uint32_t MOUSE::update_mouse()
{
	uint32_t mouse_data = 0x00;
	switch(phase) {
//	case 1: // SYNC
//		mouse_data = 0x0f;
//		break;
	case 2: // X_HIGH
		mouse_data = lx >> 0;
		break;
	case 3: // X_LOW
		mouse_data = lx >> 4;
		break;
	case 4: // Y_HIGH
		mouse_data = ly >> 0;
		break;
	case 5: // Y_LOW
		mouse_data = ly >> 4;
		// From FM Towns Technical book, Sec.1-7, P.241.
		// Ti(min)
		force_register_event(this, EVENT_MOUSE_TIMEOUT, 150.0, false, event_timeout);
		phase++;
		break;
	case 6: // END
		mouse_data = ly >> 4;
//		phase = 0;
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
			uint8_t rval = 0x70;
			rval |= (update_mouse() & 0x0f);
			int32_t stat = emu->get_mouse_button();
			if((stat & 0x01) == 0) {
				rval &= ~0x10; // Button LEFT
			}
			if((stat & 0x02) == 0) {
				rval &= ~0x20; // Button RIGHT
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

uint32_t MOUSE::check_mouse_data(bool is_send_data)
{
	// Do Not reply COM (0x40) : 20210627 K.O
	axisdata = axisdata & 0x0f;
	int32_t stat = emu->get_mouse_button();

	if(((stat & 0x01) == 0) && (trig_a)) {
		axisdata |= LINE_JOYPORT_A; // Button LEFT
	}
	if(((stat & 0x02) == 0) && (trig_b)) {
		axisdata |= LINE_JOYPORT_B; // Button RIGHT
	}
	if((d_joyport != nullptr) && (is_send_data)) {
		int _id = SIG_JOYPORT_DATA | ((port_num & 0x01) << 24)
			| SIG_JOYPORT_TYPE_MOUSE;
		d_joyport->write_signal(_id , axisdata, 0xffffffff);
	}
	return axisdata;
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
					sample_mouse_xy();

					axisdata = 0x0f;
					clear_event(this, event_timeout);
					
//					sample_mouse_xy(); // Sample next value.
					if(d_joyport != nullptr) {
						int com_d = SIG_JOYPORT_TYPE_MOUSE;
						com_d |= ((port_num & 1) << 24);
						// First, Set 0 to port
						d_joyport->write_signal(com_d | SIG_JOYPORT_COM, 0x00000000, 0xffffffff);
//						d_joyport->write_signal(com_d | SIG_JOYPORT_DATA, 0xffffffff, 0xffffffff);
						d_joyport->write_signal(com_d | SIG_JOYPORT_DATA, 0x00000000, 0xffffffff);
						// Second, read port value.
						uint8_t mouse_mask = d_joyport->read_signal(com_d | SIG_JOYPORT_MASK);
						uint8_t trig = (mouse_mask >> (port_num << 1)) & 0x03;
						uint8_t mask2 = (mouse_mask >> (port_num + 4)) & 0x01;
						trig_a = ((trig & 0x01) != 0) ? true : false;	
						trig_b = ((trig & 0x02) != 0) ? true : false;
						update_strobe(mask2, true);
						axisdata = ~(update_mouse());
						check_mouse_data(true);
					}
				} else { // on -> off
					phase = 0;
					dx = dy = 0;
					lx = ly = 0;
					strobe = false;
					axisdata = 0x0f;

					trig_a = false;
					trig_b = false;

					clear_event(this, event_timeout);
				}
			}
		}
		break;
	case SIG_MOUSE_QUERY:
		if(mouse_connected) {
			if(((data & mask) & 0x0c) == 0x04) {
				int num = (data & mask) & 1;
				if(num == port_num) {
					//axisdata = ~(update_mouse());
					check_mouse_data(true);
				}
			}
		}
		break;
	case SIG_MOUSE_DATA:
		if(mouse_connected) {
			uint8_t trig = data & 0x03;
			uint8_t mask2 = (data  & 0x04) >> 2;
			trig_a = ((trig & 0x01) != 0) ? true : false;	
			trig_b = ((trig & 0x02) != 0) ? true : false;
			update_strobe(mask2, false);
			axisdata = ~(update_mouse());
			check_mouse_data(true);
		}
		break;
	}
}

void MOUSE::sample_mouse_xy()
{
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
	emu->release_mouse_buffer(mouse_state);
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
		axisdata = 0x0f;
		break;
	case EVENT_MOUSE_SAMPLING:
		sample_mouse_xy();
		break;
	}
}
	
#define STATE_VERSION 3

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
	state_fio->StateValue(axisdata);
	
	state_fio->StateValue(phase);
	state_fio->StateValue(strobe);
	state_fio->StateValue(trig_a);
	state_fio->StateValue(trig_b);
	
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(lx);
	state_fio->StateValue(ly);
	

	state_fio->StateValue(event_timeout);
	state_fio->StateValue(event_sampling);

	return true;
}
}
