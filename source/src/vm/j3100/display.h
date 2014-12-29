/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

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
	uint8 mode;
	uint8 status;
	uint8* regs;
	uint8* vram;
	
	uint8 screen[400][640];
	uint8 font[0x800];
	scrntype palette_pc[2];
	int cblink;
	
	void draw_alpha();
	void draw_graph_320x200();
	void draw_graph_640x200();
	void draw_graph_640x400();
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique function
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void set_vram_ptr(uint8* ptr)
	{
		vram = ptr;
	}
	void draw_screen();
};

#endif

