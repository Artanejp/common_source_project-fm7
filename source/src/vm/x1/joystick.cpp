/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.16-

	[ joystick ]
*/

#include "joystick.h"
#include "../ym2203.h"

void JOYSTICK::initialize()
{
	joy_stat = emu->joy_buffer();
	
	// register event
	register_frame_event(this);
}

void JOYSTICK::event_frame()
{
	for(int i = 0; i < 2; i++) {
		uint8 val = 0xff;
#ifdef _X1TWIN
		if(!vm->cart_inserted(0)) {
#endif
			if(joy_stat[i] & 0x01) val &= ~0x01;
			if(joy_stat[i] & 0x02) val &= ~0x02;
			if(joy_stat[i] & 0x04) val &= ~0x04;
			if(joy_stat[i] & 0x08) val &= ~0x08;
			if(joy_stat[i] & 0x10) val &= ~0x20;
			if(joy_stat[i] & 0x20) val &= ~0x40;
#ifdef _X1TWIN
		}
#endif
		if(i == 0) {
			d_psg->write_signal(SIG_YM2203_PORT_A, val, 0xff);
		} else {
			d_psg->write_signal(SIG_YM2203_PORT_B, val, 0xff);
		}
	}
}

