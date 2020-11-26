/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/

#include "./joystick.h"

namespace FMTOWNS {
#define EVENT_MOUSE_TIMEOUT 1
#define EVENT_MOUSE_SAMPLING 2
	
void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0x00;
	stat_com[0] = stat_com[1] = false;
	dx = dy = 0;
	lx = ly = 0;
	mouse_state = emu->get_mouse_buffer();
	mask = 0x00;
	mouse_type = -1; // Force update data.
	mouse_phase = 0;
	mouse_strobe = false;
	mouse_data = 0x00;
	clear_event(this, mouse_timeout_event);

	// Force reset pads.
	connected_type[0] = 0xffffffff;
	connected_type[1] = 0xffffffff;
	
	update_config(); // Update MOUSE PORT.
	
//	if(mouse_sampling_event >= 0) {
//		cancel_event(this, mouse_sampling_event);
//	}
//	register_event(this, EVENT_MOUSE_SAMPLING, 16.7e3, true, &mouse_sampling_event);
}

void JOYSTICK::initialize()
{
	rawdata = emu->get_joy_buffer();
	mouse_state = emu->get_mouse_buffer();
	joydata[0] = joydata[1] = 0x00;
	dx = dy = 0;
	lx = ly = 0;
	mouse_button = 0x00;
	mouse_timeout_event = -1;
	mouse_sampling_event = -1;
	set_emulate_mouse();
	mouse_type = config.mouse_type;
	register_frame_event(this);
}

void JOYSTICK::release()
{
}
	
void JOYSTICK::event_frame()
{
	event_callback(EVENT_MOUSE_SAMPLING, 0);
}

void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		if(emulate_mouse[0]) {
			update_strobe(((data & 0x10) != 0));
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
	switch(address) {
	case 0x04d0:
	case 0x04d2:
		if(emulate_mouse[port_num]) {
			uint8_t trig = (mask >> (port_num << 1)) & 0x03;
			uint8_t mask2 = (mask >> (port_num + 4)) & 0x01;
			uint8_t rval = 0x70;
			rval |= (update_mouse() & 0x0f);
			mouse_state = emu->get_mouse_buffer();
			if(mouse_state != NULL) {
				uint32_t stat = mouse_state[2];
				mouse_button = 0x00;
				if((stat & 0x01) == 0) mouse_button |= 0x10; // left
				if((stat & 0x02) == 0) mouse_button |= 0x20; // right
			}
			if((mask2 & 0x01) == 0) { // COM
				rval = rval & ~0x40;
			}
			if((trig & 0x02) == 0) { // TRIG2
				rval = rval & ~0x20;
			}
			if((trig & 0x01) == 0) { // TRIG1
				rval = rval & ~0x10;
			}				
			if((mouse_button & 0x10) != 0) {
				rval &= ~0x10; // Button LEFT
			}
			if((mouse_button & 0x20) != 0) {
				rval &= ~0x20; // Button RIGHT
			}
			retval = rval;
		} else {
			write_signals(&outputs_query, 1 << (port_num + 0));
			uint8_t trig = (mask >> (port_num << 1)) & 0x03;
			uint8_t mask2 = (mask >> (port_num + 4)) & 0x01;
			retval = 0x7f;
			if((mask2 & 0x01) == 0) { // COM
				retval = retval & ~0x40;
			}
			if((trig & 0x02) == 0) { // TRIG2
				retval = retval & ~0x20;
			}
			if((trig & 0x01) == 0) { // TRIG1
				retval = retval & ~0x10;
			}				
			if(connected_type[port_num] == SIG_JOYPORT_TYPE_NULL) {
				// None Connected
				return retval;
			}
			// Trigger independents from direction keys.
			if((joydata[port_num] & LINE_JOYPORT_B) != 0) {
				retval = retval & ~0x20;
			}
			if((joydata[port_num] & LINE_JOYPORT_A) != 0) {
				retval = retval & ~0x10;
			}
			//if((mask & (0x10 << port_num)) == 0) {
			if((joydata[port_num] & LINE_JOYPORT_LEFT) != 0) { // LEFT
				retval = retval & ~0x04; // LEFT
			}
			if((joydata[port_num] & LINE_JOYPORT_RIGHT) != 0) { // RIGHT
				retval = retval & ~0x08; // RIGHT
			}				
			if((joydata[port_num] & LINE_JOYPORT_UP) != 0) { // UP
				retval = retval & ~0x01; // FWD
			}
			if((joydata[port_num] & LINE_JOYPORT_DOWN) != 0) { // DOWN
				retval = retval & ~0x02; // BACK
			}
		}
		return retval;
		break;
	default:
		break;
	}
	return 0xff;
}

void JOYSTICK::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_MOUSE_TIMEOUT:
		mouse_phase = 0;
		mouse_timeout_event = -1;
		dx = dy = 0;
		lx = ly = 0;
		mouse_data = 0x00;
		break;
	case EVENT_MOUSE_SAMPLING:
		if((emulate_mouse[0]) || (emulate_mouse[1])) {
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
		}
		break;
	}
}

void JOYSTICK::write_signal(int id, uint32_t data, uint32_t _mask)
{
	int ch = (id >> 24) & 1;
	int bustype = id  & 0x300;
	int num = id & 0xff;
	//out_debug_log(_T("SIGNAL SENT, CH=%d TYPE=%d  VALUE=%08X"), ch, num, data);
	if(num == connected_type[ch]) {
		switch(bustype) {
		case SIG_JOYPORT_DATA:
			joydata[ch] = data;
			break;
		case SIG_JOYPORT_COM:
			stat_com[ch] = ((data & mask) != 0) ? true : false;
			break;
		}
	}
	//if(type != connected_type[num]) return;
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
//		if(connected_type[i] != ntype[i]) {
			write_signals(&outputs_enable[i], 1 << ntype[i]);
//		}
		switch(ntype[i]) {
		case SIG_JOYPORT_TYPE_2BUTTONS:
		case SIG_JOYPORT_TYPE_6BUTTONS:
			connected_type[i] = SIG_JOYPORT_TYPE_2BUTTONS;
			break;
		default:
			connected_type[i] = ntype[i];
			break;
		}
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
	if((_bak != mouse_strobe)/* && (flag)*/) {
		if((mouse_phase == 0)) {
			// Sample data from MOUSE to registers.
			lx = -dx;
			ly = -dy;
			dx = 0;
			dy = 0;
			clear_event(this, mouse_timeout_event);
			register_event(this, EVENT_MOUSE_TIMEOUT, 600.0, false, &mouse_timeout_event);
			if(mouse_strobe) {
				mouse_phase = 1; // SYNC from MAME 0.225. 20201126 K.O
			}
		}
		mouse_phase++;
//		if(mouse_phase > 5) {
//			mouse_phase = 0;
//			mouse_strobe = false;
//			clear_event(this, mouse_timeout_event);
//		}
	}
}

uint32_t JOYSTICK::update_mouse()
{
	switch(mouse_phase) {
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
		break;
	default:
		mouse_data = 0;
		break;
	}
//	out_debug_log(_T("READ MOUSE DATA=%01X PHASE=%d STROBE=%d"), mouse_data, mouse_phase, (mouse_strobe) ? 1 : 0);
	return mouse_data;
}

#define STATE_VERSION 5

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
	state_fio->StateArray(stat_com, sizeof(stat_com), 1);
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
