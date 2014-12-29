/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.04.01-

	[ lcd ]
*/

#ifndef _LCD_H_
#define _LCD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_LCD_CHIPSEL_L	0
#define SIG_LCD_CHIPSEL_H	1

class LCD : public DEVICE
{
private:
	typedef struct {
		uint8 vram[4][50];
		int updown, disp, spg, page, ofs, ofs2;
	} seg_t;
	seg_t seg[10];
	uint16 sel;
	
	uint8 screen[64][250];
	
public:
	LCD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~LCD() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void draw_screen();
};

#endif
