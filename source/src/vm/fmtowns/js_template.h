/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.07.26 -
    History : 2020.07.26 Initial.
	[ Towns Joystick devices: Template class]

*/

#pragma once

#include "../device.h"

namespace FMTOWNS {

// OUTPUT TO PARENT PORT (SOME JOYSTICK DEVICES -> JOYSTICK PORT)
#define SIG_JSPORT_COM		0x01001
#define SIG_JSPORT_DATA		0x01002

#define SIG_JSPORT_PORT1	0x00000000
#define SIG_JSPORT_PORT2	0x00010000
#define SIG_JSPORT_PORT3	0x00020000
#define SIG_JSPORT_PORT4	0x00030000
#define SIG_JSPORT_PORT5	0x00040000
#define SIG_JSPORT_PORT6	0x00050000
#define SIG_JSPORT_PORT7	0x00060000
#define SIG_JSPORT_PORT8	0x00070000

	
// DEVICE CONTROL SIGNALS (MOSTLY PORT -> THIS)
#define SIG_JS_COM				1
#define SIG_JS_TRIG_A			2
#define SIG_JS_TRIG_B			3

// SPECIAL DEVICE CONTROL SIGNAL(S) (PORT -> THIS)
#define SIG_JS_DEVICE_RESET		0x80
	
// INPUT SIGNALS FROM PARENT PORT. (THIS -> PORT)	
#define SIG_JS_COM_OUTPUT		0x11
#define SIG_JS_TRIG_A_OUTPUT	0x12
#define SIG_JS_TRIG_B_OUTPUT	0x13
#define SIOG_JS_DATA			0x14

// PEEK STATUS OF THIS FROM ANY DEVICES.	
#define SIG_JS_PORT_NUM			0x15
#define SIG_JS_CONNECTED		0x16
#define SIG_JS_DATASHIFT		0x17
#define SIG_JS_DATAMASK			0x18
#define SIG_JS_NAGATIVE_LOGIC	0x19

#define JSPORT_MASK_DATA		0x0f
#define JSPORT_MASK_TRIG_A		0x10
#define JSPORT_MASK_TRIG_B		0x20
#define JSPORT_MASK_COM			0x40
#define JSPORT_MASK_RESERVE		0x80
	
class JSDEV_TEMPLATE : public DEVICE
{
protected:
	DEVICE* d_parent_device; // Normally FMTOWNS::JOYSTICK
	
	bool is_connected;
	bool is_negative_logic; // DATA VALUES IS NAGATIVE LOGIC.
	bool force_output_on_change;
	int parent_port_num;
	
	uint32_t signal_mask;
	int signal_shift;
	
	// INPUT (Received) Values
	bool sig_trig_a; // INPUT: trig_a
	bool sig_trig_b; // INPUT: trig_a
	bool sig_com;  // INPUT: com (or strobe).

	bool val_trig_a; // OUTPUT: trig_a
	bool val_trig_b; // OUTPUT: trig_b
	bool val_com;

	uint8_t portval_data; // May 4bit space.

	bool need_unlock;
	
	void lock_vm()
	{
		if((emu != nullptr) && (need_unlock)){
			emu->lock_vm();
		}
	}
	
	void unlock_vm()
	{
		if((emu != nullptr) && (need_unlock)){
			emu->unlock_vm();
		}
	}

