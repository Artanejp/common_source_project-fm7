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
	
	uint8_t vram[0x20000];
	uint8_t extvram[0x10000];
	
	uint8_t vgarray[0x10];
	uint8_t palette[0x10];
	int vgarray_num;
	
	uint8_t bankreg[16];
	int bankreg_num;
	
	uint8_t hires_mode;
//	int prev_width, prev_height;
	uint8_t page;
	uint8_t status;
	uint8_t* regs;
	
	uint8_t screen[512][720];
	uint8_t *font;
	uint8_t *kanji;
	scrntype_t palette_pc[32];
	int cblink;
	
	void draw_alpha();
	void draw_graph_160x200_16col();
	void draw_graph_320x200_4col();
	void draw_graph_320x200_16col();
	void draw_graph_640x200_2col();
	void draw_graph_640x200_4col();
	void draw_graph_720x512_2col();
	void draw_graph_360x512_4col();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(MEMORY* device)
	{
		d_mem = device;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void set_font_ptr(uint8_t* ptr)
	{
		font = ptr;
	}
	void set_kanji_ptr(uint8_t* ptr)
	{
		kanji = ptr;
	}
	void draw_screen();
};

#endif

