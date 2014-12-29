/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_TIMER	0

class FIFO;

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_pic;
	
	FIFO* recv_buf;
	uint8 recv_data;
	
	int kbic_cmd, kbsc_cmd;
	FIFO* cmd_param
	uint8 kbic_status;
	
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
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif

