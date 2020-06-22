/*
	National JR-800 Emulator 'eJR-800'

	Author : Takeda.Toshiya
	Origin : PockEmul
	Date   : 2017.03.13-

	[ memory mapped i/o ]
*/

#ifndef _JR800_IO_H_
#define _JR800_IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class HD44102;

namespace JR800 {
	
class IO : public DEVICE
{
private:
	HD44102 *d_lcd[8];
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Mapped I/O"));
	}
	~IO() {}
	
	// common functions
	void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_lcd(int i, HD44102 *device)
	{
		d_lcd[i] = device;
	}
};

}
#endif

