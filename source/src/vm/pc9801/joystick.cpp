/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2011.03.28-

	[ joystick ]
*/

#include "joystick.h"
#include "../ym2203.h"

void JOYSTICK::initialize()
{
	joy_status = emu->get_joy_buffer();
	select = 0xff;
	
	register_frame_event(this);
}

void JOYSTICK::write_signal(int id, uint32_t data, uint32_t mask)
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