	// @note: DEVICE MUST NOT RESET WITHIN reset(), must reset within reset_device().
	virtual void reset_device()
	{
		initialize_status();
		
		output_port_status(false);
		output_port_com(val_com, false);
	}
	virtual void initialize_status()
	{
		sig_trig_a = false;
		sig_trig_b = false;
		sig_com = false;

		val_trig_a = true;
		val_trig_b = true;
		val_com = true;

		portval_data = 0x00;

	}
	virtual void output_port_com(bool val, bool force = false)
	{
		val_com = val;
		if(!(force)) {
			val = val & sig_com;
		}
		if((d_parent_device != nullptr) && (is_connected)) {
			if((parent_port_num >= 0) && (parent_port_num < 8)) {
				uint32_t r_data = (val) ? 0x40 : 0x0;
				int signum = (parent_port_num << 16) & 0x70000;
				signum = signum | SIG_JSPORT_COM;
				lock_vm();
				write_signal(signum, r_data << signal_shift, signal_mask);
				unlock_vm();
			}
		}
	}
	virtual void output_port_signals(bool force = false)
	{
		if((d_parent_device != nullptr) && (is_connected)) {
			if((parent_port_num >= 0) && (parent_port_num < 8)) {
				uint32_t r_data;
				r_data =  (uint32_t)(portval_data & 0x0f);
				if(force) {
					r_data |= (val_trig_a == true) ? 0x10 : 0x00;
					r_data |= (val_trig_b == true) ? 0x20 : 0x00;
				} else {p
					r_data |= (((val_trig_a) && (sig_trig_a)) == true) ? 0x10 : 0x00;
					r_data |= (((val_trig_b) && (sig_trig_b)) == true) ? 0x20 : 0x00;
				}
				int signum = (parent_port_num << 16) & 0x70000;
				signum = signum | SIG_JSPORT_DATA;
				lock_vm();
				write_signal(signum, r_data << signal_shift, signal_mask);
				unlock_vm();
			}
		}
	}
	void output_port_data(uint8_t data, bool later = true, bool force = false)
	{
		if(is_negative_logic) {
			data = ~data;
		}
		port_val = data & 0x0f;
		
		if(!(later)) {
			output_port_signals(force);
		}
	}
	
	void output_trig_a(bool val,  bool later = true, bool force = false)
	{
		val_trig_a = val;
		if(!(later)) {
			output_port_signals(force);
		}
	}
	void output_trig_b(bool val,  bool later = true, bool force = false)
	{
		val_trig_b = val;
		if(!(later)) {
			output_port_signals(force);
		}
	}

	virtual void hook_changes_trig_a(bool changed)
	{
		if(changed) {
			// Please write sequences.
			output_port_signals(force_output_on_change);
		}
	}
	virtual void hook_change_trig_b(bool changed)
	{
		if(changed) {
			// Please write sequences.
			output_port_signals(force_output_on_change);
		}
	}
	virtual void hook_changed_data(bool changed)
	{
		if(changed) {
			// Please write sequences.
			output_port_signals(force_output_on_change);
		}
	}
	
	virtual void hook_changed_com(bool changed)
	{
		if(changed) {
			// Please write sequences.
			output_port_com(val_com, force_output_on_change);
		}
	}

public:
	JSDEV_TEMPLATE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_parent_device = NULL;

		parent_port_num = -1;
		signal_shift = 0;
		signal_mask = 0xffffffff;
		
		is_connected = false;
		is_nagative_logic = false;
		force_output_on_change = false;
		need_unlock = false;
		initialize_status();
	}
	// common functions
	virtual void initialize(void)
	{
		need_unlock = true; // Available to lock_vm() / unlick_vm() on VM/EMU.
		reset_device();
	}
	
	virtual void release()
	{
	}
	
