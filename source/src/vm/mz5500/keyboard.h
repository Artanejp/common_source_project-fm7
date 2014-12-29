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

#define SIG_KEYBOARD_INPUT	0

class FIFO;

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_pio, *d_pic;
	
	void drive();
	void process(int cmd);
	
	uint8 *key_stat;
	int *mouse_stat;
	FIFO *key_buf, *rsp_buf;
	bool caps, kana, graph;
	int dk, srk;	// to cpu
	int dc, stc;	// from cpu
	int send, recv;
	int phase, timeout;
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique function
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif
