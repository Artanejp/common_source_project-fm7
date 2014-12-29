/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ EPOCH TV-1 ]
*/

#ifndef _VDP_H_
#define _VDP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class VDP : public DEVICE
{
private:
	DEVICE* d_cpu;
	
	uint8* vram0;
	uint8* vram1;
	uint8* font_ptr;
	uint8 text[320][320];
	uint8 sprite[320][320];
	uint8 font[128][8];
	uint8 vdc0, vdc1, vdc2, vdc3;
	
	void draw_text_screen();
	void draw_text(int dx, int dy, uint8 data, uint8 tcol, uint8 bcol);
	void draw_block(int dx, int dy, uint8 data);
	void draw_graph(int dx, int dy, uint8 data, uint8 col);
	
	void draw_sprite_screen();
	void draw_sprite(int dx, int dy, int sx, int ex, int sy, int ey, int no, uint8 col);
	
public:
	VDP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~VDP() {}
	
	// common functions
	void initialize();
	void event_vline(int v, int clock);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_font_ptr(uint8* ptr)
	{
		font_ptr = ptr;
	}
	void set_vram_ptr(uint8* ptr)
	{
		vram0 = ptr; vram1 = ptr + 0x1000;
	}
	void draw_screen();
};

#endif
