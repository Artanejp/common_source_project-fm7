/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns PAD ]

*/

#include "./joystick.h"

namespace FMTOWNS {

void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0x00;
	dx = dy = 0;
	lx = ly = 0;
	mouse_phase = 0;
	mouse_strobe = false;
	mouse_type = config.mouse_type;
	switch(mouse_type & 0x03){
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
	mouse_state = emu->get_mouse_buffer();
	mask = 0xff;
}

void JOYSTICK::initialize()
{
	rawdata = emu->get_joy_buffer();
	mouse_state = emu->get_mouse_buffer();
	emulate_mouse[0] = emulate_mouse[1] = false;
	joydata[0] = joydata[1] = 0x00;
	dx = dy = 0;
	lx = ly = -1;
	mouse_button = 0x00;
	mouse_timeout_event = -1;
	
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
			if(val & 0x40) retval |= 0x10;  // Button A'
			if(val & 0x80) retval |= 0x20;  // Button B'
			if(val & 0x40) retval |= 0x40;  // RUN
			if(val & 0x80) retval |= 0x80;  // SELECT
			joydata[ch] = retval;
		} else { // MOUSE
		}
	}
}
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		mask = data;
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
		if((mask & (0x10 << port_num)) != 0) {
			retval = (joydata[port_num] & 0x3f) | 0x40;
		} else {
			retval = (joydata[port_num] & 0x0f) | 0x30;
		}			
		if((joydata[port_num] & 0x40) != 0) { // RUN = L+R
			retval = retval & ~0x0c;
		}
		if((joydata[port_num] & 0x80) != 0) { // RUN = UP+DOWN
			retval = retval & ~0x03;
		}
		if(((joydata[port_num] & 0x40) != 0) && (mask & (0x01 << (port_num * 2)) != 0)) {
			retval = retval & ~0x10;
		}
		if(((joydata[port_num] & 0x40) != 0) && (mask & (0x02 << (port_num * 2)) != 0)) {
			retval = retval & ~0x20;
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
}

void JOYSTICK::write_signal(int id, uint32_t data, uint32_t mask)
{
}	

void JOYSTICK::update_config(void)
{
}

#define STATE_VERSION 1

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
	state_fio->StateValue(mouse_strobe);
	state_fio->StateValue(mouse_phase);
	state_fio->StateValue(mouse_data);
	//state_fio->StateValue(mouse_timeout_event);

	
	return true;
}

}
