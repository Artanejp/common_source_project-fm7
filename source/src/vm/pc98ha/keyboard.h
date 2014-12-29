/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_RECV	0

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_sio;
	
	bool kana, caps;
	uint8 flag[256];
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	
	// unique function
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif
