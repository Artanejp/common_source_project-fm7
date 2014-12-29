/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.16 -

	[ display ]
*/

#include <math.h>
#include "display.h"
#include "../upd7220.h"
#include "../../fileio.h"

void DISPLAY::initialize()
{
#ifdef _COLOR_MONITOR
	memset(vram_r, 0, sizeof(vram_r));
	memset(vram_g, 0, sizeof(vram_g));
	memset(vram_b, 0, sizeof(vram_b));
#else
	memset(vram, 0, sizeof(vram));
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
#endif
	
	// create pc palette
#ifdef _COLOR_MONITOR
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR(i & 1 ? 255 : 0, i & 2 ? 255 : 0, (i & 4) ? 255 : 0);
	}
#else
	for(int i = 1; i < 8; i++) {
		palette_pc[i + 0] = RGB_COLOR(0, 160, 0);
		palette_pc[i + 8] = RGB_COLOR(0, 255, 0);
	}
	palette_pc[0] = palette_pc[8] = 0;
#endif
	
	// cursor blinking
	register_frame_event(this);
	blink = 0;
}

void DISPLAY::reset()
{
#ifdef _COLOR_MONITOR
	d_gdc->set_vram_ptr(vram_b, VRAM_SIZE);
#endif
	bank = 1;
}

void DISPLAY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x2d:
#ifdef _COLOR_MONITOR
		if(data & 1) {
			d_gdc->set_vram_ptr(vram_b, VRAM_SIZE);
		} else if(data & 2) {
			d_gdc->set_vram_ptr(vram_g, VRAM_SIZE);
		} else {
			d_gdc->set_vram_ptr(vram_r, VRAM_SIZE);
		}
#endif
		bank = data;
		break;
	}
}

uint32 DISPLAY::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0x2c:
#ifdef _COLOR_MONITOR
		return 0xfd;
#else
		return 0xfe;
#endif
	case 0x2d:
		return bank;
	}
	return 0xff;
}

void DISPLAY::event_frame()
{
	blink++;
}

