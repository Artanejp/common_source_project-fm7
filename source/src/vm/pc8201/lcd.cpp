/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.04.01-

	[ lcd ]
*/

#include "lcd.h"

void LCD::initialize()
{
	memset(seg, 0, sizeof(seg));
	sel = 0;
}

void LCD::write_io8(uint32 addr, uint32 data)
{
	if(addr & 1) {
		// data
		for(int b = 0; b < 10; b++) {
			if(sel & (1 << b)) {
				seg[b].vram[seg[b].page][seg[b].ofs] = data;
//				seg[b].ofs2 = seg[b].ofs;
				if(!seg[b].updown) {
					if(++seg[b].ofs > 49) {
						seg[b].ofs = 0;
					}
				} else {
					if(--seg[b].ofs < 0) {
						seg[b].ofs = 49;
					}
				}
			}
		}
	} else {
		for(int b = 0; b < 10; b++) {
			if(sel & (1 << b)) {
				// command
				switch(data) {
				case 0x32:
				case 0x33:
					seg[b].updown = data & 1;
					break;
				case 0x38:
				case 0x39:
					seg[b].disp = data & 1;
					break;
				case 0x3a:
				case 0x3b:
					seg[b].updown = (data ^ 1) & 1;
					break;
				case 0x3e:
				case 0x3f:
				case 0x7e:
				case 0x7f:
				case 0xbe:
				case 0xbf:
				case 0xfe:
				case 0xff:
					seg[b].spg = data >> 6;
					break;
				default:
					seg[b].page = data >> 6;
					seg[b].ofs = data & 0x3f;
					if(seg[b].ofs > 49) {
						seg[b].ofs = 49;
					}
					break;
				}
			}
		}
	}
}

uint32 LCD::read_io8(uint32 addr)
{
	uint8 val = 0xff;
	
	if(addr & 1) {
		// data
		for(int b = 0; b < 10; b++) {
			if(sel & (1 << b)) {
				val &= seg[b].vram[seg[b].page][seg[b].ofs2];
				seg[b].ofs2 = seg[b].ofs;
				if(!seg[b].updown) {
					if(++seg[b].ofs > 49) {
						seg[b].ofs = 0;
					}
				} else {
					if(--seg[b].ofs < 0) {
						seg[b].ofs = 49;
					}
				}
			}
		}
	} else {
		// status
		for(int b = 0; b < 10; b++) {
			if(sel & (1 << b)) {
				val &= (seg[b].updown ? 0x40 : 0) | (seg[b].disp ? 0x20 : 0) | 0xf;
			}
		}
	}
	return val;
}

void LCD::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_LCD_CHIPSEL_L) {
		sel = (sel & 0x300) | (data);
	} else if(id == SIG_LCD_CHIPSEL_H) {
		sel = (sel & 0xff) | ((data & 3) << 8);
	}
}

void LCD::draw_screen()
{
	// render screen
	memset(screen, 0, sizeof(screen));
	for(int b = 0; b < 10; b++) {
		if(seg[b].disp) {
			int xofs = (b % 5) * 50;
			for(int p = 0; p < 4; p++) {
				uint8* src = seg[b].vram[(seg[b].spg + p) & 3];
				int yofs = (b < 5 ? 0 : 32) + p * 8;
				uint8* dst0 = &screen[yofs + 0][xofs];
				uint8* dst1 = &screen[yofs + 1][xofs];
				uint8* dst2 = &screen[yofs + 2][xofs];
				uint8* dst3 = &screen[yofs + 3][xofs];
				uint8* dst4 = &screen[yofs + 4][xofs];
				uint8* dst5 = &screen[yofs + 5][xofs];
				uint8* dst6 = &screen[yofs + 6][xofs];
				uint8* dst7 = &screen[yofs + 7][xofs];
				for(int i = 0; i < 50; i++) {
					uint8 pat = src[i];
					dst0[i] = pat & 0x01;
					dst1[i] = pat & 0x02;
					dst2[i] = pat & 0x04;
					dst3[i] = pat & 0x08;
					dst4[i] = pat & 0x10;
					dst5[i] = pat & 0x20;
					dst6[i] = pat & 0x40;
					dst7[i] = pat & 0x80;
				}
			}
		}
	}
	
	// copy to real screen
	scrntype cd = RGB_COLOR(48, 56, 16);
	scrntype cb = RGB_COLOR(160, 168, 160);
	for(int y = 0; y < 64; y++) {
		scrntype* dst = emu->screen_buffer(y);
		uint8* src = screen[y];
		
		for(int x = 0; x < 240; x++) {
			dst[x] = src[x] ? cd : cb;
		}
	}
}

