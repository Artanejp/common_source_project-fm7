/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ keyboard ]
*/

#include "keyboard.h"

static const int key_map[9][8] = {
	{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37},
	{0x38, 0x39, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf},
	{0xc0, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47},
	{0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f},
	{0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57},
	{0x58, 0x59, 0x5a, 0xdb, 0xdc, 0xdd, 0xde, 0xe2},
	{0x20, 0x28, 0x26, 0x27, 0x25, 0x24, 0x00, 0x2e},
	{0x00, 0x00, 0x00, 0x08, 0x00, 0x0d, 0x00, 0x14},
	{0x11, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void KEYBOARD::initialize()
{
	key_stat = emu->key_buffer();
	joy_stat = emu->joy_buffer();
	
	// register event to update the key status
	register_frame_event(this);
}

void KEYBOARD::write_io8(uint32 addr, uint32 data)
{
	column = data;
}

uint32 KEYBOARD::read_io8(uint32 addr)
{
	return (column == 0x30) ? status[15] : (1 <= column && column <= 15) ? status[column - 1] : 0;
}

void KEYBOARD::event_frame()
{
	memset(status, 0, sizeof(status));
	
	for(int i = 0; i < 9; i++) {
		uint8 val = 0;
		val |= key_stat[key_map[i][0]] ? 0x01 : 0;
		val |= key_stat[key_map[i][1]] ? 0x02 : 0;
		val |= key_stat[key_map[i][2]] ? 0x04 : 0;
		val |= key_stat[key_map[i][3]] ? 0x08 : 0;
		val |= key_stat[key_map[i][4]] ? 0x10 : 0;
		val |= key_stat[key_map[i][5]] ? 0x20 : 0;
		val |= key_stat[key_map[i][6]] ? 0x40 : 0;
		val |= key_stat[key_map[i][7]] ? 0x80 : 0;
		status[i] = val;
	}
	
	// joystick #1
	switch(joy_stat[0] & 0x0f) {
		case 0x1: status[ 9] |= 0x11; break;	// u
		case 0x2: status[11] |= 0x11; break;	// d
		case 0x4: status[ 9] |= 0x44; break;	// l
		case 0x5: status[ 9] |= 0x22; break;	// lu
		case 0x6: status[10] |= 0x11; break;	// ld
		case 0x8: status[11] |= 0x44; break;	// r
		case 0x9: status[10] |= 0x22; break;	// ru
		case 0xa: status[11] |= 0x22; break;	// rd
	}
	if(joy_stat[0] & 0x10) status[ 9] |= 0x88;	// b1
	if(joy_stat[0] & 0x20) status[11] |= 0x88;	// b2
	
	switch(joy_stat[1] & 0x0f) {
		case 0x1: status[12] |= 0x11; break;	// u
		case 0x2: status[14] |= 0x11; break;	// d
		case 0x4: status[12] |= 0x44; break;	// l
		case 0x5: status[12] |= 0x22; break;	// lu
		case 0x6: status[13] |= 0x11; break;	// ld
		case 0x8: status[14] |= 0x44; break;	// r
		case 0x9: status[13] |= 0x22; break;	// ru
		case 0xa: status[14] |= 0x22; break;	// rd
	}
	if(joy_stat[1] & 0x10) status[12] |= 0x88;	// b1
	if(joy_stat[1] & 0x20) status[14] |= 0x88;	// b2
	
	// $30
	uint8 total = 0;
	for(int i = 0; i < 15; i++) {
		total |= status[i];
	}
	status[15] = total;
}
