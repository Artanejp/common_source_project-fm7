/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_PORT_B	0
#define SIG_DISPLAY_PORT_C	1

class DISPLAY : public DEVICE
{
private:
	int seg[6][8];
	uint8 pb, pc;
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_vline(int v, int clock);
	
	// unique function
	void draw_screen();
};

#endif

