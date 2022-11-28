/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2007.02.08 -

	[ display ]
*/

#include "display.h"

void DISPLAY::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// create pc palette
#ifdef _LCD
	for(int i = 1; i < 8; i++) {
		palette_pc[i] = RGB_COLOR(48, 56, 16);
	}
	palette_pc[0] = RGB_COLOR(160, 168, 160);
#else
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
#endif
	
	// initialize
	for(int i = 0; i < 16; i++) {
		pal[i] = i & 7;
	}
	mode = text_page = 0;
	cblink = flash_cnt = 0;
	blink = pal_dis = false;
	
	// register event
	register_frame_event(this);
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_DISPLAY_I8255_0_A) {
		mode = data;
	} else if(id == SIG_DISPLAY_I8255_1_B) {
		text_page = (data >> 4) & 7;
	} else if(id == SIG_DISPLAY_I8255_1_C) {
		blink = ((data & 0x20) != 0);
		pal_dis = ((data & 8) != 0);
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void DISPLAY::draw_screen()
{
	// clear screen buffer
	memset(screen, 0, sizeof(screen));
	
#ifdef _LCD
	if((regs[8] & 0x30) != 0x30) {
		// render screen
		uint16_t src = ((regs[12] << 11) | (regs[13] << 3)) & 0x3ff8;
		if((regs[8] & 0xc0) == 0xc0) {
			cursor = -1;
		} else {
			cursor = ((regs[14] << 11) | (regs[15] << 3)) & 0x3ff8;
		}
		
		switch(mode & 0xa0) {
		case 0x00:	// text, wide
		case 0x20:	// text, normal
			draw_text_lcd(src);
			break;
		case 0x80:	// fine graph, wide
		case 0xa0:	// fine graph, normal
			draw_fine_lcd(src);
			break;
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(64);
	
	for(int y = 0; y < 64; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src = screen[y];
		
		for(int x = 0; x < 320; x++) {
			dest0[x] = palette_pc[src[x] & 7];
		}
		if(config.scan_line) {
			for(int x = 0; x < 320; x++) {
				dest1[x] = palette_pc[0];
			}
		} else {
			memcpy(dest1, dest0, 320 * sizeof(scrntype_t));
		}
	}
#else
	if((regs[8] & 0x30) != 0x30) {
		// sync check
		uint16_t flash = 0;
		if(mode & 0x20) {
			if(regs[0] < 106 || 118 < regs[0] || 113 < regs[2]) {
				flash = 0xffff;
			}
			flash_cnt -= 320;
		} else {
			if(regs[0] < 53 || 58 < regs[0] || 56 < regs[2]) {
				flash = 0xffff;
			}
			flash_cnt -= 160;
		}
		if(regs[4] < 27 || 32 < regs[4] || 16 < regs[5] || 32 < regs[7]) {
			flash = 0xffff;
		}
		if((regs[8] & 3) == 3 || (regs[9] != 7 && regs[9] != 6)) {
			flash = 0xffff;
		}
		uint16_t src = (((regs[12] << 11) | (regs[13] << 3)) + (flash_cnt & flash)) & 0x3ff8;
		if((regs[8] & 0xc0) == 0xc0) {
			cursor = -1;
		} else {
			cursor = ((regs[14] << 11) | (regs[15] << 3)) & 0x3ff8;
		}
		
		// render screen
		if((flash != 0) || (regs[8] & 0x30) != 0x30) {
			switch(mode & 0xa0) {
			case 0x00:
				// text, wide
				draw_text_wide(src);
				flash_cnt += 40;
				break;
			case 0x20:
				// text, normal
				draw_text_normal(src);
				break;
			case 0x80:
				// fine graph, wide
				draw_fine_wide(src);
				break;
			case 0xa0:
				// fine graph, normal
				draw_fine_normal(src);
				break;
			}
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src = screen[y];
		
		for(int x = 0; x < 640; x++) {
			dest0[x] = palette_pc[src[x] & 7];
		}
		if(config.scan_line) {
//			for(int x = 0; x < 640; x++) {
//				dest1[x] = palette_pc[0];
//			}
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
#endif
	emu->screen_skip_line(true);
}

void DISPLAY::draw_text_normal(uint16_t src)
{
	// text mode, normal char (80chars)
	uint16_t src_t = (src & 0x3ff8) + text_page;
	int maxx = 79, maxy = 24, hsync_pos = 92, vsync_pos = 28;
	int _maxx = maxx + 36, _minx = -36;
	int homex = hsync_pos - regs[2], homey = vsync_pos - regs[7];
	int endx = regs[1] + homex, endy = regs[6] + homey;
	int _dstx, dstx = homex, dsty = homey;
	
	for(int y = 0; y < 200; y += 8) {
		for(int x = 0; x < 80; x++, dstx++, src = (src + 8) & 0x3ff8, src_t = (src_t + 8) & 0x3fff) {
			if(dstx >= endx) {
				dstx = homex;
				dsty++;
			}
			if(dsty >= endy) {
				// exit x and y loop
				y = 200;
				break;
			}
			if((dsty < 0)||(dsty > maxy)) {
				continue;
			}
			_dstx = dstx;
			if((regs[8] & 0x30) == 0x10) {
				if(dstx == 0) {
					_dstx = 1;
				}
				if(dstx == 1) {
					continue;
				}
			}
			if((regs[8] & 0x30) == 0x20) {
				if((dstx == 0) || (dstx == 1)) {
					continue;
				}
			}
			if(dstx <= _minx) {
				_dstx = maxx - (dstx - _minx);
			}
			if(dstx >= _maxx) {
				_dstx = dstx - _maxx;
			}
			if((_dstx < 0)||(_dstx > maxx)) {
				continue;
			}
			uint8_t code = vram_g[src_t];
			uint8_t attr = vram_a[src_t];
			uint8_t c, c_t = (mode & 8) ? (mode & 7) : (attr & 7);
			uint8_t* font_base = &font[code << 3];
			uint8_t p = pal_dis ? 0 : 0xff;
			
			for(int l = 0; l < 8; l++) {
				uint8_t p1 = vram_b[src + l] & p;
				uint8_t p2 = vram_r[src + l] & p;
				uint8_t p3 = font_base[l];
				// negative, blink
				if(attr & 8) {
					p3 = ~p3;
				}
				if((mode & 8) && !(attr & 4) && blink) {
					p3 = 0;
				}
				uint8_t* d = &screen[(dsty << 3) + l][_dstx << 3];
				
				c = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | 0;
				d[0] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x80) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | 8;
				d[1] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x40) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | 0;
				d[2] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x20) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | 8;
				d[3] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x10) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | 0;
				d[4] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x08) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | 8;
				d[5] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x04) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | 0;
				d[6] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x02) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | 8;
				d[7] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x01) ? c_t : (pal[c] & 7);
			}
			if(src == cursor) {
				int bp = regs[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[(dsty << 3) + i][_dstx << 3], 7, 8);
					}
				}
			}
		}
	}
}

