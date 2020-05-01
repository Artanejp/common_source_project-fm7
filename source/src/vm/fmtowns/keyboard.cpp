/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8259.h"
#include "../../fifo.h"

#define EVENT_KEY_CODE		1

namespace FMTOWNS {
void KEYBOARD::initialize()
{
	DEVICE::initialize();
	key_buf = new FIFO(64);
	cmd_buf = new FIFO(16);
	event_keycode = -1;
}

void KEYBOARD::release()
{
	cmd_buf->release();
	key_buf->release();
	delete key_buf;
	delete cmd_buf;
}

void KEYBOARD::reset()
{
	memset(table, 0, sizeof(table));
	key_buf->clear();
	cmd_buf->clear();
	kbstat = kbdata = kbint = kbmsk = 0;
	device_order = false;
	repeat_start_ms = 400;
	repeat_tick_ms = 30;
	enable_double_pressed_cursor = true;
	nmi_status = false;
	if(event_keycode > -1) {
		cancel_event(this, event_keycode);
		event_keycode = -1;
	}
	write_signals(&output_intr_line, 0);
	write_signals(&output_nmi_line, 0);
}

void KEYBOARD::register_key_interrupt(bool first)
{
	double usec = (first) ? 1000.0 : 50.0;
	if(event_keycode > -1) {
		cancel_event(this, event_keycode);
	}
	event_keycode = -1;
	register_event(this, EVENT_KEY_CODE, usec, false, &event_keycode);
}
	
void KEYBOARD::do_common_command(uint8_t cmd)
{
	static const int type_start_ms[] = {400, 500, 300};
	static const int type_repeat_ms[] = {50, 30, 20};
	
	switch(cmd) {
	case 0xa1:
		this->reset(); // RESET
		break;
	case 0xa4:
		// Check double press (for OYAYUBI-Shift)
		break;
	case 0xa5:
		// Don't check double press (for OYAYUBI-Shift)
		break;
	case 0xa9:
	case 0xaa:
	case 0xab:
		repeat_start_ms = type_start_ms[cmd - 0xa9];
		break;
	case 0xac:
	case 0xad:
	case 0xae:
		repeat_tick_ms = type_repeat_ms[cmd - 0xac];
		break;
	case 0xb0:
		enable_double_pressed_cursor = true;
		break;
	case 0xb1:
		enable_double_pressed_cursor = false;
		break;
	case 0xb2:
		write_signals(&output_nmi_line, 0);
		nmi_status = false;
		break;
	default:
		break;
	}
}

// Mame 0.216
void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE I/O ADDR=%04X VAL=%02X"), addr, data);
	switch(addr) {
	case 0x600:
		// data
		kbstat &= ~0x08;
		kbstat |= 1;
		break;
	case 0x602:
		// command
		if((data & 0x80) == 0x00) {
			// Second byte
			if((device_order) && (cmd_buf->count() > 0)) {
				cmd_buf->write(data & 0x7f);
			} else {
				// Illegal
				cmd_buf->clear();
				device_order = false;
			}
			kbstat |= 0x08;
		} else if(data & 0xe0 == 0xc0) {
			// Device order
			if((cmd_buf->count() > 0) && (device_order)){
				// DO DEVICE ORDER
			}
			cmd_buf->clear();
			cmd_buf->write(data & 0xff);
			device_order = false;
			kbstat |= 0x08;
		} else if(data & 0xe0 == 0xa0) {
			// Common order
			if((cmd_buf->count() > 0) && (device_order)){
				// DO DEVICE ORDER
			}
			device_order = false;
			kbstat |= 0x08;
			do_common_command(data);
		}
		break;
	case 0x604:
		kbmsk = data;
		break;
	}
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint8_t kbtmp = kbdata;
	switch(addr) {
	case 0x600:
		kbint &= ~1;
		write_signals(&output_intr_line, 0);
		if(key_buf->empty()) {
			kbstat &= ~1;
		} else {
			register_key_interrupt(false); // NEXT BYTE
		}
//		out_debug_log(_T("READ I/O ADDR=%04X VAL=%02X"), addr, kbdata);
		return kbtmp;
	case 0x602:
//		out_debug_log(_T("READ I/O ADDR=%04X VAL=%02X"), addr, kbstat);
		return kbstat;
	case 0x604:
//		out_debug_log(_T("READ I/O ADDR=%04X VAL=%02X"), addr, kbint);
		return (((kbint & 1) != 0) ? 1 : 0) | ((nmi_status) ? 2 : 0);
		break;
	}
	return 0;
}

