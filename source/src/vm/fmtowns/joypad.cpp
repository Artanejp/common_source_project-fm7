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
//	rawdata = emu->get_joy_buffer();
	register_frame_event(this);
}

void JOYPAD::event_frame(void)
{
}
	
void JOYPAD::event_pre_frame(void)
{
	if((enabled_bak != enabled) && !(enabled)) {
		enabled_bak = enabled;
		for(int i = 0; i < 2; i++) {
			write_signals(&line_a[i], 0);
			write_signals(&line_b[i], 0);
			write_signals(&line_up[i], 0);
			write_signals(&line_down[i], 0);
			write_signals(&line_left[i], 0);
			write_signals(&line_right[i], 0);
		}
		return;
	} else if(!(enabled)) {
		enabled_bak = enabled;
		return;
	}
	enabled_bak = enabled;

	// enabled
	
	rawdata = emu->get_joy_buffer();
	if(rawdata != NULL) {
		uint32_t a_status    = ((rawdata[pad_num] & 0x010) != 0) ? 0xffffffff : 0;
		uint32_t b_status    = ((rawdata[pad_num] & 0x020) != 0) ? 0xffffffff : 0;
		uint32_t ch = (type_6buttons) ? 1 : 0;
		if((sel_6buttons) && (type_6buttons)) { // 6Buttons Multiplied
			// 6Buttons PAD is seems to be this schematic:
			// http://www.awa.or.jp/home/shimojo/towpadx.htm
			// RIGHT <-> C  / LEFT <-> X / UP <-> Z / DOWN <->Y
			uint32_t c_status    = ((rawdata[pad_num] & 0x100) != 0) ? 0xffffffff : 0;
			uint32_t x_status    = ((rawdata[pad_num] & 0x200) != 0) ? 0xffffffff : 0;
			uint32_t y_status    = ((rawdata[pad_num] & 0x400) != 0) ? 0xffffffff : 0;
			uint32_t z_status    = ((rawdata[pad_num] & 0x800) != 0) ? 0xffffffff : 0;
			write_signals(&line_up[ch],    z_status);
			write_signals(&line_down[ch],  y_status);
			write_signals(&line_left[ch],  x_status);
			write_signals(&line_right[ch], c_status);
		} else {
			// 2 Buttons PAD
			uint32_t up_status     = ((rawdata[pad_num] & 0x001) != 0) ? 0xffffffff : 0;
			uint32_t down_status   = ((rawdata[pad_num] & 0x002) != 0) ? 0xffffffff : 0;
			uint32_t left_status   = ((rawdata[pad_num] & 0x004) != 0) ? 0xffffffff : 0;
			uint32_t right_status  = ((rawdata[pad_num] & 0x008) != 0) ? 0xffffffff : 0;
			uint32_t run_status  = ((rawdata[pad_num] & 0x040) != 0) ? 0xffffffff : 0;
			uint32_t sel_status  = ((rawdata[pad_num] & 0x080) != 0) ? 0xffffffff : 0;
			left_status  |= run_status;
			right_status |= run_status;
			up_status    |= sel_status;
			down_status  |= sel_status;
			write_signals(&line_up[ch],    up_status);
			write_signals(&line_down[ch],  down_status);
			write_signals(&line_left[ch],  left_status);
			write_signals(&line_right[ch], right_status);
		}
		write_signals(&line_a[ch], a_status);
		write_signals(&line_b[ch], b_status);
	}
}

void JOYPAD::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint32_t ndata = data & mask;
	switch(id) {
	case SIG_JOYPAD_ENABLE:
		if((ndata & (1 << (SIG_JOYPORT_TYPE_2BUTTONS >> 8))) != 0) {
			out_debug_log(_T("SELECT 2BUTTONS PAD"));
			enabled = true;
			type_6buttons = false;
		} else if((ndata & (1 << (SIG_JOYPORT_TYPE_6BUTTONS >> 8))) != 0) {
			out_debug_log(_T("SELECT 6BUTTONS PAD"));
			enabled = true;
			type_6buttons = true;
		} else {
			out_debug_log(_T("DISCONNECTED"));
			enabled = false;
			type_6buttons = false;
		}
		break;
	case SIG_JOYPAD_SELECT_BUS:
//		out_debug_log(_T("SIG_JOYPAD_SELECT_BUS, VALUE=%08X"), ndata);
		if(ndata != 0) {
			sel_6buttons = true;
		} else {
			sel_6buttons = false;
		}
		break;
	}
}

void JOYPAD::update_config()
{
}


#define STATE_VERSION 1
	
bool JOYPAD::process_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(sel_6buttons);
	state_fio->StateValue(type_6buttons);
	state_fio->StateValue(pad_num);
	state_fio->StateValue(enabled);
	state_fio->StateValue(enabled_bak);
	return true;
}
}
