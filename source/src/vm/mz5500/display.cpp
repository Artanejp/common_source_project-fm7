/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ display ]
*/

#include "display.h"

static const int plane_priority[8][8] = {
	{0, 1, 2, 3, 0, 1, 2, 3}, {0, 1, 2, 3, 4, 1, 2, 3},
	{0, 1, 2, 3, 4, 4, 2, 3}, {0, 1, 2, 3, 4, 4, 4, 3},
	{0, 1, 2, 3, 4, 4, 4, 4}, {0, 1, 2, 3, 4, 4, 4, 4},
	{0, 1, 2, 3, 4, 4, 4, 4}, {0, 1, 2, 3, 4, 4, 4, 4}
};

void DISPLAY::initialize()
{
	// init pallete
	for(int i = 0; i < 8; i++) {
		palette_pc_base[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
		palette[i] = i;
	}
	mode_r = mode_c = mode_p = 0;
	update_palette();
	
	// init window controller
	rno = 0;
	memset(wregs, 0, sizeof(wregs));
	memset(pri, 0, sizeof(pri));
	memset(vma, 0, sizeof(vma));
	memset(vds, 0, sizeof(vds));
	memset(back, 0, sizeof(back));
	memset(reverse, 0, sizeof(reverse));
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x3ff) {
	case 0x110:
	case 0x114:
	case 0x118:
	case 0x11c:
		rno = data & 0x0f;
		break;
	case 0x112:
	case 0x116:
	case 0x11a:
	case 0x11e:
		wregs[rno] = data;
		if(rno == 0) {
			// update priority
			if(data) {
				int p1 = (data >> 6) & 3;
				int p2 = (data >> 4) & 3;
				int p3 = (data >> 2) & 3;
				int p4 = data & 3;
				pri[ 0] = 0;
				pri[ 1] = 1;
				pri[ 2] = 2;
				pri[ 3] = (p1 >= p2) ? 1 : 2;
				pri[ 4] = 3;
				pri[ 5] = (p1 >= p3) ? 1 : 3;
				pri[ 6] = (p2 >= p3) ? 2 : 3;
				pri[ 7] = (p1 >= p2 && p1 >= p3) ? 1 : (p2 >= p1 && p2 >= p3) ? 2 : 3;
				pri[ 8] = 4;
				pri[ 9] = (p1 >= p4) ? 1 : 4;
				pri[10] = (p2 >= p4) ? 2 : 4;
				pri[11] = (p1 >= p2 && p1 >= p4) ? 1 : (p2 >= p1 && p2 >= p4) ? 2 : 4;
				pri[12] = (p3 >= p4) ? 3 : 4;
				pri[13] = (p1 >= p3 && p1 >= p4) ? 1 : (p3 >= p1 && p3 >= p4) ? 3 : 4;
				pri[14] = (p2 >= p3 && p2 >= p4) ? 2 : (p3 >= p2 && p3 >= p4) ? 3 : 4;
				pri[15] = (p1 >= p2 && p1 >= p3 && p1 >= p4) ? 1 : (p2 >= p1 && p2 >= p3 && p2 >= p4) ? 2 : (p3 >= p1 && p3 >= p2 && p3 >= p4) ? 3 : 4;
				vds[0] = 0;
			} else {
				memset(pri, 0, sizeof(pri));
				vds[0] = 7;
			}
		} else if(rno == 1 || rno == 2) {
			vma[1] = wregs[1] | (wregs[2] << 8);
		} else if(rno == 3) {
			vds[1] = wregs[3] & 7;
		} else if(rno == 4 || rno == 5) {
			vma[2] = wregs[4] | (wregs[5] << 8);
		} else if(rno == 6) {
			vds[2] = wregs[6] & 7;
		} else if(rno == 7 || rno == 8) {
			vma[3] = wregs[7] | (wregs[8] << 8);
		} else if(rno == 9) {
			vds[3] = wregs[9] & 7;
		} else if(rno == 10 || rno == 11) {
			vma[4] = wregs[10] | (wregs[11] << 8);
		} else if(rno == 12) {
			vds[4] = wregs[12] & 7;
		}
		rno = (++rno) & 0x0f;
		break;
	case 0x120:
		mode_c = data & 7;
		update_palette();
		break;
	case 0x122:
		mode_p = data & 7;
		update_palette();
		break;
	case 0x124:
		back[1] = data & 7;
		break;
	case 0x126:
		back[2] = data & 7;
		break;
	case 0x128:
		back[3] = data & 7;
		break;
	case 0x12a:
		back[4] = data & 7;
		reverse[4] = (data & 1) ? 0xff : 0;
		break;
	case 0x12c:
		reverse[1] = (data & 1) ? 0xff : 0;
		reverse[2] = (data & 2) ? 0xff : 0;
		reverse[3] = (data & 4) ? 0xff : 0;
		break;
	case 0x130:
	case 0x132:
	case 0x134:
	case 0x136:
	case 0x138:
	case 0x13a:
	case 0x13c:
	case 0x13e:
		mode_r = data;
		break;
	case 0x141:
	case 0x149:
		palette[0] = data & 7;
		update_palette();
		break;
	case 0x143:
	case 0x14b:
		palette[1] = data & 7;
		update_palette();
		break;
	case 0x145:
	case 0x14d:
		palette[2] = data & 7;
		update_palette();
		break;
	case 0x147:
	case 0x14f:
		palette[3] = data & 7;
		update_palette();
		break;
	case 0x151:
	case 0x159:
		palette[4] = data & 7;
		update_palette();
		break;
	case 0x153:
	case 0x15b:
		palette[5] = data & 7;
		update_palette();
		break;
	case 0x155:
	case 0x15d:
		palette[6] = data & 7;
		update_palette();
		break;
	case 0x157:
	case 0x15f:
		palette[7] = data & 7;
		update_palette();
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	switch(addr & 0x3ff) {
	case 0x110:
	case 0x114:
	case 0x118:
	case 0x11c:
		return rno;
	case 0x112:
	case 0x116:
	case 0x11a:
	case 0x11e:
		val = wregs[rno];
		rno = (++rno) & 0x0f;
		return val;
	case 0x141:
	case 0x149:
		return palette[0];
	case 0x143:
	case 0x14b:
		return palette[1];
	case 0x145:
	case 0x14d:
		return palette[2];
	case 0x147:
	case 0x14f:
		return palette[3];
	case 0x151:
	case 0x159:
		return palette[4];
	case 0x153:
	case 0x15b:
		return palette[5];
	case 0x155:
	case 0x15d:
		return palette[6];
	case 0x157:
	case 0x15f:
		return palette[7];
	}
	return 0xff;
}

void DISPLAY::draw_screen()
{
	// render screen
	int ymax = (cs[0] & 1) ? 200 : 400;
	if(mode_r & 4) {
		draw_320dot_screen(ymax);
	} else {
		draw_640dot_screen(ymax);
	}
	
	// copy to pc screen
	if(ymax == 400) {
		// 400 lines
		for(int y = 0; y < 400; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest[x] = palette_pc[src[x]];
			}
		}
		emu->screen_skip_line(false);
	} else {
		// 200 lines
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
			uint8_t* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest0[x] = palette_pc[src[x]];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype_t));
			} else {
				my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
			}
		}
		emu->screen_skip_line(true);
	}
}

