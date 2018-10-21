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
	struct {
		uint8_t vram[4][50];
		int updown, disp, spg, page, ofs, ofs2;
	} seg[10];
	uint16_t sel;
	
	uint8_t screen[64][250];
	
public:
	LCD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("LCD"));
	}
	~LCD() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void draw_screen();
};

#endif
