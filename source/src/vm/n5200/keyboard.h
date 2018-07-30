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
	uint8_t flag[256];
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	
	// unique functions
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void key_down(int code);
	void key_up(int code);
	bool get_caps_locked()
	{
		return caps;
	}
	bool get_kana_locked()
	{
		return kana;
	}
};

#endif
