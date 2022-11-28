/*
	Gijutsu-Hyoron-Sha Babbage-2nd Emulator 'eBabbage-2nd'

	Author : Takeda.Toshiya
	Date   : 2009.12.26 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_7SEG_LED	0
#define SIG_DISPLAY_8BIT_LED	1

class DISPLAY : public DEVICE
{
private:
	scrntype_t screen[36][256];
	
	int seg[6][7];
	uint8_t ls373;
	uint8_t pio_7seg;
	uint8_t pio_8bit;
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("7-Segment LEDs"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void draw_screen();
};

#endif

