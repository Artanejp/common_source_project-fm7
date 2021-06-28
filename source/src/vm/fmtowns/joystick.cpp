/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/

#include "./joystick.h"
#include "./joypad.h"
#include "./mouse.h"

namespace FMTOWNS {
	
void JOYSTICK::reset()
{
	mouse_mask = 0x00;
	update_config(); // Update MOUSE PORT.
	
	for(int i = 0; i < 2; i++) {
		send_signals(i, 0x00);
	}
}

void JOYSTICK::initialize()
{
	joydata[0] = joydata[1] = 0x7f;

	// Force reset pads.
	connected_type[0] = 0xffffffff;
	connected_type[1] = 0xffffffff;
	stat_com[0] = stat_com[1] = true;

}

void JOYSTICK::release()
{
}

void JOYSTICK::send_signals(int ch, uint32_t data)
{
	uint32_t _d = (data >> (ch + 2)) & 0x04; // COM
	_d |= ((data >> (ch << 1)) & 0x03); // TRIG
	if(connected_type[ch] == SIG_JOYPORT_TYPE_2BUTTONS) {
		if(d_joypad[ch] != nullptr) {
			d_joypad[ch]->write_signal(SIG_JOYPAD_SELECT_BUS, _d, 0x07);
		}
	} else if(connected_type[ch] == SIG_JOYPORT_TYPE_MOUSE) {
		if(d_mouse != nullptr) {
			d_mouse->write_signal(SIG_MOUSE_DATA, _d, 0x07);
		}
	}
	
}
	
void JOYSTICK::write_io8(uint32_t address, uint32_t data)
{
	// ToDo: Mouse
	if(address == 0x04d6) {
		if(mouse_mask != data) {
			mouse_mask = data;
			for(int ch = 0; ch < 2; ch++) {
				send_signals(ch, data);
			}
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
			retval |= (mask2 << 6);
			retval |= ((trig << 4) & 0x30);
			if(connected_type[port_num] == SIG_JOYPORT_TYPE_2BUTTONS) {
				if(d_joypad[port_num] != nullptr) {
					d_joypad[port_num]->write_signal(SIG_JOYPAD_QUERY, (1 << port_num) | 0x00, 0x0f);
				}
			} else if(connected_type[port_num] == SIG_JOYPORT_TYPE_MOUSE) {
//				if(emulate_mouse[port_num]) {
					if(d_mouse != nullptr) {
						d_mouse->write_signal(SIG_MOUSE_QUERY, (1 << port_num) | 0x04, 0x0f);
					}
//				}
			} else { 
				// None Connected
				return retval;
			}

			uint8_t _j = joydata[port_num];
			_j = (~(_j) & 0x3f) | 0x40;
			if(!(stat_com[port_num])) { // COM
				_j &= ~0x40;
			}
			retval &= _j;
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
	int bustype = id  & 0xf00;
	int num = id & 0xff;
	//out_debug_log(_T("SIGNAL SENT, CH=%d TYPE=%d  VALUE=%08X"), ch, num, data);
//	if(num == connected_type[ch]) {
		switch(bustype) {
		case SIG_JOYPORT_DATA:
			joydata[ch] = data & 0x3f;
			break;
		case SIG_JOYPORT_COM:
			stat_com[ch] = ((data & mask) != 0) ? true : false;
			break;
		}
//	}
	//if(type != connected_type[num]) return;
}	
uint32_t JOYSTICK::read_signal(int id)
{
	int ch = (id >> 24) & 1;
	int bustype = id  & 0xf00;
	int num = id & 0xff;
	uint32_t data = 0;
	switch(bustype) {
	case SIG_JOYPORT_RAWREG:
		data = mouse_mask & 0xff;
		break;
	case SIG_JOYPORT_DATA:
		{
			uint8_t trig  = ((mouse_mask >> (ch << 1)) & 0x03) << 4; 
			uint8_t __com = ((mouse_mask >> 1) & 0x10) << 2;
			data = data & (trig | __com | 0x0f);
			data = (joydata[ch] & 0x3f) | ((stat_com[ch]) ? 0x40 : 0x00);
		}
		break;
	case SIG_JOYPORT_COM:
		data = (stat_com[ch]) ? 0xffffffff : 0;
		break;
	case SIG_JOYPORT_MASK:
		data  = (mouse_mask >> (ch << 1)) & 0x03; 
		data |= ((mouse_mask >> (ch + 2)) & 0x04);
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
	} else if(emulate_mouse[1]) {
		ntype[1] = SIG_JOYPORT_TYPE_MOUSE;
	}
	
	for(int i = 0; i < 2; i++) {
		stat_com[i] = (((mouse_mask >> (i + 2)) & 0x04) != 0) ? true : false;
		uint32_t _d = (mouse_mask >> (i + 2)) & 0x04; // COM
		_d |= ((mouse_mask >> (i << 1)) & 0x03); // TRIG
		
		switch(ntype[i]) {
			// Already disabled a mouse.
		case SIG_JOYPORT_TYPE_2BUTTONS:
		case SIG_JOYPORT_TYPE_6BUTTONS:
			connected_type[i] = SIG_JOYPORT_TYPE_2BUTTONS;
			// Disable mouse
			if(d_mouse != nullptr) {
				d_mouse->write_signal(SIG_MOUSE_ENABLE, (0x00000000 | (i & 1)) , 0xffffffff);
				d_mouse->write_signal(SIG_MOUSE_DATA, 0x07, 0x07);
			}
			// Enable Joypad
			if(d_joypad[i] != nullptr) {
				d_joypad[i]->write_signal(SIG_JOYPAD_ENABLE,
										(1 << SIG_JOYPORT_TYPE_2BUTTONS),
										0x0f);
				d_joypad[i]->write_signal(SIG_JOYPAD_SELECT_BUS, _d, 0x07);
			}
			break;
		case SIG_JOYPORT_TYPE_MOUSE:
			// Already disabled a joypad.
			connected_type[i] = SIG_JOYPORT_TYPE_MOUSE;
			// Disable Joypad
			if(d_joypad[i] != nullptr) {
				d_joypad[i]->write_signal(SIG_JOYPAD_ENABLE,
										0,
										0x0f);
				d_joypad[i]->write_signal(SIG_JOYPAD_SELECT_BUS, 0x07, 0x07);
			}
			// Enable mouse
			if(d_mouse != nullptr) {
				d_mouse->write_signal(SIG_MOUSE_ENABLE, (0xfffffffe | (i & 1)) , 0xffffffff);
				d_mouse->write_signal(SIG_MOUSE_DATA, _d, 0x07);
			}
			break;
		default:
			connected_type[i] = SIG_JOYPORT_TYPE_NULL;
			// Disable mouse
			if(d_mouse != nullptr) {
				d_mouse->write_signal(SIG_MOUSE_ENABLE, (0x00000000 | (i & 1)) , 0xffffffff);
				d_mouse->write_signal(SIG_MOUSE_DATA, 0x07, 0x07);
			}
			if(d_joypad[i] != nullptr) {
				d_joypad[i]->write_signal(SIG_JOYPAD_ENABLE,
										0x00,
										0x0f);
				d_joypad[i]->write_signal(SIG_JOYPAD_SELECT_BUS, 0x07, 0x07);
			}
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
