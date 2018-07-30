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
	uint8_t mode;
	uint8_t status;
	uint8_t* regs;
	uint8_t* vram;
	
	uint8_t screen[400][640];
	uint8_t font[0x800];
	scrntype_t palette_pc[2];
	int cblink;
	
	void draw_alpha();
	void draw_graph_320x200();
	void draw_graph_640x200();
	void draw_graph_640x400();
	
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
	
	// unique functions
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
	void draw_screen();
};

#endif

