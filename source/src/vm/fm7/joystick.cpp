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
#include "../ym2203.h"
#include "./joystick.h"
#include "../../config.h"
#include "../../emu.h"

JOYSTICK::JOYSTICK(VM_TEMPLATE* parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	rawdata = NULL;
	mouse_state = NULL;
	lpt_type = 0;
	opn = NULL;
	set_device_name(_T("JOYSTICK"));
}

JOYSTICK::~JOYSTICK()
{
}

void JOYSTICK::initialize()
{
	rawdata = emu->get_joy_buffer();
	mouse_state = emu->get_mouse_buffer();
	emulate_mouse[0] = emulate_mouse[1] = false;
	joydata[0] = joydata[1] = 0xff;
	dx = dy = 0;
	lx = ly = -1;
	mouse_button = 0x00;
	mouse_timeout_event = -1;
	lpmask = 0x00;
	lpt_type = config.printer_type;
	port_b_val = 0;
	register_frame_event(this);
	//register_vline_event(this);
}

void JOYSTICK::reset()
{
	joydata[0] = joydata[1] = 0xff;
	lpt_type = config.printer_type;
#if !defined(_FM8)
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
#endif	
	if(opn != NULL) {
		opn->write_signal(SIG_YM2203_PORT_A, 0xff, 0xff);
	}
}

void JOYSTICK::event_frame()
{
	int ch;
	int stat = 0x00;
	uint32_t retval = 0xff;
	uint32_t val;
#if !defined(_FM8)
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
#endif	
	rawdata = emu->get_joy_buffer();
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
		if((mouse_phase == 0)) {
			lx = -dx;
			ly = -dy;
			dx = 0;
			dy = 0;
			register_event(this, EVENT_MOUSE_TIMEOUT, 2000.0, false, &mouse_timeout_event);
		}
		{
			mouse_phase++;
			if(mouse_phase >= 4) {
				mouse_phase = 0;
				//cancel_event(this, mouse_timeout_event);
			}
		}
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
	if(mouse_type == (uint32_t)config.mouse_type) return;
	mouse_type = config.mouse_type;
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

bool JOYSTICK::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}

#if !defined(_FM8)
	state_fio->StateArray(emulate_mouse, sizeof(emulate_mouse), 1);
#endif		
	state_fio->StateArray(joydata, sizeof(emulate_mouse), 1);

#if !defined(_FM8)
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(lx);
	state_fio->StateValue(ly);
	state_fio->StateValue(mouse_button);
	state_fio->StateValue(mouse_strobe);
	state_fio->StateValue(mouse_phase);
	state_fio->StateValue(mouse_data);
	//state_fio->StateValue(mouse_timeout_event);
#endif	
	// Version 3
	state_fio->StateValue(lpmask);
	// Version 4
	state_fio->StateValue(port_b_val);

	return true;
}
void JOYSTICK::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
}

bool JOYSTICK::load_state(FILEIO *state_fio)
{
	bool mb = decl_state(state_fio, true);
	if(!mb) return false;
	return true;
}
		
	
