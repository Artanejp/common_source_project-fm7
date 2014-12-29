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
#include "../../fileio.h"

void CRTC::initialize()
{
	// config
	monitor_200line = ((config.monitor_type & 2) != 0);
	scan_line = scan_tmp = (monitor_200line && config.scan_line);
	monitor_digital = monitor_tmp = ((config.monitor_type & 1) != 0);
	
	// set 16/4096 palette
	for(int i = 0; i < 16; i++) {
		uint8 r, g, b, r8, g8, b8;
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
		palette256[i] = RGB_COLOR(((i & 0x20) ? 128 : 0) | ((i & 2) ? 64 : 0) | ((i & 0x80) ? 32 : 0),
		                          ((i & 0x40) ? 128 : 0) | ((i & 4) ? 64 : 0) | ((i & 0x80) ? 32 : 0),
		                          ((i & 0x10) ? 128 : 0) | ((i & 1) ? 64 : 0) | ((i & 0x80) ? 32 : 0));
	}
	for(int i = 0; i < 8; i++) {
		palette256[i + 256] = RGB_COLOR(((i & 2) ? 255 : 0), ((i & 4) ? 255 : 0), ((i & 1) ? 255 : 0));
	}
	for(int i = 0; i < 64; i++) {
		palette256[i + 256 + 16] = RGB_COLOR(((i & 2) ? 64 : 0) | ((i & 0x10) ? 128 : 0), 
		                                     ((i & 4) ? 64 : 0) | ((i & 0x20) ? 128 : 0), 
		                                     ((i & 1) ? 64 : 0) | ((i & 0x08) ? 128 : 0));
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
	
	cgreg_num = 0x80;
	cgreg[0x00] = cgreg[0x01] = cgreg[0x02] = cgreg[0x03] = cgreg[0x06] = 0xff;
	GDEVS =   0; cgreg[0x08] = 0x00; cgreg[0x09] = 0x00;
	GDEVE = 400; cgreg[0x0a] = 0x90; cgreg[0x0b] = 0x01;
	GDEHS =   0; cgreg[0x0c] = 0x00;
	GDEHSC = (int)(CPU_CLOCKS * GDEHS / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
	GDEHE =  80; cgreg[0x0d] = 0x50;
	GDEHEC = (int)(CPU_CLOCKS * GDEHE / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
	
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
	screen_mask = false;
	blink = false;
	latch[0] = latch[1] = latch[2] = latch[3] = 0;
	hblank = vblank = false;
	map_init = trans_init = true;
	
	// register events
	register_vline_event(this);
	register_event(this, EVENT_BLINK, 500000, true, NULL);
}

void CRTC::write_data8(uint32 addr, uint32 data)
{
	// read modify write
	if(cgreg[0x0e] == 0x03) {
		// for Yukara K2
		uint8 *vram_b1 = ((cgreg[0x18] & 3) == 1) ? vram_b + 0x4000 : vram_g;
		uint8 *vram_r1 = ((cgreg[0x18] & 3) == 1) ? vram_r + 0x4000 : vram_i;
		
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

uint32 CRTC::read_data8(uint32 addr)
{
	// read modify write
	uint8 b, r, g, i, pl;
	
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
		uint8 compare = cgreg[7] & 0x0f;
		uint8 val = (compare == (((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((i & 0x80) >> 4))) ? 0x80 : 0;
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

void CRTC::write_io8(uint32 addr, uint32 data)
{
	uint8 haddr = (addr >> 8) & 0xff;
	uint8 num, r, g, b, prev;
	
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
				uint16 st, sz;
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
			GDEVS = (cgreg[0x08] | ((cgreg[0x09] & 1) << 8)); //* ((scrn_size == SCRN_640x400) ? 1 : 2);
			break;
		case 0x0a:
			cgreg[0x0b] = 0;
		case 0x0b:
			GDEVE = (cgreg[0x0a] | ((cgreg[0x0b] & 1) << 8)); //* ((scrn_size == SCRN_640x400) ? 1 : 2);
			break;
		case 0x0c:
			GDEHS = cgreg[0x0c] & 0x7f;
			GDEHSC = (int)(CPU_CLOCKS * GDEHS / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
			break;
		case 0x0d:
			GDEHE = cgreg[0x0d] & 0x7f;
			GDEHEC = (int)(CPU_CLOCKS * GDEHE / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
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
			}
			break;
		// scroll
		case 0x0f:
			map_init |= ((prev & 0x07) != (uint8)(data & 0x07));
			break;
		case 0x10: case 0x12: case 0x14: case 0x16:
			map_init |= (prev != (uint8)(data & 0xff));
			break;
		case 0x11: case 0x13: case 0x15:
			map_init |= ((prev & 0x7f) != (uint8)(data & 0x7f));
			break;
		case 0x17:
			map_init |= ((prev & 0x01) != (uint8)(data & 0x01));
			break;
		// screen mask
		case 0x18:
			cg_mask256_init |= (prev != (uint8)(data & 0xff));
			break;
		}
		// inc cgreg num
		if(cgreg_num & 0x80) {
			cgreg_num = (cgreg_num & 0xfc) | ((cgreg_num + 1) & 0x03);
		}
		break;
	case 0xf4:
		// textreg num
		textreg_num = data;
		break;
	case 0xf5:
		// text/palette reg
		if(textreg_num < 0x10) {
			if(textreg_num == 0) {
				trans_init |= ((textreg[0] & 2) != (uint8)(data & 2));
			} else if(textreg_num == 0x0a) {
				// update 256 colors palette
				if((textreg[0x0a] & 0x3f) != (uint8)(data & 0x3f)) {
					for(int i = 0; i < 256; i++) {
						uint8 b0 = (data & 0x03) >> 0;
						uint8 r0 = (data & 0x0c) >> 2;
						uint8 g0 = (data & 0x30) >> 4;
						uint16 b = ((i & 0x10) ? 128 : 0) | ((i & 1) ? 64 : 0) | ((b0 == 0 && (i & 0x80)) || (b0 == 1 && (i & 8)) || (b0 == 2) ? 32 : 0);
						uint16 r = ((i & 0x20) ? 128 : 0) | ((i & 2) ? 64 : 0) | ((r0 == 0 && (i & 0x80)) || (r0 == 1 && (i & 8)) || (r0 == 2) ? 32 : 0);
						uint16 g = ((i & 0x40) ? 128 : 0) | ((i & 4) ? 64 : 0) | ((g0 == 0 && (i & 0x80)) || (g0 == 1 && (i & 8)) || (g0 == 2) ? 32 : 0);
						palette256[i] = RGB_COLOR(r, g, b);
					}
					update256 = true;
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
			
			if((prev & 0x0f) != (uint8)(data & 0x0f)) {
				update16 = true;
			}
			if((prev & 0x10) != (uint8)(data & 0x10)) {
				// update priority
				for(int i = 1; i < 8; i++) {
					priority16[c][i] = p ? c : (i + 16);
				}
				priority16[c][0] = c; // transparent black
				priority16[c][8] = p ? c : (0 + 16); // non transparent black
				update16 = true;
			}
			if((prev & 0x1f) != (uint8)(data & 0x1f)) {
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
		// cg mask reg
		prev = cg_mask;
		cg_mask = (data & 7) | ((data & 1) ? 8 : 0);
		if(prev != cg_mask) {
			cg_mask256_init = true;
			update16 = true;
		}
		break;
	case 0xf7:
		// font size reg
		font_size = ((data & 1) != 0);
		break;
	}
}

uint32 CRTC::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xbc:
		// read plane b
		if(cgreg[7] & 0x10) {
			uint8 b = latch[0];
			uint8 r = latch[1];
			uint8 g = latch[2];
			uint8 i = latch[3];
			uint8 compare = cgreg[7] & 0x0f;
			
			uint8 val = (compare == (((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((i & 0x80) >> 4))) ? 0x80 : 0;
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
			return (vblank ? 0 : 0x80) | clear_flag;
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
		return (vblank ? 0 : 1) | (hblank ? 0 : 2);
	}
	return 0xff;
}

void CRTC::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CRTC_COLUMN_SIZE) {
		column_size = ((data & mask) != 0);	// from z80pio port a
	} else if(id == SIG_CRTC_PALLETE) {
		pal_select = ((data & mask) == 0);	// from ym2203 port a
	} else if(id == SIG_CRTC_MASK) {
		screen_mask = ((data & mask) != 0);	// from i8255 port c
	}
}

void CRTC::event_callback(int event_id, int err)
{
	if(event_id & 512) {
		blink = !blink;
	} else {
		set_hsync(event_id);
	}
}

void CRTC::event_vline(int v, int clock)
{
	bool next = !(GDEVS <= v && v < GDEVE);	// vblank = true
	if(vblank != next) {
		d_pio->write_signal(SIG_I8255_PORT_B, next ? 0 : 1, 1);
		d_int->write_signal(SIG_INTERRUPT_CRTC, next ? 1 : 0, 1);
		d_mem->write_signal(SIG_MEMORY_VBLANK, next ? 1 : 0, 1);
		vblank = next;
	}
	// complete clear screen
	if(v == 400) {
		clear_flag = 0;
	}
	// register hsync events
	if(!GDEHS) {
		set_hsync(0);
	} else if(GDEHS < CHARS_PER_LINE) {
		register_event_by_clock(this, GDEHS, GDEHSC, false, NULL);
	}
	if(!GDEHE) {
		set_hsync(0);
	} else if(GDEHE < CHARS_PER_LINE) {
		register_event_by_clock(this, GDEHE, GDEHEC, false, NULL);
	}
}

void CRTC::set_hsync(int h)
{
	bool next = !(GDEHS <= h && h < GDEHE);	// hblank = true
	if(hblank != next) {
		d_mem->write_signal(SIG_MEMORY_HBLANK, next ? 1 : 0, 1);
		hblank = next;
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
	// update config
	scan_line = scan_tmp;
	
	if(monitor_digital != monitor_tmp) {
		monitor_digital = monitor_tmp;
		// set 16 colors palette
		for(int i = 0; i < 16; i++) {
			uint8 r, g, b, r8, g8, b8;
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
	uint8 back16 = ((textreg[0x0b] & 4) >> 2) | ((textreg[0x0b] & 0x20) >> 4) | ((textreg[0x0c] & 1) << 2) | ((textreg[0x0b] & 1) << 3);
	if(back16 != prev16) {
		prev16 = back16;
		update16 = true;
	}
	if(update16) {
		scrntype palette16tmp[16 + 8], palette4096tmp[16 + 8];
		for(int i = 0; i < 16 + 8; i++) {
			palette16tmp[i] = palette16[(i & 16) ? i : (palette_reg[i] & 0x0f) ? (palette_reg[i] & cg_mask) : (back16 & cg_mask)];
			uint8 col = (i == 16) ? 0 : (i & 16) ? (i & 0x0f) + 8 : i;
			palette4096tmp[i] = palette4096[(palette_reg[col] & 0x0f) ? (palette_reg[col] & cg_mask) : (back16 & cg_mask)];
		}
		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < 9; j++) {
				palette16pri[i][j] = palette16tmp[priority16[i][j]];
				palette4096pri[i][j] = palette4096tmp[priority16[i][j]];
			}
		}
		memcpy(palette16txt, &palette16tmp[16], sizeof(scrntype) * 8);
//		palette16txt[0] = (back16 == 0 && (palette_reg[0] & 0x0f)== 2) ? 0 : palette16[palette_reg[back16] & 0x0f]; // tower of doruaga
		palette16txt[0] = palette16[palette_reg[back16] & 0x0f];
		palette16txt[8] = 0;
		memcpy(palette4096txt, &palette4096tmp[16], sizeof(scrntype) * 8);
		palette4096txt[0] = palette4096[palette_reg[back16] & 0x0f];
		palette4096txt[8] = 0;
		update16 = false;
	}
	
	// update 256 palette
	scrntype back256 = RGB_COLOR((textreg[0x0b] & 0x38) << 2, ((textreg[0x0b] & 0xc0) >> 1) | ((textreg[0x0c] & 1) << 7), (textreg[0x0b] & 7) << 5);
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
		memcpy(palette256txt, &palette256[256], sizeof(scrntype) * (16 + 64));
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
	int vs = (GDEVS <= GDEVE) ? GDEVS * (scrn_size == SCRN_640x400 ? 1 : 2) : 0;
	int ve = (GDEVS <= GDEVE) ? GDEVE * (scrn_size == SCRN_640x400 ? 1 : 2) : 400;
	int hs = (GDEHS <= GDEHE && GDEHS < 80) ? (GDEHS << 3) : 0;
	int he = (GDEHS <= GDEHE && GDEHE < 80) ? (GDEHE << 3) : 640;
	
	// mix screens
	if(screen_mask) {
		// screen is masked
		for(int y = 0; y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			memset(dest, 0, sizeof(scrntype) * 640);
		}
	} else if(cgreg[0x0e] == 0x1d || cgreg[0x0e] == 0x9d) {
		// 256 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette256txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
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
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette256txt[src_text[x]];
			}
		}
	} else if(!pal_select) {
		// 16 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
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
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
	} else {
		// 4096 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
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
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_cg = &cg[640 * y], *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
	}
	emu->screen_skip_line = monitor_200line;
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
	uint16 src = textreg[1] | ((textreg[2] & 0x07) << 8);
	uint8 line = (textreg[0] & 0x10) ? 2 : 0;
	uint8 height = (textreg[0] & 0x10) ? 20 : 16;
	uint8 vd = (textreg[9] & 0x0f) << 1;
	
	// 80x20(25)
	for(int y = line; y < 416; y += height) {
		int dest = (y - vd) * 640;
		for(int x = 0; x < 80; x++) {
			draw_80column_font((src++) & 0x7ff, dest, y - vd);
			dest += 8;
		}
	}
}

void CRTC::draw_40column_screen()
{
	uint16 src1 = textreg[1] | ((textreg[2] & 0x07) << 8);
	uint16 src2 = src1 + 0x400;
	uint8 line = (textreg[0] & 0x10) ? 2 : 0;
	uint8 height = (textreg[0] & 0x10) ? 20 : 16;
	uint8 vd = (textreg[9] & 0x0f) << 1;
	
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
			uint32 src1 = 640 * y;
			uint32 src2 = 640 * y + 640 * 480;
			uint32 dest = 640 * y;
			uint8 col;
			for(int x = 0; x < 640; x++) {
				if((text[src1] & 8) && (text[src2] & 8)) {
					col = 8; // non transparent black
				} else {
					col = (((text[src1] & 7) << 3) | (text[src2] & 7)) + 16;
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
			uint8* tsrc1 = &text[dest];
			uint8* tsrc2 = &text[dest + 640 * 480];
			uint8* tdest = &text[dest];
			for(int x = 0; x < 640; x++) {
				tdest[x] = (tsrc1[x] & 7) ? tsrc1[x] : (tsrc2[x] & 7) ? tsrc2[x] : ((tsrc1[x] & 8) | (tsrc2[x] & 8));
			}
		}
		break;
	}
}

void CRTC::draw_80column_font(uint16 src, int dest, int y)
{
	// draw char (80 column)
	uint8* pattern1;
	uint8* pattern2;
	uint8* pattern3;
	
	uint32 code;
	uint8 sel, col, pat1, pat2, pat3;
	uint8 t1 = tvram1[src], t2 = tvram2[src], attr = attrib[src];
	
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
					uint8 val = (attr & 0x40) ? 7 : trans_color;
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
						uint8* tdest = &text[dest];
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
							uint8* tdest = &text[dest];
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
						uint8* tdest = &text[dest];
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
			uint32 dest1 = dest;
			uint32 dest2 = (dest >= 640 * 399) ? dest - 640 * 399 : dest + 640;
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

void CRTC::draw_40column_font(uint16 src, int dest, int y)
{
	// draw char (40 column)
	uint8* pattern1;
	uint8* pattern2;
	uint8* pattern3;
	
	uint32 code;
	uint8 sel, col, pat1, pat2, pat3;
	uint8 t1 = tvram1[src], t2 = tvram2[src], attr = attrib[src];
	
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
					uint8 val = (attr & 0x40) ? 7 : trans_color;
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
						uint8* tdest = &text[dest];
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
							uint8* tdest = &text[dest];
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
						uint8* tdest = &text[dest];
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
		draw_320x200x256screen(0);
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
	case 0x9d:
		draw_320x200x256screen(1);
		break;
	}
	
	// fill scan line
	if(!scan_line && !(cgreg[0x0e] == 0x03 || cgreg[0x0e] == 0x93)) {
		for(int y = 0; y < 400; y += 2) {
			memcpy(cg + (y + 1) * 640, cg + y * 640, 640);
		}
	}
}

void CRTC::draw_320x200x16screen(uint8 pl)
{
	uint8 B, R, G, I, col;
	uint32 dest = 0;
	
	if(map_init) {
		create_addr_map(40, 200);
	}
	for(int y = 0; y < 200; y++) {
		for(int x = 0; x < 40; x++) {
			uint16 src = (map_addr[y][x] + (0x2000 * pl)) & 0x7fff;
			uint32 dest2 = dest + map_hdsc[y][x];
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

void CRTC::draw_320x200x256screen(uint8 pl)
{
	uint8 B0, B1, R0, R1, G0, G1, I0, I1;
	uint32 dest = 0;
	
	if(map_init) {
		create_addr_map(40, 200);
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
	for(int y = 0; y < 200; y++) {
		for(int x = 0; x < 40; x++) {
			uint16 src1 = (map_addr[y][x] + (0x4000 * pl)) & 0x7fff;
			uint16 src2 = (src1 + 0x2000) & 0x7fff;
			uint32 dest2 = dest + map_hdsc[y][x];
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
		dest += 640;
	}
}

void CRTC::draw_640x200x16screen(uint8 pl)
{
	uint8 B, R, G, I;
	uint32 dest = 0;
	
	if(map_init) {
		create_addr_map(80, 200);
	}
	for(int y = 0; y < 200; y++) {
		for(int x = 0; x < 80; x++) {
			uint16 src = (map_addr[y][x] + (0x4000 * pl)) & 0x7fff;
			uint32 dest2 = dest + map_hdsc[y][x];
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
	uint8 B, R;
	uint32 dest = 0;
	// for Yukara K2
	uint8 *vram_b1 = ((cgreg[0x18] & 3) == 1) ? vram_b + 0x4000 : vram_g;
	uint8 *vram_r1 = ((cgreg[0x18] & 3) == 1) ? vram_r + 0x4000 : vram_i;
	
	if(map_init) {
		create_addr_map(80, 400);
	}
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 80; x++) {
			uint16 src = map_addr[y][x];
			uint32 dest2 = dest + map_hdsc[y][x];
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
	uint8 B, R, G, I;
	uint32 dest = 0;
	
	if(map_init) {
		create_addr_map(80, 400);
	}
	for(int y = 0; y < 400; y++) {
		for(int x = 0; x < 80; x++) {
			uint16 src = map_addr[y][x];
			uint32 dest2 = dest + map_hdsc[y][x];
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
	uint8 HDSC = cgreg[0x0f] & 0x07;
	uint16 SAD0 = cgreg[0x10] | ((cgreg[0x11] & 0x7f) << 8);
	uint16 SAD1 = cgreg[0x12] | ((cgreg[0x13] & 0x7f) << 8);
	uint16 SAD2 = cgreg[0x14] | ((cgreg[0x15] & 0x7f) << 8);
	uint16 SLN1 = cgreg[0x16] | ((cgreg[0x17] & 0x01) << 8);
	
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
			map_addr[y][x] = (SAD2++) & 0x7fff;
		}
	}
	map_init = false;
}

#define STATE_VERSION	1

void CRTC::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(scan_line);
	state_fio->FputBool(scan_tmp);
	state_fio->FputBool(monitor_200line);
	state_fio->FputBool(monitor_digital);
	state_fio->FputBool(monitor_tmp);
	state_fio->FputUint8(textreg_num);
	state_fio->Fwrite(textreg, sizeof(textreg), 1);
	state_fio->FputUint8(cgreg_num);
	state_fio->Fwrite(cgreg, sizeof(cgreg), 1);
	state_fio->FputUint8(scrn_size);
	state_fio->FputUint8(cg_mask);
	state_fio->FputUint8(cg_mask256);
	state_fio->FputBool(cg_mask256_init);
	state_fio->FputBool(font_size);
	state_fio->FputBool(column_size);
	state_fio->Fwrite(latch, sizeof(latch), 1);
	state_fio->FputUint16(GDEVS);
	state_fio->FputUint16(GDEVE);
	state_fio->FputUint8(GDEHS);
	state_fio->FputUint8(GDEHE);
	state_fio->FputInt32(GDEHSC);
	state_fio->FputInt32(GDEHEC);
	state_fio->FputBool(hblank);
	state_fio->FputBool(vblank);
	state_fio->FputBool(blink);
	state_fio->FputUint8(clear_flag);
	state_fio->Fwrite(palette_reg, sizeof(palette_reg), 1);
	state_fio->FputBool(pal_select);
	state_fio->FputBool(screen_mask);
	state_fio->Fwrite(priority16, sizeof(priority16), 1);
	state_fio->Fwrite(palette16, sizeof(palette16), 1);
	state_fio->Fwrite(palette4096, sizeof(palette4096), 1);
	state_fio->Fwrite(palette4096r, sizeof(palette4096r), 1);
	state_fio->Fwrite(palette4096g, sizeof(palette4096g), 1);
	state_fio->Fwrite(palette4096b, sizeof(palette4096b), 1);
	state_fio->Fwrite(palette16txt, sizeof(palette16txt), 1);
	state_fio->Fwrite(palette4096txt, sizeof(palette4096txt), 1);
	state_fio->Fwrite(palette16pri, sizeof(palette16pri), 1);
	state_fio->Fwrite(palette4096pri, sizeof(palette4096pri), 1);
	state_fio->FputUint8(prev16);
	state_fio->FputBool(update16);
	state_fio->Fwrite(priority256, sizeof(priority256), 1);
	state_fio->Fwrite(palette256, sizeof(palette256), 1);
	state_fio->Fwrite(palette256txt, sizeof(palette256txt), 1);
	state_fio->Fwrite(palette256pri, sizeof(palette256pri), 1);
	state_fio->FputUint32((uint32)prev256);
	state_fio->FputBool(update256);
	state_fio->Fwrite(map_addr, sizeof(map_addr), 1);
	state_fio->Fwrite(map_hdsc, sizeof(map_hdsc), 1);
	state_fio->Fwrite(text_matrix, sizeof(text_matrix), 1);
	state_fio->Fwrite(text_matrixw, sizeof(text_matrixw), 1);
	state_fio->FputUint8(trans_color);
	state_fio->FputBool(map_init);
	state_fio->FputBool(trans_init);
}

bool CRTC::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	scan_line = state_fio->FgetBool();
	scan_tmp = state_fio->FgetBool();
	monitor_200line = state_fio->FgetBool();
	monitor_digital = state_fio->FgetBool();
	monitor_tmp = state_fio->FgetBool();
	textreg_num = state_fio->FgetUint8();
	state_fio->Fread(textreg, sizeof(textreg), 1);
	cgreg_num = state_fio->FgetUint8();
	state_fio->Fread(cgreg, sizeof(cgreg), 1);
	scrn_size = state_fio->FgetUint8();
	cg_mask = state_fio->FgetUint8();
	cg_mask256 = state_fio->FgetUint8();
	cg_mask256_init = state_fio->FgetBool();
	font_size = state_fio->FgetBool();
	column_size = state_fio->FgetBool();
	state_fio->Fread(latch, sizeof(latch), 1);
	GDEVS = state_fio->FgetUint16();
	GDEVE = state_fio->FgetUint16();
	GDEHS = state_fio->FgetUint8();
	GDEHE = state_fio->FgetUint8();
	GDEHSC = state_fio->FgetInt32();
	GDEHEC = state_fio->FgetInt32();
	hblank = state_fio->FgetBool();
	vblank = state_fio->FgetBool();
	blink = state_fio->FgetBool();
	clear_flag = state_fio->FgetUint8();
	state_fio->Fread(palette_reg, sizeof(palette_reg), 1);
	pal_select = state_fio->FgetBool();
	screen_mask = state_fio->FgetBool();
	state_fio->Fread(priority16, sizeof(priority16), 1);
	state_fio->Fread(palette16, sizeof(palette16), 1);
	state_fio->Fread(palette4096, sizeof(palette4096), 1);
	state_fio->Fread(palette4096r, sizeof(palette4096r), 1);
	state_fio->Fread(palette4096g, sizeof(palette4096g), 1);
	state_fio->Fread(palette4096b, sizeof(palette4096b), 1);
	state_fio->Fread(palette16txt, sizeof(palette16txt), 1);
	state_fio->Fread(palette4096txt, sizeof(palette4096txt), 1);
	state_fio->Fread(palette16pri, sizeof(palette16pri), 1);
	state_fio->Fread(palette4096pri, sizeof(palette4096pri), 1);
	prev16 = state_fio->FgetUint8();
	update16 = state_fio->FgetBool();
	state_fio->Fread(priority256, sizeof(priority256), 1);
	state_fio->Fread(palette256, sizeof(palette256), 1);
	state_fio->Fread(palette256txt, sizeof(palette256txt), 1);
	state_fio->Fread(palette256pri, sizeof(palette256pri), 1);
	prev256 = (scrntype)state_fio->FgetUint32();
	update256 = state_fio->FgetBool();
	state_fio->Fread(map_addr, sizeof(map_addr), 1);
	state_fio->Fread(map_hdsc, sizeof(map_hdsc), 1);
	state_fio->Fread(text_matrix, sizeof(text_matrix), 1);
	state_fio->Fread(text_matrixw, sizeof(text_matrixw), 1);
	trans_color = state_fio->FgetUint8();
	map_init = state_fio->FgetBool();
	trans_init = state_fio->FgetBool();
	return true;
}

