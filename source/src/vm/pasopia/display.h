/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2007.02.08 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_I8255_1_A	0

class DISPLAY : public DEVICE
{
private:
	DEVICE* d_crtc;
	
	uint8_t* regs;
	uint8_t mode;
	uint16_t cursor, cblink;
	
	uint8_t screen[200][640];
	uint8_t font[0x800];
	uint8_t* vram;
	uint8_t* attr;
	scrntype_t palette_pc[8];
	
	void draw_screen0_normal(uint16_t src);
	void draw_screen0_wide(uint16_t src);
	void draw_screen1_normal(uint16_t src);
	void draw_screen1_wide(uint16_t src);
	void draw_screen2_normal(uint16_t src);
	void draw_screen2_wide(uint16_t src);
	void draw_screen15_normal(uint16_t src);
	void draw_screen15_wide(uint16_t src);
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
	void set_attr_ptr(uint8_t* ptr)
	{
		attr = ptr;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

