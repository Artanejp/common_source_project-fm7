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
#define EVENT_DELAY_PUSH			2

namespace FMTOWNS {
void KEYBOARD::initialize()
{
	DEVICE::initialize();
	key_buf = new FIFO(64);
	cmd_buf = new FIFO(16);
	event_keycode = -1;
	event_key_reset = -1;
	memset(table, 0, sizeof(table));
	memset(boot_code, 0x00, sizeof(boot_code));
	boot_ptr = 0;
	boot_seq = false;
	boot_code_ptr = 0;
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
//	boot_seq = true;
	reset_device();
}

void KEYBOARD::reset_device()
{
	out_debug_log("RESET\n");
	special_boot_num = -1;
	
	last_cmd = 0x00;
	memset(table, 0, sizeof(table));
	key_buf->clear();
	cmd_buf->clear();
	
	kbstat = kbdata = kbint = kbmsk = 0;
	device_order = false;
	enable_double_pressed_cursor = true;
	nmi_status = false;
	reserved_key_num = -1;
	if(event_keycode > -1) {
		cancel_event(this, event_keycode);
		event_keycode = -1;
	}
	if(event_key_reset > -1) {
		cancel_event(this, event_key_reset);
	}
	event_key_reset = -1;
	write_signals(&output_intr_line, 0);
	write_signals(&output_nmi_line, 0);
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
	memset(boot_code, 0x00, sizeof(boot_code));
	if(num < 0) return;
	if(num >= 12) return;
	special_boot_num = num;
	boot_ptr = 0;
	boot_seq = true;
	boot_code_ptr = 0;
	kbstat |= 1;
	key_buf->clear();
	enqueue_key(0x7f);
	
	switch(num) {
	case 11: // DEBUG
		enqueue_key(0x20);
		enqueue_key(0x13);
		enqueue_key(0x2E);
		enqueue_key(0x17);
		enqueue_key(0x22);
		break;
	default:
//		register_event(this, EVENT_DELAY_PUSH, 1.0, false, NULL);
		event_callback(EVENT_DELAY_PUSH, 0); 
//		boot_seq = false;
//		kbstat &= ~1;
//		return;
		break;
	}
	
	register_key_interrupt(true); // NEXT BYTE
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
	out_debug_log(_T("CMD %02X"), cmd);
	switch(cmd) {
	case 0xa0:
		this->reset_device(); // RESET
		// From Tsugaru
//		key_buf->write(0xa0);
//		key_buf->write(0x7f);
//		memset(boot_code, 0x00, sizeof(boot_code));
//		boot_ptr = 0;
//		boot_seq = false;
//		boot_code_ptr = false;
		break;
	case 0xa1:
		if(last_cmd != 0xa0) {
			this->reset_device(); // RESET
			memset(boot_code, 0x00, sizeof(boot_code));
			boot_ptr = 0;
			boot_seq = false;
			boot_code_ptr = false;
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
	uint8_t kbtmp;
	kbtmp = kbdata;
	
	switch(addr) {
	case 0x600:
		kbint &= ~1;
		write_signals(&output_intr_line, 0);
		if(key_buf->empty()) {
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
	case EVENT_DELAY_PUSH:
		if(boot_seq) {
			int num = special_boot_num;
			switch(special_boot_num) {
			case 0: //  CD
				enqueue_key(0x2c);
				enqueue_key(0x20);
				enqueue_key(0x2c);
				enqueue_key(0x20);
				enqueue_key(0x2c);
				enqueue_key2(0x20);
				break;
			case 1: // F0
				enqueue_key(0x21);
				enqueue_key(0x0b);
				enqueue_key(0x21);
				enqueue_key(0x0b);
				enqueue_key(0x21);
				enqueue_key2(0x0b);
				break;
			case 2: // F1
			case 3: // F2
			case 4: // F3
				enqueue_key(0x21);
				enqueue_key(0x01 + (num - 1));
				enqueue_key(0x21);
				enqueue_key(0x01 + (num - 1));
				enqueue_key(0x21);
				enqueue_key2(0x01 + (num - 1));
				break;
			case 5: // H0
				enqueue_key(0x23);
				enqueue_key(0x0B);
				enqueue_key(0x23);
				enqueue_key(0x0B);
				enqueue_key(0x23);
				enqueue_key2(0x0B);
				break;
			case 6: // H1 - H4
			case 7: // H1 - H4
			case 8: // H1 - H4
			case 9: // H1 - H4
				enqueue_key(0x23);
				enqueue_key(0x01 + (num - 5));
				enqueue_key(0x23);
				enqueue_key(0x01 + (num - 5));
				enqueue_key(0x23);
				enqueue_key2(0x01 + (num - 5));
				break;
			case 10: // ICM
				enqueue_key(0x18);
				enqueue_key(0x2C);
				enqueue_key(0x30);
				enqueue_key(0x18);
				enqueue_key(0x2C);
				enqueue_key2(0x30);
				break;
			}
		}
		boot_seq = false;
		boot_ptr = 0;
		boot_code_ptr = 0;
		special_boot_num = -1;
		memset(boot_code, 0x00, sizeof(boot_code));
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

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_KEYBOARD_BOOTSEQ_END:
//		out_debug_log(_T("SIG_KEYBOARD_BOOTSEQ_END"));
		break;
	}
}
	
#define STATE_VERSION	2

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
	state_fio->StateValue(reserved_key_num);
	state_fio->StateValue(key_count_phase);
	state_fio->StateValue(special_boot_num);
	
	state_fio->StateArray(boot_code, sizeof(boot_code), 1);
	state_fio->StateValue(boot_ptr);
	state_fio->StateValue(boot_seq);
	state_fio->StateValue(boot_code_ptr);
	
	state_fio->StateValue(event_keycode);
	state_fio->StateValue(event_key_reset);
	return true;
}

}

