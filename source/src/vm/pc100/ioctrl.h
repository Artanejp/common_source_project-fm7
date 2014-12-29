/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.14 -

	[ i/o controller ]
*/

#ifndef _IOCTRL_H_
#define _IOCTRL_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IOCTRL_RESET	0

class FIFO;

class IOCTRL : public DEVICE
{
private:
	DEVICE *d_pic, *d_fdc, *d_beep, *d_pcm;
	
	void update_key();
	uint8* key_stat;
	int* mouse_stat;
	bool caps, kana;
	FIFO* key_buf;
	uint32 key_val, key_mouse;
	int key_prev;
	bool key_res, key_done;
	int register_id;
	uint8 ts;
	
public:
	IOCTRL(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IOCTRL() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_beep(DEVICE* device)
	{
		d_beep = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif

