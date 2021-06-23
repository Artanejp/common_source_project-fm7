/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/

#include "./joystick.h"
#include "./mouse.h"

namespace FMTOWNS {
	
void JOYSTICK::reset()
{
	mouse_mask = 0x00;
	update_config(); // Update MOUSE PORT.
	
	for(int i = 0; i < 2; i++) {
		if(connected_type[i] == SIG_JOYPORT_TYPE_2BUTTONS) {
			write_signals(&outputs_query, 1 << i);
		} else if(connected_type[i] == SIG_JOYPORT_TYPE_MOUSE) {
			write_signals(&outputs_query, (1 << i) | 0x04);
		}

	}
}

void JOYSTICK::initialize()
{
	joydata[0] = joydata[1] = 0x00;

	// Force reset pads.
	connected_type[0] = 0xffffffff;
	connected_type[1] = 0xffffffff;
	stat_com[0] = stat_com[1] = false;

}

void JOYSTICK::release()
{
}
	
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		if(mouse_mask != data) {
			mouse_mask = data;
			write_signals(&outputs_mask, data);
		}
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
		{
			uint8_t trig = (mouse_mask >> (port_num << 1)) & 0x03;
			uint8_t mask2 = (mouse_mask >> (port_num + 4)) & 0x01;
			retval = 0x0f;
			if(connected_type[port_num] == SIG_JOYPORT_TYPE_2BUTTONS) {
				write_signals(&outputs_query, 1 << port_num);
			} else if(connected_type[port_num] == SIG_JOYPORT_TYPE_MOUSE) {
				if(emulate_mouse[port_num]) {
					write_signals(&outputs_query, (1 << port_num) | 0x04);
				}
			} else { 
				// None Connected
				if(mask2 != 0) { // COM
					retval |= 0x40;
				}
				retval |= 0x30;
				return retval;
			}
			if((mask2 != 0) && (stat_com[port_num])) { // COM
				retval |= 0x40;
			}
			// Trigger independents from direction keys.
			if(((trig & 0x02) != 0) && ((joydata[port_num] & LINE_JOYPORT_B) == 0)) {
				retval = retval | 0x20;
			}
			if(((trig & 0x01) != 0) && ((joydata[port_num] & LINE_JOYPORT_A) == 0)) {
				retval = retval | 0x10;
			}
			//if((mask & (0x10 << port_num)) == 0) {
//				if((mask2 & 0x01) == 0) { // COM
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
//				}
			return retval;
		}
		break;
	default:
		break;
	}
	return 0xff;
}


void JOYSTICK::write_signal(int id, uint32_t data, uint32_t mask)
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
uint32_t JOYSTICK::read_signal(int id)
{
	int ch = (id >> 24) & 1;
	int bustype = id  & 0x300;
	int num = id & 0xff;
	uint32_t data = 0;
	switch(bustype) {
	case SIG_JOYPORT_DATA:
		data = joydata[ch];
		break;
	case SIG_JOYPORT_COM:
		data = (stat_com[ch]) ? 0xffffffff : 0;
		break;
	case SIG_JOYPORT_MASK:
		data = mouse_mask;
		break;
	}
	return data;
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
	
	write_signals(&outputs_mask, mouse_mask);
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
	
}

void JOYSTICK::set_emulate_mouse()
{
	switch(config.mouse_type & 0x03){
	case 1:
		emulate_mouse[0] = true;
		emulate_mouse[1] = false;
		if(d_mouse != nullptr) {
			d_mouse->write_signal(SIG_MOUSE_ENABLE, 0xfffffffe, 0xffffffff);
		}
		break;
	case 2:
		emulate_mouse[0] = false;
		emulate_mouse[1] = true;
		if(d_mouse != nullptr) {
			d_mouse->write_signal(SIG_MOUSE_ENABLE, 0xffffffff, 0xffffffff);
		}
		break;
	default:
		emulate_mouse[0] = false;
		emulate_mouse[1] = false;
		if(d_mouse != nullptr) {
			d_mouse->write_signal(SIG_MOUSE_ENABLE, 0x00000000, 0xffffffff);
		}
		break;
	}
}

#define STATE_VERSION 6

bool JOYSTICK::process_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(mouse_mask);
	state_fio->StateArray(joydata, sizeof(joydata), 1);
	state_fio->StateArray(connected_type, sizeof(connected_type), 1);
	
	state_fio->StateArray(emulate_mouse, sizeof(emulate_mouse), 1);
	state_fio->StateArray(stat_com, sizeof(stat_com), 1);

	return true;
}

}
