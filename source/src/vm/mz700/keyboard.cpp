/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"
#include "../../fileio.h"

static const int key_map[10][8] = {
#if defined(_MZ800)
	{0x0d, 0xba, 0xbb, 0x14, 0x09, 0x78, 0x21, 0x22},
#else
	{0x0d, 0xba, 0xbb, 0x00, 0x09, 0x78, 0x21, 0x22},
#endif
	{0x00, 0x00, 0x00, 0xdd, 0xdb, 0xc0, 0x5a, 0x59},
	{0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51},
	{0x50, 0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49},
	{0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41},
	{0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31},
	{0xbe, 0xbc, 0x39, 0x30, 0x20, 0xbd, 0xde, 0xdc},
	{0xbf, 0xe2, 0x25, 0x27, 0x28, 0x26, 0x2e, 0x2d},
	{0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x08},
	{0x00, 0x00, 0x00, 0x74, 0x73, 0x72, 0x71, 0x70}
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	column = 0;
	
	// register event
	register_frame_event(this);
}

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	column = data & 0x0f;
	update_key();
}

void KEYBOARD::event_frame()
{
	update_key();
}

void KEYBOARD::update_key()
{
	uint8 stat = 0xff;
	
	if(column < 10) {
		for(int i = 0; i < 8; i++) {
			if(key_stat[key_map[column][i]]) {
				stat &= ~(1 << i);
			}
		}
	}
	d_pio->write_signal(SIG_I8255_PORT_B, stat, 0xff);
}

#define STATE_VERSION	1

void KEYBOARD::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(column);
}

bool KEYBOARD::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	column = state_fio->FgetUint8();
	return true;
}