void DISPLAY::draw_screen()
{
	uint8 cg = sync[0] & 0x22;
	int al = (sync[6] | (sync[7] << 8)) & 0x3ff;
	
	for(int i = 0, total = 0; i < 4 && total < al; i++) {
		uint32 tmp = ra[4 * i];
		tmp |= ra[4 * i + 1] << 8;
		tmp |= ra[4 * i + 2] << 16;
		tmp |= ra[4 * i + 3] << 24;
		
		int ptr = tmp & ((cg == 0x20) ? 0x1fff : 0x3ffff);
		ptr <<= 1;
		int line = (tmp >> 20) & 0x3ff;
		bool gfx = (cg == 2) ? true : (cg == 0x20) ? false : ((tmp & 0x40000000) != 0);
		bool wide = ((tmp & 0x80000000) != 0);
		int caddr = ((cs[0] & 0x80) && ((cs[1] & 0x20) || !(blink & 0x10))) ? (*ead << 1) : -1;
		
#ifdef _COLOR_MONITOR
//		if(gfx) {
			for(int y = total; y < total + line && y < 400; y++) {
				if(wide) {
					for(int x = 0; x < 640; x+= 16) {
						uint8 r = vram_r[ptr];
						uint8 g = vram_g[ptr];
						uint8 b = vram_b[ptr++];
						ptr &= VRAM_SIZE - 1;
						
						screen[y][x +  0] = screen[y][x +  1] = ((r & 0x01) ? 1 : 0) | ((g & 0x01) ? 2 : 0) | ((b & 0x01) ? 4 : 0);
						screen[y][x +  2] = screen[y][x +  3] = ((r & 0x02) ? 1 : 0) | ((g & 0x02) ? 2 : 0) | ((b & 0x02) ? 4 : 0);
						screen[y][x +  4] = screen[y][x +  5] = ((r & 0x04) ? 1 : 0) | ((g & 0x04) ? 2 : 0) | ((b & 0x04) ? 4 : 0);
						screen[y][x +  6] = screen[y][x +  7] = ((r & 0x08) ? 1 : 0) | ((g & 0x08) ? 2 : 0) | ((b & 0x08) ? 4 : 0);
						screen[y][x +  8] = screen[y][x +  9] = ((r & 0x10) ? 1 : 0) | ((g & 0x10) ? 2 : 0) | ((b & 0x10) ? 4 : 0);
						screen[y][x + 10] = screen[y][x + 11] = ((r & 0x20) ? 1 : 0) | ((g & 0x20) ? 2 : 0) | ((b & 0x20) ? 4 : 0);
						screen[y][x + 12] = screen[y][x + 13] = ((r & 0x40) ? 1 : 0) | ((g & 0x40) ? 2 : 0) | ((b & 0x40) ? 4 : 0);
						screen[y][x + 14] = screen[y][x + 15] = ((r & 0x80) ? 1 : 0) | ((g & 0x80) ? 2 : 0) | ((b & 0x80) ? 4 : 0);
					}
				} else {
					for(int x = 0; x < 640; x+= 8) {
						uint8 r = vram_r[ptr];
						uint8 g = vram_g[ptr];
						uint8 b = vram_b[ptr++];
						ptr &= VRAM_SIZE - 1;
						
						screen[y][x + 0] = ((r & 0x01) ? 1 : 0) | ((g & 0x01) ? 2 : 0) | ((b & 0x01) ? 4 : 0);
						screen[y][x + 1] = ((r & 0x02) ? 1 : 0) | ((g & 0x02) ? 2 : 0) | ((b & 0x02) ? 4 : 0);
						screen[y][x + 2] = ((r & 0x04) ? 1 : 0) | ((g & 0x04) ? 2 : 0) | ((b & 0x04) ? 4 : 0);
						screen[y][x + 3] = ((r & 0x08) ? 1 : 0) | ((g & 0x08) ? 2 : 0) | ((b & 0x08) ? 4 : 0);
						screen[y][x + 4] = ((r & 0x10) ? 1 : 0) | ((g & 0x10) ? 2 : 0) | ((b & 0x10) ? 4 : 0);
						screen[y][x + 5] = ((r & 0x20) ? 1 : 0) | ((g & 0x20) ? 2 : 0) | ((b & 0x20) ? 4 : 0);
						screen[y][x + 6] = ((r & 0x40) ? 1 : 0) | ((g & 0x40) ? 2 : 0) | ((b & 0x40) ? 4 : 0);
						screen[y][x + 7] = ((r & 0x80) ? 1 : 0) | ((g & 0x80) ? 2 : 0) | ((b & 0x80) ? 4 : 0);
					}
				}
			}
//		}
#else
		if(gfx) {
			for(int y = total; y < total + line && y < 400; y++) {
				if(wide) {
					for(int x = 0; x < 640; x+= 16) {
						uint8 pat = vram[ptr++];
						ptr &= VRAM_SIZE - 1;
						
						screen[y][x +  0] = screen[y][x +  1] = (pat & 0x01) ? 1 : 0;
						screen[y][x +  2] = screen[y][x +  3] = (pat & 0x02) ? 1 : 0;
						screen[y][x +  4] = screen[y][x +  5] = (pat & 0x04) ? 1 : 0;
						screen[y][x +  6] = screen[y][x +  7] = (pat & 0x08) ? 1 : 0;
						screen[y][x +  8] = screen[y][x +  9] = (pat & 0x10) ? 1 : 0;
						screen[y][x + 10] = screen[y][x + 11] = (pat & 0x20) ? 1 : 0;
						screen[y][x + 12] = screen[y][x + 13] = (pat & 0x40) ? 1 : 0;
						screen[y][x + 14] = screen[y][x + 15] = (pat & 0x80) ? 1 : 0;
					}
				} else {
					for(int x = 0; x < 640; x+= 8) {
						uint8 pat = vram[ptr++];
						ptr &= VRAM_SIZE - 1;
						
						screen[y][x + 0] = (pat & 0x01) ? 1 : 0;
						screen[y][x + 1] = (pat & 0x02) ? 1 : 0;
						screen[y][x + 2] = (pat & 0x04) ? 1 : 0;
						screen[y][x + 3] = (pat & 0x08) ? 1 : 0;
						screen[y][x + 4] = (pat & 0x10) ? 1 : 0;
						screen[y][x + 5] = (pat & 0x20) ? 1 : 0;
						screen[y][x + 6] = (pat & 0x40) ? 1 : 0;
						screen[y][x + 7] = (pat & 0x80) ? 1 : 0;
					}
				}
			}
		} else {
			for(int y = total; y < total + line;) {
				if(wide) {
					for(int x = 0; x < 640; x += 16) {
						bool cursor = (ptr == caddr);
						uint8 code = vram[ptr++];
						uint8 attrib = vram[ptr++];
						ptr &= VRAM_SIZE - 1;
						uint8* pattern = &font[code * 16];
						
						for(int l = y % 16; l < 16 && (y + l) < 400; l++) {
							uint8 pat = pattern[l];
							// attribute
							if((attrib & 0x40) || ((attrib & 0x80) && (blink & 0x10))) {
								pat = 0;
							}
							if(attrib & 8) {
								pat = ~pat;
							}
							uint8 col = (attrib & 4) ? 9 : 1;
							
							screen[y + l][x +  0] = screen[y + l][x +  1] = (pat & 0x01) ? col : 0;
							screen[y + l][x +  2] = screen[y + l][x +  3] = (pat & 0x02) ? col : 0;
							screen[y + l][x +  4] = screen[y + l][x +  5] = (pat & 0x04) ? col : 0;
							screen[y + l][x +  6] = screen[y + l][x +  7] = (pat & 0x08) ? col : 0;
							screen[y + l][x +  8] = screen[y + l][x +  9] = (pat & 0x10) ? col : 0;
							screen[y + l][x + 10] = screen[y + l][x + 11] = (pat & 0x20) ? col : 0;
							screen[y + l][x + 12] = screen[y + l][x + 13] = (pat & 0x40) ? col : 0;
							screen[y + l][x + 14] = screen[y + l][x + 15] = (pat & 0x80) ? col : 0;
						}
						if(cursor) {
							int top = cs[1] & 0x1f, bottom = cs[2] >> 3;
							for(int l = top; l < bottom && l < 16; l++) {
								memset(&screen[y + l][x], 1, 16);
							}
						}
					}
				} else {
					for(int x = 0; x < 640; x += 8) {
						bool cursor = (ptr == caddr);
						uint8 code = vram[ptr++];
						ptr &= VRAM_SIZE - 1;
						uint8 attrib = vram[ptr++];
						ptr &= VRAM_SIZE - 1;
						uint8* pattern = &font[code * 16];
						
						for(int l = y % 16; l < 16 && (y + l) < 400; l++) {
							uint8 pat = pattern[l];
							// attribute
							if((attrib & 0x40) || ((attrib & 0x80) && (blink & 0x10))) {
								pat = 0;
							}
							if(attrib & 8) {
								pat = ~pat;
							}
							uint8 col = (attrib & 4) ? 9 : 1;
							
							screen[y + l][x + 0] = (pat & 0x01) ? col : 0;
							screen[y + l][x + 1] = (pat & 0x02) ? col : 0;
							screen[y + l][x + 2] = (pat & 0x04) ? col : 0;
							screen[y + l][x + 3] = (pat & 0x08) ? col : 0;
							screen[y + l][x + 4] = (pat & 0x10) ? col : 0;
							screen[y + l][x + 5] = (pat & 0x20) ? col : 0;
							screen[y + l][x + 6] = (pat & 0x40) ? col : 0;
							screen[y + l][x + 7] = (pat & 0x80) ? col : 0;
						}
						if(cursor) {
							int top = cs[1] & 0x1f, bottom = cs[2] >> 3;
							for(int l = top; l < bottom && l < 16; l++) {
								memset(&screen[y + l][x], 1, 8);
							}
						}
					}
				}
				y += 16 - (y % 16);
			}
		}
#endif
		total += line;
	}
	
	// copy to pc screen
	if(*zoom) {
		for(int y = 0, dy = 0; y < 400 && dy < 400; y++) {
			uint8* src = screen[y];
			
			for(int x = 0, dx = 0; x < 640 && dx < 640; x++) {
				scrntype col = palette_pc[src[x] & 0xf];
				for(int zx = 0; zx < *zoom + 1; zx++) {
					if(dx >= 640) {
						break;
					}
					tmp[dx++] = col;
				}
			}
			// copy line
			for(int zy = 1; zy < *zoom + 1; zy++) {
				if(dy >= 400) {
					break;
				}
				scrntype *dest = emu->screen_buffer(dy++);
				memcpy(dest, tmp, sizeof(scrntype) * 640);
			}
		}
	} else {
		for(int y = 0; y < 400; y++) {
			scrntype* dest = emu->screen_buffer(y);
			uint8* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
#ifdef _COLOR_MONITOR
				dest[x] = palette_pc[src[x] & 7];
#else
				dest[x] = palette_pc[src[x] & 0x0f];
#endif
			}
		}
	}
#ifdef _COLOR_MONITOR
	emu->screen_skip_line = false;
#endif
}

