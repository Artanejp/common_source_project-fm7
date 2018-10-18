/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2007.02.08 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_I8255_B	0

class DISPLAY : public DEVICE
{
private:
	uint8_t* regs;
	uint8_t pal[8];
	bool text_wide, text_color;
	uint8_t graph_color, graph_page;
	uint16_t cursor, cblink;
	bool hsync, vsync, display, blink;
	
	uint8_t screen[200][640];
	uint8_t font[0x800];
	uint8_t* vram_b;
	uint8_t* vram_r;
	uint8_t* vram_g;
	uint8_t* vram_t;
	uint8_t* vram_a;
	scrntype_t palette_pc[8];
	
	void draw_graph_color();
	void draw_graph_mono();
	void draw_text_wide();
	void draw_text_normal();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_vram_ptr(uint8_t* ptr)
	{
		vram_b = ptr + 0x0000;
		vram_r = ptr + 0x4000;
		vram_g = ptr + 0x8000;
		vram_t = ptr + 0xc000;
		vram_a = ptr + 0xc800;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

