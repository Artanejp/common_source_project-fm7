/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm_template.h"
#include "../../emu_template.h"
#include "../device.h"

namespace BX1 {
class KEYBOARD : public DEVICE
{
private:
	int key_code;
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique functions
	void key_down(int code);
	void key_up(int code);
};
}

#endif

