/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_RST	0
#define SIG_KEYBOARD_RECV	1

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_sio;
	
	bool kana, caps, rst;
	uint8 flag[256];
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif
