/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.06-

	[ joystick ]
*/

#include "joystick.h"
#include "../ay_3_891x.h"

void JOYSTICK::initialize()
{
	joy_stat = emu->get_joy_buffer();
	
	// register event to update the key status
	register_frame_event(this);
}

void JOYSTICK::event_frame()
{
	d_psg->write_signal(SIG_AY_3_891X_PORT_A, ~(joy_stat[0] & 0x1f), 0xff);
	d_psg->write_signal(SIG_AY_3_891X_PORT_B, ~(joy_stat[1] & 0x1f), 0xff);
}
