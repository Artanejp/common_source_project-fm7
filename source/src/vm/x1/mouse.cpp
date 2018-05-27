/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2013.08.08-

	[ mouse ]
*/

#include "mouse.h"
#include "../z80sio.h"

void MOUSE::initialize()
{
	stat = emu->get_mouse_buffer();
}

void MOUSE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MOUSE_RTS) {
		if(data & mask) {
			return;
		}
		// Z80SIO Ch.B RTS H->L
		uint32_t d0 = (stat[0] >= 128 ? 0x10 : stat[0] < -128 ? 0x20 : 0) |
		            (stat[1] >= 128 ? 0x40 : stat[1] < -128 ? 0x80 : 0) |
		            ((stat[2] & 1) ? 1 : 0) | ((stat[2] & 2) ? 2 : 0);
		uint32_t d1 = (uint8_t)stat[0];
		uint32_t d2 = (uint8_t)stat[1];
		
		d_sio->write_signal(SIG_Z80SIO_CLEAR_CH1, 1, 1);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, d0, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, d1, 0xff);
		d_sio->write_signal(SIG_Z80SIO_RECV_CH1, d2, 0xff);
	}
}
