/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_ACKC	0
#define SIG_KEYBOARD_STC	1
#define SIG_KEYBOARD_DC		2

class FIFO;

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_subcpu, *d_ls244;
	const uint8_t *key_stat;
	
	FIFO *key_buf;
	int phase;
	uint16_t send_data;
	uint32_t stc_clock;
	uint8_t recv_data;
	bool recv_ok;
	bool stc, dc;
	bool caps, kana;
	bool pro_mode;
	
	void drive();
	void set_stk(bool value);
	void set_dk(bool value);
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_subcpu(DEVICE* device)
	{
		d_subcpu = device;
	}
	void set_context_ls244(DEVICE* device)
	{
		d_ls244 = device;
	}
	void key_down(int code);
	void key_up(int code);
	bool get_caps_locked()
	{
		return caps;
	}
	bool get_kana_locked()
	{
		return kana;
	}
};

#endif