void DISPLAY::draw_text_wide(uint16_t src)
{
	// text mode, wide char (40chars)
	uint16_t src_t = (src & 0x3ff8) + text_page;
	int maxx = 39, maxy = 24, hsync_pos = 47, vsync_pos = 28;
	int _maxx = maxx + 18, _minx = -18;
	int homex = hsync_pos - regs[2], homey = vsync_pos - regs[7];
	int endx = regs[1] + homex, endy = regs[6] + homey;
	int _dstx, dstx = homex, dsty = homey;
	
	for(int y = 0; y < 200; y += 8) {
		for(int x = 0; x < 40; x++, dstx++, src = (src + 8) & 0x3ff8, src_t = (src_t + 8) & 0x3fff) {
			if(dstx >= endx) {
				dstx = homex;
				dsty++;
			}
			if(dsty >= endy) {
				// exit x and y loop
				y = 200;
				break;
			}
			if((dsty < 0)||(dsty > maxy)) {
				continue;
			}
			_dstx = dstx;
			if((regs[8] & 0x30) == 0x10) {
				if(dstx == 0) {
					_dstx = 1;
				}
				if(dstx == 1) {
					continue;
				}
			}
			if((regs[8] & 0x30) == 0x20) {
				if((dstx == 0) || (dstx == 1)) {
					continue;
				}
			}
			if(dstx <= _minx) {
				_dstx = maxx - (dstx - _minx);
			}
			if(dstx >= _maxx) {
				_dstx = dstx - _maxx;
			}
			if((_dstx < 0)||(_dstx > maxx)) {
				continue;
			}
			
			uint8_t code = vram_g[src_t];
			uint8_t attr = vram_a[src_t];
			uint8_t c, c_t = (mode & 8) ? (mode & 7) : (attr & 7);
			uint8_t* font_base = &font[code << 3];
			uint8_t p = pal_dis ? 0 : 0xff;
			
			for(int l = 0; l < 8; l++) {
				uint8_t p1 = vram_b[src + l] & p;
				uint8_t p2 = vram_r[src + l] & p;
				uint8_t p3 = font_base[l];
				// negative, blink
				if(attr & 8) {
					p3 = ~p3;
				}
				if((mode & 8) && !(attr & 4) && blink) {
					p3 = 0;
				}
				uint8_t* d = &screen[(dsty << 3) + l][_dstx << 4];
				
				c = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6);
				d[ 0] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x80) ? c_t : (pal[c | 0] & 7);
				d[ 1] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x80) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5);
				d[ 2] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x40) ? c_t : (pal[c | 0] & 7);
				d[ 3] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x40) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4);
				d[ 4] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x20) ? c_t : (pal[c | 0] & 7);
				d[ 5] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x20) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3);
				d[ 6] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x10) ? c_t : (pal[c | 0] & 7);
				d[ 7] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x10) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2);
				d[ 8] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x08) ? c_t : (pal[c | 0] & 7);
				d[ 9] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x08) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1);
				d[10] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x04) ? c_t : (pal[c | 0] & 7);
				d[11] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x04) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0);
				d[12] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x02) ? c_t : (pal[c | 0] & 7);
				d[13] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x02) ? c_t : (pal[c | 8] & 7);
				c = ((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1);
				d[14] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x01) ? c_t : (pal[c | 0] & 7);
				d[15] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x01) ? c_t : (pal[c | 8] & 7);
			}
			if(src == cursor) {
				int bp = regs[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[(dsty << 3) + i][_dstx << 4], 7, 16);
					}
				}
			}
		}
	}
}

