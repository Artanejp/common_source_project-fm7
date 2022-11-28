/*
	National JR-800 Emulator 'eJR-800'

	Author : Takeda.Toshiya
	Origin : PockEmul
	Date   : 2017.03.13-

	[ memory mapped i/o ]
*/

#include "io.h"
#include "../hd44102.h"

void IO::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	int chip = 0;
	
	switch(addr & 0xf00) {
	case 0xa00:
	case 0xb00:
		switch(addr & 0xff) {
		case 0x01: chip = 0; break;
		case 0x02: chip = 1; break;
		case 0x04: chip = 2; break;
		case 0x08: chip = 3; break;
		case 0x10: chip = 4; break;
		case 0x20: chip = 5; break;
		case 0x40: chip = 6; break;
		case 0x80: chip = 7; break;
		}
		d_lcd[chip]->write(addr >> 8, data);
		break;
	}
}

uint32_t IO::read_memory_mapped_io8(uint32_t addr)
{
	const uint8_t* key_stat = emu->get_key_buffer();
	uint8_t data = 0;
	int chip = 0;
	
	switch(addr & 0xf00) {
	case 0xa00:
	case 0xb00:
		switch(addr & 0xff) {
		case 0x01: chip = 0; break;
		case 0x02: chip = 1; break;
		case 0x04: chip = 2; break;
		case 0x08: chip = 3; break;
		case 0x10: chip = 4; break;
		case 0x20: chip = 5; break;
		case 0x40: chip = 6; break;
		case 0x80: chip = 7; break;
		}
		return d_lcd[chip]->read(addr >> 8);
	case 0xd00:
	case 0xe00:
	case 0xf00:
		if(!(addr & 0x001)) {
			if(key_stat[0x60]) data |= 0x01; // NumPad 0
			if(key_stat[0x61]) data |= 0x02; // NumPad 1
			if(key_stat[0x62]) data |= 0x04; // NumPad 2
			if(key_stat[0x63]) data |= 0x08; // NumPad 3
			if(key_stat[0x64]) data |= 0x10; // NumPad 4
			if(key_stat[0x65]) data |= 0x20; // NumPad 5
			if(key_stat[0x66]) data |= 0x40; // NumPad 6
			if(key_stat[0x67]) data |= 0x80; // NumPad 7
		}
		if(!(addr & 0x002)) {
			if(key_stat[0x68]) data |= 0x01; // NumPad 8
			if(key_stat[0x69]) data |= 0x02; // NumPad 9
			if(key_stat[0x6a]) data |= 0x04; // NumPad *
			if(key_stat[0x6b]) data |= 0x08; // NumPad +
			if(key_stat[0x6c]) data |= 0x10; // NumPad =
			if(key_stat[0x6d]) data |= 0x20; // NumPad -
			if(key_stat[0x6e]) data |= 0x40; // NumPad .
			if(key_stat[0x6f]) data |= 0x80; // NumPad /
			//
			if(key_stat[0xdb]) data |= 0x04; // [ -> NumPad *
			if(key_stat[0xe2]) data |= 0x08; // _ -> NumPad +
			if(key_stat[0xbf]) data |= 0x10; // / -> NumPad =
			if(key_stat[0xdd]) data |= 0x20; // ] -> NumPad -
			if(key_stat[0xdc]) data |= 0x80; // \ -> NumPad /
		}
		if(!(addr & 0x004)) {
			if(key_stat[0x30]) data |= 0x01; // 0
			if(key_stat[0x31]) data |= 0x02; // 1
			if(key_stat[0x32]) data |= 0x04; // 2
			if(key_stat[0x33]) data |= 0x08; // 3
			if(key_stat[0x34]) data |= 0x10; // 4
			if(key_stat[0x35]) data |= 0x20; // 5
			if(key_stat[0x36]) data |= 0x40; // 6
			if(key_stat[0x37]) data |= 0x80; // 7
		}
		if(!(addr & 0x008)) {
			if(key_stat[0x38]) data |= 0x01; // 8
			if(key_stat[0x39]) data |= 0x02; // 9
			if(key_stat[0xba]) data |= 0x04; // :
			if(key_stat[0xbb]) data |= 0x08; // ;
			if(key_stat[0xbc]) data |= 0x10; // ,
			if(key_stat[0xde]) data |= 0x20; // ^
			if(key_stat[0xbe]) data |= 0x40; // .
			if(key_stat[0x24]) data |= 0x80; // Home
		}
		if(!(addr & 0x010)) {
			if(key_stat[0x20]) data |= 0x01; // Space
			if(key_stat[0x41]) data |= 0x02; // A
			if(key_stat[0x42]) data |= 0x04; // B
			if(key_stat[0x43]) data |= 0x08; // C
			if(key_stat[0x44]) data |= 0x10; // D
			if(key_stat[0x45]) data |= 0x20; // E
			if(key_stat[0x46]) data |= 0x40; // F
			if(key_stat[0x47]) data |= 0x80; // G
		}
		if(!(addr & 0x020)) {
			if(key_stat[0x48]) data |= 0x01; // H
			if(key_stat[0x49]) data |= 0x02; // I
			if(key_stat[0x4a]) data |= 0x04; // J
			if(key_stat[0x4b]) data |= 0x08; // K
			if(key_stat[0x4c]) data |= 0x10; // L
			if(key_stat[0x4d]) data |= 0x20; // M
			if(key_stat[0x4e]) data |= 0x40; // N
			if(key_stat[0x4f]) data |= 0x80; // O
		}
		if(!(addr & 0x040)) {
			if(key_stat[0x50]) data |= 0x01; // P
			if(key_stat[0x51]) data |= 0x02; // Q
			if(key_stat[0x52]) data |= 0x04; // R
			if(key_stat[0x53]) data |= 0x08; // S
			if(key_stat[0x54]) data |= 0x10; // T
			if(key_stat[0x55]) data |= 0x20; // U
			if(key_stat[0x56]) data |= 0x40; // V
			if(key_stat[0x57]) data |= 0x80; // W
		}
		if(!(addr & 0x080)) {
			if(key_stat[0x58]) data |= 0x01; // X
			if(key_stat[0x59]) data |= 0x02; // Y
			if(key_stat[0x5a]) data |= 0x04; // Z
			if(key_stat[0x2d]) data |= 0x08; // Ins
			if(key_stat[0x27]) data |= 0x10; // Right
			if(key_stat[0x26]) data |= 0x20; // Up
			if(key_stat[0x0d]) data |= 0x40; // Enter
			if(key_stat[0x03]) data |= 0x80; // Break
		}
		if(!(addr & 0x100)) {
			if(key_stat[0x70]) data |= 0x01; // F1
			if(key_stat[0x71]) data |= 0x02; // F2
			if(key_stat[0x72]) data |= 0x04; // F3
			if(key_stat[0x73]) data |= 0x08; // F4
			if(key_stat[0x74]) data |= 0x10; // F5
			if(key_stat[0x75]) data |= 0x20; // F6
			if(key_stat[0x76]) data |= 0x40; // F7
			if(key_stat[0x77]) data |= 0x80; // F8
		}
		if(!(addr & 0x200)) {
			if(key_stat[0x12]) data |= 0x04; // Alt
			if(key_stat[0x10]) data |= 0x08; // Shift
			if(key_stat[0x11]) data |= 0x10; // Ctrl
		}
		return (data ^ 0xff);
	}
	return 0xff;
}

#define STATE_VERSION	1

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return true;
}

