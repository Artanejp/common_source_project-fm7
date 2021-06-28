/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.00.26 -
    History : 2020.09.26 Separate from joystick.cpp/joystick.h .
	[ Towns PAD ]

*/

#include "./joystick.h"
#include "./joypad.h"

namespace FMTOWNS {
void JOYPAD::initialize()
{
	DEVICE::initialize();
	_TCHAR tmps[128] = {0};
	my_stprintf_s(tmps, sizeof(tmps), _T("FM-Towns JOY PAD #%d"), pad_num);
	set_device_name(_T("%s"), tmps);

	sel_line = false;
	trig_a = false;
	trig_b = false;
}

void JOYPAD::reset()
{
	query_joystick(true);
}


void JOYPAD::query_joystick(bool _force)
{
	// enabled
	bool __sigf = _force;
	const uint32_t* rawdata = emu->get_joy_buffer();
	uint32_t stat = 0;
	if((rawdata != nullptr) && (enabled)) {
		__sigf = true;
		uint32_t d = rawdata[pad_num];
//		out_debug_log(_T("DATA: %08X"), d);
		if((type_6buttons) && (sel_line)) { // 6Buttons Multiplied
			// If COM == 1 THEN CHECK BUTTONS
			// 6Buttons PAD is seems to be this schematic:
			// http://www.awa.or.jp/home/shimojo/towpadx.htm
			// RIGHT <-> C  / LEFT <-> X / UP <-> Z / DOWN <->Y
			uint8_t buttons = (rawdata[pad_num] >> 8) & 0x0f;
			if((buttons & 0x01) != 0) stat |= LINE_JOYPORT_RIGHT;
			if((buttons & 0x02) != 0) stat |= LINE_JOYPORT_LEFT;
			if((buttons & 0x04) != 0) stat |= LINE_JOYPORT_DOWN;
			if((buttons & 0x08) != 0) stat |= LINE_JOYPORT_UP;
		} else {
			// If ((COM != 0) OR (PAD IS 2BUTTONS))  THEN CHECK AXIS
			uint8_t axis = rawdata[pad_num] & 0x0f;
			//if((axis & 0x01) != 0) stat |= LINE_JOYPORT_UP;
			//if((axis & 0x02) != 0) stat |= LINE_JOYPORT_DOWN;
			//if((axis & 0x04) != 0) stat |= LINE_JOYPORT_LEFT;
			//if((axis & 0x08) != 0) stat |= LINE_JOYPORT_RIGHT;
			stat |= axis;
		}
		if(trig_a) {
			stat |= (rawdata[pad_num] & 0x10);
			//if((rawdata[pad_num] & 0x10) != 0) stat |= LINE_JOYPORT_A;
		}
		if(trig_b) {
			stat |= (rawdata[pad_num] & 0x20);
			//if((rawdata[pad_num] & 0x20) != 0) stat |= LINE_JOYPORT_B;
		}
		// SEL = UP + DOWN, RUN = LEFT + RIGHT 
		if((rawdata[pad_num] & 0x40) != 0) stat |= (LINE_JOYPORT_LEFT | LINE_JOYPORT_RIGHT);
		if((rawdata[pad_num] & 0x80) != 0) stat |= (LINE_JOYPORT_UP | LINE_JOYPORT_DOWN); 
	} else {
		// None Connected
		stat = 0x00;
	}
	emu->release_joy_buffer(rawdata);
	if(__sigf) {
		if(d_joyport != nullptr) {
			int com_d = (SIG_JOYPORT_CH1 & ((pad_num & 1) << 24));
			d_joyport->write_signal(com_d | SIG_JOYPORT_DATA,
									stat,
									0xffffffff);
		}
	}
}

void JOYPAD::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint32_t ndata = data & mask;

	switch(id) {
	case SIG_JOYPAD_QUERY:
		if(ndata == (1 << pad_num)) {
			if((enabled) && ((ndata & 0x0c) == 0)) {
				query_joystick(false);
			}
		}
		break;
	case SIG_JOYPAD_ENABLE:
		if((ndata & (1 << SIG_JOYPORT_TYPE_2BUTTONS)) != 0) {
			//out_debug_log(_T("SELECT 2BUTTONS PAD"));
			if(!(enabled) || (type_6buttons)) {
				enabled = true;
				type_6buttons = false;
				reset();
			}
		} else if((ndata & (1 << SIG_JOYPORT_TYPE_6BUTTONS)) != 0) {
			//out_debug_log(_T("SELECT 6BUTTONS PAD"));
			if(!(enabled) || !(type_6buttons)) {
				enabled = true;
				type_6buttons = true;
				reset();
			}
		} else {
			out_debug_log(_T("DISCONNECTED"));
			type_6buttons = false;
			if((enabled) || (sel_line)) {
				enabled = false;
				sel_line = false;
				trig_a = false;
				trig_b = false;
				reset();
			}
		}
		break;
	case SIG_JOYPAD_SELECT_BUS:
//		out_debug_log(_T("SIG_JOYPAD_SELECT_BUS, VALUE=%08X"), ndata);
		if(enabled) {
			trig_a   = ((data & 0x01) != 0) ? true : false;
			trig_b   = ((data & 0x02) != 0) ? true : false;
			sel_line = ((data & 0x04) != 0) ? true : false;
			query_joystick(false);
		}
		break;
	}
}

	
#define STATE_VERSION 4
	
bool JOYPAD::process_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(sel_line);
	state_fio->StateValue(trig_a);
	state_fio->StateValue(trig_b);
	state_fio->StateValue(type_6buttons);
	state_fio->StateValue(pad_num);
	state_fio->StateValue(enabled);
	return true;
}
}