	// @note: DEVICE MUST NOT RESET WITHIN reset(), must reset within reset_device().
	virtual void reset()
	{
	}

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask)
	{
		// id_structure
		// Bit 0  - 15:  PERSONAL SIGNAL NUMBER.
		// Bit 16 - 18:  PORT NUMBER (0 - 7)
		int sig_num = id & 0xffff;
		int port_num = (id & 0x70000) >> 16;

		if(port_num != parent_port_num) return; 
		switch(sig_num) {
		case SIG_JS_TRIG_A:
			if(is_connected) {
				bool bak = sig_trig_a;
				sig_trig_a = ((data & mask) != 0) ? true : false;
				hook_changed_trig_a((bak != sig_trig_a) ? true : false);
			}
			break;
		case SIG_JS_TRIG_B:
			if(is_connected) {
				bool bak = sig_trig_b;
				sig_trig_b = ((data & mask) != 0) ? true : false;
				hook_changed_trig_b((bak != sig_trig_b) ? true : false);
			}
			break;
		case SIG_JS_COM:
			if(is_connected) {
				bool bak = sig_com;
				sig_com = ((data & mask) != 0) ? true : false;
				hook_changed_com((bak != sig_com) ? true : false);
			}
			break;
		case SIG_JS_DEVICE_RESET:
			if((data & mask) != 0) {
				reset_device();
			}
			break;
		}
	}
	
	virtual uint32_t __FASTCALL read_signal(int id)
	{
		switch(id) {
		case SIG_JS_TRIG_A_OUTPUT:
			return (val_trig_a == true) ? 0xffffffff : 0x00000000;
			break;
		case SIG_JS_TRIG_B_OUTPUT:
			return (val_trig_b == true) ? 0xffffffff : 0x00000000;
			break;
		case SIG_JS_COM_OUTPUT:
			return (val_com == true) ? 0xffffffff : 0x00000000;
			break;
		case SIG_JS_DATA:
			return ((uint32_t)portval_data) & 0x0f;
			break;
		case SIG_JS_TRIG_A:
			return (sig_trig_a == true) ? 0xffffffff : 0x00000000;
			break;
		case SIG_JS_TRIG_B:
			return (sig_trig_b == true) ? 0xffffffff : 0x00000000;
			break;
		case SIG_JS_COM:
			return (sig_com == true) ? 0xffffffff : 0x00000000;
			break;
		case SIG_JS_PORT_NUM:
			if((parent_port_num >= 0) && (parent_port_num < 8)) {
				return (uint32_t)parent_port_num;
			} else {
				return 0xffffffff;
			}
			break;
		case SIG_JS_CONNECTED:
			return (is_connected == true) ? 0xffffffff : 0;
			break;
		case SIG_JS_DATASHIFT:
			return (uint32_t)signal_shift;
			break;
		case SIG_JS_DATAMASK:
			return signal_mask;
			break;
		case SIG_JS_NAGATIVE_LOGIC:
			return (is_nagative_logic == true) ? 0xffffffff : 0;
			break;
		}
		return 0;
	}

	virtual void update_config()
	{
	}

#define TOWNS_JS_TEMPLATE_STATE_VERSION 1	
	virtual bool process_state(FILEIO* state_fio, bool loading)
	{
		if(!state_fio->StateCheckUint32(TOWNS_JS_TEMPLATE_STATE_VERSION)) {
			return false;
		}
		if(!state_fio->StateCheckInt32(this_device_id)) {
			return false;
		}
		state_fio->StateValue(is_connected);
		state_fio->StateValue(is_nagative_logic);
		
		state_fio->StateValue(parent_port_num);
		state_fio->StateValue(portval_data);

		
		state_fio->StateValue(sig_trig_a);
		state_fio->StateValue(sig_trig_b);
		state_fio->StateValue(sig_com);

		state_fio->StateValue(val_trig_a);
		state_fio->StateValue(val_trig_b);
		state_fio->StateValue(val_com);
		
		state_fio->StateValue(portval_data);

		state_fio->StateValue(signal_shift);
		state_fio->StateValue(signal_mask);
		
		return true;
	}
#undef TOWNS_JS_TEMPLATE_STATE_VERSION
	
	// unique functions
	// Set parent port
	void set_context_parent_port(int num, DEVICE* dev, int shift, uint32_t mask)
	{
		lock_vm();
		parent_port_num = num;
		d_parent_device = dev;
		
		signal_shift = shift;
		signal_mask = mask;
		unlock_vm();

		force_output_on_change  = false;
		reset_device()
	}

	virtual void remove_from_parent_port()
	{
		// You should make all events cancel here.
		lock_vm();
		parent_port_num = -1;
		signal_shift = 0;
		signal_mask = 0xffffffff;
		
		d_parent_device = NULL;
		is_connected = false;
		is_nagative_logic = false;
		force_output_on_change  = false;
		
		initialize_status();
		
		unlock_vm();
	}
	virtual void set_force_output_on_change(bool is_enable)
	{
		lock_vm();
		force_output_on_change = val;
		unlock_vm();
	}
	virtual void set_enable(bool is_enable)
	{
		lock_vm();
		if(is_enable) {
			// You may implement initialize sequence.
		} else {
			// You may implement removing sequence.
		}
		is_connected = is_enable;
		unlock_vm();
	}
	virtual void set_negative_logic(bool val)
	{
		lock_vm();
		is_negative_logic = val;
		unlock_vm();
	}
	virtual bool is_enabled()
	{
		return is_connected;
	}
};
}

