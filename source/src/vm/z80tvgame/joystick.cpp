/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ joystick ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#include "joystick.h"
#ifdef _USE_I8255
#include "../i8255.h"
#else
#include "../z80pio.h"
#endif

void JOYSTICK::initialize()
{
	joy_stat = emu->get_joy_buffer();
	register_frame_event(this);
}

void JOYSTICK::event_frame()
{
	uint32_t val = 0x3f;
	if(joy_stat[0] & 0x01) val &= ~0x01;	// up
	if(joy_stat[0] & 0x02) val &= ~0x02;	// dpwn
	if(joy_stat[0] & 0x04) val &= ~0x04;	// left
	if(joy_stat[0] & 0x08) val &= ~0x08;	// right
	if(joy_stat[0] & 0x10) val &= ~0x10;	// trigger #1
	if(joy_stat[0] & 0x20) val &= ~0x20;	// trigger #2
#ifdef _USE_I8255
	d_pio->write_signal(SIG_I8255_PORT_A, val, 0xff);
#else
	d_pio->write_signal(SIG_Z80PIO_PORT_A, val, 0xff);
#endif
}

