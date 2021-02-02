/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ keyboard ]
*/

#include "keyboard.h"

namespace BX1 {
void KEYBOARD::reset()
{
	key_code = -1;
}

void KEYBOARD::write_io8(uint32_t addr, uint32_t data)
{
	
}

uint32_t KEYBOARD::read_io8(uint32_t addr)
{
	uint32_t value = 0xff;
	
	switch(addr & 0xffff) {
	case 0xe122:
/*
		{
			const uint8_t* key_stat = emu->get_key_buffer();
			if(key_stat['1']) value &= ~0x01;
			if(key_stat['2']) value &= ~0x02;
			if(key_stat['3']) value &= ~0x04;
			if(key_stat['4']) value &= ~0x08;
			if(key_stat['5']) value &= ~0x10;
			if(key_stat['6']) value &= ~0x20;
			if(key_stat['7']) value &= ~0x40;
			if(key_stat['8']) value &= ~0x80;
		}
		break;
*/
		if(key_code != -1) {
			value = key_code;
			key_code = -1;
		}
		break;
	case 0xe121:
		{
			const uint8_t* key_stat = emu->get_key_buffer();
			static int c = 0, p = 0;
			if(key_stat[VK_F7]) {
				value = (c | 0x80) & 0xbf;
				if(p == 0) {
					c++;
					p = 1;
				}
			} else {
				p = 0;
				if(key_stat[VK_F1]) value &= ~0x01;
				if(key_stat[VK_F2]) value &= ~0x02;
				if(key_stat[VK_F3]) value &= ~0x04;
				if(key_stat[VK_F4]) value &= ~0x08;
				if(key_stat[VK_F5]) value &= ~0x10;
				if(key_stat[VK_F6]) value &= ~0x20;
				if(key_stat[VK_F7]) value &= ~0x40;
				if(key_stat[VK_F8]) value &= ~0x80;
			}
		}
		
		
		break;
	case 0xe212:
		{
			const uint8_t* key_stat = emu->get_key_buffer();
			if(key_stat['1']) value &= ~0x01;
			if(key_stat['2']) value &= ~0x02;
			if(key_stat['3']) value &= ~0x04;
			if(key_stat['4']) value &= ~0x08;
			if(key_stat['5']) value &= ~0x10;
			if(key_stat['6']) value &= ~0x20;
			if(key_stat['7']) value &= ~0x40;
			if(key_stat['8']) value &= ~0x80;
			value^=0xff;
		}
		break;
	}
	return value;
}

void KEYBOARD::key_down(int code)
{
	key_code = code;
}

void KEYBOARD::key_up(int code)
{
	key_code = -1;
}
}
