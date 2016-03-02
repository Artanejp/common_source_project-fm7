/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ display ]
*/

#include "display.h"
#include "../i8259.h"

void DISPLAY::initialize()
{
	register_vline_event(this);
}

void DISPLAY::reset()
{
	vsync_enb = true;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x64:
		vsync_enb = true;
		break;
	}
}

void DISPLAY::event_vline(int v, int clock)
{
	if(v == 400 && vsync_enb) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR2, 1, 1);
		vsync_enb = false;
	}
}

void DISPLAY::draw_screen()
{
	emu->screen_skip_line(false);
}

