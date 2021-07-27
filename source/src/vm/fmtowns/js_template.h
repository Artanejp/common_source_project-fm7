/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.07.26 -
    History : 2020.07.26 Initial.
	[ Towns Joystick devices: Template class]

*/

#pragma once

#include "../device.h"
#include "./joystick.h"

#include <mutex>

namespace FMTOWNS {
	
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
#define SIG_JS_DATA				0x14

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

enum {
	PAD_TYPE_NULL = 0,
	PAD_TYPE_2BUTTONS,
	PAD_TYPE_6BUTTONS,
	PAD_TYPE_ANALOG,
	PAD_TYPE_MOUSE,
	PAD_TYPE_END
};
	
class JSDEV_TEMPLATE : public DEVICE
{
protected:
	DEVICE* d_parent_device; // Normally FMTOWNS::JOYSTICK

	std::mutex _locker;
	
	bool is_connected;
	bool is_negative_logic; // DATA VALUES IS NAGATIVE LOGIC.
	bool force_output_on_change;
	int parent_port_num;
	int pad_num;
	int pad_type;
	
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

	std::unique_lock<std::mutex> lock_device()
	{
		return  std::unique_lock<std::mutex>(_locker, std::adopt_lock);
	}
	void unlock_device(std::unique_lock<std::mutex> _l)
	{
		std::mutex* p = _l.release();
		if(p != nullptr) {
			p->unlock();
		}
	}

	virtual void initialize_status()
	{
		//std::unique_lock<std::mutex> _l = lock_device();
		sig_trig_a = false;
		sig_trig_b = false;
		sig_com = false;

		val_trig_a = true;
		val_trig_b = true;
		val_com = sig_com;

		portval_data = 0x00;

	}
	
	virtual uint8_t  __FASTCALL output_port_com(bool val, bool force = false)
	{
		//std::unique_lock<std::mutex> _l = lock_device();

		val_com = val;
		if(!(force)) {
			val = val & sig_com;
		}
		if(is_negative_logic) {
			val = ~(val);
		}
		if((d_parent_device != nullptr) && (is_connected)) {
			if((parent_port_num >= 0) && (parent_port_num < 8)) {
				uint32_t r_data = (val) ? 0x40 : 0x0;
				int signum = (parent_port_num << 16) & 0x70000;
				signum = signum | SIG_JSPORT_COM;
				d_parent_device->write_signal(signum, r_data << signal_shift, signal_mask);
				return r_data;
			}
		}
		return 0;
	}
	
	virtual uint8_t  __FASTCALL output_port_signals(bool force = false)
	{
//		std::unique_lock<std::mutex> _l = lock_device();

		if((d_parent_device != nullptr) && (is_connected)) {
			if((parent_port_num >= 0) && (parent_port_num < 8)) {
				uint32_t r_data;
				r_data =  (uint32_t)(portval_data & 0x0f);
				if(force) {
					r_data |= ((val_trig_a == true) ? 0x10 : 0x00);
					r_data |= ((val_trig_b == true) ? 0x20 : 0x00);
				} else {
					r_data |= ((((val_trig_a) && (sig_trig_a)) == true) ? 0x10 : 0x00);
					r_data |= ((((val_trig_b) && (sig_trig_b)) == true) ? 0x20 : 0x00);
				}
				if(is_negative_logic) {
					r_data = (~(r_data) & 0x3f);
				}
				int signum = (parent_port_num << 16) & 0x70000;
				signum = signum | SIG_JSPORT_DATA;
				d_parent_device->write_signal(signum, r_data << signal_shift, signal_mask);
				return r_data;
			}
		}
		return 0;
	}
	uint8_t __FASTCALL output_port_data(uint8_t data, bool later = true, bool force = false)
	{
		portval_data = data & 0x0f;
		if(!(later)) {
			return output_port_signals(force);
		}
		return 0;
	}
	
	uint8_t __FASTCALL output_trig_a(bool val,  bool later = true, bool force = false)
	{
		val_trig_a = val;
		if(!(later)) {
			return output_port_signals(force);
		}
		return 0;
	}
	uint8_t __FASTCALL output_trig_b(bool val,  bool later = true, bool force = false)
	{
		val_trig_b = val;
		if(!(later)) {
			return output_port_signals(force);
		}
		return 0;
	}

