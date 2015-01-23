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
	uint8* regs;
	uint8 pal[8];
	bool text_wide, text_color;
	uint8 graph_color, graph_page;
	uint16 cursor, cblink;
	bool hsync, vsync, display, blink;
	
	uint8 screen[200][640];
	uint8 font[0x800];
	uint8* vram_b;
	uint8* vram_r;
	uint8* vram_g;
	uint8* vram_t;
	uint8* vram_a;
	scrntype palette_pc[8];
	
	void draw_graph_color();
	void draw_graph_mono();
	void draw_text_wide();
	void draw_text_normal();
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_vram_ptr(uint8* ptr)
	{
		vram_b = ptr + 0x0000;
		vram_r = ptr + 0x4000;
		vram_g = ptr + 0x8000;
		vram_t = ptr + 0xc000;
		vram_a = ptr + 0xc800;
	}
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

