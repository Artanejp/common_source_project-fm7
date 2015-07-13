/*
 * FM-7 Main I/O [joystick.cpp]
 *  - Joystick
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jun 16, 2015 : Initial, split from sound.cpp.
 *
 */
#include "fm7_mainio.h"
#include "./joystick.h"
#include "../../config.h"

JOYSTICK::JOYSTICK(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
	rawdata = NULL;
	mouse_state = NULL;
}

JOYSTICK::~JOYSTICK()
{
}

void JOYSTICK::initialize()
{
	rawdata = p_emu->joy_buffer();
	mouse_state = p_emu->mouse_buffer();
	emulate_mouse[0] = emulate_mouse[1] = false;
	joydata[0] = joydata[1] = 0xff;
	dx = dy = 0;
	lx = ly = -1;
	mouse_button = 0x00;
	mouse_timeout_event = -1;
	port_a_val = 0;
	port_b_val = 0;
}

void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0xff;
	dx = dy = 0;
	lx = ly = 0;
	mouse_phase = 0;
	mouse_strobe = false;
	mouse_type = config.device_type;
	if(mouse_type == 1) emulate_mouse[0] = true;
	if(mouse_type == 2) emulate_mouse[1] = true;
	mouse_state = p_emu->mouse_buffer();
}

void JOYSTICK::event_frame()
{
	int ch;
	int stat;
	uint32 retval = 0xff;
	uint32 val;
 
	if(mouse_state != NULL) {
		dx += (mouse_state[0] / 2);
		dy += (mouse_state[1] / 2);
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
		stat = mouse_state[2];
	}		
	mouse_button = 0x00;
	if((stat & 0x01) == 0) mouse_button |= 0x10; // left
	if((stat & 0x02) == 0) mouse_button |= 0x20; // right
	rawdata = p_emu->joy_buffer();
	if(rawdata == NULL) return;
   
	for(ch = 0; ch < 2; ch++) {
		if(!emulate_mouse[ch]) { // Joystick
			val = rawdata[ch];
			retval = 0xff;	   
			if(val & 0x01) retval &= ~0x01;
			if(val & 0x02) retval &= ~0x02;
			if(val & 0x04) retval &= ~0x04;
			if(val & 0x08) retval &= ~0x08;
			if(val & 0x10) retval &= ~0x10;
			if(val & 0x20) retval &= ~0x20;
			if(val & 0x40) retval &= ~0x10;  // Button A'
			if(val & 0x80) retval &= ~0x20;  // Button B'
			retval |= 0xc0;
			joydata[ch] = retval;
		} else { // MOUSE
		   
		}
	}
}


uint32 JOYSTICK::update_mouse(uint32 mask)
{
	uint32 button = mouse_button;
	switch(mouse_phase) {
			case 1:
				mouse_data = lx & 0x0f;
				break;
			case 2:
				mouse_data = (lx >> 4) & 0x0f;
				break;
			case 3:
				mouse_data = ly & 0x0f;
				break;
			case 0:
				mouse_data = (ly >> 4) & 0x0f;
				break;
	}
	//mouse_button = 0x00;
	return (mouse_data | (mask & button) | 0xc0);
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

void JOYSTICK::update_strobe(bool flag)
{
	if(mouse_strobe != flag) {
		mouse_strobe = flag;
		if(mouse_phase == 0) {
			lx = -dx;
			ly = -dy;
			dx = 0;
			dy = 0;
			register_event(this, EVENT_MOUSE_TIMEOUT, 2000.0, false, &mouse_timeout_event);
		}
		mouse_phase++;
		if(mouse_phase >= 4) mouse_phase = 0;
	}
}

uint32 JOYSTICK::read_data8(uint32 addr)
{
	uint32 val = 0xff;
	uint32 opnval;
	bool flag = false;
	//if(opn == NULL) return 0xff;
	
	switch(addr) {
		case 0:
			//opn->write_io8(0, 0x0f);
			//opnval = opn->read_io8(1);
			opnval = port_b_val;
			if(emulate_mouse[0]) {
				if((opnval & 0xc0) == 0x00) {
					return update_mouse((opnval & 0x03) << 4);
				}
			} else if(emulate_mouse[1]) {
				if((opnval & 0xc0) == 0x40) {
					return update_mouse((opnval & 0x0c) << 2);
				}
			}
			
			switch(opnval & 0xf0) {
				case 0x20:
					val = joydata[0];
					break;
				case 0x50:
					val = joydata[1];
					break;
			}
			break;
	}
	return val;
}

void JOYSTICK::write_data8(uint32 addr, uint32 data)
{
}

void JOYSTICK::write_signal(int id, uint32 data, uint32 mask)
{
	uint32 val = data & mask;
	bool val_b = (val != 0);
	switch(id) {
		case FM7_JOYSTICK_EMULATE_MOUSE_0:
			emulate_mouse[0] = val_b;
			break;
		case FM7_JOYSTICK_EMULATE_MOUSE_1:
			emulate_mouse[1] = val_b;
			break;
		case FM7_JOYSTICK_MOUSE_STROBE:
			port_b_val = data;
			if(emulate_mouse[0]) {
				update_strobe(((data & 0x10) != 0));
			} else 	if(emulate_mouse[1]) {
				update_strobe(((data & 0x20) != 0));
			}
			break;
	   
	}
}

void JOYSTICK::update_config(void)
{
	if(mouse_type == config.device_type) return;
	mouse_type = config.device_type;
	switch(config.device_type){
	case 1:
		emulate_mouse[0] = true;
		emulate_mouse[1] = false;
		this->reset();
		break;
	case 2:
		emulate_mouse[0] = false;
		emulate_mouse[1] = true;
		this->reset();
		break;
	default:
		emulate_mouse[0] = false;
		emulate_mouse[1] = false;
		this->reset();
		break;
	}
}
#define STATE_VERSION 2
void JOYSTICK::save_state(FILEIO *state_fio)
{
	int ch;
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	// Version 1
	for(ch = 0; ch < 2; ch++) {
		state_fio->FputBool(emulate_mouse[ch]);
		state_fio->FputUint32(joydata[ch]);
	}
	// After Version2.
	state_fio->FputInt32(dx);
	state_fio->FputInt32(dy);
	state_fio->FputInt32(lx);
	state_fio->FputInt32(ly);
	state_fio->FputUint32(mouse_button);
	state_fio->FputBool(mouse_strobe);
	state_fio->FputUint32(mouse_phase);
	state_fio->FputUint32(mouse_data);
	//state_fio->FputInt32(mouse_timeout_event);
	// Version 3
}

bool JOYSTICK::load_state(FILEIO *state_fio)
{
	uint32 version = state_fio->FgetUint32();
	uint32 devid = state_fio->FgetUint32();
	bool stat = false;
	int ch;
	if(version >= 1) {
		for(ch = 0; ch < 2; ch++) {
			state_fio->FputBool(emulate_mouse[ch]);
			state_fio->FputUint32(joydata[ch]);
		}
		if(version == 1) stat = true;
	}
	// After version 2.
	dx = state_fio->FgetInt32();
	dy = state_fio->FgetInt32();
	lx = state_fio->FgetInt32();
	ly = state_fio->FgetInt32();
	mouse_button = state_fio->FgetUint32();
	mouse_strobe = state_fio->FgetBool();
	mouse_phase = state_fio->FgetUint32();
	mouse_data = state_fio->FgetUint32();
	//mouse_timeout_event = state_fio->FgetInt32();
	if(version == 2) stat = true; 
	return stat;
}
		
	
