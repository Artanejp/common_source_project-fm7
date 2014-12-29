/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.18-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_ADDR_L	0
#define SIG_DISPLAY_ADDR_H	1
#define SIG_DISPLAY_MODE	2

class DISPLAY : public DEVICE
{
private:
	uint8* regs;
	bool chr, wide;
	uint16 cursor, cblink;
	bool scanline;
	
	uint8 screen[200][640];
	uint8 font[0x800], cg[0x800];
	uint8 vram[0x800];
	uint16 vram_addr;
	
	void draw_40column();
	void draw_80column();
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void update_config();
	
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unique function
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