void DISPLAY::draw_640dot_screen(int ymax)
{
	int al = (sync[6] | (sync[7] << 8)) & 0x3ff;
	bool wy1 = false, wy2 = false, wy3 = false, wy4 = false;
	
	for(int i = 0, total = 0; i < 4 && total < al; i++) {
		uint32_t tmp = ra[4 * i];
		tmp |= ra[4 * i + 1] << 8;
		tmp |= ra[4 * i + 2] << 16;
		tmp |= ra[4 * i + 3] << 24;
		int ptr = (tmp << 1) & 0xfffe;
		int line = (tmp >> 20) & 0x3ff;
		bool wide = ((tmp & 0x80000000) != 0);
		
		for(int y = total; y < total + line && y < ymax; y++) {
			uint8_t mapy = mapram[y << 1];
			if(mapy & 1) wy1 = !wy1;
			if(mapy & 2) wy2 = !wy2;
			if(mapy & 4) wy3 = !wy3;
			if(mapy & 8) wy4 = !wy4;
			bool wx1 = false, wx2 = false, wx3 = false, wx4 = false;
			
			if(wide) {
				for(int x = 0; x < 640; x+= 16) {
					uint8_t mapx = mapram[(x >> 4) << 1];
					if(mapx & 0x10) wx1 = !wx1;
					if(mapx & 0x20) wx2 = !wx2;
					if(mapx & 0x40) wx3 = !wx3;
					if(mapx & 0x80) wx4 = !wx4;
					int wn = pri[(wx1 && wy1 ? 1 : 0) | (wx2 && wy2 ? 2 : 0) | (wx3 && wy3 ? 4 : 0) | (wx4 && wy4 ? 8 : 0)];
					
					int vaddr = ((ptr++) + vma[wn] * 2) & 0xffff;
					ptr &= 0xffff;
					uint8_t b = (vds[wn] & 1) ? vram_b[vaddr] : 0;
					uint8_t r = (vds[wn] & 2) ? vram_r[vaddr] : 0;
					uint8_t g = (vds[wn] & 4) ? vram_g[vaddr] : 0;
					uint8_t col, bcol = back[wn];
					if(mode_c & 1) {
						bcol = 0;
						b ^= reverse[wn];
						r ^= reverse[wn];
						g ^= reverse[wn];
					}
					
					col = ((b & 0x01) ? 1 : 0) | ((r & 0x01) ? 2 : 0) | ((g & 0x01) ? 4 : 0);
					screen[y][x +  0] = screen[y][x +  1] = col ? col : bcol;
					col = ((b & 0x02) ? 1 : 0) | ((r & 0x02) ? 2 : 0) | ((g & 0x02) ? 4 : 0);
					screen[y][x +  2] = screen[y][x +  3] = col ? col : bcol;
					col = ((b & 0x04) ? 1 : 0) | ((r & 0x04) ? 2 : 0) | ((g & 0x04) ? 4 : 0);
					screen[y][x +  4] = screen[y][x +  5] = col ? col : bcol;
					col = ((b & 0x08) ? 1 : 0) | ((r & 0x08) ? 2 : 0) | ((g & 0x08) ? 4 : 0);
					screen[y][x +  6] = screen[y][x +  7] = col ? col : bcol;
					col = ((b & 0x10) ? 1 : 0) | ((r & 0x10) ? 2 : 0) | ((g & 0x10) ? 4 : 0);
					screen[y][x +  8] = screen[y][x +  9] = col ? col : bcol;
					col = ((b & 0x20) ? 1 : 0) | ((r & 0x20) ? 2 : 0) | ((g & 0x20) ? 4 : 0);
					screen[y][x + 10] = screen[y][x + 11] = col ? col : bcol;
					col = ((b & 0x40) ? 1 : 0) | ((r & 0x40) ? 2 : 0) | ((g & 0x40) ? 4 : 0);
					screen[y][x + 12] = screen[y][x + 13] = col ? col : bcol;
					col = ((b & 0x80) ? 1 : 0) | ((r & 0x80) ? 2 : 0) | ((g & 0x80) ? 4 : 0);
					screen[y][x + 14] = screen[y][x + 15] = col ? col : bcol;
				}
			} else {
				for(int x = 0; x < 640; x+= 8) {
					if(!(x & 8)) {
						uint8_t mapx = mapram[(x >> 4) << 1];
						if(mapx & 0x10) wx1 = !wx1;
						if(mapx & 0x20) wx2 = !wx2;
						if(mapx & 0x40) wx3 = !wx3;
						if(mapx & 0x80) wx4 = !wx4;
					}
					int wn = pri[(wx1 && wy1 ? 1 : 0) | (wx2 && wy2 ? 2 : 0) | (wx3 && wy3 ? 4 : 0) | (wx4 && wy4 ? 8 : 0)];
					
					int vaddr = ((ptr++) + vma[wn] * 2) & 0xffff;
					ptr &= 0xffff;
					uint8_t b = (vds[wn] & 1) ? vram_b[vaddr] : 0;
					uint8_t r = (vds[wn] & 2) ? vram_r[vaddr] : 0;
					uint8_t g = (vds[wn] & 4) ? vram_g[vaddr] : 0;
					uint8_t col, bcol = back[wn];
					if(mode_c & 1) {
						bcol = 0;
						b ^= reverse[wn];
						r ^= reverse[wn];
						g ^= reverse[wn];
					}
					
					col = ((b & 0x01) ? 1 : 0) | ((r & 0x01) ? 2 : 0) | ((g & 0x01) ? 4 : 0);
					screen[y][x + 0] = col ? col : bcol;
					col = ((b & 0x02) ? 1 : 0) | ((r & 0x02) ? 2 : 0) | ((g & 0x02) ? 4 : 0);
					screen[y][x + 1] = col ? col : bcol;
					col = ((b & 0x04) ? 1 : 0) | ((r & 0x04) ? 2 : 0) | ((g & 0x04) ? 4 : 0);
					screen[y][x + 2] = col ? col : bcol;
					col = ((b & 0x08) ? 1 : 0) | ((r & 0x08) ? 2 : 0) | ((g & 0x08) ? 4 : 0);
					screen[y][x + 3] = col ? col : bcol;
					col = ((b & 0x10) ? 1 : 0) | ((r & 0x10) ? 2 : 0) | ((g & 0x10) ? 4 : 0);
					screen[y][x + 4] = col ? col : bcol;
					col = ((b & 0x20) ? 1 : 0) | ((r & 0x20) ? 2 : 0) | ((g & 0x20) ? 4 : 0);
					screen[y][x + 5] = col ? col : bcol;
					col = ((b & 0x40) ? 1 : 0) | ((r & 0x40) ? 2 : 0) | ((g & 0x40) ? 4 : 0);
					screen[y][x + 6] = col ? col : bcol;
					col = ((b & 0x80) ? 1 : 0) | ((r & 0x80) ? 2 : 0) | ((g & 0x80) ? 4 : 0);
					screen[y][x + 7] = col ? col : bcol;
				}
			}
		}
		total += line;
	}
}

void DISPLAY::draw_320dot_screen(int ymax)
{
	int al = (sync[6] | (sync[7] << 8)) & 0x3ff;
	bool wy1 = false, wy2 = false, wy3 = false, wy4 = false;
	
	for(int i = 0, total = 0; i < 4 && total < al; i++) {
		uint32_t tmp = ra[4 * i];
		tmp |= ra[4 * i + 1] << 8;
		tmp |= ra[4 * i + 2] << 16;
		tmp |= ra[4 * i + 3] << 24;
		int ptr = (tmp << 1) & 0xfffe;
		int line = (tmp >> 20) & 0x3ff;
		bool wide = ((tmp & 0x80000000) != 0);
		
		for(int y = total; y < total + line && y < ymax; y++) {
			uint8_t mapy = mapram[y << 1];
			if(mapy & 1) wy1 = !wy1;
			if(mapy & 2) wy2 = !wy2;
			if(mapy & 4) wy3 = !wy3;
			if(mapy & 8) wy4 = !wy4;
			bool wx1 = false, wx2 = false, wx3 = false, wx4 = false;
			
			if(wide) {
				for(int x = 0; x < 640; x+= 32) {
					uint8_t mapx = mapram[(x >> 4) << 1];
					if(mapx & 0x10) wx1 = !wx1;
					if(mapx & 0x20) wx2 = !wx2;
					if(mapx & 0x40) wx3 = !wx3;
					if(mapx & 0x80) wx4 = !wx4;
					int wn = pri[(wx1 && wy1 ? 1 : 0) | (wx2 && wy2 ? 2 : 0) | (wx3 && wy3 ? 4 : 0) | (wx4 && wy4 ? 8 : 0)];
					
					int vaddr = ((ptr++) + vma[wn] * 2) & 0xffff;
					ptr &= 0xffff;
					uint8_t b = (vds[wn] & 1) ? vram_b[vaddr] : 0;
					uint8_t r = (vds[wn] & 2) ? vram_r[vaddr] : 0;
					uint8_t g = (vds[wn] & 4) ? vram_g[vaddr] : 0;
					uint8_t col, bcol = back[wn];
					if(mode_c & 1) {
						bcol = 0;
						b ^= reverse[wn];
						r ^= reverse[wn];
						g ^= reverse[wn];
					}
					
					col = ((b & 0x01) ? 1 : 0) | ((r & 0x01) ? 2 : 0) | ((g & 0x01) ? 4 : 0);
					screen[y][x +  0] = screen[y][x +  1] = screen[y][x +  2] = screen[y][x +  3] = col ? col : bcol;
					col = ((b & 0x02) ? 1 : 0) | ((r & 0x02) ? 2 : 0) | ((g & 0x02) ? 4 : 0);
					screen[y][x +  4] = screen[y][x +  5] = screen[y][x +  6] = screen[y][x +  7] = col ? col : bcol;
					col = ((b & 0x04) ? 1 : 0) | ((r & 0x04) ? 2 : 0) | ((g & 0x04) ? 4 : 0);
					screen[y][x +  8] = screen[y][x +  9] = screen[y][x + 10] = screen[y][x + 11] = col ? col : bcol;
					col = ((b & 0x08) ? 1 : 0) | ((r & 0x08) ? 2 : 0) | ((g & 0x08) ? 4 : 0);
					screen[y][x + 12] = screen[y][x + 13] = screen[y][x + 14] = screen[y][x + 15] = col ? col : bcol;
					col = ((b & 0x10) ? 1 : 0) | ((r & 0x10) ? 2 : 0) | ((g & 0x10) ? 4 : 0);
					screen[y][x + 16] = screen[y][x + 17] = screen[y][x + 18] = screen[y][x + 19] = col ? col : bcol;
					col = ((b & 0x20) ? 1 : 0) | ((r & 0x20) ? 2 : 0) | ((g & 0x20) ? 4 : 0);
					screen[y][x + 20] = screen[y][x + 21] = screen[y][x + 22] = screen[y][x + 23] = col ? col : bcol;
					col = ((b & 0x40) ? 1 : 0) | ((r & 0x40) ? 2 : 0) | ((g & 0x40) ? 4 : 0);
					screen[y][x + 24] = screen[y][x + 25] = screen[y][x + 26] = screen[y][x + 27] = col ? col : bcol;
					col = ((b & 0x80) ? 1 : 0) | ((r & 0x80) ? 2 : 0) | ((g & 0x80) ? 4 : 0);
					screen[y][x + 28] = screen[y][x + 29] = screen[y][x + 30] = screen[y][x + 31] = col ? col : bcol;
				}
			} else {
				for(int x = 0; x < 640; x+= 16) {
					uint8_t mapx = mapram[(x >> 4) << 1];
					if(mapx & 0x10) wx1 = !wx1;
					if(mapx & 0x20) wx2 = !wx2;
					if(mapx & 0x40) wx3 = !wx3;
					if(mapx & 0x80) wx4 = !wx4;
					int wn = pri[(wx1 && wy1 ? 1 : 0) | (wx2 && wy2 ? 2 : 0) | (wx3 && wy3 ? 4 : 0) | (wx4 && wy4 ? 8 : 0)];
					
					int vaddr = ((ptr++) + vma[wn] * 2) & 0xffff;
					ptr &= 0xffff;
					uint8_t b = (vds[wn] & 1) ? vram_b[vaddr] : 0;
					uint8_t r = (vds[wn] & 2) ? vram_r[vaddr] : 0;
					uint8_t g = (vds[wn] & 4) ? vram_g[vaddr] : 0;
					uint8_t col, bcol = back[wn];
					if(mode_c & 1) {
						bcol = 0;
						b ^= reverse[wn];
						r ^= reverse[wn];
						g ^= reverse[wn];
					}
					
					col = ((b & 0x01) ? 1 : 0) | ((r & 0x01) ? 2 : 0) | ((g & 0x01) ? 4 : 0);
					screen[y][x +  0] = screen[y][x +  1] = col ? col : bcol;
					col = ((b & 0x02) ? 1 : 0) | ((r & 0x02) ? 2 : 0) | ((g & 0x02) ? 4 : 0);
					screen[y][x +  2] = screen[y][x +  3] = col ? col : bcol;
					col = ((b & 0x04) ? 1 : 0) | ((r & 0x04) ? 2 : 0) | ((g & 0x04) ? 4 : 0);
					screen[y][x +  4] = screen[y][x +  5] = col ? col : bcol;
					col = ((b & 0x08) ? 1 : 0) | ((r & 0x08) ? 2 : 0) | ((g & 0x08) ? 4 : 0);
					screen[y][x +  6] = screen[y][x +  7] = col ? col : bcol;
					col = ((b & 0x10) ? 1 : 0) | ((r & 0x10) ? 2 : 0) | ((g & 0x10) ? 4 : 0);
					screen[y][x +  8] = screen[y][x +  9] = col ? col : bcol;
					col = ((b & 0x20) ? 1 : 0) | ((r & 0x20) ? 2 : 0) | ((g & 0x20) ? 4 : 0);
					screen[y][x + 10] = screen[y][x + 11] = col ? col : bcol;
					col = ((b & 0x40) ? 1 : 0) | ((r & 0x40) ? 2 : 0) | ((g & 0x40) ? 4 : 0);
					screen[y][x + 12] = screen[y][x + 13] = col ? col : bcol;
					col = ((b & 0x80) ? 1 : 0) | ((r & 0x80) ? 2 : 0) | ((g & 0x80) ? 4 : 0);
					screen[y][x + 14] = screen[y][x + 15] = col ? col : bcol;
				}
			}
		}
		total += line;
	}
}

void DISPLAY::update_palette()
{
	if(mode_c & 1) {
		// mono
		for(int i = 1; i < 8; i++) {
			palette_pc[i] = palette_pc_base[7];
		}
		palette_pc[0] = palette_pc_base[0];
	} else if(mode_c & 4) {
		// plane priority
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = palette_pc_base[plane_priority[mode_p][palette[i]]];
		}
	} else {
		// color
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = palette_pc_base[palette[i]];
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
	state_fio->StateArray(palette_pc, sizeof(palette_pc), 1);
	state_fio->StateArray(palette, sizeof(palette), 1);
	state_fio->StateArray(back, sizeof(back), 1);
	state_fio->StateArray(reverse, sizeof(reverse), 1);
	state_fio->StateValue(rno);
	state_fio->StateArray(wregs, sizeof(wregs), 1);
	state_fio->StateArray(pri, sizeof(pri), 1);
	state_fio->StateArray(vma, sizeof(vma), 1);
	state_fio->StateArray(vds, sizeof(vds), 1);
	state_fio->StateValue(mode_r);
	state_fio->StateValue(mode_c);
	state_fio->StateValue(mode_p);
	return true;
}

