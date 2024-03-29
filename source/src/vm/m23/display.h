/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace M23 {
class DISPLAY : public DEVICE
{
private:
	uint8_t* regs;
	uint16_t cblink;
	bool hsync, vsync, display, blink;
	uint8_t vd_control;

	uint8_t screen[200][640];
	uint8_t font[0x1000];
	uint8_t* vram_t;
	uint8_t* vram_a;
	scrntype_t palette_pc[8];

	void draw_text();

public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}

	// common functions
	void initialize() override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	void event_frame() override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_vram_ptr(uint8_t* ptr)
	{
		vram_a = ptr;
		vram_t = ptr + 0x800;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};
}

#endif
