/*
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ keyboard ]
*/

#ifndef _SL_KEYBOARD_H_
#define _SL_KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_TIMER	0

class FIFO;

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_pic;
	
	FIFO* key_buf;
	uint8_t key_code, key_ctrl;
	bool key_read;
	
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
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	void event_callback(int event_id, int err);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif

