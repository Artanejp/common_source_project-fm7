/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ crtc ]
*/

#include "crtc.h"
#include "../i8255.h"
#include "../i8259.h"

#define EVENT_HSYNC	0
#define EVENT_BLINK	256

#define SCRN_640x400	1
#define SCRN_640x200	2

void CRTC::initialize()
{
	// set 16/4096 palette
	for(int i = 0; i < 16; i++) {
		uint8 r = ((i & 0x0f) == 8) ? 152 : ((i & 0x0a) == 0x0a) ? 255 : ((i & 0x0a) == 2) ? 127 : 0;
		uint8 g = ((i & 0x0f) == 8) ? 152 : ((i & 0x0c) == 0x0c) ? 255 : ((i & 0x0c) == 4) ? 127 : 0;
		uint8 b = ((i & 0x0f) == 8) ? 152 : ((i & 0x09) == 0x09) ? 255 : ((i & 0x09) == 1) ? 127 : 0;
		palette16[i] = RGB_COLOR(r, g, b);
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
	
	// set 65536 palette
	for(int i = 0; i < 65536; i++) {
		// BBBB RRRR GGGG IGRB
		uint8 b = ((i & 0xf000) >> 8) | ((i & 1) << 3) | ((i & 8) >> 1);
		uint8 r = ((i & 0x0f00) >> 4) | ((i & 2) << 2) | ((i & 8) >> 1);
		uint8 g = ((i & 0x00f0) >> 0) | ((i & 4) << 1) | ((i & 8) >> 1);
		palette65536[i] = RGB_COLOR(r, g, b);
	}
	
	// extract cg optimize matrix
	for(int p1 = 0; p1 < 256; p1++) {
		for(int p2 = 0; p2 < 256; p2++) {
			for(int i = 0; i < 8; i++) {
				cg_matrix0[p1][p2][i] = (p1 & (0x80 >> i) ? 0x01 : 0) | (p2 & (0x80 >> i) ? 0x02 : 0);
				cg_matrix1[p1][p2][i] = (p1 & (0x80 >> i) ? 0x04 : 0) | (p2 & (0x80 >> i) ? 0x08 : 0);
				cg_matrix2[p1][p2][i] = (p1 & (0x80 >> i) ? 0x10 : 0) | (p2 & (0x80 >> i) ? 0x20 : 0);
				cg_matrix3[p1][p2][i] = (p1 & (0x80 >> i) ? 0x40 : 0) | (p2 & (0x80 >> i) ? 0x80 : 0);
			}
		}
	}
	
	// initialize crtc
	memset(textreg, 0, sizeof(textreg));
	memset(rmwreg, 0, sizeof(rmwreg));
	memset(cgreg, 0, sizeof(cgreg));
	memset(latch, 0, sizeof(latch));
	
	rmwreg_num[0] = rmwreg_num[1] = cgreg_num = 0x80;
	GDEVS =   0; cgreg[0] = 0x00; cgreg[1] = 0x00;
	GDEVE = 400; cgreg[2] = 0x90; cgreg[3] = 0x01;
	GDEHS =   0; cgreg[4] = 0x00;
	GDEHSC = (int)(CPU_CLOCKS * GDEHS / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
	GDEHE =  80; cgreg[5] = 0x50;
	GDEHEC = (int)(CPU_CLOCKS * GDEHE / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
	
	for(int i = 0; i < 16; i++) {
		palette_reg[i] = i;
	}
	scrn_size = SCRN_640x400;
	font_size = true;
	column_size = true;
	cg_mask = 0x0f;
	clear_flag = 0;
	pal_select = false;
	screen_mask = false;
	blink = false;
	blank = hblank = vblank = false;
	map_init = trans_init = true;
	
	// register events
	register_vline_event(this);
	register_event(this, EVENT_BLINK, 500000, true, NULL);
}

void CRTC::write_data8(uint32 addr, uint32 data)
{
	// read modify write
	int lh = addr & 1;
	addr &= 0x1ffff;
	if((rmwreg[lh][5] & 0xc0) == 0x00) {
		// REPLACE
		if(rmwreg[lh][5] & 1) {
			vram_b[addr] &= ~rmwreg[lh][6];
			vram_b[addr] |= (rmwreg[lh][4] & 1) ? (data & rmwreg[lh][0] & rmwreg[lh][6]) : 0;
		}
		if(rmwreg[lh][5] & 2) {
			vram_r[addr] &= ~rmwreg[lh][6];
			vram_r[addr] |= (rmwreg[lh][4] & 2) ? (data & rmwreg[lh][1] & rmwreg[lh][6]) : 0;
		}
		if(rmwreg[lh][5] & 4) {
			vram_g[addr] &= ~rmwreg[lh][6];
			vram_g[addr] |= (rmwreg[lh][4] & 4) ? (data & rmwreg[lh][2] & rmwreg[lh][6]) : 0;
		}
		if(rmwreg[lh][5] & 8) {
			vram_i[addr] &= ~rmwreg[lh][6];
			vram_i[addr] |= (rmwreg[lh][4] & 8) ? (data & rmwreg[lh][3] & rmwreg[lh][6]) : 0;
		}
	} else if((rmwreg[lh][5] & 0xc0) == 0x40) {
		// PSET
		if(rmwreg[lh][5] & 1) {
			vram_b[addr] &= ~data;
			vram_b[addr] |= (rmwreg[lh][4] & 1) ? (data & rmwreg[lh][0]) : 0;
		}
		if(rmwreg[lh][5] & 2) {
			vram_r[addr] &= ~data;
			vram_r[addr] |= (rmwreg[lh][4] & 2) ? (data & rmwreg[lh][1]) : 0;
		}
		if(rmwreg[lh][5] & 4) {
			vram_g[addr] &= ~data;
			vram_g[addr] |= (rmwreg[lh][4] & 4) ? (data & rmwreg[lh][2]) : 0;
		}
		if(rmwreg[lh][5] & 8) {
			vram_i[addr] &= ~data;
			vram_i[addr] |= (rmwreg[lh][4] & 8) ? (data & rmwreg[lh][3]) : 0;
		}
	}
}

uint32 CRTC::read_data8(uint32 addr)
{
	// read modify write
	int lh = addr & 1;
	addr &= 0x1ffff;
	uint8 b = latch[lh][0] = vram_b[addr];
	uint8 r = latch[lh][1] = vram_r[addr];
	uint8 g = latch[lh][2] = vram_g[addr];
	uint8 i = latch[lh][3] = vram_i[addr];
	uint8 pl = rmwreg[lh][7] & 3;
	
	if(rmwreg[lh][7] & 0x10) {
		uint8 compare = rmwreg[lh][7] & 0x0f;
		uint8 val = (compare == (((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((i & 0x80) >> 4))) ? 0x80 : 0;
		val |= (compare == (((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4) | ((i & 0x40) >> 3))) ? 0x40 : 0;
		val |= (compare == (((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3) | ((i & 0x20) >> 2))) ? 0x20 : 0;
		val |= (compare == (((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2) | ((i & 0x10) >> 1))) ? 0x10 : 0;
		val |= (compare == (((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1) | ((i & 0x08)     ))) ? 0x08 : 0;
		val |= (compare == (((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04)     ) | ((i & 0x04) << 1))) ? 0x04 : 0;
		val |= (compare == (((b & 0x02) >> 1) | ((r & 0x02)     ) | ((g & 0x02) << 1) | ((i & 0x02) << 2))) ? 0x02 : 0;
		val |= (compare == (((b & 0x01)     ) | ((r & 0x01) << 1) | ((g & 0x01) << 2) | ((i & 0x01) << 3))) ? 0x01 : 0;
		return val;
	} else {
		return latch[lh][pl];
	}
}

void CRTC::write_io8(uint32 addr, uint32 data)
{
	uint8 num, r, g, b, prev;
	int lh = addr & 1;
	
	switch(addr & 0x7fff) {
	case 0x00ae: case 0x01ae: case 0x02ae: case 0x03ae: case 0x04ae: case 0x05ae: case 0x06ae: case 0x07ae:
	case 0x08ae: case 0x09ae: case 0x0aae: case 0x0bae: case 0x0cae: case 0x0dae: case 0x0eae: case 0x0fae:
	case 0x10ae: case 0x11ae: case 0x12ae: case 0x13ae: case 0x14ae: case 0x15ae: case 0x16ae: case 0x17ae:
	case 0x18ae: case 0x19ae: case 0x1aae: case 0x1bae: case 0x1cae: case 0x1dae: case 0x1eae: case 0x1fae:
		// 4096 palette reg
		num = (addr & 0x1f00) >> 9;
		r = palette4096r[num];
		g = palette4096g[num];
		b = palette4096b[num];
		if(addr & 0x100) {
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
	case 0x170:
		// textreg num
		textreg_num = data;
		break;
	case 0x172:
		// text/palette reg
		if(textreg_num < 0x10) {
			if(textreg_num == 0) {
				trans_init |= ((textreg[0] & 2) != (uint8)(data & 2));
			}
			textreg[textreg_num] = data;
		} else if(0x80 <= textreg_num && textreg_num < 0x90) {
			int c = textreg_num & 0x0f;
			
			prev = palette_reg[c];
			palette_reg[c] = data;
			
			if((prev & 0x0f) != (uint8)(data & 0x0f)) {
				update16 = true;
			}
			if((prev & 0x10) != (uint8)(data & 0x10)) {
				int c16 = c << 4;
				int col = data & 0x0f;
				int col16 = col << 4;
				int p = data & 0x10;
				
				// update priority
				for(int i = 1; i < 8; i++) {
					priority16[c][i] = p ? c : (i + 16);
				}
				priority16[c][0] = c; // transparent black
				priority16[c][8] = p ? c : (0 + 16); // non transparent black
				update16 = true;
			}
		}
		break;
	case 0x174:
		// cg mask reg
		prev = cg_mask;
		cg_mask = (data & 7) | ((data & 1) ? 8 : 0);
		update16 |= (prev != cg_mask);
		break;
	case 0x176:
		// font size reg
		font_size = ((data & 1) != 0);
		break;
	case 0x178:
	case 0x179:
		// rmwreg num
		rmwreg_num[lh] = data;
		break;
	case 0x17a:
	case 0x17b:
		// rmwreg
		rmwreg[lh][rmwreg_num[lh] & 0x1f] = data;
		// clear screen
		if((rmwreg_num[lh] & 0x1f) == 5 && (data & 0xc0) == 0x80) {
			uint16 st, sz;
			switch(rmwreg[lh][0x0e]) {
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
			if(rmwreg[lh][5] & 1) {
				memset(vram_b + st, 0, sz);
			}
			if(rmwreg[lh][5] & 2) {
				memset(vram_r + st, 0, sz);
			}
			if(rmwreg[lh][5] & 4) {
				memset(vram_g + st, 0, sz);
			}
			if(rmwreg[lh][5] & 8) {
				memset(vram_i + st, 0, sz);
			}
			clear_flag = 1;
		}
		// inc rmwreg num
		if(rmwreg_num[lh] & 0x80) {
			rmwreg_num[lh] = (rmwreg_num[lh] & 0xfc) | ((rmwreg_num[lh] + 1) & 3);
		}
		break;
	case 0x270:
		// cgreg num
		cgreg_num = data;
		break;
	case 0x272:
		// cgreg
		prev = cgreg[cgreg_num & 0x1f];;
		cgreg[cgreg_num & 0x1f] = data;
		
		switch(cgreg_num & 0x1f) {
		// view range
		case 0x00:
			cgreg[1] = 0;
		case 0x01:
			GDEVS = (cgreg[0] | ((cgreg[1] & 1) << 8)); //* ((scrn_size == SCRN_640x400) ? 1 : 2);
			break;
		case 0x02:
			cgreg[3] = 0;
		case 0x03:
			GDEVE = (cgreg[2] | ((cgreg[3] & 1) << 8)); //* ((scrn_size == SCRN_640x400) ? 1 : 2);
			break;
		case 0x04:
			GDEHS = cgreg[4] & 0x7f;
			GDEHSC = (int)(CPU_CLOCKS * GDEHS / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
			break;
		case 0x05:
			GDEHE = cgreg[5] & 0x7f;
			GDEHEC = (int)(CPU_CLOCKS * GDEHE / FRAMES_PER_SEC / LINES_PER_FRAME / CHARS_PER_LINE + 0.5);
			break;
		// screen size
		case 0x06:
			switch(data) {
			case 0x17: case 0x1f:
				map_init |= (scrn_size != SCRN_640x200);
				scrn_size = SCRN_640x200;
				break;
			case 0x13: case 0x1b:
				map_init |= (scrn_size != SCRN_640x400);
				scrn_size = SCRN_640x400;
				break;
			}
			break;
		// scroll
		case 0x07:
			map_init |= ((prev & 0x07) != (uint8)(data & 0x07));
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c:
			map_init |= (prev != (uint8)(data & 0xff));
			break;
		case 0x0d:
			map_init |= ((prev & 0x01) != (uint8)(data & 0x01));
			break;
		}
		// inc cgreg num
		if(cgreg_num & 0x80) {
			cgreg_num = (cgreg_num & 0xfc) | ((cgreg_num + 1) & 3);
		}
		break;
	}
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
	// vblank
	bool next = !(GDEVS <= v && v < GDEVE);
	if(vblank != next) {
		// vblank on / off
//		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR1, next ? 2 : 0, 2);
//		d_pio->write_signal(SIG_I8255_PORT_B, next ? 2 : 0, 2);
		vblank = next;
	}
#if 1
	// force vblank
	if(v == 0) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR1, 0, 2);
		d_pio->write_signal(SIG_I8255_PORT_B, 0, 2);
	} else if(v == 400) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR1, 2, 2);
		d_pio->write_signal(SIG_I8255_PORT_B, 2, 2);
	}
#endif
	
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
	hblank = !(GDEHS <= h && h < GDEHE);
	bool next = (hblank || vblank);
	if(blank != next) {
		d_pio->write_signal(SIG_I8255_PORT_B, next ? 1 : 0, 1);
		blank = next;
	}
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void CRTC::draw_screen()
{
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
		palette16txt[0] = palette16[palette_reg[back16] & 0x0f];
		palette16txt[8] = 0;
		memcpy(palette4096txt, &palette4096tmp[16], sizeof(scrntype) * 8);
		palette4096txt[0] = palette4096[palette_reg[back16] & 0x0f];
		palette4096txt[8] = 0;
		update16 = false;
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
	} else if(cgreg[6] == 0x1b || cgreg[6] == 0x1f) {
		// 65536 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint16 *src_cg = &cg[640 * y];
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < hs && x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
			for(int x = hs; x < he && x < 640; x++) {
				dest[x] = src_text[x] ? palette16txt[src_text[x]] : palette65536[src_cg[x]];
			}
			for(int x = he; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
		for(int y = ve; y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
	} else if(!pal_select) {
		// 16 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint16 *src_cg = &cg[640 * y];
			uint8 *src_text = &text[640 * y];
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
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette16txt[src_text[x]];
			}
		}
	} else {
		// 4096 colors
		for(int y = 0; y < vs && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
		for(int y = vs; y < ve && y < 400; y++) {
			scrntype *dest = emu->screen_buffer(y);
			uint16 *src_cg = &cg[640 * y];
			uint8 *src_text = &text[640 * y];
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
			uint8 *src_text = &text[640 * y];
			for(int x = 0; x < 640; x++) {
				dest[x] = palette4096txt[src_text[x]];
			}
		}
	}
	emu->screen_skip_line = false;
}

// ----------------------------------------------------------------------------
// draw text screen
// ----------------------------------------------------------------------------

void CRTC::draw_text()
{
	// extract text optimize matrix
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
	int SL = (textreg[3] - 17) << 1, SC;
	int EL = (textreg[5] - 17) << 1, EC;
	if(column_size) {
		// 80 colums
		SC = (textreg[7] & 0x7f) - 9;
		EC = (textreg[8] & 0x7f) - 9;
	} else {
		// 40 column
		SC = (textreg[7] & 0x7f) - 8;
		EC = (textreg[8] & 0x7f) - 8;
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
	uint16 src = (textreg[1] << 2) | ((textreg[2] & 7) << 10);
	uint8 line = (textreg[0] & 0x10) ? 2 : 0;
	uint8 height = (textreg[0] & 0x10) ? 20 : 16;
	uint8 vd = (textreg[9] & 0x0f) << 1;
	
	// 80x20(25)
	for(int y = line; y < 416; y += height) {
		int dest = (y - vd) * 640;
		for(int x = 0; x < 80; x++) {
			draw_80column_font(src & 0x1fff, dest, y - vd);
			src += 4;
			dest += 8;
		}
	}
}

void CRTC::draw_40column_screen()
{
	uint16 src1 = (textreg[1] << 2) | ((textreg[2] & 7) << 10);
	uint16 src2 = src1 + 0x1000;
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
				draw_40column_font(src1 & 0x1fff, dest1, y - vd);
				draw_40column_font(src2 & 0x1fff, dest2, y - vd);
				src1 += 4;
				src2 += 4;
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
				//if((text[src1] & 8) | (text[src2] & 8))
				if((text[src1] & 8) && (text[src2] & 8)) {
					// non transparent black
					col = 8;
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
				draw_40column_font(src1 & 0x1fff, dest, y - vd);
				src1 += 4;
				dest += 16;
			}
		}
		break;
	case 0x08:
		// 40x20(25), No.2 Screen
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640;
			for(int x = 0; x < 40; x++) {
				draw_40column_font(src2 & 0x1fff, dest, y - vd);
				src2 += 4;
				dest += 16;
			}
		}
		break;
	case 0x0c:
		// 40x20(25), No.1 + No.2 Screens (No.1 > No.2)
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640;
			for(int x = 0; x < 40; x++) {
				draw_40column_font(src1 & 0x1fff, dest, y - vd);
				src1 += 4;
				dest += 16;
			}
		}
		for(int y = line; y < 416; y += height) {
			int dest = (y - vd) * 640 + 640 * 480;
			for(int x = 0; x < 40; x++) {
				draw_40column_font(src2 & 0x1fff, dest, y - vd);
				src2 += 4;
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
	uint8 t1 = tvram[src], t2 = tvram[src + 1], attr = tvram[src + 2];
	
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
					if(dest >= 0) {
						memset(text + dest, val, 8);
					}
					dest += 640;
				} else {
					if(attr & 0x40) {
						pat1 = ~pattern1[(code + i) << 1];
						pat2 = ~pattern2[(code + i) << 1];
						pat3 = ~pattern3[(code + i) << 1];
					} else {
						pat1 = pattern1[(code + i) << 1];
						pat2 = pattern2[(code + i) << 1];
						pat3 = pattern3[(code + i) << 1];
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
					if(dest >= 0) {
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
							pat1 = ~pattern1[(code + i) << 1];
							pat2 = ~pattern2[(code + i) << 1];
							pat3 = ~pattern3[(code + i) << 1];
						} else {
							pat1 = pattern1[(code + i) << 1];
							pat2 = pattern2[(code + i) << 1];
							pat3 = pattern3[(code + i) << 1];
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
		col = attr & 7;
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
					pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[(code + i) << 1];
				} else {
					pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[(code + i) << 1];
				}
				if(dest >= 0) {
					memcpy(&text[dest], text_matrix[pat1][col], 8);
				}
				dest += 640;
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0) {
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
						pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[(code + i) << 1];
					} else {
						pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[(code + i) << 1];
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
	uint8 t1 = tvram[src], t2 = tvram[src + 1], attr = tvram[src + 2];
	
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
						memset(text + dest, val, 16);
					}
					dest += 640;
					// check end line of screen
					if(!(y++ < 480)) {
						break;
					}
					if(dest >= 0) {
						memset(text + dest, val, 16);
					}
					dest += 640;
				} else {
					if(attr & 0x40) {
						pat1 = ~pattern1[(code + i) << 1];
						pat2 = ~pattern2[(code + i) << 1];
						pat3 = ~pattern3[(code + i) << 1];
					} else {
						pat1 = pattern1[(code + i) << 1];
						pat2 = pattern2[(code + i) << 1];
						pat3 = pattern3[(code + i) << 1];
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
					if(dest >= 0) {
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
							pat1 = ~pattern1[(code + i) << 1];
							pat2 = ~pattern2[(code + i) << 1];
							pat3 = ~pattern3[(code + i) << 1];
						} else {
							pat1 = pattern1[(code + i) << 1];
							pat2 = pattern2[(code + i) << 1];
							pat3 = pattern3[(code + i) << 1];
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
		col = attr & 7;
		// draw
		if(font_size) {
			for(int i = 0; i < 8; i++) {
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				// reverse, blink
				if(attr & 0x40) {
					pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[(code + i) << 1];
				} else {
					pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[(code + i) << 1];
				}
				if(dest >= 0) {
					memcpy(&text[dest], text_matrixw[pat1][col], 16);
				}
				dest += 640;
				// check end line of screen
				if(!(y++ < 480)) {
					break;
				}
				if(dest >= 0) {
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
						pat1 = ((attr & 0x80) && blink) ? 0xff : ~pattern1[(code + i) << 1];
					} else {
						pat1 = ((attr & 0x80) && blink) ? 0x00 : pattern1[(code + i) << 1];
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
	int is_400l, is_16col, ymax, ofs;
	uint32 dest = 0;
	
	switch(cgreg[6]) {
	case 0x13: is_400l = 1; is_16col = 1; ymax = 400; ofs = 1; break; // 640x400x16
	case 0x17: is_400l = 0; is_16col = 1; ymax = 200; ofs = 1; break; // 640x200x16
	case 0x1b: is_400l = 1; is_16col = 0; ymax = 400; ofs = 4; break; // 640x400x65536
	case 0x1f: is_400l = 0; is_16col = 0; ymax = 200; ofs = 4; break; // 640x200x65536
	default:
		return;
	}
	
	if(map_init) {
		// update vram addr map for the hardware scroll
		uint8 HDSC = cgreg[0x07] & 7;
		uint32 SAD0 = (cgreg[0x08] << 1) | (cgreg[0x09] << 9);
		uint32 SAD1 = (cgreg[0x0a] << 1) | (cgreg[0x0b] << 9);
		uint16 SLN1 = cgreg[0x0c] | ((cgreg[0x0d] & 1) << 8);
		
		for(int y = 0; y < SLN1 && y < ymax; y++) {
			for(int x = 0; x < 80; x++) {
				map_hdsc[y][x] = HDSC;
				map_addr[y][x] = SAD0 & 0x1ffff;
				SAD0 += ofs;
			}
		}
		for(int y = SLN1; y < ymax; y++) {
			for(int x = 0; x < 80; x++) {
				map_hdsc[y][x] = 0;
				map_addr[y][x] = SAD1 & 0x1ffff;
				SAD1 += ofs;
			}
		}
		map_init = false;
	}
	if(is_16col) {
		// 16/4096 color mode
		for(int y = 0; y < ymax; y++) {
			for(int x = 0; x < 80; x++) {
				uint32 src = map_addr[y][x];
				uint32 dest2 = dest + map_hdsc[y][x];
				dest += 8;
				
				uint8 B = (cgreg[0x10] & 1) ? vram_b[src] : 0;
				uint8 R = (cgreg[0x10] & 2) ? vram_r[src] : 0;
				uint8 G = (cgreg[0x10] & 4) ? vram_g[src] : 0;
				uint8 I = (cgreg[0x10] & 8) ? vram_i[src] : 0;
				
				cg[dest2    ] = cg_matrix0[B][R][0] | cg_matrix1[G][I][0];
				cg[dest2 + 1] = cg_matrix0[B][R][1] | cg_matrix1[G][I][1];
				cg[dest2 + 2] = cg_matrix0[B][R][2] | cg_matrix1[G][I][2];
				cg[dest2 + 3] = cg_matrix0[B][R][3] | cg_matrix1[G][I][3];
				cg[dest2 + 4] = cg_matrix0[B][R][4] | cg_matrix1[G][I][4];
				cg[dest2 + 5] = cg_matrix0[B][R][5] | cg_matrix1[G][I][5];
				cg[dest2 + 6] = cg_matrix0[B][R][6] | cg_matrix1[G][I][6];
				cg[dest2 + 7] = cg_matrix0[B][R][7] | cg_matrix1[G][I][7];
			}
			if(!is_400l) {
				dest += 640;
			}
		}
	} else {
		// 65536 color mode
		uint8 B0 = 0, R0 = 0, G0 = 0, I0 = 0;
		uint8 B1 = 0, R1 = 0, G1 = 0, I1 = 0;
		uint8 B2 = 0, R2 = 0, G2 = 0, I2 = 0;
		uint8 B3 = 0, R3 = 0, G3 = 0, I3 = 0;
		uint8 plane_mask = cgreg[0x10] & cg_mask;
		
		for(int y = 0; y < ymax; y++) {
			for(int x = 0; x < 80; x++) {
				uint32 src0 = map_addr[y][x];
				uint32 src1 = (src0 + 1) & 0x1ffff;
				uint32 src2 = (src0 + 2) & 0x1ffff;
				uint32 src3 = (src0 + 3) & 0x1ffff;
				uint32 dest2 = dest + map_hdsc[y][x];
				dest += 8;
				
				if(plane_mask & 1) {
					B0 = vram_b[src0];
					B1 = vram_b[src1];
					B2 = vram_b[src2];
					B3 = vram_b[src3];
				}
				if(plane_mask & 2) {
					R0 = vram_r[src0];
					R1 = vram_r[src1];
					R2 = vram_r[src2];
					R3 = vram_r[src3];
				}
				if(plane_mask & 4) {
					G0 = vram_g[src0];
					G1 = vram_g[src1];
					G2 = vram_g[src2];
					G3 = vram_g[src3];
				}
				if(plane_mask & 8) {
					I0 = vram_i[src0];
					I1 = vram_i[src1];
					I2 = vram_i[src2];
					I3 = vram_i[src3];
				}
				cg[dest2    ] = ((B0 & 0xf0) <<  8) | ((R0 & 0xf0) << 4) | ((G0 & 0xf0)     ) | ((I0 & 0xf0) >> 4);
				cg[dest2 + 1] = ((B0 & 0x0f) << 12) | ((R0 & 0x0f) << 8) | ((G0 & 0x0f) << 4) | ((I0 & 0x0f)     );
				cg[dest2 + 2] = ((B1 & 0xf0) <<  8) | ((R1 & 0xf0) << 4) | ((G1 & 0xf0)     ) | ((I1 & 0xf0) >> 4);
				cg[dest2 + 3] = ((B1 & 0x0f) << 12) | ((R1 & 0x0f) << 8) | ((G1 & 0x0f) << 4) | ((I1 & 0x0f)     );
				cg[dest2 + 4] = ((B2 & 0xf0) <<  8) | ((R2 & 0xf0) << 4) | ((G2 & 0xf0)     ) | ((I2 & 0xf0) >> 4);
				cg[dest2 + 5] = ((B2 & 0x0f) << 12) | ((R2 & 0x0f) << 8) | ((G2 & 0x0f) << 4) | ((I2 & 0x0f)     );
				cg[dest2 + 6] = ((B3 & 0xf0) <<  8) | ((R3 & 0xf0) << 4) | ((G3 & 0xf0)     ) | ((I3 & 0xf0) >> 4);
				cg[dest2 + 7] = ((B3 & 0x0f) << 12) | ((R3 & 0x0f) << 8) | ((G3 & 0x0f) << 4) | ((I3 & 0x0f)     );
			}
			if(!is_400l) {
				dest += 640;
			}
		}
	}
	// fill scan line
	if(!is_400l) {
		for(int y = 0; y < 400; y += 2) {
			memcpy(cg + (y + 1) * 640, cg + y * 640, 1280);
		}
	}
}

