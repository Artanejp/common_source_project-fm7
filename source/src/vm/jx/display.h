/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.09-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_ENABLE	0
#define SIG_DISPLAY_VBLANK	1
#define SIG_DISPLAY_PIO		2

class MEMORY;

class DISPLAY : public DEVICE
{
private:
	MEMORY *d_mem;
	
	uint8 vram[0x20000];
	uint8 extvram[0x10000];
	
	uint8 vgarray[0x10];
	uint8 palette[0x10];
	int vgarray_num;
	
	uint8 bankreg[16];
	int bankreg_num;
	
	uint8 hires_mode;
	int prev_width, prev_height;
	uint8 page;
	uint8 status;
	uint8* regs;
	
	uint8 screen[512][720];
	uint8 *font;
	uint8 *kanji;
	scrntype palette_pc[32];
	int cblink;
	bool scanline;
	
	void draw_alpha();
	void draw_graph_160x200_16col();
	void draw_graph_320x200_4col();
	void draw_graph_320x200_16col();
	void draw_graph_640x200_2col();
	void draw_graph_640x200_4col();
	void draw_graph_720x512_2col();
	void draw_graph_360x512_4col();
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void update_config();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique function
	void set_context_mem(MEMORY* device)
	{
		d_mem = device;
	}
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void set_font_ptr(uint8* ptr)
	{
		font = ptr;
	}
	void set_kanji_ptr(uint8* ptr)
	{
		kanji = ptr;
	}
	void draw_screen();
};

#endif

