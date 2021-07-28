/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2021.06.16 -
    History : 2021.06.16 Initial
	[ Towns Mouse]

*/

#include "./mouse.h"

namespace FMTOWNS {

#define EVENT_MOUSE_TIMEOUT		1
#define EVENT_MOUSE_SAMPLING	2
	
void MOUSE::initialize()
{
	pad_type = PAD_TYPE_MOUSE;
	mouse_state = NULL;
	sig_com = true;
	val_com = true;
	
	event_timeout = -1;
	event_sampling = -1;

	initialize_status();
}

void MOUSE::initialize_status()
{
	JSDEV_TEMPLATE::initialize_status();
	phase = 0;
	
	portval_data = 0x00;
	val_trig_a = false;
	val_trig_b = false;
	val_com = false;
	
	dx = dy = 0;
	lx = ly = 0;
}	
	
void MOUSE::release()
{
}

uint8_t MOUSE::output_port_com(bool val, bool force)
{
	// Mouse don't output to com.
	val_com = sig_com;
	return (val_com) ? 0x01 : 0x00;
	//return JSDEV_TEMPLATE::output_port_com(val, force);
}

void MOUSE::reset_device(bool port_out)
{
	initialize_status();
	val_com = false;
	sample_mouse_xy();
	
	if(port_out) {
		output_port_signals(true);
		output_port_com(val_com, true);
	}
}

uint8_t MOUSE::query(bool& status)
{
	check_mouse_data();

	status = true;
	return portval_data;
}
void MOUSE::update_strobe()
{
	if(phase == 0) {
		if(sig_com) {
			// Sample data from MOUSE to registers.
			sample_mouse_xy(); // Sample next value.
			lx = -dx;
			ly = -dy;
			dx = 0;
			dy = 0;
			// From FM Towns Technical book, Sec.1-7, P.241.
			// (Ta + Tj * 3 + Ti) <= 920.0uS 
			//force_register_event(this, EVENT_MOUSE_TIMEOUT, 920.0, false, event_timeout);
			force_register_event(this, EVENT_MOUSE_TIMEOUT, 2000.0, false, event_timeout);
			phase = 2; // SYNC from MAME 0.225. 20201126 K.O
		}
	} else {
		phase++;
	}
}


uint32_t MOUSE::update_mouse()
{
	uint32_t mouse_data = 0x00;
	switch(phase) {
//	case 1: // SYNC
//		mouse_data = 0x0f;
//		break;
	case 2: // X_HIGH
		mouse_data = lx >> 0;
		break;
	case 3: // X_LOW
		mouse_data = lx >> 4;
		break;
	case 4: // Y_HIGH
		mouse_data = ly >> 0;
		break;
	case 5: // Y_LOW
		mouse_data = ly >> 4;
		// From FM Towns Technical book, Sec.1-7, P.241.
		// Ti(min)
		force_register_event(this, EVENT_MOUSE_TIMEOUT, 150.0, false, event_timeout);
		phase++;
		break;
	case 6: // END
		mouse_data = ly >> 4;
//		phase = 0;
		break;
	}
//	out_debug_log(_T("READ MOUSE DATA=%01X PHASE=%d STROBE=%d"), mouse_data, mouse_phase, (mouse_strobe) ? 1 : 0);
	return mouse_data;
}


void MOUSE::check_mouse_data()
{
	// Do Not reply COM (0x40) : 20210627 K.O
	portval_data = update_mouse() & 0x0f;
	int32_t stat = emu->get_mouse_button();

	val_trig_a = ((stat & 0x01) != 0) ? true : false;
	val_trig_b = ((stat & 0x02) != 0) ? true : false;

	output_port_signals(true);
}

void MOUSE::set_enable(bool is_enable)
{
	if(is_enable != is_connected) {
		clear_event(this, event_timeout);
		clear_event(this, event_sampling);

		if(is_enable) { // disconnect->connect
			initialize_status();
			reset_device(true);
			sig_com = false;
		} else { // connect->disconnect
			initialize_status();
			sig_com = true;
		}
	}
	is_connected = is_enable;
}
void MOUSE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_JS_COM) {
		if(is_connected) {
			bool bak = sig_com;
			sig_com = ((data & mask) != 0) ? true : false;
			val_com = sig_com;
			//out_debug_log(_T("SIG_JS_COM: BEFORE=%d AFTER=%d"), (bak) ? 1 : 0, (sig_com) ? 1 : 0);
			if(bak != sig_com) {
				update_strobe();
			}
		}
		return;
	}
	JSDEV_TEMPLATE::write_signal(id, data, mask);
}

void MOUSE::sample_mouse_xy()
{
	mouse_state = emu->get_mouse_buffer();
	if(mouse_state != NULL) {
		dx += mouse_state[0];
		dy += mouse_state[1];
		if(dx < -127) {
			dx += 128;
		} else if(dx > 127) {
			dx -= 128;
		}
		if(dy < -127) {
			dy += 128;
		} else if(dy > 127) {
			dy -= 128;
		}
	}
	emu->release_mouse_buffer(mouse_state);
}
void MOUSE::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_MOUSE_TIMEOUT:
		event_timeout = -1;
		reset_device(true);
		break;
	case EVENT_MOUSE_SAMPLING:
		sample_mouse_xy();
		break;
	}
}
	
#define STATE_VERSION 16

bool MOUSE::process_state(FILEIO *state_fio, bool loading)
{
	if(!(JSDEV_TEMPLATE::process_state(state_fio, loading))) {
		return false;
	}
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	
	state_fio->StateValue(phase);
	
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(lx);
	state_fio->StateValue(ly);

	state_fio->StateValue(event_timeout);
	state_fio->StateValue(event_sampling);

	return true;
}
}
