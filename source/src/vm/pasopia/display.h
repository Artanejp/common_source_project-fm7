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
	
	uint8* regs;
	uint8 mode;
	uint16 cursor, cblink;
	
	uint8 screen[200][640];
	uint8 font[0x800];
	uint8* vram;
	uint8* attr;
	scrntype palette_pc[8];
	
	void draw_screen0_normal(uint16 src);
	void draw_screen0_wide(uint16 src);
	void draw_screen1_normal(uint16 src);
	void draw_screen1_wide(uint16 src);
	void draw_screen2_normal(uint16 src);
	void draw_screen2_wide(uint16 src);
	void draw_screen15_normal(uint16 src);
	void draw_screen15_wide(uint16 src);
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_vram_ptr(uint8* ptr)
	{
		vram = ptr;
	}
	void set_attr_ptr(uint8* ptr)
	{
		attr = ptr;
	}
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

