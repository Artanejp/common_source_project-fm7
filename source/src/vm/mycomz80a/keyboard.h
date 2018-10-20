/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.18-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FIFO;

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pio1, *d_pio2;
	
	FIFO* key_buf;
	const uint8_t* key_stat;
	int key_code;
	bool kana;
	int event_cnt;
	
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
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio1(DEVICE* device)
	{
		d_pio1 = device;
	}
	void set_context_pio2(DEVICE* device)
	{
		d_pio2 = device;
	}
	void key_down(int code);
	void key_up(int code);
	bool get_caps_locked()
	{
//		return caps;
		return true;
	}
	bool get_kana_locked()
	{
		return kana;
	}
};

#endif

