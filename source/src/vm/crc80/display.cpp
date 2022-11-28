/*
	Computer Research CRC-80 Emulator 'eCRC-80'

	Author : Takeda.Toshiya
	Date   : 2022.06.05-

	[ display ]
*/

#include "display.h"
#include "../z80pio.h"

static const int led_pattern[LED_SIZE_Y][LED_SIZE_X] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	{0, 0, 0, 6, 0, 1, 1, 1, 1, 1, 1, 0, 2, 0},
	{0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0},
	{0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0},
	{0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0},
	{0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0},
	{0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0},
	{0, 0, 5, 0, 7, 7, 7, 7, 7, 7, 0, 3, 0, 0},
	{0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0},
	{0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0},
	{0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0},
	{0, 5, 0, 4, 4, 4, 4, 4, 4, 0, 3, 0, 0, 0},
	{0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 8, 8},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8},
};

void DISPLAY::initialize()
{
	memset(led, 0, sizeof(led));
	pa = pb = 0;
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_DISPLAY_PA) {
		pa = data & mask;
		led[pb & 15] = pa;
	} else if(id == SIG_DISPLAY_PB) {
		pb = data & mask;
		led[pb & 15] = pa;
		
		// update keyin
		const uint8_t *key = emu->get_key_buffer();
		uint8_t val = 0xff;
		
		switch(pb & 15) {
		case 0: // K0
			if(key[0x70]) val &= ~0x01;	// GO
			if(key[0x71]) val &= ~0x02;	// DA
			if(key[0x72]) val &= ~0x04;	// AD
			if(key[0x73]) val &= ~0x08;	// INC
			if(key[0x74]) val &= ~0x10;	// DEC
			if(key[0x75]) val &= ~0x20;	// LD
			if(key[0x77]) val &= ~0x40;	// ST
			if(key[0x76]) val &= ~0x80;	// STP
			d_pio->write_signal(SIG_Z80PIO_PORT_A, val, 0xff);
			break;
		case 1: // K1
			if(key[0x30]) val &= ~0x01;
			if(key[0x31]) val &= ~0x02;
			if(key[0x32]) val &= ~0x04;
			if(key[0x33]) val &= ~0x08;
			if(key[0x34]) val &= ~0x10;
			if(key[0x35]) val &= ~0x20;
			if(key[0x36]) val &= ~0x40;
			if(key[0x37]) val &= ~0x80;
			d_pio->write_signal(SIG_Z80PIO_PORT_A, val, 0xff);
			break;
		case 2: // K2
			if(key[0x38]) val &= ~0x01;
			if(key[0x39]) val &= ~0x02;
			if(key[0x41]) val &= ~0x04;
			if(key[0x42]) val &= ~0x08;
			if(key[0x43]) val &= ~0x10;
			if(key[0x44]) val &= ~0x20;
			if(key[0x45]) val &= ~0x40;
			if(key[0x46]) val &= ~0x80;
			d_pio->write_signal(SIG_Z80PIO_PORT_A, val, 0xff);
			break;
		}
	}
}

void DISPLAY::draw_screen()
{
	// draw 7-seg LEDs
	scrntype_t col[10];
	scrntype_t color_on  = RGB_COLOR(255, 8, 72);
	scrntype_t color_off = RGB_COLOR(60, 30, 30);
	col[0] = RGB_COLOR(40, 20, 20);
	col[9] = color_off;
	
	for(int i = 0; i < 6; i++) {
		uint8_t pat = ~led[9 - i];
		col[1] = pat & 0x01 ? color_on : color_off;
		col[2] = pat & 0x02 ? color_on : color_off;
		col[3] = pat & 0x04 ? color_on : color_off;
		col[4] = pat & 0x08 ? color_on : color_off;
		col[5] = pat & 0x10 ? color_on : color_off;
		col[6] = pat & 0x20 ? color_on : color_off;
		col[7] = pat & 0x40 ? color_on : color_off;
		col[8] = pat & 0x80 ? color_on : color_off;
		for(int y = 0; y < LED_SIZE_Y; y++) {
			scrntype_t* dest = emu->get_screen_buffer(vm_ranges[i].y + y) + vm_ranges[i].x;
			for(int x = 0; x < LED_SIZE_X; x++) {
				dest[x] = col[led_pattern[y][x]];
			}
		}
	}
}

#define STATE_VERSION	1

bool DISPLAY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(led, sizeof(led), 1);
	state_fio->StateValue(pa);
	state_fio->StateValue(pb);
	return true;
}

