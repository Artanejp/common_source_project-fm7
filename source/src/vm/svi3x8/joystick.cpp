/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/joystick.cpp

	modified by tanam
	Date   : 2018.12.09-

	[ joystick ]
*/

#include "joystick.h"
#include "memory_ex.h"
#include "../ay_3_891x.h"

namespace SVI_3X8 {
#if defined(MSX_KEYBOARD_50ON)
#define PSG14_MASK 0x3f
#else
#define PSG14_MASK 0x7f
#endif

void JOYSTICK::initialize()
{
//	joy_stat = emu->get_joy_buffer();
	select = 0;
	
	// register event to update the key status
	register_frame_event(this);
}

void JOYSTICK::event_frame()
{
	uint32_t _n[2];
	joy_stat = emu->get_joy_buffer();
	_n[0] = joy_stat[0];
	_n[1] = joy_stat[1];
	emu->release_joy_buffer(joy_stat);
///	d_psg->write_signal(SIG_AY_3_891X_PORT_A, PSG14_MASK & ~(_n[select] & 0x3f), 0x7f);
	d_psg->write_signal(SIG_AY_3_891X_PORT_A, ~((_n[0] & 0x0f)|(_n[1] & 0x0f)<<4), 0xff);
	d_memory->write_io8(select, ~((_n[0] & 0x10)|(_n[1] & 0x10)<<1));
}

#define STATE_VERSION	1

bool JOYSTICK::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(select);
	return true;
}
}
