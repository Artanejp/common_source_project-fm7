/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns PAD ]

*/

#include "./joystick.h"

namespace FMTOWNS {
#define EVENT_MOUSE_TIMEOUT 1
#define EVENT_MOUSE_SAMPLING 2
	
void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0x00;
	dx = dy = 0;
	lx = ly = 0;
	mouse_state = emu->get_mouse_buffer();
	mask = 0x00;
	mouse_type = -1; // Force update data.
	mouse_phase = 0;
	mouse_strobe = false;
	mouse_data = 0x0f;
	if(mouse_timeout_event >= 0) {
		cancel_event(this, mouse_timeout_event);
	}
	mouse_timeout_event = -1;
	update_config(); // Update MOUSE PORT.
	
	if(mouse_sampling_event >= 0) {
		cancel_event(this, mouse_sampling_event);
	}
	register_event(this, EVENT_MOUSE_SAMPLING, 8.0e3, true, &mouse_sampling_event);
}

void JOYSTICK::initialize()
{
	rawdata = emu->get_joy_buffer();
	mouse_state = emu->get_mouse_buffer();
	joydata[0] = joydata[1] = 0x00;
	dx = dy = 0;
	lx = ly = -1;
	mouse_button = 0x00;
	mouse_timeout_event = -1;
	mouse_sampling_event = -1;
	set_emulate_mouse();
	mouse_type = config.mouse_type;
//	register_frame_event(this);
}

void JOYSTICK::release()
{
}
	
void JOYSTICK::event_frame()
{
	int ch;
	int stat = 0x00;
	uint32_t retval = 0x00;
	uint32_t val;
#if 1
	mouse_state = emu->get_mouse_buffer();
	if(mouse_state != NULL) {
		dx += mouse_state[0];
		dy += mouse_state[1];
		if(dx < -127) {
			dx = -127;
		} else if(dx > 127) {
			dx = 127;
		}
		if(dy < -255) {
			dy = -255;
		} else if(dy > 255) {
			dy = 255;
		}
	}		
	if(mouse_state != NULL) {
		stat = mouse_state[2];
		mouse_button = 0x00;
		if((stat & 0x01) == 0) mouse_button |= 0x10; // left
		if((stat & 0x02) == 0) mouse_button |= 0x20; // right
	}
#endif
}
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		if(emulate_mouse[0]) {
//			update_strobe(((data & 0x10) != 0));
//			if((data & 0x10) != 0) {
			update_strobe(((data & 0x10) != 0));
//			}
		} else if(emulate_mouse[1]) {
			update_strobe(((data & 0x20) != 0));
		}
		mask = data;
		write_signals(&outputs_mask, data);
	}
}

uint32_t JOYSTICK::read_io8(uint32_t address)
{
	// ToDo: Implement 6 buttons pad. & mouse
	uint8_t retval = 0;
	uint8_t port_num = (address & 0x02) >> 1;
	uint8_t trig = (mask >> (address & 0x02)) & 0x03;
	switch(address) {
	case 0x04d0:
	case 0x04d2:
		if(emulate_mouse[port_num]) {
//			uint8_t rval = 0xb0;
			uint8_t rval = 0xf0;
			rval |= update_mouse() & 0x0f; 
			if((mouse_button & 0x10) != 0) {
				rval &= ~0x10; // Button LEFT
			}
			if((mouse_button & 0x20) != 0) {
				rval &= ~0x20; // Button RIGHT
			}
			retval = rval;
		} else {
			if((mask & (0x10 << port_num)) != 0) {
				retval = 0xff; // COM ON
			} else {
				retval = 0xbf; // COM OFF
			}			
			//if((mask & (0x10 << port_num)) == 0) {
			if((joydata[port_num] & 0x04) != 0) { // LEFT
				retval = retval & ~0x04; // LEFT
			} else if((joydata[port_num] & 0x08) != 0) { // RIGHT
				retval = retval & ~0x08; // RIGHT
			}				
			if((joydata[port_num] & 0x01) != 0) { // UP
				retval = retval & ~0x01; // FWD
			} else if((joydata[port_num] & 0x02) != 0) { // DOWN
				retval = retval & ~0x02; // BACK
			}				
			if(((trig & 0x01) != 0) && ((joydata[port_num] & 0x10) != 0)) { // TRIGGER1
				retval = retval & ~0x10;
			}
			if(((trig & 0x02) != 0) && ((joydata[port_num] & 0x20) != 0)) { // TRIGGER2
				retval = retval & ~0x20;
			}
		}
		return retval;
		break;
	default:
		break;
	}
	return 0x00;
}

void JOYSTICK::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_MOUSE_TIMEOUT:
		mouse_phase = 0;
		mouse_strobe = false;
		mouse_timeout_event = -1;
		dx = dy = lx = ly = 0;
		mouse_data = 0x0f;
		break;
	case EVENT_MOUSE_SAMPLING:
		mouse_state = emu->get_mouse_buffer();
		if(mouse_state != NULL) {
			dx += mouse_state[0];
			dy += mouse_state[1];
			if(dx < -127) {
				dx = -127;
			} else if(dx > 127) {
				dx = 127;
			}
			if(dy < -127) {
				dy = -127;
			} else if(dy > 127) {
				dy = 127;
			}
		}		
		if(mouse_state != NULL) {
			uint32_t stat = mouse_state[2];
			mouse_button = 0x00;
			if((stat & 0x01) == 0) mouse_button |= 0x10; // left
			if((stat & 0x02) == 0) mouse_button |= 0x20; // right
			break;
		}
	}
}

