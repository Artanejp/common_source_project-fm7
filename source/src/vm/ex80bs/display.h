/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_PC		0

namespace EX80BS {
class DISPLAY : public DEVICE
{
private:
	DEVICE *d_cpu;
	uint8_t font[0x400];	// EX-80
	uint8_t font1[0x800];	// EX-80BS
	uint8_t font2[0x800];	// EX-80BS (user defined)
	uint8_t screen[TV_HEIGHT][TV_WIDTH];
	
	uint8_t *ram;
	uint8_t *vram;
	int odd_even;
	uint8_t pc;
	
	void draw_tv();
	void draw_bs();

public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void event_frame() override;
	void event_vline(int v, int clock) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_ram_ptr(uint8_t* ptr)
	{
		ram = ptr;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
	void draw_screen();
};
}
#endif

