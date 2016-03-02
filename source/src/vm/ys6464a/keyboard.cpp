/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
}

void KEYBOARD::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint8_t val = 0x0f;
	
	switch(data & 0xf0) {
	case 0x80:
		if(key_stat[0x30]) val &= ~1;	// 0
		if(key_stat[0x34]) val &= ~2;	// 4
		if(key_stat[0x38]) val &= ~4;	// 8
		if(key_stat[0x43]) val &= ~8;	// c
		break;
	case 0x90:
		if(key_stat[0x31]) val &= ~1;	// 1
		if(key_stat[0x35]) val &= ~2;	// 5
		if(key_stat[0x39]) val &= ~4;	// 9
		if(key_stat[0x44]) val &= ~8;	// d
		break;
	case 0xa0:
		if(key_stat[0x32]) val &= ~1;	// 2
		if(key_stat[0x36]) val &= ~2;	// 6
		if(key_stat[0x41]) val &= ~4;	// a
		if(key_stat[0x45]) val &= ~8;	// e
		break;
	case 0xb0:
		if(key_stat[0x33]) val &= ~1;	// 3
		if(key_stat[0x37]) val &= ~2;	// 7
		if(key_stat[0x42]) val &= ~4;	// b
		if(key_stat[0x46]) val &= ~8;	// f
		break;
	case 0xc0:
		if(key_stat[0x70]) val &= ~1;	// wr inc
		if(key_stat[0x71]) val &= ~2;	// rd dec
		if(key_stat[0x72]) val &= ~4;	// rd inc
		if(key_stat[0x73]) val &= ~8;	// ad run
		break;
	}
	d_pio->write_signal(SIG_I8255_PORT_C, val, 0xf);
}

