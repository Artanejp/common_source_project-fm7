/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class KEYBOARD : public DEVICE
{
private:
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	uint32_t read_io8(uint32_t addr);
};

#endif
