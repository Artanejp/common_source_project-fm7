/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2011.03.28-

	[ joystick ]
*/

#include "joystick.h"
#include "../ym2203.h"
#include "../../fileio.h"

void JOYSTICK::initialize()
{
	joy_status = emu->joy_buffer();
	select = 0xff;
	
	register_frame_event(this);
}

void JOYSTICK::write_signal(int id, uint32 data, uint32 mask)
{
	// ym2203 port-b
	select = data & mask;
}

void JOYSTICK::event_frame()
{
	if(select & 0x80) {
		d_opn->write_signal(SIG_YM2203_PORT_A, ~joy_status[(select & 0x40) >> 6], 0x3f);
	}
}

#define STATE_VERSION	1

void JOYSTICK::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(select);
}

bool JOYSTICK::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	select = state_fio->FgetUint8();
	return true;
}

