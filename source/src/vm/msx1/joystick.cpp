/*
	ASCII MSX1 Emulator 'yaMSX1'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya, umaiboux

	[ joystick ]
*/

#include "joystick.h"
#include "../ym2203.h"

void JOYSTICK::initialize()
{
	joy_stat = emu->joy_buffer();
	select = 0;
	
	// register event to update the key status
	register_frame_event(this);
}

void JOYSTICK::event_frame()
{
	d_psg->write_signal(SIG_YM2203_PORT_A, ~(joy_stat[select] & 0x3f), 0x7f);
}

void JOYSTICK::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_JOYSTICK_SEL) {
		if (select != ((data & mask) != 0)) {
			select = ((data & mask) != 0);
			d_psg->write_signal(SIG_YM2203_PORT_A, ~(joy_stat[select] & 0x3f), 0x7f);
		}
	}
}
