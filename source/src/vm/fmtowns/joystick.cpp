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
	mouse_data = 0x00;
	if(mouse_timeout_event >= 0) {
		cancel_event(this, mouse_timeout_event);
	}
	mouse_timeout_event = -1;
	update_config(); // Update MOUSE PORT.
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
	set_emulate_mouse();
	mouse_type = config.mouse_type;
	register_frame_event(this);
	
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
	mouse_state = emu->get_mouse_buffer();
	if(mouse_state != NULL) {
		dx += mouse_state[0];
		dy += mouse_state[1];
		if(dx < -255) {
			dx = -255;
		} else if(dx > 255) {
			dx = 255;
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
	rawdata = emu->get_joy_buffer();
	if(rawdata == NULL) return;
   
	for(ch = 0; ch < 2; ch++) {
		if(!emulate_mouse[ch]) { // Joystick
			val = rawdata[ch];
			retval = 0x00;	   
			if(val & 0x01) retval |= 0x01;
			if(val & 0x02) retval |= 0x02;
			if(val & 0x04) retval |= 0x04;
			if(val & 0x08) retval |= 0x08;
			if(val & 0x10) retval |= 0x10;
			if(val & 0x20) retval |= 0x20;
			if(val & 0x40) retval |= 0x40;  // RUN
			if(val & 0x80) retval |= 0x80;  // SELECT
//			out_debug_log(_T("JOY DATA[%d]=%02X"), ch, retval);
			joydata[ch] = retval;
		}
		// Note: MOUSE don't update at vsync.
	}
}
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		if(emulate_mouse[0]) {
			update_strobe(((data & 0x20) != 0));
		} else if(emulate_mouse[1]) {
			update_strobe(((data & 0x20) != 0));
		}
		mask = data;
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
			uint8_t rval = 0xb0;
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
				if((joydata[port_num] & 0x40) != 0) { // RUN = L+R
					retval = retval & ~0x0c; // LEFT + RIGHT
				} else {
					if((joydata[port_num] & 0x04) != 0) { // LEFT
						retval = retval & ~0x04; // LEFT
					} else if((joydata[port_num] & 0x08) != 0) { // RIGHT
						retval = retval & ~0x08; // RIGHT
					}				
				}
				if((joydata[port_num] & 0x80) != 0) { // RUN = UP+DOWN
					retval = retval & ~0x03; // FWD + BACK
				} else {
					if((joydata[port_num] & 0x01) != 0) { // UP
						retval = retval & ~0x01; // FWD
					} else if((joydata[port_num] & 0x02) != 0) { // DOWN
						retval = retval & ~0x02; // BACK
					}				
				}
			//}
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
		mouse_data = ly & 0x0f;
		break;
	}
}

void JOYSTICK::write_signal(int id, uint32_t data, uint32_t mask)
{
}	

void JOYSTICK::update_config(void)
{
	set_emulate_mouse();
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
	if(mouse_strobe != flag) {
		mouse_strobe = flag;
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
//			if(mouse_timeout_event >= 0) {
//				cancel_event(this, mouse_timeout_event);
//				mouse_timeout_event = -1;
//			}
		}
	}
}

uint32_t JOYSTICK::update_mouse()
{
	switch(mouse_phase) {
	case 1:
		mouse_data = (lx >> 1) & 0x0f;
		break;
	case 2:
		mouse_data = (lx >> 5) & 0x0f;
		break;
	case 3:
		mouse_data = (ly >> 1) & 0x0f;
		break;
	case 0:
		mouse_data = (ly >> 5) & 0x0f;
		break;
	}
	return mouse_data;
}

#define STATE_VERSION 2

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

	return true;
}

}
