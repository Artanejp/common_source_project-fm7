/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ joystick ]
*/

#include "joystick.h"
#if defined(_PC6001MK2SR) || defined(_PC6601SR)
#include "../ym2203.h"
#else
#include "../ay_3_891x.h"
#endif

void JOYSTICK::initialize()
{
	joy_stat = emu->get_joy_buffer();
	
	// register event to update the key status
	register_frame_event(this);
}

void JOYSTICK::event_frame()
{
#if defined(_PC6001MK2SR) || defined(_PC6601SR)
	d_psg->write_signal(SIG_YM2203_PORT_A, ~(joy_stat[0] & 0x3f), 0xff);
	d_psg->write_signal(SIG_YM2203_PORT_B, ~(joy_stat[1] & 0x1f), 0xff);
#else
	d_psg->write_signal(SIG_AY_3_891X_PORT_A, ~(joy_stat[0] & 0x3f), 0xff);
	d_psg->write_signal(SIG_AY_3_891X_PORT_B, ~(joy_stat[1] & 0x1f), 0xff);
#endif
}
