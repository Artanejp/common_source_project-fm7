/*
	NEC PC-6001 Emulator 'yaPC-6001'

	Author : Takeda.Toshiya
	Date   : 2013.08.22-

	[ joystick ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MC6847;

namespace PC6001 {
	class TIMER;
}
namespace PC6001 {

class DISPLAY : public DEVICE
{
private:
	MC6847 *d_vdp;
	PC6001::TIMER *d_timer;
	
	uint8_t *ram_ptr;
	uint8_t *vram_ptr;

	int tmp_vram_size;
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_vdp(MC6847* device)
	{
		d_vdp = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		ram_ptr = vram_ptr = ptr;
	}
	void set_context_timer(PC6001::TIMER* device)
	{
		d_timer = device;
	}
	void draw_screen();
};

}
#endif
