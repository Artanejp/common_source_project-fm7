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

#define EVENT_KEY_CODE				1
#define EVENT_BOOT_TIMEOUT			2
#define EVENT_REPEAT_UP				3
#define EVENT_REPEAT_DOWN			4

namespace FMTOWNS {
void KEYBOARD::initialize()
{
	DEVICE::initialize();
	key_buf = new FIFO(64);
	cmd_buf = new FIFO(16);
	event_keycode = -1;
	event_key_reset = -1;
	event_repeat = -1;
	
	memset(table, 0, sizeof(table));
	repeat_start_ms = 400;
	repeat_tick_ms = 30;
	
	special_boot_num = -1;
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
	boot_seq = false;
	reset_device();
}

void KEYBOARD::reset_device()
{
	out_debug_log("RESET\n");
	
	last_cmd = 0x00;
	memset(table, 0, sizeof(table));
	key_buf->clear();
	cmd_buf->clear();
	
	kbstat = kbdata = kbint = kbmsk = 0;
	device_order = false;
	enable_double_pressed_cursor = true;
	nmi_status = false;
	
	clear_event(this, event_keycode);
	clear_event(this, event_key_reset);
	clear_event(this, event_keycode);

	write_signals(&output_intr_line, 0);
	write_signals(&output_nmi_line, 0);
	memset(boot_code, 0x00, sizeof(boot_code));
	boot_seq = false;
	boot_code_ptr = 0;
}

void KEYBOARD::enqueue_key(uint8_t code)
{
	key_buf->write(0xa0);
	key_buf->write(code);
}

void KEYBOARD::enqueue_key2(uint8_t code)
{
	key_buf->write(0xf0);
	key_buf->write(code);
}
	
void KEYBOARD::special_reset(int num)
{
	out_debug_log("SPECIAL RESET %d\n", num);	
	if(num < 0) return;
	if(num >= 12) return;
	reset_device();
	special_boot_num = num;
	boot_seq = true;
//	kbstat |= 1;

	static const uint8_t bootcode_debug[] = {0x20, 0x13, 0x2E, 0x17, 0x22};
	static const uint8_t bootcode_cd[] = {0x2C, 0x20};
	static const uint8_t bootcode_icm[] = {0x18, 0x2C, 0x30, 0x18, 0x2C};
	static const uint8_t dnum[] = {0x0B, 0x02, 0x03, 0x04, 0x05};
	switch(num) {
	case 0: // CD
//		enqueue_key(0x7f);
		memcpy(boot_code, bootcode_cd, sizeof(bootcode_cd));
		break;
	case 1:
	case 2:
	case 3:
	case 4:
//		enqueue_key(0x7f);
		boot_code[0] = 0x21;
		boot_code[1] = dnum[num - 1];
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		boot_code[0] = 0x23;
		boot_code[1] = dnum[num - 5];
		break;
	case 10: // ICM
//		enqueue_key(0x7f);
		memcpy(boot_code, bootcode_icm, sizeof(bootcode_icm));
		break;;
	case 11: // DEBUG
//		enqueue_key(0x7f);
		memcpy(boot_code, bootcode_debug, sizeof(bootcode_debug));
		break;;
	}
	if((num >= 0) && (num < 12)) {
		register_key_interrupt(true); // NEXT BYTE
	}
//	register_event(this, EVENT_BOOT_TIMEOUT, 30.0e6, false, &event_key_reset);
}
	
void KEYBOARD::register_key_interrupt(bool first)
{
	double usec = (first) ? 1000.0 : 50.0;
	clear_event(this, event_keycode);
	register_event(this, EVENT_KEY_CODE, usec, false, &event_keycode);
}
	
void KEYBOARD::do_common_command(uint8_t cmd)
{
	static const int type_start_ms[] = {400, 500, 300};
	static const int type_repeat_ms[] = {50, 30, 20};
	out_debug_log(_T("CMD %02X"), cmd);
	switch(cmd) {
	case 0xa0:
		this->reset_device(); // RESET
		special_boot_num = -1;
		break;
	case 0xa1:
		if(last_cmd != 0xa0) {
			this->reset_device(); // RESET
			special_boot_num = -1;
			memset(boot_code, 0x00, sizeof(boot_code));
		}
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
	last_cmd = cmd;
}

// Mame 0.216
void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE I/O ADDR=%04X VAL=%02X"), addr, data);
	switch(addr) {
	case 0x0600:
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
	uint8_t kbtmp;
	
	switch(addr) {
	case 0x600:
		kbtmp = get_key_code();
		kbint &= ~1;
		write_signals(&output_intr_line, 0);
		if((key_buf->empty()) && !(boot_seq)) {
			kbstat &= ~1;
		} else {
			register_key_interrupt(false); // NEXT BYTE
		}
		out_debug_log(_T("READ I/O ADDR=%04X VAL=%02X"), addr, kbdata);
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

uint8_t KEYBOARD::get_key_code()
{
	if(boot_seq) {
		if((boot_code_ptr & 1) == 0) {
			kbdata = ((boot_code_ptr >> 1) < 6) ? 0xA0 : 0xF0;
		} else {
			if((boot_code_ptr >> 1) == 0) {
				kbdata = 0x7F;
			} else {
				switch(special_boot_num) {
				case 0: // CD
				case 1: // F0
				case 2: // F1
				case 3: // F2
				case 4: // F3
				case 5: // H0
				case 6: // H1
				case 7: // H2
				case 8: // H3
				case 9: // H4
					kbdata = boot_code[(boot_code_ptr >> 1) & 1];
					break;
				case 10: // ICM
					kbdata = boot_code[(boot_code_ptr >> 1) % 3];
					break;
				case 11: // DEBUG
					kbdata = boot_code[(boot_code_ptr >> 1) % 5];
					break;
				default:
					kbdata = key_buf->read();
					break;
				}
			}
		}
		boot_code_ptr++;
	} else {
		if(key_buf->empty()) {
			kbdata = 0x00;
		} else {
			kbdata = key_buf->read();
		}
	}
	return kbdata;
}
void KEYBOARD::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_KEY_CODE:
		if((boot_seq) || !(key_buf->empty())) {
			kbstat |= 1;
			if(kbmsk & 1) {
				kbint |= 1;
				write_signals(&output_intr_line, 0xffffffff);
			}
		} else {
			kbstat &= ~0x1;
		}
		event_keycode = -1;
		break;
	case EVENT_REPEAT_UP:
		{
			bool still_press = false;
			for(int code = 1; code < 256; code++) {
				if((table[code] != 0) && (key_table[code] != 0)) {
					if(key_table[code] < 0x52) {
						key_up2(code); // Enqueue as code up.
						still_press = true;
					}
				}
			}
			if(still_press) {
				double usec = ((double)repeat_tick_ms) * 1000.0;
				register_key_interrupt(true);
				register_event(this, EVENT_REPEAT_DOWN, usec, false, &event_repeat);
			} else {
				event_repeat = -1;
			}
		}
		break;
	case EVENT_REPEAT_DOWN:
		{
			bool still_press = false;
			for(int code = 1; code < 256; code++) {
				if((table[code] != 0) && (key_table[code] != 0)) {
					if(key_table[code] < 0x52) {
						key_down2(code); // Enqueue as code down.
						still_press = true;
					}
				}
			}
			if(still_press) {
				double usec = ((double)repeat_tick_ms) * 1000.0;
				register_key_interrupt(true);
				register_event(this, EVENT_REPEAT_UP, usec, false, &event_repeat);
			} else {
				event_repeat = -1;
			}
		}
		break;
	case EVENT_BOOT_TIMEOUT:
		event_key_reset = -1;
		boot_seq = false;
		break;
	}
}
	
void KEYBOARD::key_down(int code)
{
	if(table[code] == 0) {
		table[code] = 1;
		key_down2(code);

		code = key_table[code];
		if(code != 0) {
			register_key_interrupt(true);
		}
		if(event_repeat < 0) {
			if(code < 0x52) {
				double usec = ((double)repeat_start_ms) * 1000.0;
				register_event(this, EVENT_REPEAT_UP, usec, false, &event_repeat);
			}
		}
	}
}

void KEYBOARD::key_down2(int code)
{
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
		if(boot_seq) {
			key_buf->write(0xa0);
		} else {
			key_buf->write(0xc0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
		}
		key_buf->write(code & 0x7f);
	}
}


void KEYBOARD::key_up(int code)
{
	table[code] = 0;
	key_up2(code);
	
	code = key_table[code];
	if(code != 0) {
		register_key_interrupt(true);
	}
	
	bool all_clear = true;
	for(int i = 1; i < 256; i++) {
		if((table[i] != 0) && (key_table[i] != 0)) {
			all_clear = false;
			break;
		}
	}
	if(all_clear) {
		clear_event(this, event_repeat);
	}
}

void KEYBOARD::key_up2(int code)
{
//	if(table[code]) {
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
		}
//	}
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_KEYBOARD_BOOTSEQ_END:
		if(((data & mask) != 0) && (boot_seq)) {
			out_debug_log(_T("SIG_KEYBOARD_BOOTSEQ_END"));
			boot_seq = false;
		}
		break;
	}
}
	
#define STATE_VERSION	4

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

	state_fio->StateValue(last_cmd);
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
	state_fio->StateValue(special_boot_num);
	
	state_fio->StateArray(boot_code, sizeof(boot_code), 1);
	state_fio->StateValue(boot_seq);
	state_fio->StateValue(boot_code_ptr);
	
	state_fio->StateValue(event_keycode);
	state_fio->StateValue(event_key_reset);
	state_fio->StateValue(event_repeat);
	return true;
}

}

