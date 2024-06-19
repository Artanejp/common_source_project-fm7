/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.03 -

	[ crtc ]
*/

#include "crtc.h"
#include "interrupt.h"
#include "memory.h"
#include "../i8255.h"

#define EVENT_HDISP_TEXT_S	0
#define EVENT_HDISP_TEXT_E	1
#define EVENT_HDISP_GRAPH_S	2
#define EVENT_HDISP_GRAPH_E	3
#define EVENT_BLINK		4

#define SCRN_640x400	1
#define SCRN_640x200	2
#define SCRN_320x200	3
#define SCRN_320x400	4

void CRTC::initialize()
{
	// config
	monitor_200line = ((config.monitor_type & 2) != 0);
	scan_line = scan_tmp = (monitor_200line && config.scan_line);
	monitor_digital = monitor_tmp = ((config.monitor_type & 1) != 0);
	boot_mode = config.boot_mode;
	
	// thanks Mr.Sato (http://x1center.org/)
	if(monitor_200line) {
		frames_per_sec = 60.99;
		lines_per_frame = 262;
		chars_per_line = 112;
	} else {
		frames_per_sec = 55.49;
		lines_per_frame = 448;
		chars_per_line = 108;
	}
	set_frames_per_sec(frames_per_sec);
	set_lines_per_frame(lines_per_frame);
	
	// set 16/4096 palette
	for(int i = 0; i < 16; i++) {
		uint8_t r, g, b, r8, g8, b8;
		if((i & 0x0f) == 8) {
			// gray
			r = r8 = 152;
			g = g8 = 152;
			b = b8 = 152;
		} else {
			r = ((i & 0x0a) == 0x0a) ? 255 : ((i & 0x0a) == 2) ? 127 : 0;
			g = ((i & 0x0c) == 0x0c) ? 255 : ((i & 0x0c) == 4) ? 127 : 0;
			b = ((i & 0x09) == 0x09) ? 255 : ((i & 0x09) == 1) ? 127 : 0;
			r8 = (i & 2) ? 255 : 0;
			g8 = (i & 4) ? 255 : 0;
			b8 = (i & 1) ? 255 : 0;
		}
		
		if(monitor_digital) {
			palette16[i] = RGB_COLOR(r8, g8, b8);
		} else {
			palette16[i] = RGB_COLOR(r, g, b);
		}
		palette4096r[i] = r;
		palette4096g[i] = g;
		palette4096b[i] = b;
		palette4096[i] = RGB_COLOR(r, g, b);
	}
	for(int i = 0; i < 8; i++) {
		palette16[i + 16] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	for(int i = 0; i < 16; i++) {
		for(int j = 1; j < 8; j++) {
			priority16[i][j] = j + 16;
		}
		priority16[i][0] = i; // transparent black
		priority16[i][8] = 0 + 16; // non transparent black
	}
	prev16 = 0xff;
	update16 = true;
	
	// set 256 palette
	for(int i = 0; i < 256; i++) {
		uint16_t b = ((i & 0x20) ? 128 : 0) | ((i & 2) ? 64 : 0) | ((i & 0x80) ? 32 : 0);
		uint16_t r = ((i & 0x40) ? 128 : 0) | ((i & 4) ? 64 : 0) | ((i & 0x80) ? 32 : 0);
		uint16_t g = ((i & 0x10) ? 128 : 0) | ((i & 1) ? 64 : 0) | ((i & 0x80) ? 32 : 0);
		b |= (b >> 3) | (b >> 6);
		r |= (r >> 3) | (r >> 6);
		g |= (g >> 3) | (g >> 6);
		palette256[i] = RGB_COLOR(b,r,g);
	}
	for(int i = 0; i < 8; i++) {
		palette256[i + 256] = RGB_COLOR(((i & 2) ? 255 : 0), ((i & 4) ? 255 : 0), ((i & 1) ? 255 : 0));
	}
	for(int i = 0; i < 64; i++) {
		palette256[i + 256 + 16] = palette256[((i & 0x38) << 1) | (i & 0x07)];
	}
	for(int i = 0; i < 256; i++) {
		for(int j = 1; j < 16 + 64; j++) {
			priority256[i][j] = j + 256;
		}
		priority256[i][0] = i; // transparent black
		priority256[i][8] = 0 + 256; // non transparent black
		priority256[i][16] = i; // transparent black (64 colors)
	}
	prev256 = -1;
	update256 = true;
	
	// extract cg optimization matrix
	for(int p1 = 0; p1 < 256; p1++) {
		for(int p2 = 0; p2 < 256; p2++) {
			for(int i = 0; i < 8; i++) {
				cg_matrix0[p1][p2][i] = (p1 & (1 << i) ? 0x01 : 0) | (p2 & (1 << i) ? 0x02 : 0);
				cg_matrix1[p1][p2][i] = (p1 & (1 << i) ? 0x04 : 0) | (p2 & (1 << i) ? 0x08 : 0);
				cg_matrix2[p1][p2][i] = (p1 & (1 << i) ? 0x10 : 0) | (p2 & (1 << i) ? 0x20 : 0);
				cg_matrix3[p1][p2][i] = (p1 & (1 << i) ? 0x40 : 0) | (p2 & (1 << i) ? 0x80 : 0);
			}
		}
	}
	
	// initialize crtc
	memset(textreg, 0, sizeof(textreg));
	memset(cgreg, 0, sizeof(cgreg));
	
	textreg[0x03] = monitor_200line ? 38 : 17;
	textreg[0x05] = textreg[0x03] + 200;
	textreg[0x07] = monitor_200line ? 11 : 9;
	textreg[0x08] = textreg[0x07] + 80;
	
	cgreg_num = 0x80;
	cgreg[0x00] = cgreg[0x01] = cgreg[0x02] = cgreg[0x03] = cgreg[0x06] = 0xff;
	GDEVS = 0; cgreg[0x08] = 0x00; cgreg[0x09] = 0x00;
	GDEVE = monitor_200line ? 200 : 400; cgreg[0x0a] = GDEVE & 0xff; cgreg[0x0b] = GDEVE >> 8;
	GDEHS = 0; cgreg[0x0c] = 0x00;
	GDEHE = 80; cgreg[0x0d] = GDEHE;
	
	for(int i = 0; i < 16; i++) {
		palette_reg[i] = i;
	}
	scrn_size = SCRN_320x200;
	font_size = true;
	column_size = false;
	cg_mask = 0x0f;
	cg_mask256 = 0;
	cg_mask256_init = false;
	clear_flag = 0;
	pal_select = false;
	screen_reverse = false;
	screen_mask = false;
	blink = false;
	latch[0] = latch[1] = latch[2] = latch[3] = 0;
	hblank_t = vblank_t = true;
	hblank_g = vblank_g = true;
	map_init = trans_init = true;
	
	// MZ-2000
	memset(font, 0, sizeof(font));
	vram_page = vram_mask = 0;
	back_color = 0;
	text_color = 7;
	
	// register events
	register_vline_event(this);
	register_event(this, EVENT_BLINK, 500000, true, NULL);
}

void CRTC::reset()
{
	// MZ-2000/80B
	for(int i = 0, s = 0x6018, d = 0; i < 256; i++, d += 8, s += 32) {
		memcpy(font + d, kanji1 + s, 8);
	}
	textreg[0x0f] &= ~3;
}

void CRTC::write_data8(uint32_t addr, uint32_t data)
{
	// read modify write
	if(cgreg[0x0e] == 0x03) {
		// for Yukara K2
		uint8_t *vram_b1 = ((cgreg[0x18] & 3) == 1) ? vram_b + 0x4000 : vram_g;
		uint8_t *vram_r1 = ((cgreg[0x18] & 3) == 1) ? vram_r + 0x4000 : vram_i;
		
		// 4 colors
		if((cgreg[5] & 0xc0) == 0x00) {
			// REPLACE
			if(addr & 0x4000) {
				addr &= 0x3fff;
				if(cgreg[5] & 1) {
					vram_b1[addr] &= ~cgreg[6];
					vram_b1[addr] |= (cgreg[4] & 1) ? (data & cgreg[0] & cgreg[6]) : 0;
				}
				if(cgreg[5] & 2) {
					vram_r1[addr] &= ~cgreg[6];
					vram_r1[addr] |= (cgreg[4] & 2) ? (data & cgreg[1] & cgreg[6]) : 0;
				}
			} else {
				addr &= 0x3fff;
				if(cgreg[5] & 1) {
					vram_b[addr] &= ~cgreg[6];
					vram_b[addr] |= (cgreg[4] & 1) ? (data & cgreg[0] & cgreg[6]) : 0;
				}
				if(cgreg[5] & 2) {
					vram_r[addr] &= ~cgreg[6];
					vram_r[addr] |= (cgreg[4] & 2) ? (data & cgreg[1] & cgreg[6]) : 0;
				}
			}
		} else if((cgreg[5] & 0xc0) == 0x40) {
			// PSET
			if(addr & 0x4000) {
				addr &= 0x3fff;
				if(cgreg[5] & 1) {
					vram_b1[addr] &= ~data;
					vram_b1[addr] |= (cgreg[4] & 1) ? (data & cgreg[0]) : 0;
				}
				if(cgreg[5] & 2) {
					vram_r1[addr] &= ~data;
					vram_r1[addr] |= (cgreg[4] & 2) ? (data & cgreg[1]) : 0;
				}
			} else {
				addr &= 0x3fff;
				if(cgreg[5] & 1) {
					vram_b[addr] &= ~data;
					vram_b[addr] |= (cgreg[4] & 1) ? (data & cgreg[0]) : 0;
				}
				if(cgreg[5] & 2) {
					vram_r[addr] &= ~data;
					vram_r[addr] |= (cgreg[4] & 2) ? (data & cgreg[1]) : 0;
				}
			}
		}
	} else {
		addr &= 0x7fff;
		if((cgreg[5] & 0xc0) == 0x00) {
			// REPLACE
			if(cgreg[5] & 1) {
				vram_b[addr] &= ~cgreg[6];
				vram_b[addr] |= (cgreg[4] & 1) ? (data & cgreg[0] & cgreg[6]) : 0;
			}
			if(cgreg[5] & 2) {
				vram_r[addr] &= ~cgreg[6];
				vram_r[addr] |= (cgreg[4] & 2) ? (data & cgreg[1] & cgreg[6]) : 0;
			}
			if(cgreg[5] & 4) {
				vram_g[addr] &= ~cgreg[6];
				vram_g[addr] |= (cgreg[4] & 4) ? (data & cgreg[2] & cgreg[6]) : 0;
			}
			if(cgreg[5] & 8) {
				vram_i[addr] &= ~cgreg[6];
				vram_i[addr] |= (cgreg[4] & 8) ? (data & cgreg[3] & cgreg[6]) : 0;
			}
		} else if((cgreg[5] & 0xc0) == 0x40) {
			// PSET
			if(cgreg[5] & 1) {
				vram_b[addr] &= ~data;
				vram_b[addr] |= (cgreg[4] & 1) ? (data & cgreg[0]) : 0;
			}
			if(cgreg[5] & 2) {
				vram_r[addr] &= ~data;
				vram_r[addr] |= (cgreg[4] & 2) ? (data & cgreg[1]) : 0;
			}
			if(cgreg[5] & 4) {
				vram_g[addr] &= ~data;
				vram_g[addr] |= (cgreg[4] & 4) ? (data & cgreg[2]) : 0;
			}
			if(cgreg[5] & 8) {
				vram_i[addr] &= ~data;
				vram_i[addr] |= (cgreg[4] & 8) ? (data & cgreg[3]) : 0;
			}
		}
	}
}

uint32_t CRTC::read_data8(uint32_t addr)
{
	// read modify write
	uint8_t b, r, g, i, pl;
	
	if(cgreg[0x0e] == 0x03) {
		// 4 colors
		b = latch[0] = (addr & 0x4000) ? vram_g[addr & 0x3fff] : vram_b[addr & 0x3fff];
		r = latch[1] = (addr & 0x4000) ? vram_i[addr & 0x3fff] : vram_r[addr & 0x3fff];
		g = latch[2] = 0;
		i = latch[3] = 0;
		pl = cgreg[7] & 1;
	} else {
		addr &= 0x7fff;
		b = latch[0] = vram_b[addr];
		r = latch[1] = vram_r[addr];
		g = latch[2] = vram_g[addr];
		i = latch[3] = vram_i[addr];
		pl = cgreg[7] & 3;
	}
	
	if(cgreg[7] & 0x10) {
		uint8_t compare = cgreg[7] & 0x0f;
		uint8_t val = (compare == (((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((i & 0x80) >> 4))) ? 0x80 : 0;
		val |= (compare == (((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4) | ((i & 0x40) >> 3))) ? 0x40 : 0;
		val |= (compare == (((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3) | ((i & 0x20) >> 2))) ? 0x20 : 0;
		val |= (compare == (((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2) | ((i & 0x10) >> 1))) ? 0x10 : 0;
		val |= (compare == (((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1) | ((i & 0x08) >> 0))) ? 0x08 : 0;
		val |= (compare == (((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0) | ((i & 0x04) << 1))) ? 0x04 : 0;
		val |= (compare == (((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1) | ((i & 0x02) << 2))) ? 0x02 : 0;
		val |= (compare == (((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2) | ((i & 0x01) << 3))) ? 0x01 : 0;
		return val;
	} else {
		return latch[pl];
	}
}

void CRTC::write_io8(uint32_t addr, uint32_t data)
{
	uint8_t haddr = (addr >> 8) & 0xff;
	uint8_t num, r, g, b, prev;
	
	switch(addr & 0xff) {
	case 0xae:
		// 4096 palette reg
		num = (haddr & 0x1f) >> 1;
		r = palette4096r[num];
		g = palette4096g[num];
		b = palette4096b[num];
		if(haddr & 1) {
			g = (data & 0x0f) << 4;
		} else {
			r = data & 0xf0;
			b = (data & 0x0f) << 4;
		}
		if(palette4096r[num] != r || palette4096g[num] != g || palette4096b[num] != b) {
			palette4096r[num] = r;
			palette4096g[num] = g;
			palette4096b[num] = b;
			palette4096[num] = RGB_COLOR(r, g, b);
			update16 = true;
		}
		// never change palette 0
		//palette4096[0] = 0;
		break;
	case 0xbc:
		// cgreg num
		cgreg_num = data;
		break;
	case 0xbd:
		// cgreg
		prev = cgreg[cgreg_num & 0x1f];
		cgreg[cgreg_num & 0x1f] = data;
		
		switch(cgreg_num & 0x1f) {
		// clear screen
		case 0x05:
			if((data & 0xc0) == 0x80) {
				uint16_t st, sz;
				switch(cgreg[0x0e]) {
				case 0x03: case 0x14: case 0x15: case 0x17: case 0x1d:
					// clear 0x0000 - 0x4000
					st = 0x0000;
					sz = 0x4000;
					break;
				case 0x94: case 0x95: case 0x97: case 0x9d:
					// clear 0x4000 - 0x7fff
					st = 0x4000;
					sz = 0x4000;
					break;
				default:
					// clear 0x0000 - 0x7fff
					st = 0x0000;
					sz = 0x8000;
				}
				if(cgreg[5] & 0x01) {
					memset(vram_b + st, 0, sz);
				}
				if(cgreg[5] & 0x02) {
					memset(vram_r + st, 0, sz);
				}
				if(cgreg[5] & 0x04) {
					memset(vram_g + st, 0, sz);
				}
				if(cgreg[5] & 0x08) {
					memset(vram_i + st, 0, sz);
				}
				clear_flag = 1;
			}
			break;
		// view range
		case 0x08:
			cgreg[0x09] = 0;
		case 0x09:
			GDEVS = (cgreg[0x08] | ((cgreg[0x09] & 1) << 8));
			break;
		case 0x0a:
			cgreg[0x0b] = 0;
		case 0x0b:
			GDEVE = (cgreg[0x0a] | ((cgreg[0x0b] & 1) << 8));
			break;
		case 0x0c:
			GDEHS = cgreg[0x0c] & 0x7f;
			break;
		case 0x0d:
			GDEHE = cgreg[0x0d] & 0x7f;
			break;
		// screen size
		case 0x0e:
			switch(data) {
			case 0x15: case 0x14: case 0x1d: case 0x95: case 0x94: case 0x9d:
				map_init |= (scrn_size != SCRN_320x200);
				scrn_size = SCRN_320x200;
				break;
			case 0x17: case 0x97:
				map_init |= (scrn_size != SCRN_640x200);
				scrn_size = SCRN_640x200;
				break;
			case 0x03: case 0x93:
				map_init |= (scrn_size != SCRN_640x400);
				scrn_size = SCRN_640x400;
				break;
			case 0x19: case 0x99:
				map_init |= (scrn_size != SCRN_320x400);
				scrn_size = SCRN_320x400;
				break;
			}
			break;
		// scroll
		case 0x0f:
			map_init |= ((prev & 0x07) != (uint8_t)(data & 0x07));
			break;
		case 0x10: case 0x12: case 0x14: case 0x16:
			map_init |= (prev != (uint8_t)(data & 0xff));
			break;
		case 0x11: case 0x13: case 0x15:
			map_init |= ((prev & 0x7f) != (uint8_t)(data & 0x7f));
			break;
		case 0x17:
			map_init |= ((prev & 0x01) != (uint8_t)(data & 0x01));
			break;
		// screen mask
		case 0x18:
			cg_mask256_init |= (prev != (uint8_t)(data & 0xff));
			break;
		}
		// inc cgreg num
		if(cgreg_num & 0x80) {
			cgreg_num = (cgreg_num & 0xfc) | ((cgreg_num + 1) & 0x03);
		}
		break;
	case 0xf4:
		// MZ-2000/80B
		if((textreg[0x0f] & 3) == 1) {
			back_color = data & 7;
			break;
		} else if((textreg[0x0f] & 3) == 2) {
			d_mem->write_io8(addr, data);
			vram_page = data & 7;
			break;
		}
		// textreg num
		textreg_num = data;
		break;
	case 0xf5:
		// MZ-2000/80B
		if((textreg[0x0f] & 3) == 1) {
			text_color = data;
			break;
		} else if((textreg[0x0f] & 3) == 2) {
			d_mem->write_io8(addr, data);
			vram_page = data & 7;
			break;
		}
		// text/palette reg
		if(textreg_num < 0x10) {
			if(textreg_num == 0) {
				trans_init |= ((textreg[0] & 2) != (uint8_t)(data & 2));
			} else if(textreg_num == 0x0a) {
				// update 256 colors palette
				if((textreg[0x0a] & 0x3f) != (uint8_t)(data & 0x3f)) {
					for(int i = 0; i < 256; i++) {
						uint8_t b0 = (data & 0x03) >> 0;
						uint8_t r0 = (data & 0x0c) >> 2;
						uint8_t g0 = (data & 0x30) >> 4;
						uint16_t b = ((i & 0x10) ? 128 : 0) | ((i & 1) ? 64 : 0) | ((b0 == 0 && (i & 0x80)) || (b0 == 1 && (i & 8)) || (b0 == 2) ? 32 : 0);
						uint16_t r = ((i & 0x20) ? 128 : 0) | ((i & 2) ? 64 : 0) | ((r0 == 0 && (i & 0x80)) || (r0 == 1 && (i & 8)) || (r0 == 2) ? 32 : 0);
						uint16_t g = ((i & 0x40) ? 128 : 0) | ((i & 4) ? 64 : 0) | ((g0 == 0 && (i & 0x80)) || (g0 == 1 && (i & 8)) || (g0 == 2) ? 32 : 0);
						b |= (b >> 3) | (b >> 6);
						r |= (r >> 3) | (r >> 6);
						g |= (g >> 3) | (g >> 6);
						palette256[i] = RGB_COLOR(r, g, b);
					}
					update256 = true;
				}
			} else if(textreg_num == 0x0f) {
				if(textreg[0x0f] & 3) {
					data = (data & ~3) | (textreg[0x0f] & 3);
				}
			}
			textreg[textreg_num] = data;
			
			// kugyokuden 400line patch
			if(get_cpu_pc(0) == 0xc27e && !monitor_200line) {
				if(textreg[3] == 0x26 && textreg[5] == 0xee) {
					textreg[3] = 0x11;
					textreg[5] = 0xd9;
				}
			}
		} else if(0x80 <= textreg_num && textreg_num < 0x90) {
			int c = textreg_num & 0x0f;
			int p = data & 0x10;
			
			prev = palette_reg[c];
			palette_reg[c] = data;
			
			if((prev & 0x0f) != (uint8_t)(data & 0x0f)) {
				update16 = true;
			}
			if((prev & 0x10) != (uint8_t)(data & 0x10)) {
				// update priority
				for(int i = 1; i < 8; i++) {
					priority16[c][i] = p ? c : (i + 16);
				}
				priority16[c][0] = c; // transparent black
				priority16[c][8] = p ? c : (0 + 16); // non transparent black
				update16 = true;
			}
			if((prev & 0x1f) != (uint8_t)(data & 0x1f)) {
				// update priority (256 colors)
				int c16 = c << 4;
				int col16 = (data & 0x0f) << 4;
				
				for(int i = 0; i < 16; i++) {
					for(int j = 1; j < 16 + 64; j++) {
						priority256[c16 | i][j] = p ? (col16 | i) : (j + 256);
					}
					priority256[c16 | i][0] = col16 | i; // transparent black
					priority256[c16 | i][8] = p ? (col16 | i) : (0 + 256); // non transparent black
					priority256[c16 | i][16] = col16 | i; // transparent black (64 colors)
				}
				update256 = true;
			}
		}
		break;
	case 0xf6:
		// MZ-2000/80B
		if((textreg[0x0f] & 3) == 1) {
			vram_mask = data;
			break;
		} else if((textreg[0x0f] & 3) == 2) {
			d_mem->write_io8(addr, data);
			vram_page = data & 7;
			break;
		}
		// cg mask reg
		prev = cg_mask;
		cg_mask = (data & 7) | ((data & 1) ? 8 : 0);
		if(prev != cg_mask) {
			cg_mask256_init = true;
			update16 = true;
		}
		break;
	case 0xf7:
		// MZ-2000/80B
		if((textreg[0x0f] & 3) == 1 || (textreg[0x0f] & 3) == 2) {
			d_mem->write_io8(addr, data);
			vram_page = data & 7;
			break;
		}
		// font size reg
		font_size = ((data & 1) != 0);
		break;
	}
}

uint32_t CRTC::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xbc:
		// read plane b
		if(cgreg[7] & 0x10) {
			uint8_t b = latch[0];
			uint8_t r = latch[1];
			uint8_t g = latch[2];
			uint8_t i = latch[3];
			uint8_t compare = cgreg[7] & 0x0f;
			
			uint8_t val = (compare == (((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((i & 0x80) >> 4))) ? 0x80 : 0;
			val |= (compare == (((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4) | ((i & 0x40) >> 3))) ? 0x40 : 0;
			val |= (compare == (((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3) | ((i & 0x20) >> 2))) ? 0x20 : 0;
			val |= (compare == (((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2) | ((i & 0x10) >> 1))) ? 0x10 : 0;
			val |= (compare == (((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1) | ((i & 0x08) >> 0))) ? 0x08 : 0;
			val |= (compare == (((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0) | ((i & 0x04) << 1))) ? 0x04 : 0;
			val |= (compare == (((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1) | ((i & 0x02) << 2))) ? 0x02 : 0;
			val |= (compare == (((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2) | ((i & 0x01) << 3))) ? 0x01 : 0;
			return val;
		} else {
			return latch[0];
		}
	case 0xbd:
		// read plane r
		if(cgreg[7] & 0x10) {
			return (vblank_g ? 0 : 0x80) | clear_flag;
		} else {
			return latch[1];
		}
	case 0xbe:
		// read plane g
		return latch[2];
	case 0xbf:
		// read plane i
		return latch[3];
	case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		// get blank state
		return (vblank_t ? 0 : 1) | (hblank_t ? 0 : 2);
	}
	return 0xff;
}

void CRTC::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CRTC_COLUMN_SIZE) {
		column_size = ((data & mask) != 0);	// from z80pio port a
	} else if(id == SIG_CRTC_PALLETE) {
		pal_select = ((data & mask) == 0);	// from ym2203 port a
	} else if(id == SIG_CRTC_REVERSE) {
		screen_reverse = ((data & mask) == 0);	// from i8255 port a
	} else if(id == SIG_CRTC_MASK) {
		screen_mask = ((data & mask) != 0);	// from i8255 port c
	}
}

void CRTC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HDISP_TEXT_S) {
		hblank_t = false;
		d_mem->write_signal(SIG_MEMORY_HBLANK_TEXT, 0, 1);
	} else if(event_id == EVENT_HDISP_TEXT_E) {
		hblank_t = true;
		d_mem->write_signal(SIG_MEMORY_HBLANK_TEXT, 1, 1);
	} else if(event_id == EVENT_HDISP_GRAPH_S) {
		hblank_g = false;
		d_mem->write_signal(SIG_MEMORY_HBLANK_GRAPH, 0, 1);
	} else if(event_id == EVENT_HDISP_GRAPH_E) {
		hblank_g = true;
		d_mem->write_signal(SIG_MEMORY_HBLANK_GRAPH, 1, 1);
	} else if(event_id == EVENT_BLINK) {
		blink = !blink;
	}
}

void CRTC::event_vline(int v, int clock)
{
	double clocks_per_char = get_event_clocks() / frames_per_sec / lines_per_frame / chars_per_line;
	
	// 2022.09.05 adjust hsync timing
	// https://twitter.com/youkan700/status/794210717398286337
	
	// vblank/hblank of text screen
	if(v == textreg[5] * (monitor_200line ? 1 : 2)) {
		d_mem->write_signal(SIG_MEMORY_VBLANK_TEXT, 1, 1);
		vblank_t = true;
	} else if(v == textreg[3] * (monitor_200line ? 1 : 2)) {
		d_mem->write_signal(SIG_MEMORY_VBLANK_TEXT, 0, 1);
		vblank_t = false;
	}
	if((textreg[7] & 0x7f) < (textreg[8] & 0x7f)) {
		register_event_by_clock(this, EVENT_HDISP_TEXT_S, (int)(clocks_per_char * (textreg[7] & 0x7f) + 0.5) - 2, false, NULL);
		register_event_by_clock(this, EVENT_HDISP_TEXT_E, (int)(clocks_per_char * (textreg[8] & 0x7f) + 0.5) + 6, false, NULL);
	} else {
		event_callback(EVENT_HDISP_TEXT_E, 0);
	}
	
	// vblank/hblank of graph screen
	if(v == GDEVE) {
		d_pio->write_signal(SIG_I8255_PORT_B, 0, 1);
		d_int->write_signal(SIG_INTERRUPT_CRTC, 1, 1);
		d_mem->write_signal(SIG_MEMORY_VBLANK_GRAPH, 1, 1);
		vblank_g = true;
		clear_flag = 0;
	} else if(v == GDEVS) {
		d_pio->write_signal(SIG_I8255_PORT_B, 1, 1);
		d_int->write_signal(SIG_INTERRUPT_CRTC, 0, 1);
		d_mem->write_signal(SIG_MEMORY_VBLANK_GRAPH, 0, 1);
		vblank_g = false;
	}
	if(GDEHS < GDEHE) {
		register_event_by_clock(this, EVENT_HDISP_GRAPH_S, (int)(clocks_per_char * (GDEHS + 10) + 0.5) - 2, false, NULL);
		register_event_by_clock(this, EVENT_HDISP_GRAPH_E, (int)(clocks_per_char * (GDEHE + 10) + 0.5) + 4, false, NULL);
	} else {
		event_callback(EVENT_HDISP_GRAPH_E, 0);
	}
}

void CRTC::update_config()
{
	//monitor_200line = ((config.monitor_type & 2) != 0);
	scan_tmp = (monitor_200line && config.scan_line);
	monitor_tmp = ((config.monitor_type & 1) != 0);
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void CRTC::draw_screen()
{
	// MZ-2000/80B
	if((textreg[0x0f] & 3) == 1 || boot_mode == 1) {
		draw_screen_2000();
		return;
	} else if((textreg[0x0f] & 3) == 2 || boot_mode == 2) {
		draw_screen_80b();
		return;
	}
	
	// update config
	scan_line = scan_tmp;
	
	if(monitor_digital != monitor_tmp) {
		monitor_digital = monitor_tmp;
		// set 16 colors palette
		for(int i = 0; i < 16; i++) {
			uint8_t r, g, b, r8, g8, b8;
			if((i & 0x0f) == 0x08) {
				// gray
				r = r8 = 152;
				g = g8 = 152;
				b = b8 = 152;
			} else {
				r = ((i & 0x0a) == 0x0a) ? 255 : ((i & 0x0a) == 0x02) ? 127 : 0;
				g = ((i & 0x0c) == 0x0c) ? 255 : ((i & 0x0c) == 0x04) ? 127 : 0;
				b = ((i & 0x09) == 0x09) ? 255 : ((i & 0x09) == 0x01) ? 127 : 0;
				r8 = (i & 0x02) ? 255 : 0;
				g8 = (i & 0x04) ? 255 : 0;
				b8 = (i & 0x01) ? 255 : 0;
			}
			
			if(monitor_digital) {
				palette16[i] = RGB_COLOR(r8, g8, b8);
			} else {
				palette16[i] = RGB_COLOR(r, g, b);
			}
		}
		update16 = true;
	}
	
	// update 16/4096 palette
	uint8_t back16 = ((textreg[0x0b] & 4) >> 2) | ((textreg[0x0b] & 0x20) >> 4) | ((textreg[0x0c] & 1) << 2) | ((textreg[0x0b] & 1) << 3);
	if(back16 != prev16) {
		prev16 = back16;
		update16 = true;
	}
	if(update16) {
		scrntype_t palette16tmp[16 + 8], palette4096tmp[16 + 8];
		for(int i = 0; i < 16 + 8; i++) {
			palette16tmp[i] = palette16[(i & 16) ? i : (palette_reg[i] & 0x0f) ? (palette_reg[i] & cg_mask) : (back16 & cg_mask)];
			uint8_t col = (i == 16) ? 0 : (i & 16) ? (i & 0x0f) + 8 : i;
			palette4096tmp[i] = palette4096[(palette_reg[col] & 0x0f) ? (palette_reg[col] & cg_mask) : (back16 & cg_mask)];
		}
		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < 9; j++) {
				palette16pri[i][j] = palette16tmp[priority16[i][j]];
				palette4096pri[i][j] = palette4096tmp[priority16[i][j]];
			}
		}
		memcpy(palette16txt, &palette16tmp[16], sizeof(scrntype_t) * 8);
//		palette16txt[0] = (back16 == 0 && (palette_reg[0] & 0x0f)== 2) ? 0 : palette16[palette_reg[back16] & 0x0f]; // tower of doruaga
		palette16txt[0] = palette16[palette_reg[back16] & 0x0f];
		palette16txt[8] = 0;
		memcpy(palette4096txt, &palette4096tmp[16], sizeof(scrntype_t) * 8);
		palette4096txt[0] = palette4096[palette_reg[back16] & 0x0f];
		palette4096txt[8] = 0;
		update16 = false;
	}
	
	// update 256 palette
	scrntype_t back256 = RGB_COLOR((textreg[0x0b] & 0x38) << 2, ((textreg[0x0b] & 0xc0) >> 1) | ((textreg[0x0c] & 1) << 7), (textreg[0x0b] & 7) << 5);
	if(back256 != prev256) {
		prev256 = back256;
		update256 = true;
	}
	if(update256) {
		palette256[0] = back256;
		for(int i = 0; i < 256; i++) {
			for(int j = 0; j < 16 + 64; j++) {
				palette256pri[i][j] = palette256[priority256[i][j]];
			}
		}
		memcpy(palette256txt, &palette256[256], sizeof(scrntype_t) * (16 + 64));
		palette256txt[0] = back256;
		palette256txt[8] = 0;
		update256 = false;
	}
	
	// draw cg screen
	memset(cg, 0, sizeof(cg));
	draw_cg();
	
	// draw text screen
	memset(text, 0, sizeof(text));
	draw_text();
	
	// view port
	int vs = (GDEVS <= GDEVE) ? GDEVS * ((scrn_size == SCRN_640x400 || scrn_size == SCRN_320x400) ? 1 : 2) : 0;
	int ve = (GDEVS <= GDEVE) ? GDEVE * ((scrn_size == SCRN_640x400 || scrn_size == SCRN_320x400) ? 1 : 2) : 400;
	int hs = (GDEHS <= GDEHE && GDEHS < 80) ? (GDEHS << 3) : 0;
	int he = (GDEHS <= GDEHE && GDEHE < 80) ? (GDEHE << 3) : 640;
	
	// mix screens
	emu->set_vm_screen_lines(ve);
	
	if(screen_mask) {
		// screen is masked
		for(int y = 0; y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			memset(dest, 0, sizeof(scrntype_t) * 640);
		}
	} else if(cgreg[0x0e] == 0x1d || cgreg[0x0e] == 0x9d ||
	          cgreg[0x0e] == 0x19 || cgreg[0x0e] == 0x99) {
		// 256 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette256txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < hs && x < 640; x++) {
				dest[x] = palette256txt[src_text[x]];
			}
			for(int x = hs; x < he && x < 640; x++) {
				dest[x] = palette256pri[src_cg[x]][src_text[x]];
			}
			for(int x = he; x < 640; x++) {
				dest[x] = palette256txt[src_text[x]];
			}
		}
		for(int y = ve; y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette256txt[src_text[x]];
			}
		}
	} else if(!pal_select) {
		// 16 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < hs && x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
			for(int x = hs; x < he && x < 640; x++) {
				dest[x] = palette16pri[src_cg[x]][src_text[x]];
			}
			for(int x = he; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
		for(int y = ve; y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
	} else {
		// 4096 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < hs && x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
			for(int x = hs; x < he && x < 640; x++) {
				dest[x] = palette4096pri[src_cg[x]][src_text[x]];
			}
			for(int x = he; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
		for(int y = ve; y < 400; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
	}
	emu->screen_skip_line(monitor_200line);
}

// ----------------------------------------------------------------------------
// draw text screen
// ----------------------------------------------------------------------------

void CRTC::draw_text()
{
	// extract text optimization matrix
	if(trans_init) {
		trans_color = (textreg[0] & 2) ? 8 : 0;
		for(int pat = 0; pat < 256; pat++) {
			for(int col = 0; col < 8; col++) {
				int fore_color = col ? col : 8;
				text_matrix[pat][col][0] = text_matrixw[pat][col][ 0] = text_matrixw[pat][col][ 1] = (pat & 0x80) ? fore_color : trans_color;
				text_matrix[pat][col][1] = text_matrixw[pat][col][ 2] = text_matrixw[pat][col][ 3] = (pat & 0x40) ? fore_color : trans_color;
				text_matrix[pat][col][2] = text_matrixw[pat][col][ 4] = text_matrixw[pat][col][ 5] = (pat & 0x20) ? fore_color : trans_color;
				text_matrix[pat][col][3] = text_matrixw[pat][col][ 6] = text_matrixw[pat][col][ 7] = (pat & 0x10) ? fore_color : trans_color;
				text_matrix[pat][col][4] = text_matrixw[pat][col][ 8] = text_matrixw[pat][col][ 9] = (pat & 0x08) ? fore_color : trans_color;
				text_matrix[pat][col][5] = text_matrixw[pat][col][10] = text_matrixw[pat][col][11] = (pat & 0x04) ? fore_color : trans_color;
				text_matrix[pat][col][6] = text_matrixw[pat][col][12] = text_matrixw[pat][col][13] = (pat & 0x02) ? fore_color : trans_color;
				text_matrix[pat][col][7] = text_matrixw[pat][col][14] = text_matrixw[pat][col][15] = (pat & 0x01) ? fore_color : trans_color;
			}
		}
		trans_init = false;
	}
	
	// draw text
	if(column_size) {
		draw_80column_screen();
	} else {
		draw_40column_screen();
	}
	
	// display period
	int SL, EL, SC, EC;
	if(monitor_200line) {
		// 200 lines
		SL = (textreg[3] - 38) << 1;
		EL = (textreg[5] - 38) << 1;
		if(column_size) {
			// 80 column
			SC = (textreg[7] & 0x7f) - 11;
			EC = (textreg[8] & 0x7f) - 11;
		} else {
			// 40 column
			SC = (textreg[7] & 0x7f) - 10;
			EC = (textreg[8] & 0x7f) - 10;
		}
	} else {
		// 400 lines
		SL = (textreg[3] - 17) << 1;
		EL = (textreg[5] - 17) << 1;
		if(column_size) {
			// 80 colums
			SC = (textreg[7] & 0x7f) - 9;
			EC = (textreg[8] & 0x7f) - 9;
		} else {
			// 40 column
			SC = (textreg[7] & 0x7f) - 8;
			EC = (textreg[8] & 0x7f) - 8;
		}
	}
	SL = (SL < 0) ? 0 : (SL > 400) ? 400 : SL;
	EL = (EL < 0) ? 0 : (EL > 400) ? 400 : EL;
	SC = (SC < 0) ? 0 : (SC > 80) ? 80 : SC;
	EC = (EC < 0) ? 0 : (EC > 80) ? 80 : EC;
	
	if(EL >= SL) {
		for(int y = 0; y < SL; y++) {
			memset(text + 640 * y, trans_color, 640);
		}
		for(int y = EL; y < 400; y++) {
			memset(text + 640 * y, trans_color, 640);
		}
	} else {
		for(int y = EL; y < SL; y++) {
			memset(text + 640 * y, trans_color, 640);
		}
	}
	if(EC >= SC) {
		for(int y = 0; y < 400; y++) {
			memset(text + 640 * y, trans_color, SC << 3);
			memset(text + 640 * y + (EC << 3), trans_color, (80 - EC) << 3);
		}
	} else {
		for(int y = 0; y < 400; y++) {
			memset(text + 640 * y + (EC << 3), trans_color, (SC - EC) << 3);
		}
	}
}

void CRTC::draw_80column_screen()
{
	uint16_t src = textreg[1] | ((textreg[2] & 0x07) << 8);
	uint8_t line = (textreg[0] & 0x10) ? 2 : 0;
	uint8_t height = (textreg[0] & 0x10) ? 20 : 16;
	uint8_t vd = (textreg[9] & 0x0f) << 1;
	
	// 80x20(25)
	for(int y = line; y < 416; y += height) {
		int dest = (y - vd) * 640;
		for(int x = 0; x < 80; x++) {
			draw_80column_font((src++) & 0x7ff, dest, y - vd);
			dest += 8;
		}
	}
	
	//
	// if in 256 color mode but 40-column function is not set as 64 color mode,
	// convert its color as the bottom plane is forced to 0.
	//
	if ((textreg[0] & 0x01) == 0x00) {
		for(int y = line; y < 400; y++) {
			uint8_t* tdest = &text[y * 640];
			for(int x = 0; x < 640; x++) {
			        if (!(tdest[x] & 8)) tdest[x] = ((tdest[x] & 7) << 3) + 16;
			}
		}
	}
}

void CRTC::draw_40column_screen()
{
	uint16_t src1 = textreg[1] | ((textreg[2] & 0x07) << 8);
	uint16_t src2 = src1 + 0x400;
	uint8_t line = (textreg[0] & 0x10) ? 2 : 0;
	uint8_t height = (textreg[0] & 0x10) ? 20 : 16;
	uint8_t vd = (textreg[9] & 0x0f) << 1;
	
	switch(textreg[0] & 0x0c) {
	case 0x00:
		// 40x20(25), 64colors
		for(int y = line; y < 416; y += height) {
			int dest1 = (y - vd) * 640;
			int dest2 = (y - vd) * 640 + 640 * 480;
			for(int x = 0; x < 40; x++) {
				draw_40column_font((src1++) & 0x7ff, dest1, y - vd);
				draw_40column_font((src2++) & 0x7ff, dest2, y - vd);
				dest1 += 16;
				dest2 += 16;
			}
		}
		for(int y = 0; y < 400; y++) {
			uint32_t src1 = 640 * y;
			uint32_t src2 = 640 * y + 640 * 480;
			uint32_t dest = 640 * y;
			uint8_t col;
			for(int x = 0; x < 640; x++) {
				// thanks Mr.Koucha-Youkan
				col = (((text[src1] & 7) << 3) | (text[src2] & 7)) + 16;
				if(col == 16 && ((text[src1] & 8) && (text[src2] & 8))) {
					col = 8; // non transparent black
				}
				text[dest++] = col;
				src1++;
				src2++;
			}
		}
		break;
	case 0x04:
		// 40x20(25), No.1 Screen
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640;
			for(int x = 0; x < 40; x++) {
				draw_40column_font((src1++) & 0x7ff, dest, y - vd);
				dest += 16;
			}
		}
		break;
	case 0x08:
		// 40x20(25), No.2 Screen
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640;
			for(int x = 0; x < 40; x++) {
				draw_40column_font((src2++) & 0x7ff, dest, y - vd);
				dest += 16;
			}
		}
		break;
	case 0x0c:
		// 40x20(25), No.1 + No.2 Screens (No.1 > No.2)
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640;
			for(int x = 0; x < 40; x++) {
				draw_40column_font((src1++) & 0x7ff, dest, y - vd);
				dest += 16;
			}
		}
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640 + 640 * 480;
			for(int x = 0; x < 40; x++) {
				draw_40column_font((src2++) & 0x7ff, dest, y - vd);
				dest += 16;
			}
		}
		for(int y = line; y < 400; y++) {
			int dest = (y - vd) * 640;
			uint8_t* tsrc1 = &text[dest];
			uint8_t* tsrc2 = &text[dest + 640 * 480];
			uint8_t* tdest = &text[dest];
			for(int x = 0; x < 640; x++) {
				tdest[x] = (tsrc1[x] & 7) ? tsrc1[x] : (tsrc2[x] & 7) ? tsrc2[x] : ((tsrc1[x] & 8) | (tsrc2[x] & 8));
			}
		}
		break;
	}
	
	//
	// if in 256 color mode but 40-column function is not set as 64 color mode,
	// convert its color as the bottom plane is forced to 0.
	//
	if ((textreg[0] & 0x0c) != 0x00 && (textreg[0] & 0x01) == 0x00) {
		for(int y = line; y < 400; y++) {
			uint8_t* tdest = &text[y * 640];
			for(int x = 0; x < 640; x++) {
			        if (!(tdest[x] & 8)) tdest[x] = ((tdest[x] & 7) << 3) + 16;
			}
		}
	}
}

void CRTC::draw_80column_font(uint16_t src, int dest, int y)
{
	// draw char (80 column)
	uint8_t* pattern1;
	uint8_t* pattern2;
	uint8_t* pattern3;
	
	uint32_t code;
	uint8_t sel, col, pat1, pat2, pat3;
	uint8_t t1 = tvram1[src], t2 = tvram2[src], attr = attrib[src];
	
	// select char type
	sel = (t2 & 0xc0) | (attr & 0x38);
	switch(sel) {
	case 0x00: case 0x40:
		pattern1 = pcg0;
		break;
	case 0x80:
		pattern1 = kanji1;
		break;
	case 0xc0:
		pattern1 = kanji2;
		break;
	case 0x10: case 0x50: case 0x90: case 0xd0:
		pattern1 = pcg1;
		break;
	case 0x20: case 0x60: case 0xa0: case 0xe0:
		pattern1 = pcg2;
		break;
	case 0x30: case 0x70: case 0xb0: case 0xf0:
		pattern1 = pcg3;
		break;
	default:
		pattern1 = pcg1;
		pattern2 = pcg2;
		pattern3 = pcg3;
		break;
	}
	if(sel & 8) {
		// PCG1 + PCG2 + PCG3 8colors
		
		// generate addr
		code = font_size ? t1 << 3 : (t1 & 0xfe) << 3;
		// draw
		if(font_size) {
			for(int i = 0; i < 8; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if((attr & 0x80) && blink) {
					// blink
					uint8_t val = (attr & 0x40) ? 7 : trans_color;
					if(dest >= 0) {
						memset(text + dest, val, 8);
					}
					dest += 640;
					// check end line of screen
					if(!(y++ < 480)) {
						break;
					}
					if(dest >= 0 && !scan_line) {
						memset(text + dest, val, 8);
					}
					dest += 640;
				} else {
					if(attr & 0x40) {
						pat1 = ~pattern1[code + i];
						pat2 = ~pattern2[code + i];
						pat3 = ~pattern3[code + i];
					} else {
						pat1 = pattern1[code + i];
						pat2 = pattern2[code + i];
						pat3 = pattern3[code + i];
					}
					if(dest >= 0) {
						uint8_t* tdest = &text[dest];
						col = ((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5); tdest[0] = col ? col : trans_color;
						col = ((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4); tdest[1] = col ? col : trans_color;
						col = ((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3); tdest[2] = col ? col : trans_color;
						col = ((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2); tdest[3] = col ? col : trans_color;
						col = ((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1); tdest[4] = col ? col : trans_color;
						col = ((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ); tdest[5] = col ? col : trans_color;
						col = ((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1); tdest[6] = col ? col : trans_color;
						col = ((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2); tdest[7] = col ? col : trans_color;
					}
					dest += 640;
					// check end line of screen
					if(!(y++ < 480)) {
						break;
					}
					if(dest >= 0 && !scan_line) {
						if(dest >= 640) {
							memcpy(text + dest, text + dest - 640, 8);
						} else {
							uint8_t* tdest = &text[dest];
							col = ((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5); tdest[0] = col ? col : trans_color;
							col = ((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4); tdest[1] = col ? col : trans_color;
							col = ((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3); tdest[2] = col ? col : trans_color;
							col = ((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2); tdest[3] = col ? col : trans_color;
							col = ((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1); tdest[4] = col ? col : trans_color;
							col = ((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ); tdest[5] = col ? col : trans_color;
							col = ((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1); tdest[6] = col ? col : trans_color;
							col = ((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2); tdest[7] = col ? col : trans_color;
						}
					}
					dest += 640;
				}
			}
		} else {
			for(int i = 0; i < 16; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0) {
					if((attr & 0x80) && blink) {
						if(attr & 0x40) {
							memset(text + dest, 7, 8);
						} else {
							memset(text + dest, trans_color, 8);
						}
					} else {
						if(attr & 0x40) {
							pat1 = ~pattern1[code + i];
							pat2 = ~pattern2[code + i];
							pat3 = ~pattern3[code + i];
						} else {
							pat1 = pattern1[code + i];
							pat2 = pattern2[code + i];
							pat3 = pattern3[code + i];
						}
						uint8_t* tdest = &text[dest];
						col = ((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5); tdest[0] = col ? col : trans_color;
						col = ((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4); tdest[1] = col ? col : trans_color;
						col = ((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3); tdest[2] = col ? col : trans_color;
						col = ((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2); tdest[3] = col ? col : trans_color;
						col = ((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1); tdest[4] = col ? col : trans_color;
						col = ((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ); tdest[5] = col ? col : trans_color;
						col = ((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1); tdest[6] = col ? col : trans_color;
						col = ((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2); tdest[7] = col ? col : trans_color;
					}
				}
				dest += 640;
			}
		}
	} else {
		// monochrome
		
		// generate addr
		if(font_size) {
			if(sel == 0x80 || sel == 0xc0) {
				code = ((t2 & 0x3f) << 11) | (t1 << 3);
			} else {
				code = t1 << 3;
			}
		} else {
			if(sel == 0x80 || sel == 0xc0) {
				code = ((t2 & 0x3f) << 11) | ((t1 & 0xfe) << 3);
			} else {
				code = (t1 & 0xfe) << 3;
			}
		}
		// color
		col = attr & 0x07;
		// draw
		if(font_size) {
			uint32_t dest1 = dest;
			uint32_t dest2 = (dest >= 640 * 399) ? dest - 640 * 399 : dest + 640;
			for(int i = 0; i < 8; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				// reverse, blink
				if(attr & 0x40) {
					pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[code + i];
				} else {
					pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[code + i];
				}
				if(dest >= 0) {
					memcpy(&text[dest], text_matrix[pat1][col], 8);
				}
				dest += 640;
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0 && !scan_line) {
					memcpy(&text[dest], text_matrix[pat1][col], 8);
				}
				dest += 640;
			}
		} else {
			for(int i = 0; i < 16; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0) {
					// reverse, blink
					if(attr & 0x40) {
						pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[code + i];
					} else {
						pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[code + i];
					}
					memcpy(&text[dest], text_matrix[pat1][col], 8);
				}
				dest += 640;
			}
		}
	}
}

void CRTC::draw_40column_font(uint16_t src, int dest, int y)
{
	// draw char (40 column)
	uint8_t* pattern1;
	uint8_t* pattern2;
	uint8_t* pattern3;
	
	uint32_t code;
	uint8_t sel, col, pat1, pat2, pat3;
	uint8_t t1 = tvram1[src], t2 = tvram2[src], attr = attrib[src];
	
	// select char type
	sel = (t2 & 0xc0) | (attr & 0x38);
	switch(sel) {
	case 0x00: case 0x40:
		pattern1 = pcg0;
		break;
	case 0x80:
		pattern1 = kanji1;
		break;
	case 0xc0:
		pattern1 = kanji2;
		break;
	case 0x10: case 0x50: case 0x90: case 0xd0:
		pattern1 = pcg1;
		break;
	case 0x20: case 0x60: case 0xa0: case 0xe0:
		pattern1 = pcg2;
		break;
	case 0x30: case 0x70: case 0xb0: case 0xf0:
		pattern1 = pcg3;
		break;
	default:
		pattern1 = pcg1;
		pattern2 = pcg2;
		pattern3 = pcg3;
		break;
	}
	if(sel & 0x08) {
		// PCG1 + PCG2 + PCG3 8colors
		
		// generate addr
		code = font_size ? t1 << 3 : (t1 & 0xfe) << 3;
		// draw
		if(font_size) {
			for(int i = 0; i < 8; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if((attr & 0x80) && blink) {
					// blink
					uint8_t val = (attr & 0x40) ? 7 : trans_color;
					if(dest >= 0) {
						memset(text + dest, val, 16);
					}
					dest += 640;
					// check end line of screen
					if(!(y++ < 480)) {
						break;
					}
					if(dest >= 0 && !scan_line) {
						memset(text + dest, val, 16);
					}
					dest += 640;
				} else {
					if(attr & 0x40) {
						pat1 = ~pattern1[code + i];
						pat2 = ~pattern2[code + i];
						pat3 = ~pattern3[code + i];
					} else {
						pat1 = pattern1[code + i];
						pat2 = pattern2[code + i];
						pat3 = pattern3[code + i];
					}
					if(dest >= 0) {
						uint8_t* tdest = &text[dest];
						col = ((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5); tdest[ 0] = tdest[ 1] = col ? col : trans_color;
						col = ((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4); tdest[ 2] = tdest[ 3] = col ? col : trans_color;
						col = ((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3); tdest[ 4] = tdest[ 5] = col ? col : trans_color;
						col = ((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2); tdest[ 6] = tdest[ 7] = col ? col : trans_color;
						col = ((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1); tdest[ 8] = tdest[ 9] = col ? col : trans_color;
						col = ((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ); tdest[10] = tdest[11] = col ? col : trans_color;
						col = ((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1); tdest[12] = tdest[13] = col ? col : trans_color;
						col = ((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2); tdest[14] = tdest[15] = col ? col : trans_color;
					}
					dest += 640;
					// check end line of screen
					if(!(y++ < 480)) {
						break;
					}
					if(dest >= 0 && !scan_line) {
						if(dest >= 640) {
							memcpy(text + dest, text + dest - 640, 16);
						} else {
							uint8_t* tdest = &text[dest];
							col = ((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5); tdest[ 0] = tdest[ 1] = col ? col : trans_color;
							col = ((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4); tdest[ 2] = tdest[ 3] = col ? col : trans_color;
							col = ((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3); tdest[ 4] = tdest[ 5] = col ? col : trans_color;
							col = ((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2); tdest[ 6] = tdest[ 7] = col ? col : trans_color;
							col = ((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1); tdest[ 8] = tdest[ 9] = col ? col : trans_color;
							col = ((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ); tdest[10] = tdest[11] = col ? col : trans_color;
							col = ((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1); tdest[12] = tdest[13] = col ? col : trans_color;
							col = ((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2); tdest[14] = tdest[15] = col ? col : trans_color;
						}
					}
					dest += 640;
				}
			}
		} else {
			for(int i = 0; i < 16; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0) {
					if((attr & 0x80) && blink) {
						if(attr & 0x40) {
							memset(text + dest, 7, 16);
						} else {
							memset(text + dest, trans_color, 16);
						}
					} else {
						if(attr & 0x40) {
							pat1 = ~pattern1[code + i];
							pat2 = ~pattern2[code + i];
							pat3 = ~pattern3[code + i];
						} else {
							pat1 = pattern1[code + i];
							pat2 = pattern2[code + i];
							pat3 = pattern3[code + i];
						}
						uint8_t* tdest = &text[dest];
						col = ((pat1 & 0x80) >> 7) | ((pat2 & 0x80) >> 6) | ((pat3 & 0x80) >> 5); tdest[ 0] = tdest[ 1] = col ? col : trans_color;
						col = ((pat1 & 0x40) >> 6) | ((pat2 & 0x40) >> 5) | ((pat3 & 0x40) >> 4); tdest[ 2] = tdest[ 3] = col ? col : trans_color;
						col = ((pat1 & 0x20) >> 5) | ((pat2 & 0x20) >> 4) | ((pat3 & 0x20) >> 3); tdest[ 4] = tdest[ 5] = col ? col : trans_color;
						col = ((pat1 & 0x10) >> 4) | ((pat2 & 0x10) >> 3) | ((pat3 & 0x10) >> 2); tdest[ 6] = tdest[ 7] = col ? col : trans_color;
						col = ((pat1 & 0x08) >> 3) | ((pat2 & 0x08) >> 2) | ((pat3 & 0x08) >> 1); tdest[ 8] = tdest[ 9] = col ? col : trans_color;
						col = ((pat1 & 0x04) >> 2) | ((pat2 & 0x04) >> 1) | ((pat3 & 0x04)     ); tdest[10] = tdest[11] = col ? col : trans_color;
						col = ((pat1 & 0x02) >> 1) | ((pat2 & 0x02)     ) | ((pat3 & 0x02) << 1); tdest[12] = tdest[13] = col ? col : trans_color;
						col = ((pat1 & 0x01)     ) | ((pat2 & 0x01) << 1) | ((pat3 & 0x01) << 2); tdest[14] = tdest[15] = col ? col : trans_color;
					}
				}
				dest += 640;
			}
		}
	} else {
		// monochrome
		
		// generate addr
		if(font_size) {
			if(sel == 0x80 || sel == 0xc0) {
				code = ((t2 & 0x3f) << 11) | (t1 << 3);
			} else {
				code = t1 << 3;
			}
		} else {
			if(sel == 0x80 || sel == 0xc0) {
				code = ((t2 & 0x3f) << 11) | ((t1 & 0xfe) << 3);
			} else {
				code = (t1 & 0xfe) << 3;
			}
		}
		// color
		col = attr & 0x07;
		// draw
		if(font_size) {
			for(int i = 0; i < 8; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				// reverse, blink
				if(attr & 0x40) {
					pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[code + i];
				} else {
					pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[code + i];
				}
				if(dest >= 0) {
					memcpy(&text[dest], text_matrixw[pat1][col], 16);
				}
				dest += 640;
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0 && !scan_line) {
					memcpy(&text[dest], text_matrixw[pat1][col], 16);
				}
				dest += 640;
			}
		} else {
			for(int i = 0; i < 16; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0) {
					// reverse, blink
					if(attr & 0x40) {
						pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[code + i];
					} else {
						pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[code + i];
					}
					memcpy(&text[dest], text_matrixw[pat1][col], 16);
				}
				dest += 640;
			}
		}
	}
}

// ----------------------------------------------------------------------------
// drive cg screen
// ----------------------------------------------------------------------------

void CRTC::draw_cg()
{
	// draw cg screen
	switch(cgreg[0x0e]) {
	case 0x03:
		draw_640x400x4screen();
		break;
	case 0x14:
		draw_320x200x16screen(0);
		draw_320x200x16screen(1);
		break;
	case 0x15:
		draw_320x200x16screen(1);
		draw_320x200x16screen(0);
		break;
	case 0x17:
		draw_640x200x16screen(0);
		break;
	case 0x1d:
	case 0x9d:
		draw_320x200x256screen(200);
		break;
	case 0x93:
		draw_640x400x16screen();
		break;
	case 0x94:
		draw_320x200x16screen(2);
		draw_320x200x16screen(3);
		break;
	case 0x95:
		draw_320x200x16screen(3);
		draw_320x200x16screen(2);
		break;
	case 0x97:
		draw_640x200x16screen(1);
		break;
	case 0x19:
	case 0x99:
		draw_320x200x256screen(400);
		break;
	}
	
	// fill scan line
	if(!scan_line && (cgreg[0x0e] & 0x04) != 0) {
		for(int y = 0; y < 400; y += 2) {
			memcpy(cg + (y + 1) * 640, cg + y * 640, 640);
		}
	}
}

void CRTC::draw_320x200x16screen(uint8_t pl)
{
	uint8_t B, R, G, I, col;
	uint32_t dest = 0;
	uint16_t ex;
	uint16_t subplane;
	
	if(map_init) {
		create_addr_map(40, 200);
	}
	ex = (cgreg[0x0e] & 0x80) << 7;
	subplane = (pl & 1) ? 0x2000 : 0x0000;
	for(int y = 0; y < 200; y++) {
		for(int x = 0; x < 40; x++) {
			uint16_t src = (map_addr[y][x] ^ subplane) | ex;
			uint32_t dest2 = dest + map_hdsc[y][x];
			dest += 16;
			
			if(pl == 0 || pl == 2) {
				B = (cgreg[0x18] & 0x01) ? vram_b[src] : 0;
				R = (cgreg[0x18] & 0x02) ? vram_r[src] : 0;
				G = (cgreg[0x18] & 0x04) ? vram_g[src] : 0;
				I = (cgreg[0x18] & 0x08) ? vram_i[src] : 0;
			} else {
				B = (cgreg[0x18] & 0x10) ? vram_b[src] : 0;
				R = (cgreg[0x18] & 0x20) ? vram_r[src] : 0;
				G = (cgreg[0x18] & 0x40) ? vram_g[src] : 0;
				I = (cgreg[0x18] & 0x80) ? vram_i[src] : 0;
			}
			
			col = cg_matrix0[B][R][0] | cg_matrix1[G][I][0]; if(col) cg[dest2     ] = cg[dest2 +  1] = col;
			col = cg_matrix0[B][R][1] | cg_matrix1[G][I][1]; if(col) cg[dest2 +  2] = cg[dest2 +  3] = col;
			col = cg_matrix0[B][R][2] | cg_matrix1[G][I][2]; if(col) cg[dest2 +  4] = cg[dest2 +  5] = col;
			col = cg_matrix0[B][R][3] | cg_matrix1[G][I][3]; if(col) cg[dest2 +  6] = cg[dest2 +  7] = col;
			col = cg_matrix0[B][R][4] | cg_matrix1[G][I][4]; if(col) cg[dest2 +  8] = cg[dest2 +  9] = col;
			col = cg_matrix0[B][R][5] | cg_matrix1[G][I][5]; if(col) cg[dest2 + 10] = cg[dest2 + 11] = col;
			col = cg_matrix0[B][R][6] | cg_matrix1[G][I][6]; if(col) cg[dest2 + 12] = cg[dest2 + 13] = col;
			col = cg_matrix0[B][R][7] | cg_matrix1[G][I][7]; if(col) cg[dest2 + 14] = cg[dest2 + 15] = col;
		}
		dest += 640;
	}
}

void CRTC::draw_320x200x256screen(int ymax)
{
	uint8_t B0, B1, R0, R1, G0, G1, I0, I1;
	uint32_t dest = 0, to_nextline;
	uint16_t ex;
	
	if(map_init) {
		create_addr_map(40, ymax);
	}
	if(cg_mask256_init) {
		cg_mask256 = cgreg[0x18];
		if(!(cg_mask & 1)) {
			cg_mask256 &= ~(0x11 | 0x88);
		}
		if(!(cg_mask & 2)) {
			cg_mask256 &= ~0x22;
		}
		if(!(cg_mask & 4)) {
			cg_mask256 &= ~0x44;
		}
		cg_mask256_init = false;
	}
	to_nextline = (ymax == 200) ? 640 : 0;
	ex = (cgreg[0x0e] & 0x80) << 7;
	for(int y = 0; y < ymax; y++) {
		for(int x = 0; x < 40; x++) {
			uint16_t src1 = (map_addr[y][x]         ) | ex;
			uint16_t src2 = (map_addr[y][x] ^ 0x2000) | ex;
			uint32_t dest2 = dest + map_hdsc[y][x];
			dest += 16;
			
			B1 = (cg_mask256 & 0x01) ? vram_b[src1] : 0;
			B0 = (cg_mask256 & 0x10) ? vram_b[src2] : 0;
			R1 = (cg_mask256 & 0x02) ? vram_r[src1] : 0;
			R0 = (cg_mask256 & 0x20) ? vram_r[src2] : 0;
			G1 = (cg_mask256 & 0x04) ? vram_g[src1] : 0;
			G0 = (cg_mask256 & 0x40) ? vram_g[src2] : 0;
			I1 = (cg_mask256 & 0x08) ? vram_i[src1] : 0;
			I0 = (cg_mask256 & 0x80) ? vram_i[src2] : 0;
			
			cg[dest2     ] = cg[dest2 +  1] = cg_matrix0[B0][R0][0] | cg_matrix1[G0][I0][0] | cg_matrix2[B1][R1][0] | cg_matrix3[G1][I1][0];
			cg[dest2 +  2] = cg[dest2 +  3] = cg_matrix0[B0][R0][1] | cg_matrix1[G0][I0][1] | cg_matrix2[B1][R1][1] | cg_matrix3[G1][I1][1];
			cg[dest2 +  4] = cg[dest2 +  5] = cg_matrix0[B0][R0][2] | cg_matrix1[G0][I0][2] | cg_matrix2[B1][R1][2] | cg_matrix3[G1][I1][2];
			cg[dest2 +  6] = cg[dest2 +  7] = cg_matrix0[B0][R0][3] | cg_matrix1[G0][I0][3] | cg_matrix2[B1][R1][3] | cg_matrix3[G1][I1][3];
			cg[dest2 +  8] = cg[dest2 +  9] = cg_matrix0[B0][R0][4] | cg_matrix1[G0][I0][4] | cg_matrix2[B1][R1][4] | cg_matrix3[G1][I1][4];
			cg[dest2 + 10] = cg[dest2 + 11] = cg_matrix0[B0][R0][5] | cg_matrix1[G0][I0][5] | cg_matrix2[B1][R1][5] | cg_matrix3[G1][I1][5];
			cg[dest2 + 12] = cg[dest2 + 13] = cg_matrix0[B0][R0][6] | cg_matrix1[G0][I0][6] | cg_matrix2[B1][R1][6] | cg_matrix3[G1][I1][6];
			cg[dest2 + 14] = cg[dest2 + 15] = cg_matrix0[B0][R0][7] | cg_matrix1[G0][I0][7] | cg_matrix2[B1][R1][7] | cg_matrix3[G1][I1][7];
		}
		dest += to_nextline;
	}
}

void CRTC::draw_640x200x16screen(uint8_t pl)
{
	uint8_t B, R, G, I;
	uint32_t dest = 0;
	uint16_t ex;
	uint16_t subplane;
	
	if(map_init) {
		create_addr_map(80, 200);
	}
	ex = (cgreg[0x0e] & 0x80) << 7;
	subplane = (pl & 1) ? 0x4000 : 0x0000; // thanks Mr.856
	for(int y = 0; y < 200; y++) {
		for(int x = 0; x < 80; x++) {
			uint16_t src = (map_addr[y][x] ^ subplane) | ex;
			uint32_t dest2 = dest + map_hdsc[y][x];
			dest += 8;
			
			B = (cgreg[0x18] & 0x01) ? vram_b[src] : 0;
			R = (cgreg[0x18] & 0x02) ? vram_r[src] : 0;
			G = (cgreg[0x18] & 0x04) ? vram_g[src] : 0;
			I = (cgreg[0x18] & 0x08) ? vram_i[src] : 0;
			
			cg[dest2    ] = cg_matrix0[B][R][0] | cg_matrix1[G][I][0];
			cg[dest2 + 1] = cg_matrix0[B][R][1] | cg_matrix1[G][I][1];
			cg[dest2 + 2] = cg_matrix0[B][R][2] | cg_matrix1[G][I][2];
			cg[dest2 + 3] = cg_matrix0[B][R][3] | cg_matrix1[G][I][3];
			cg[dest2 + 4] = cg_matrix0[B][R][4] | cg_matrix1[G][I][4];
			cg[dest2 + 5] = cg_matrix0[B][R][5] | cg_matrix1[G][I][5];
			cg[dest2 + 6] = cg_matrix0[B][R][6] | cg_matrix1[G][I][6];
			cg[dest2 + 7] = cg_matrix0[B][R][7] | cg_matrix1[G][I][7];
		}
		dest += 640;
	}
}

void CRTC::draw_640x400x4screen()
{
	uint8_t B, R;
	uint32_t dest = 0;
	// for Yukara K2
	uint8_t *vram_b1 = ((cgreg[0x18] & 3) == 1) ? vram_b + 0x4000 : vram_g;
	uint8_t *vram_r1 = ((cgreg[0x18] & 3) == 1) ? vram_r + 0x4000 : vram_i;
	
	if(map_init) {
		create_addr_map(80, 400);
	}
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 80; x++) {
			uint16_t src = map_addr[y][x];
			uint32_t dest2 = dest + map_hdsc[y][x];
			dest += 8;
			
			B = (cgreg[0x18] & 0x01) ? ((src & 0x4000) ? vram_b1[src & 0x3fff] : vram_b[src]) : 0;
			R = (cgreg[0x18] & 0x02) ? ((src & 0x4000) ? vram_r1[src & 0x3fff] : vram_r[src]) : 0;
			
			cg[dest2    ] = cg_matrix0[B][R][0];
			cg[dest2 + 1] = cg_matrix0[B][R][1];
			cg[dest2 + 2] = cg_matrix0[B][R][2];
			cg[dest2 + 3] = cg_matrix0[B][R][3];
			cg[dest2 + 4] = cg_matrix0[B][R][4];
			cg[dest2 + 5] = cg_matrix0[B][R][5];
			cg[dest2 + 6] = cg_matrix0[B][R][6];
			cg[dest2 + 7] = cg_matrix0[B][R][7];
		}
	}
}

void CRTC::draw_640x400x16screen()
{
	uint8_t B, R, G, I;
	uint32_t dest = 0;
	
	if(map_init) {
		create_addr_map(80, 400);
	}
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 80; x++) {
			uint16_t src = map_addr[y][x];
			uint32_t dest2 = dest + map_hdsc[y][x];
			dest += 8;
			
			B = vram_b[src];
			R = vram_r[src];
			G = vram_g[src];
			I = vram_i[src];
			
			cg[dest2    ] = cg_matrix0[B][R][0] | cg_matrix1[G][I][0];
			cg[dest2 + 1] = cg_matrix0[B][R][1] | cg_matrix1[G][I][1];
			cg[dest2 + 2] = cg_matrix0[B][R][2] | cg_matrix1[G][I][2];
			cg[dest2 + 3] = cg_matrix0[B][R][3] | cg_matrix1[G][I][3];
			cg[dest2 + 4] = cg_matrix0[B][R][4] | cg_matrix1[G][I][4];
			cg[dest2 + 5] = cg_matrix0[B][R][5] | cg_matrix1[G][I][5];
			cg[dest2 + 6] = cg_matrix0[B][R][6] | cg_matrix1[G][I][6];
			cg[dest2 + 7] = cg_matrix0[B][R][7] | cg_matrix1[G][I][7];
		}
	}
}

void CRTC::create_addr_map(int xmax, int ymax)
{
	uint8_t HDSC = cgreg[0x0f] & 0x07;
	uint16_t SAD0 = cgreg[0x10] | ((cgreg[0x11] & 0x7f) << 8);
	uint16_t SAD1 = cgreg[0x12] | ((cgreg[0x13] & 0x7f) << 8);
	uint16_t SAD2 = cgreg[0x14] | ((cgreg[0x15] & 0x7f) << 8);
	uint16_t SLN1 = cgreg[0x16] | ((cgreg[0x17] & 0x01) << 8);
	
	for(int y = 0; y < SLN1 && y < ymax; y++) {
		for(int x = 0; x < xmax; x++) {
			map_hdsc[y][x] = HDSC;
			map_addr[y][x] = SAD0;
			SAD0 = (SAD0 == SAD1) ? 0 : ((SAD0 + 1) & 0x7fff);
		}
	}
	for(int y = SLN1; y < ymax; y++) {
		for(int x = 0; x < xmax; x++) {
			map_hdsc[y][x] = 0;
			map_addr[y][x] = SAD2;
			SAD2 = (SAD2 == SAD1) ? 0 : ((SAD2 + 1) & 0x7fff);
		}
	}
	map_init = false;
}

// ----------------------------------------------------------------------------
// draw screen (MZ-2000/80B)
// ----------------------------------------------------------------------------

void CRTC::draw_screen_2000()
{
	scrntype_t palette_color[8];
	
	for(int i = 0; i < 8; i++) {
		palette_color[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	
	// render text
	uint8_t color = (text_color & 7) ? (text_color & 7) : 8;
	
	for(int y = 0, addr = 0; y < 200; y += 8) {
		for(int x = 0; x < (column_size ? 80 : 40); x++) {
			uint8_t code = tvram1[addr++];
			for(int l = 0; l < 8; l++) {
				uint8_t pat = font[(code << 3) + l];
				uint8_t* d = &screen_txt[y + l][x << 3];
				
				d[0] = (pat & 0x80) ? color : 0;
				d[1] = (pat & 0x40) ? color : 0;
				d[2] = (pat & 0x20) ? color : 0;
				d[3] = (pat & 0x10) ? color : 0;
				d[4] = (pat & 0x08) ? color : 0;
				d[5] = (pat & 0x04) ? color : 0;
				d[6] = (pat & 0x02) ? color : 0;
				d[7] = (pat & 0x01) ? color : 0;
			}
		}
	}
	
	// render graphics
//	if(config.monitor_type != MONITOR_TYPE_COLOR && (vram_mask & 8)) {
//		memset(screen_gra, 0, sizeof(screen_gra));
//	} else {
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0; x < 80; x++) {
				uint8_t b = (vram_mask & 1) ? vram_r[addr] : 0;
				uint8_t r = (vram_mask & 2) ? vram_g[addr] : 0;
				uint8_t g = (vram_mask & 4) ? vram_i[addr] : 0;
				addr++;
				uint8_t* d = &screen_gra[y][x << 3];
				
				d[0] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
				d[1] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
				d[2] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
				d[3] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
				d[4] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
				d[5] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
				d[6] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
				d[7] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			}
		}
//	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src_txt = screen_txt[y];
		uint8_t* src_gra = screen_gra[y];
		
		// VGATE (Forces display to be blank) or Reverse
		if(screen_mask || screen_reverse) {
			for(int x = 0; x < 640; x++) {
				dest0[x] = 0;
			}
		} else {
			if(text_color & 8) {
				// graphics > text
				for(int x = 0; x < 640; x++) {
					uint8_t txt = src_txt[column_size ? x : (x >> 1)], gra = src_gra[x];
					dest0[x] = palette_color[gra ? gra : txt ? (txt & 7) : back_color];
				}
			} else {
				// text > graphics
				for(int x = 0; x < 640; x++) {
					uint8_t txt = src_txt[column_size ? x : (x >> 1)], gra = src_gra[x];
					dest0[x] = palette_color[txt ? (txt & 7) : gra ? gra : back_color];
				}
			}
		}
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}

void CRTC::draw_screen_80b()
{
	scrntype_t palette_green[2];
	
	palette_green[screen_reverse ? 1 : 0] = RGB_COLOR(0, 0, 0);
	palette_green[screen_reverse ? 0 : 1] = RGB_COLOR(0, 255, 0);
	
	// render text
	uint8_t color = 1;
	
	for(int y = 0, addr = 0; y < 200; y += 8) {
		for(int x = 0; x < (column_size ? 80 : 40); x++) {
			uint8_t code = tvram1[addr++];
			for(int l = 0; l < 8; l++) {
				uint8_t pat = font[(code << 3) + l];
				uint8_t* d = &screen_txt[y + l][x << 3];
				
				d[0] = (pat & 0x80) ? color : 0;
				d[1] = (pat & 0x40) ? color : 0;
				d[2] = (pat & 0x20) ? color : 0;
				d[3] = (pat & 0x10) ? color : 0;
				d[4] = (pat & 0x08) ? color : 0;
				d[5] = (pat & 0x04) ? color : 0;
				d[6] = (pat & 0x02) ? color : 0;
				d[7] = (pat & 0x01) ? color : 0;
			}
		}
	}
	
	// render graphics
	if(!(vram_page & 6)) {
		memset(screen_gra, 0, sizeof(screen_gra));
	} else {
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0; x < 40; x++) {
				uint8_t pat;
				pat  = (vram_page & 2) ? vram_b[addr] : 0;
				pat |= (vram_page & 4) ? vram_r[addr] : 0;
				addr++;
				uint8_t* d = &screen_gra[y][x << 3];
				
				d[0] = (pat & 0x01) >> 0;
				d[1] = (pat & 0x02) >> 1;
				d[2] = (pat & 0x04) >> 2;
				d[3] = (pat & 0x08) >> 3;
				d[4] = (pat & 0x10) >> 4;
				d[5] = (pat & 0x20) >> 5;
				d[6] = (pat & 0x40) >> 6;
				d[7] = (pat & 0x80) >> 7;
			}
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
		scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
		uint8_t* src_txt = screen_txt[y];
		uint8_t* src_gra = screen_gra[y];
		
		for(int x = 0; x < 640; x++) {
			uint8_t txt = src_txt[column_size ? x : (x >> 1)], gra = src_gra[x >> 1];
			dest0[x] = palette_green[(txt || gra) ? 1 : 0];
		}
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
		} else {
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}

#define STATE_VERSION	2

bool CRTC::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(scan_line);
	state_fio->StateValue(scan_tmp);
	state_fio->StateValue(monitor_200line);
	state_fio->StateValue(monitor_digital);
	state_fio->StateValue(monitor_tmp);
	state_fio->StateValue(boot_mode);
	state_fio->StateValue(textreg_num);
	state_fio->StateArray(textreg, sizeof(textreg), 1);
	state_fio->StateValue(cgreg_num);
	state_fio->StateArray(cgreg, sizeof(cgreg), 1);
	state_fio->StateValue(scrn_size);
	state_fio->StateValue(cg_mask);
	state_fio->StateValue(cg_mask256);
	state_fio->StateValue(cg_mask256_init);
	state_fio->StateValue(font_size);
	state_fio->StateValue(column_size);
	state_fio->StateArray(latch, sizeof(latch), 1);
	state_fio->StateValue(GDEVS);
	state_fio->StateValue(GDEVE);
	state_fio->StateValue(GDEHS);
	state_fio->StateValue(GDEHE);
	state_fio->StateValue(hblank_t);
	state_fio->StateValue(vblank_t);
	state_fio->StateValue(hblank_g);
	state_fio->StateValue(vblank_g);
	state_fio->StateValue(blink);
	state_fio->StateValue(clear_flag);
	state_fio->StateArray(palette_reg, sizeof(palette_reg), 1);
	state_fio->StateValue(pal_select);
	state_fio->StateValue(screen_reverse);
	state_fio->StateValue(screen_mask);
	state_fio->StateArray(&priority16[0][0], sizeof(priority16), 1);
	state_fio->StateArray(palette16, sizeof(palette16), 1);
	state_fio->StateArray(palette4096, sizeof(palette4096), 1);
	state_fio->StateArray(palette4096r, sizeof(palette4096r), 1);
	state_fio->StateArray(palette4096g, sizeof(palette4096g), 1);
	state_fio->StateArray(palette4096b, sizeof(palette4096b), 1);
	state_fio->StateArray(palette16txt, sizeof(palette16txt), 1);
	state_fio->StateArray(palette4096txt, sizeof(palette4096txt), 1);
	state_fio->StateArray(&palette16pri[0][0], sizeof(palette16pri), 1);
	state_fio->StateArray(&palette4096pri[0][0], sizeof(palette4096pri), 1);
	state_fio->StateValue(prev16);
	state_fio->StateValue(update16);
	state_fio->StateArray(&priority256[0][0], sizeof(priority256), 1);
	state_fio->StateArray(palette256, sizeof(palette256), 1);
	state_fio->StateArray(palette256txt, sizeof(palette256txt), 1);
	state_fio->StateArray(&palette256pri[0][0], sizeof(palette256pri), 1);
	state_fio->StateValue((uint32_t)prev256);
	state_fio->StateValue(update256);
	state_fio->StateArray(&map_addr[0][0], sizeof(map_addr), 1);
	state_fio->StateArray(&map_hdsc[0][0], sizeof(map_hdsc), 1);
	state_fio->StateArray(&text_matrix[0][0][0], sizeof(text_matrix), 1);
	state_fio->StateArray(&text_matrixw[0][0][0], sizeof(text_matrixw), 1);
	state_fio->StateValue(trans_color);
	state_fio->StateValue(map_init);
	state_fio->StateValue(trans_init);
	state_fio->StateValue(vram_page);
	state_fio->StateValue(vram_mask);
	state_fio->StateValue(back_color);
	state_fio->StateValue(text_color);
	return true;
}

