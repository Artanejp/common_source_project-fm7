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
	uint8 *key_stat;
	
	FIFO *key_buf;
	int phase;
	uint16 send_data;
	uint32 stc_clock;
	uint8 recv_data;
	bool recv_ok;
	bool stc, dc;
	bool caps, kana;
	bool pro_mode;
	
	void drive();
	void set_stk(bool value);
	void set_dk(bool value);
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
};

#endif
