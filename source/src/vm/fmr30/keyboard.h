/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

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
	DEVICE *d_sio;
	uint8_t table[256];
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common function
	void reset();
	
	// unique functions
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif
