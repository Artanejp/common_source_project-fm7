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
	uint8 key_code, key_ctrl;
	bool key_read;
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
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

