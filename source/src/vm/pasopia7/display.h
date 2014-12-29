/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2007.02.08 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_I8255_0_A	0
#define SIG_DISPLAY_I8255_1_B	1
#define SIG_DISPLAY_I8255_1_C	2

class DISPLAY : public DEVICE
{
private:
	uint8* regs;
	uint8 mode, text_page;
	uint16 cursor, cblink, flash_cnt;
	bool blink, pal_dis;
	
	uint8 screen[200][640];
	uint8 font[0x800];
	uint8* vram_b;
	uint8* vram_r;
	uint8* vram_g;
	uint8* vram_a;
	uint8* pal;
	scrntype palette_pc[8];
	bool scanline;
	
	void draw_text_normal(uint16 src);
	void draw_text_wide(uint16 src);
	void draw_fine_normal(uint16 src);
	void draw_fine_wide(uint16 src);
	void draw_text_lcd(uint16 src);
	void draw_fine_lcd(uint16 src);
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void update_config();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique function
	void set_vram_ptr(uint8* ptr)
	{
		vram_b = ptr + 0x0000;
		vram_r = ptr + 0x4000;
		vram_g = ptr + 0x8000;
		vram_a = ptr + 0xc000;
	}
	void set_pal_ptr(uint8* ptr)
	{
		pal = ptr;
	}
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

