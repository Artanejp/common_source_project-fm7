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
#include "../vm.h"
#include "fm7_mainio.h"
#include "./joystick.h"
#include "../../config.h"
#include "../../emu.h"

JOYSTICK::JOYSTICK(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
	rawdata = NULL;
	mouse_state = NULL;
	lpt_type = 0;
	set_device_name(_T("JOYSTICK"));
}

JOYSTICK::~JOYSTICK()
{
}

void JOYSTICK::initialize()
{
	rawdata = p_emu->get_joy_buffer();
	mouse_state = p_emu->get_mouse_buffer();
	emulate_mouse[0] = emulate_mouse[1] = false;
	joydata[0] = joydata[1] = 0xff;
	dx = dy = 0;
	lx = ly = -1;
	mouse_button = 0x00;
	mouse_timeout_event = -1;
	lpmask = 0x00;
	lpt_type = config.printer_device_type;
	port_b_val = 0;
}

void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0xff;
	lpt_type = config.printer_device_type;
#if !defined(_FM8)
	dx = dy = 0;
	lx = ly = 0;
	mouse_phase = 0;
	mouse_strobe = false;
	mouse_type = config.device_type;
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
	mouse_state = p_emu->get_mouse_buffer();
#endif	
}

void JOYSTICK::event_frame()
{
	int ch;
	int stat = 0x00;
	uint32_t retval = 0xff;
	uint32_t val;
#if !defined(_FM8)
	mouse_state = p_emu->get_mouse_buffer();
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
#endif	
	rawdata = p_emu->get_joy_buffer();
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


uint32_t JOYSTICK::update_mouse(uint32_t mask)
{
#if !defined(_FM8)
	uint32_t button = mouse_button;
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
#else
	return 0x00;
#endif	
}

void JOYSTICK::event_callback(int event_id, int err)
{
#if !defined(_FM8)
	switch(event_id) {
	case EVENT_MOUSE_TIMEOUT:
		mouse_phase = 0;
		mouse_strobe = false;
		mouse_timeout_event = -1;
		dx = dy = lx = ly = 0;
		mouse_data = ly & 0x0f;
		break;
	}
#endif	
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

uint32_t JOYSTICK::read_data8(uint32_t addr)
{
	uint32_t val = 0xff;
	uint32_t opnval;
	//if(opn == NULL) return 0xff;
	
	switch(addr) {
#if !defined(_FM8)		
	case 0: // OPN
			//opn->write_io8(0, 0x0f);
			//opnval = opn->read_io8(1);
			opnval = (uint32_t)port_b_val;
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
					if(lpt_type != 1) val = joydata[0];
					break;
				case 0x50:
					if(lpt_type != 2) val = joydata[1];
					break;
			}
			break;
#endif			
	case 2: // Get Printer Joystick (CH0)
	case 3: // Get Printer Joystick (CH1)
		int ch = addr - 1;
		if(lpt_type == ch) {
			uint8_t raw = rawdata[ch - 1];
			bool f = false;
			f |= ((raw & 0x08) && !(lpmask & 0x01));	
			f |= ((raw & 0x04) && !(lpmask & 0x02));	
			f |= ((raw & 0x01) && !(lpmask & 0x04));	
			f |= ((raw & 0x02) && !(lpmask & 0x08));	
			f |= ((raw & 0x20) && !(lpmask & 0x10));	
			f |= ((raw & 0x10) && !(lpmask & 0x20));
			if(f) val = 0x00;
		}
	}
	return val;
}

void JOYSTICK::write_data8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x00ff) {
	case 1: // JOYSTICK PRINTER(ch1)
		if((lpt_type == 1) || (lpt_type == 2)) {
	   		lpmask = data & 0x3f;
		}
		break;
	}		
}

void JOYSTICK::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint32_t val = data & mask;
	bool val_b = (val != 0);
	switch(id) {
		case FM7_JOYSTICK_EMULATE_MOUSE_0:
			emulate_mouse[0] = val_b;
			break;
		case FM7_JOYSTICK_EMULATE_MOUSE_1:
			emulate_mouse[1] = val_b;
			break;
		case FM7_JOYSTICK_MOUSE_STROBE:
			port_b_val = (uint8_t)data;
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
#if !defined(_FM8)
	if(mouse_type == (uint32_t)config.device_type) return;
	mouse_type = config.device_type;
	switch(mouse_type & 0x03){
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
#endif	
}
#define STATE_VERSION 4
void JOYSTICK::save_state(FILEIO *state_fio)
{
	int ch;
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log(_T("Save State: JOYSTICK: id=%d ver=%d\n"), this_device_id, STATE_VERSION);
	// Version 1
	for(ch = 0; ch < 2; ch++) {
#if !defined(_FM8)
		state_fio->FputBool(emulate_mouse[ch]);
#endif		
		state_fio->FputUint32_BE(joydata[ch]);
	}
	// After Version2.
#if !defined(_FM8)
	state_fio->FputInt32_BE(dx);
	state_fio->FputInt32_BE(dy);
	state_fio->FputInt32_BE(lx);
	state_fio->FputInt32_BE(ly);
	state_fio->FputUint32_BE(mouse_button);
	state_fio->FputBool(mouse_strobe);
	state_fio->FputUint32_BE(mouse_phase);
	state_fio->FputUint32_BE(mouse_data);
	//state_fio->FputInt32(mouse_timeout_event);
#endif	
	// Version 3
	state_fio->FputUint8(lpmask);
	// Version 4
	state_fio->FputUint8(port_b_val);
}

bool JOYSTICK::load_state(FILEIO *state_fio)
{
	uint32_t version = state_fio->FgetUint32_BE();
	int32_t devid = state_fio->FgetInt32_BE();
	bool stat = false;
	int ch;
	this->out_debug_log(_T("Load State: JOYSTICK: id=%d ver=%d\n"), devid, version);
	if(devid != this_device_id) return stat;
	if(version >= 1) {
		for(ch = 0; ch < 2; ch++) {
#if !defined(_FM8)
			emulate_mouse[ch] = state_fio->FgetBool();
#endif			
			joydata[ch] = state_fio->FgetUint32_BE();
		}
		//if(version == 1) stat = true;
	}
#if !defined(_FM8)
	// After version 2.
	dx = state_fio->FgetInt32_BE();
	dy = state_fio->FgetInt32_BE();
	lx = state_fio->FgetInt32_BE();
	ly = state_fio->FgetInt32_BE();
	mouse_button = state_fio->FgetUint32_BE();
	mouse_strobe = state_fio->FgetBool();
	mouse_phase = state_fio->FgetUint32_BE();
	mouse_data = state_fio->FgetUint32_BE();
	//mouse_timeout_event = state_fio->FgetInt32();
#endif	
	// V3
	lpmask = state_fio->FgetUint8();
	lpt_type = config.printer_device_type;
	// V4
	port_b_val = state_fio->FgetUint8();
	if(version == STATE_VERSION) stat = true; 
	return stat;
}
		
	