void JOYSTICK::write_signal(int id, uint32_t data, uint32_t _mask)
{
	uint32_t num = (id >> 12) & 1;
	uint32_t type = (id >> 8) & 15;
	uint32_t line = id & 0xff;
	//if(type != connected_type[num]) return;
	//out_debug_log(_T("SIGNAL SENT, NUM=%d TYPE=%d LINE=%d VALUE=%08X"), num, type << 8, line, data);
	if((data & _mask) != 0) {
		switch(line) {
		case SIG_JOYPORT_LINE_UP:
			joydata[num] |= 0x01;
			break;
		case SIG_JOYPORT_LINE_DOWN:
			joydata[num] |= 0x02;
			break;
		case SIG_JOYPORT_LINE_LEFT:
			joydata[num] |= 0x04;
			break;
		case SIG_JOYPORT_LINE_RIGHT:
			joydata[num] |= 0x08;
			break;
		case SIG_JOYPORT_LINE_A:
			joydata[num] |= 0x10;
			break;
		case SIG_JOYPORT_LINE_B:
			joydata[num] |= 0x20;
			break;
		}
	} else {
		switch(line) {
		case SIG_JOYPORT_LINE_UP:
			joydata[num] &= ~0x01;
			break;
		case SIG_JOYPORT_LINE_DOWN:
			joydata[num] &= ~0x02;
			break;
		case SIG_JOYPORT_LINE_LEFT:
			joydata[num] &= ~0x04;
			break;
		case SIG_JOYPORT_LINE_RIGHT:
			joydata[num] &= ~0x08;
			break;
		case SIG_JOYPORT_LINE_A:
			joydata[num] &= ~0x10;
			break;
		case SIG_JOYPORT_LINE_B:
			joydata[num] &= ~0x20;
			break;
		}
	}
}	

void JOYSTICK::update_config(void)
{
	uint32_t ntype[2] = {0};
	for(int i = 0; i < 2; i++) {
		switch(config.joystick_type) {
		case 0:
			ntype[i] = SIG_JOYPORT_TYPE_NULL;
			break;
		case 1:
			ntype[i] = SIG_JOYPORT_TYPE_2BUTTONS;
			break;
		case 2:
			ntype[i] = SIG_JOYPORT_TYPE_6BUTTONS;
			break;
		}
	}
	set_emulate_mouse();
	if(emulate_mouse[0]) {
		ntype[0] = SIG_JOYPORT_TYPE_MOUSE;
	}
	if(emulate_mouse[1]) {
		ntype[1] = SIG_JOYPORT_TYPE_MOUSE;
	}
	for(int i = 0; i < 2; i++) {
		if(connected_type[i] != ntype[i]) {
			write_signals(&outputs_enable[i], 1 << (ntype[i] >> 8));
		}
		connected_type[i] = ntype[i];
	}
	mouse_type = config.mouse_type;
}

void JOYSTICK::set_emulate_mouse()
{
	switch(config.mouse_type & 0x03){
	case 1:
		emulate_mouse[0] = true;
		emulate_mouse[1] = false;
		break;
	case 2:
		emulate_mouse[0] = false;
		emulate_mouse[1] = true;
		break;
	default:
		emulate_mouse[0] = false;
		emulate_mouse[1] = false;
		break;
	}
}

void JOYSTICK::update_strobe(bool flag)
{
	bool _bak = mouse_strobe;
	mouse_strobe = flag;
	if((_bak != flag)/* && (flag)*/) {
		if((mouse_phase == 0)) {
			// Sample data from MOUSE to registers.
			lx = -dx;
			ly = -dy;
			dx = 0;
			dy = 0;
			if(mouse_timeout_event >= 0) {
				cancel_event(this, mouse_timeout_event);
			}
			register_event(this, EVENT_MOUSE_TIMEOUT, 2000.0, false, &mouse_timeout_event);
		}
		mouse_phase++;
		if(mouse_phase >= 4) {
			mouse_phase = 0;
			if(mouse_timeout_event >= 0) {
				cancel_event(this, mouse_timeout_event);
				mouse_timeout_event = -1;
			}
		}
	}
}

uint32_t JOYSTICK::update_mouse()
{
	switch(mouse_phase) {
	case 1:
		mouse_data = (lx >> 0) & 0x0f;
		break;
	case 2:
		mouse_data = (lx >> 4) & 0x0f;
		break;
	case 3:
		mouse_data = (ly >> 0) & 0x0f;
		break;
	case 0:
		mouse_data = (ly >> 4) & 0x0f;
		break;
	}
//	out_debug_log(_T("READ MOUSE DATA=%01X PHASE=%d STROBE=%d"), mouse_data, mouse_phase, (mouse_strobe) ? 1 : 0);
	return mouse_data;
}

#define STATE_VERSION 4

bool JOYSTICK::process_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(mask);
	state_fio->StateArray(joydata, sizeof(joydata), 1);
	state_fio->StateArray(connected_type, sizeof(connected_type), 1);
	
	state_fio->StateArray(emulate_mouse, sizeof(emulate_mouse), 1);
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(lx);
	state_fio->StateValue(ly);
	state_fio->StateValue(mouse_button);
	state_fio->StateValue(mouse_type);
	state_fio->StateValue(mouse_strobe);
	state_fio->StateValue(mouse_phase);
	state_fio->StateValue(mouse_data);
	state_fio->StateValue(mouse_timeout_event);
	state_fio->StateValue(mouse_sampling_event);

	return true;
}

}
