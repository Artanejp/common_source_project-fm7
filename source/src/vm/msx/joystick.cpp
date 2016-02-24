/*
	ASCII MSX1 Emulator 'yaMSX1'
	ASCII MSX2 Emulator 'yaMSX2'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya
	modified by umaiboux

	[ joystick ]
*/

#include "joystick.h"
#include "../ym2203.h"

void JOYSTICK::initialize()
{
	joy_stat = emu->get_joy_buffer();
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
		if(select != ((data & mask) != 0)) {
			select = ((data & mask) != 0);
			d_psg->write_signal(SIG_YM2203_PORT_A, ~(joy_stat[select] & 0x3f), 0x7f);
		}
	}
}

#define STATE_VERSION	1

void JOYSTICK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(select);
}

bool JOYSTICK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	select = state_fio->FgetInt32();
	return true;
}

