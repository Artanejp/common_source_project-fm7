/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2004.09.05 -

	[ mouse ]
*/

#include "mouse.h"
#include "../z80sio.h"
#include "../../fileio.h"

void MOUSE::initialize()
{
	stat = emu->mouse_buffer();
	select = false;
}

void MOUSE::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MOUSE_SEL) {
		select = ((data & mask) != 0);
	} else if(id == SIG_MOUSE_DTR) {
		if(!select || (data & mask)) {
			return;
		}
		// Z80SIO Ch.B DTR H->L
		uint32 d0 = (stat[0] >= 128 ? 0x10 : stat[0] < -128 ? 0x20 : 0) |
		            (stat[1] >= 128 ? 0x40 : stat[1] < -128 ? 0x80 : 0) |
		            ((stat[2] & 1) ? 1 : 0) | ((stat[2] & 2) ? 2 : 0);
		uint32 d1 = (uint8)stat[0];
		uint32 d2 = (uint8)stat[1];
		
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH1, 1, 1);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, d0, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, d1, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, d2, 0xff);
	}
}

#define STATE_VERSION	1

void MOUSE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(select);
}

bool MOUSE::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	select = state_fio->FgetBool();
	return true;
}