void DISPLAY::draw_fine_normal(uint16_t src)
{
	// fine graph mode, normal char (80chars)
	int maxx = 79, maxy = 24, hsync_pos = 92, vsync_pos = 28;
	int _maxx = maxx + 36, _minx = -36;
	int homex = hsync_pos - regs[2], homey = vsync_pos - regs[7];
	int endx = regs[1] + homex, endy = regs[6] + homey;
	int _dstx, dstx = homex, dsty = homey;
	
	for(int y = 0; y < 200; y += 8) {
		for(int x = 0; x < 80; x++, dstx++, src = (src + 8) & 0x3ff8) {
			if(dstx >= endx) {
				dstx = homex;
				dsty++;
			}
			if(dsty >= endy) {
				// exit x and y loop
				y = 200;
				break;
			}
			if((dsty < 0)||(dsty > maxy)) {
				continue;
			}
			_dstx = dstx;
			if((regs[8] & 0x30) == 0x10) {
				if(dstx == 0) {
					_dstx = 1;
				}
				if(dstx == 1) {
					continue;
				}
			}
			if((regs[8] & 0x30) == 0x20) {
				if((dstx == 0) || (dstx == 1)) {
					continue;
				}
			}
			if(dstx <= _minx) {
				_dstx = maxx - (dstx - _minx);
			}
			if(dstx >= _maxx) {
				_dstx = dstx - _maxx;
			}
			if((_dstx < 0)||(_dstx > maxx)) {
				continue;
			}
			
			for(int l = 0; l < 8; l++) {
				uint8_t code = vram_g[src + l];
				uint8_t attr = vram_a[src + l];
				uint8_t c, c_t = (mode & 8) ? (mode & 7) : (attr & 7);
				uint8_t* font_base = &font[code << 3];
				uint8_t p = pal_dis ? 0 : 0xff;
				uint8_t* d = &screen[(dsty << 3) + l][_dstx << 3];
				
				if(attr & 8) {
					// dot mode
					uint8_t p1 = vram_b[src + l] & p;
					uint8_t p2 = vram_r[src + l] & p;
					uint8_t p3 = vram_g[src + l] & p;
					
					d[0] = pal[((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((p3 & 0x80) >> 5) | 0] & 7;
					d[1] = pal[((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | ((p3 & 0x40) >> 4) | 8] & 7;
					d[2] = pal[((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | ((p3 & 0x20) >> 3) | 0] & 7;
					d[3] = pal[((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | ((p3 & 0x10) >> 2) | 8] & 7;
					d[4] = pal[((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | ((p3 & 0x08) >> 1) | 0] & 7;
					d[5] = pal[((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | ((p3 & 0x04) >> 0) | 8] & 7;
					d[6] = pal[((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | ((p3 & 0x02) << 1) | 0] & 7;
					d[7] = pal[((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | ((p3 & 0x01) << 2) | 8] & 7;
				} else {
					// text
					uint8_t p1 = vram_b[src + l] & p;
					uint8_t p2 = vram_r[src + l] & p;
					uint8_t p3 = font_base[l];
					if(mode & 8) {
						// negative, blink
						if(attr & 8) {
							p3 = ~p3;
						}
						if(!(attr & 4) && blink) {
							p3 = 0;
						}
					}
					c = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | 0;
					d[0] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x80) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | 8;
					d[1] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x40) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | 0;
					d[2] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x20) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | 8;
					d[3] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x10) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | 0;
					d[4] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x08) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | 8;
					d[5] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x04) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | 0;
					d[6] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x02) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | 8;
					d[7] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x01) ? c_t : (pal[c] & 7);
				}
			}
			if(src == cursor) {
				int bp = regs[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[(dsty << 3) + i][_dstx << 3], 7, 8);
					}
				}
			}
		}
	}
}

void DISPLAY::draw_fine_wide(uint16_t src)
{
	// fine graph mode, wide char (40chars)
	int maxx = 39, maxy = 24, hsync_pos = 47, vsync_pos = 28;	//WIDTH 40
	int _maxx = maxx + 18, _minx = -18;	//WIDTH 40
	int homex = hsync_pos - regs[2], homey = vsync_pos - regs[7];
	int endx = regs[1] + homex, endy = regs[6] + homey;
	int _dstx, dstx = homex, dsty = homey;
	
	for(int y = 0; y < 200; y += 8) {
		for(int x = 0; x < 40; x++, dstx++, src = (src + 8) & 0x3ff8) {
			if(dstx >= endx) {
				dstx = homex;
				dsty++;
			}
			if(dsty >= endy) {
				// exit x and y loop
				y = 200;
				break;
			}
			if((dsty < 0)||(dsty > maxy)) {
				continue;
			}
			_dstx = dstx;
			if((regs[8] & 0x30) == 0x10) {
				if(dstx == 0) {
					_dstx = 1;
				}
				if(dstx == 1) {
					continue;
				}
			}
			if((regs[8] & 0x30) == 0x20) {
				if((dstx == 0) || (dstx == 1)) {
					continue;
				}
			}
			if(dstx <= _minx) {
				_dstx = maxx - (dstx - _minx);
			}
			if(dstx >= _maxx) {
				_dstx = dstx - _maxx;
			}
			if((_dstx < 0)||(_dstx > maxx)) {
				continue;
			}
			
			for(int l = 0; l < 8; l++) {
				uint8_t code = vram_g[src + l];
				uint8_t attr = vram_a[src + l];
				uint8_t c, c_t = (mode & 8) ? (mode & 7) : (attr & 7);
				uint8_t* font_base = &font[code << 3];
				uint8_t p = pal_dis ? 0 : 0xff;
				uint8_t* d = &screen[(dsty << 3) + l][_dstx << 4];
				
				if(attr & 8) {
					// dot mode
					uint8_t p1 = vram_b[src + l] & p;
					uint8_t p2 = vram_r[src + l] & p;
					uint8_t p3 = vram_g[src + l] & p;
					
					d[ 0] = pal[((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((p3 & 0x80) >> 5) | 0] & 7;
					d[ 1] = pal[((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((p3 & 0x80) >> 5) | 8] & 7;
					d[ 2] = pal[((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | ((p3 & 0x40) >> 4) | 0] & 7;
					d[ 3] = pal[((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | ((p3 & 0x40) >> 4) | 8] & 7;
					d[ 4] = pal[((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | ((p3 & 0x20) >> 3) | 0] & 7;
					d[ 5] = pal[((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | ((p3 & 0x20) >> 3) | 8] & 7;
					d[ 6] = pal[((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | ((p3 & 0x10) >> 2) | 0] & 7;
					d[ 7] = pal[((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | ((p3 & 0x10) >> 2) | 8] & 7;
					d[ 8] = pal[((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | ((p3 & 0x08) >> 1) | 0] & 7;
					d[ 9] = pal[((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | ((p3 & 0x08) >> 1) | 8] & 7;
					d[10] = pal[((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | ((p3 & 0x04) >> 0) | 0] & 7;
					d[11] = pal[((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | ((p3 & 0x04) >> 0) | 8] & 7;
					d[12] = pal[((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | ((p3 & 0x02) << 1) | 0] & 7;
					d[13] = pal[((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | ((p3 & 0x02) << 1) | 8] & 7;
					d[14] = pal[((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | ((p3 & 0x01) << 2) | 0] & 7;
					d[15] = pal[((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | ((p3 & 0x01) << 2) | 8] & 7;
				} else {
					// text
					uint8_t p1 = vram_b[src + l] & p;
					uint8_t p2 = vram_r[src + l] & p;
					uint8_t p3 = font_base[l];
					if(mode & 8) {
						// negative, blink
						if(attr & 8) {
							p3 = ~p3;
						}
						if(!(attr & 4) && blink) {
							p3 = 0;
						}
					}
					c = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6);
					d[ 0] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x80) ? c_t : (pal[c | 0] & 7);
					d[ 1] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x80) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5);
					d[ 2] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x40) ? c_t : (pal[c | 0] & 7);
					d[ 3] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x40) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4);
					d[ 4] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x20) ? c_t : (pal[c | 0] & 7);
					d[ 5] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x20) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3);
					d[ 6] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x10) ? c_t : (pal[c | 0] & 7);
					d[ 7] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x10) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2);
					d[ 8] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x08) ? c_t : (pal[c | 0] & 7);
					d[ 9] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x08) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1);
					d[10] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x04) ? c_t : (pal[c | 0] & 7);
					d[11] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x04) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0);
					d[12] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x02) ? c_t : (pal[c | 0] & 7);
					d[13] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x02) ? c_t : (pal[c | 8] & 7);
					c = ((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1);
					d[14] = (pal[c | 0] & 8) ? (pal[c | 0] & 7) : (p3 & 0x01) ? c_t : (pal[c | 0] & 7);
					d[15] = (pal[c | 8] & 8) ? (pal[c | 8] & 7) : (p3 & 0x01) ? c_t : (pal[c | 8] & 7);
				}
			}
			if(src == cursor) {
				int bp = regs[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[(dsty << 3) + i][_dstx << 4], 7, 16);
					}
				}
			}
		}
	}
}

void DISPLAY::draw_text_lcd(uint16_t src)
{
	// text mode, nomarl char (80chars)
	for(int y = 0, yy = 0; y < 64; y += 8, yy++) {
		int src_g = (src + 640 * (yy & 3) + (yy & 4 ? 8 : 0)) & 0x3ff8;
		int src_t = src_g;// + text_page;
		
		for(int x = 0; x < 40; x++) {
			uint8_t code = vram_g[src_t];
			uint8_t attr = vram_a[src_t];
			uint8_t c, c_t = (mode & 8) ? (mode & 7) : (attr & 7);
			uint8_t* font_base = &font[code << 3];
			
			for(int l = 0; l < 8; l++) {
				uint8_t p1 = vram_b[src_g + l];
				uint8_t p2 = vram_r[src_g + l];
				uint8_t p3 = font_base[l];
				// negative, blink
				if(attr & 8) {
					p3 = ~p3;
				}
				if((mode & 8) && !(attr & 4) && blink) {
					p3 = 0;
				}
				uint8_t* d = &screen[y + l][x << 3];
				
				c = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | 0;
				d[0] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x80) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | 8;
				d[1] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x40) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | 0;
				d[2] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x20) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | 8;
				d[3] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x10) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | 0;
				d[4] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x08) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | 8;
				d[5] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x04) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | 0;
				d[6] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x02) ? c_t : (pal[c] & 7);
				c = ((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | 8;
				d[7] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x01) ? c_t : (pal[c] & 7);
			}
			if(src_g == cursor) {
				int bp = regs[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 3], 7, 8);
					}
				}
			}
			src_g = (src_g + 16) & 0x3ff8;
			src_t = (src_t + 16) & 0x3fff;
		}
	}
}

void DISPLAY::draw_fine_lcd(uint16_t src)
{
	// fine graph mode, normal char (80chars)
	for(int y = 0, yy = 0; y < 64; y += 8, yy++) {
		int src_g = (src + 640 * (yy & 3) + (yy & 4 ? 8 : 0)) & 0x3ff8;
		
		for(int x = 0; x < 80; x++) {
			uint8_t code = vram_g[src_g];
			uint8_t attr = vram_a[src_g];
			uint8_t c, c_t = (mode & 8) ? (mode & 7) : (attr & 7);
			uint8_t* font_base = &font[code << 3];
			
			if(attr & 8) {
				// dot mode
				for(int l = 0; l < 8; l++) {
					uint8_t p1 = vram_b[src_g + l];
					uint8_t p2 = vram_r[src_g + l];
					uint8_t p3 = vram_g[src_g + l];
					uint8_t* d = &screen[y + l][x << 3];
					
					d[0] = pal[((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((p3 & 0x80) >> 5) | 0] & 7;
					d[1] = pal[((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | ((p3 & 0x40) >> 4) | 8] & 7;
					d[2] = pal[((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | ((p3 & 0x20) >> 3) | 0] & 7;
					d[3] = pal[((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | ((p3 & 0x10) >> 2) | 8] & 7;
					d[4] = pal[((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | ((p3 & 0x08) >> 1) | 0] & 7;
					d[5] = pal[((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | ((p3 & 0x04) >> 0) | 8] & 7;
					d[6] = pal[((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | ((p3 & 0x02) << 1) | 0] & 7;
					d[7] = pal[((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | ((p3 & 0x01) << 2) | 8] & 7;
				}
			} else {
				// text
				for(int l = 0; l < 8; l++) {
					uint8_t p1 = vram_b[src_g + l];
					uint8_t p2 = vram_r[src_g + l];
					uint8_t p3 = font_base[l];
					if(mode & 8) {
						// negative, blink
						if(attr & 8) {
							p3 = ~p3;
						}
						if(!(attr & 4) && blink) {
							p3 = 0;
						}
					}
					uint8_t* d = &screen[y + l][x << 3];
					
					c = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | 0;
					d[0] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x80) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x40) >> 6) | ((p2 & 0x40) >> 5) | 8;
					d[1] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x40) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x20) >> 5) | ((p2 & 0x20) >> 4) | 0;
					d[2] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x20) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x10) >> 4) | ((p2 & 0x10) >> 3) | 8;
					d[3] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x10) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x08) >> 3) | ((p2 & 0x08) >> 2) | 0;
					d[4] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x08) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x04) >> 2) | ((p2 & 0x04) >> 1) | 8;
					d[5] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x04) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x02) >> 1) | ((p2 & 0x02) >> 0) | 0;
					d[6] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x02) ? c_t : (pal[c] & 7);
					c = ((p1 & 0x01) >> 0) | ((p2 & 0x01) << 1) | 8;
					d[7] = (pal[c] & 8) ? (pal[c] & 7) : (p3 & 0x01) ? c_t : (pal[c] & 7);
				}
			}
			if(src_g == cursor) {
				int bp = regs[10] & 0x60;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 3], 7, 8);
					}
				}
			}
			src_g = (src_g + 16) & 0x3ff8;
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
	state_fio->StateValue(mode);
	state_fio->StateValue(text_page);
	state_fio->StateValue(cursor);
	state_fio->StateValue(cblink);
	state_fio->StateValue(flash_cnt);
	state_fio->StateValue(blink);
	state_fio->StateValue(pal_dis);
	return true;
}

