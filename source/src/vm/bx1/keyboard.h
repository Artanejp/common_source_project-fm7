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

class FIFO;

namespace BX1 {
class KEYBOARD : public DEVICE
{
private:
	FIFO *fifo_down;
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		fifo_down = NULL;
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	virtual void __FASTCALL key_down(int code);
	virtual void __FASTCALL key_up(int code);
};
}

#endif

