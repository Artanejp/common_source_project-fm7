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

namespace FMTOWNS {
void KEYBOARD::initialize()
{
	DEVICE::initialize();
	key_buf = new FIFO(64);
	cmd_buf = new FIFO(16);
	register_frame_event(this);
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
	write_signals(&output_intr_line, 0);
	write_signals(&output_nmi_line, 0);
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
			kbdata = key_buf->read();
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

void KEYBOARD::event_frame()
{
	if(!(kbstat & 1) && !key_buf->empty()) {
		kbstat |= 1;
		kbdata = key_buf->read();
	}
	if((kbstat & 1) && (kbmsk & 1) && !(kbint & 1)) {
		kbint |= 1;
		write_signals(&output_intr_line, 0xffffffff);
	}
//	kbstat &= ~2;
}

void KEYBOARD::key_down(int code)
{
//	if(!table[code]) {
		table[code] = 1;
		if(code = key_table[code]) {
			// $11:CTRL, $10:SHIFT
			key_buf->write(0xc0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
			key_buf->write(code & 0x7f);
		}
//	}
}

void KEYBOARD::key_up(int code)
{
//	if(table[code]) {
		table[code] = 0;
		if(code = key_table[code]) {
			key_buf->write(0xd0 | (table[0x11] ? 8 : 0) | (table[0x10] ? 4 : 0));
			key_buf->write(code & 0x7f);
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

	return true;
}

}