void KEYBOARD::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_KEY_CODE:
		kbdata = key_buf->read();
		kbstat |= 1;
		if(kbmsk & 1) {
			kbint |= 1;
			write_signals(&output_intr_line, 0xffffffff);
		}
		event_keycode = -1;
		break;
	}
}
	

void KEYBOARD::key_down(int code)
{
//	if(!table[code]) {
		table[code] = 1;
		if(code = key_table[code]) {
			// $11:CTRL, $10:SHIFT
			if((code >= 0x70) && (code < 0x7a)) {
				// If PFkey, press with /*SHIFT or */RALT, PF += 10.
				if((table[0xa5]) /*|| (table[0x10])*/) { // RALT /*or SHIFT*/
					static const int pf1xtbl[] = { 0x69, 0x5b, 0x74, 0x75, 0x76,
												   0x77, 0x78, 0x79, 0x7a, 0x7b};
					code = pf1xtbl[code - 0x70];
				}
			} else if((code == 0x7d)) { // Print Screen + RALT -> Kanji Jisho
				if(table[0xa5]) { // RALT
					code = 0x6b;
				}
			} else if((code == 0x7c)) { // Pause Break + RALT -> Tango Touroku
				if(table[0xa5]) { // RALT
					code = 0x6d;
				}
			} else if((code == 0x6c)) { // Scroll Lock + RALT -> Tango Massyou.
				if(table[0xa5]) { // RALT
					code = 0x6c;
				}
			}
			key_buf->write(0xc0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
			key_buf->write(code & 0x7f);
			register_key_interrupt(true);
		}
//	}
}

void KEYBOARD::key_up(int code)
{
//	if(table[code]) {
		table[code] = 0;
		if(code = key_table[code]) {
			if((code >= 0x70) && (code < 0x7a)) {
				// If PFkey, press with /*SHIFT or */RALT, PF += 10.
				if((table[0xa5]) /*|| (table[0x10])*/) { // RALT /*or SHIFT*/
					static const int pf1xtbl[] = { 0x69, 0x5b, 0x74, 0x75, 0x76,
												   0x77, 0x78, 0x79, 0x7a, 0x7b};
					code = pf1xtbl[code - 0x70];
				}
			} else if((code == 0x7d)) { // Print Screen + RALT -> Kanji Jisho
				if(table[0xa5]) { // RALT
					code = 0x6b;
				}
			} else if((code == 0x7c)) { // Pause Break + RALT -> Tango Touroku
				if(table[0xa5]) { // RALT
					code = 0x6d;
				}
			} else if((code == 0x6c)) { // Scroll Lock + RALT -> Tango Massyou.
				if(table[0xa5]) { // RALT
					code = 0x6c;
				}
			}
			key_buf->write(0xd0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
			key_buf->write(code & 0x7f);
			register_key_interrupt(true);
		}
//	}
}

#define STATE_VERSION	1

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!(key_buf->process_state((void *)state_fio, loading))) {
		return false;
	}
	if(!(cmd_buf->process_state((void *)state_fio, loading))) {
		return false;
	}

	state_fio->StateValue(kbstat);
	state_fio->StateValue(kbdata);
	state_fio->StateValue(kbint);
	state_fio->StateValue(kbmsk);
	
	state_fio->StateValue(nmi_status);
	state_fio->StateValue(repeat_start_ms);
	state_fio->StateValue(repeat_tick_ms);
	state_fio->StateValue(enable_double_pressed_cursor);
	state_fio->StateValue(device_order);

	state_fio->StateArray(table, sizeof(table), 1);
	
	state_fio->StateValue(event_keycode);
	return true;
}

}