	virtual uint8_t __FASTCALL hook_changed_trig_a(bool changed)
	{
		if(changed) {
			// Please write sequences.
			return output_port_signals(force_output_on_change);
		}
		return 0;
	}
	virtual uint8_t __FASTCALL hook_changed_trig_b(bool changed)
	{
		if(changed) {
			// Please write sequences.
			return output_port_signals(force_output_on_change);
		}
		return 0;
	}
	virtual uint8_t __FASTCALL hook_changed_data(bool changed)
	{
		if(changed) {
			// Please write sequences.
			return output_port_signals(force_output_on_change);
		}
		return 0;
	}
	
	virtual uint8_t __FASTCALL hook_changed_com(bool changed)
	{
		if(changed) {
			// Please write sequences.
			return output_port_com(val_com, force_output_on_change);
		}
		return 0;
	}

public:
	JSDEV_TEMPLATE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		pad_num = -1;
		parent_port_num = -1;
		
		signal_shift = 0;
		signal_mask = 0xffffffff;
		
		is_connected = false;
		is_negative_logic = false;
		force_output_on_change = false;
		
		set_device_name(_T("JOYSTICK TEMPLATE CLASS"));

		initialize_status();
	}
	// common functions
	virtual void initialize(void)
	{
		pad_type = PAD_TYPE_NULL;
		reset_device();
	}
	
	virtual void release()
	{
		//std::unique_lock<std::mutex> _l = lock_device();
	}
	
	// @note: DEVICE MUST NOT RESET WITHIN reset(), must reset within reset_device().
	virtual void reset()
	{
	}

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask)
	{
		//std::unique_lock<std::mutex> _l = lock_device();
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
		//std::unique_lock<std::mutex> _l = lock_device();

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
			return (is_negative_logic == true) ? 0xffffffff : 0;
			break;
		}
		return 0;
	}

	virtual void update_config()
	{
	}

#define TOWNS_JS_TEMPLATE_STATE_VERSION 2	
	virtual bool process_state(FILEIO* state_fio, bool loading)
	{
		//std::unique_lock<std::mutex> _l = lock_device();

		if(!state_fio->StateCheckUint32(TOWNS_JS_TEMPLATE_STATE_VERSION)) {
			return false;
		}
		if(!state_fio->StateCheckInt32(this_device_id)) {
			return false;
		}
		state_fio->StateValue(is_connected);
		state_fio->StateValue(is_negative_logic);
		
		state_fio->StateValue(pad_num);
		state_fio->StateValue(pad_type);
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
	virtual uint8_t __FASTCALL query(bool& status)
	{
		status = false;
		return 0x00;
	}
	void set_context_pad_num(int num)
	{
		//std::unique_lock<std::mutex> _l = lock_device();
		pad_num = num;
	}
	
	void set_context_parent_port(int num, DEVICE* dev, int shift, uint32_t mask)
	{
		//std::unique_lock<std::mutex> _l = lock_device();

		parent_port_num = num;
		d_parent_device = dev;
		
		signal_shift = shift;
		signal_mask = mask;

		force_output_on_change  = false;
		reset_device();
	}

	virtual void remove_from_parent_port()
	{
		// You should make all events cancel here.
		//std::unique_lock<std::mutex> _l = lock_device();

		parent_port_num = -1;
		d_parent_device = NULL;
		pad_num = -1;
		pad_type = 0;
		signal_shift = 0;
		signal_mask = 0xffffffff;
		
		is_connected = false;
		force_output_on_change  = false;

		//unlock_device(_l);
		
		initialize_status();
		
	}
	virtual void set_force_output_on_change(bool val)
	{
		//std::unique_lock<std::mutex> _l = lock_device();

		force_output_on_change = val;

	}
	// @note: DEVICE MUST NOT RESET WITHIN reset(), must reset within reset_device().
	virtual void reset_device()
	{
		initialize_status();
		output_port_signals(false);
		output_port_com(val_com, false);
	}
	virtual void set_enable(bool is_enable)
	{
		//std::unique_lock<std::mutex> _l = lock_device();

		if(is_enable) {
			// You may implement initialize sequence.
		} else {
			// You may implement removing sequence.
		}
		is_connected = is_enable;

	}
	virtual void set_negative_logic(bool val)
	{
		//std::unique_lock<std::mutex> _l = lock_device();

		is_negative_logic = val;
	}
	int get_pad_num()
	{
		return pad_num;
	}
	int get_pad_type()
	{
		return pad_type;
	}
	int get_parent_port_num()
	{
		return parent_port_num;
	}
	virtual bool is_enabled()
	{
		return is_connected;
	}
};
}

