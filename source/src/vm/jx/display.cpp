/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.09-

	[ display ]
*/


#include "display.h"
#include "../memory.h"

void DISPLAY::initialize()
{
	scanline = config.scan_line;
	
	// create pc palette
	for(int i = 0; i < 16; i++) {
		int val = (i & 8) ? 127 : 0;
		palette_pc[i] = RGB_COLOR((i & 4) ? 255 : val, (i & 2) ? 255 : val, (i & 1) ? 255 : val);
	}
	palette_pc[8] = RGB_COLOR(127, 127, 127);
	
	hires_mode = 1;
	prev_width = 640;
	prev_height = 400;
	
	cblink = 0;
	register_frame_event(this);
}

void DISPLAY::reset()
{
	memset(vgarray, 0, sizeof(vgarray));
	memset(palette, 0, sizeof(palette));
	vgarray_num = -1;
	
	memset(bankreg, 0, sizeof(bankreg));
	bankreg_num = 0;
	d_mem->set_memory_r(0x80000, 0xb7fff, kanji);
	
	page = 0;
	d_mem->set_memory_rw(0xb8000, 0xbbfff, vram);
	d_mem->set_memory_rw(0xbc000, 0xbffff, vram);
	
	status = 0x04;
}

void DISPLAY::update_config()
{
	scanline = config.scan_line;
}

void DISPLAY::write_io8(uint32 addr, uint32 data)
{
	switch(addr) {
	case 0x1ff:
		if(data & 0xf0) {
			if(bankreg_num == 7 && (bankreg[7] & 0x80) != (data & 0x80)) {
				if(data & 0x80) {
					// open kanji rom
				} else {
					// close kanji rom
				}
			} else if(bankreg_num == 10 && (bankreg[10] & 0x80) != (data & 0x80)) {
				if(data & 0x80) {
					// open ext-vram
					d_mem->set_memory_rw(0xa0000, 0xaffff, extvram);
				} else {
					// close ext-vram
					d_mem->set_memory_r(0xa0000, 0xaffff, kanji + 0x20000);
					d_mem->unset_memory_w(0xa0000, 0xaffff);
				}
			}
			bankreg[bankreg_num] = data;
		} else {
			bankreg_num = data;
		}
		break;
	case 0x3da:
		if(vgarray_num == -1) {
			vgarray_num = data & 0x1f;
			break;
		} else {
			if(vgarray_num < 0x10) {
				vgarray[vgarray_num] = data;
			} else {
				palette[vgarray_num & 0x0f] = data & 0x0f;
			}
			vgarray_num = -1;
		}
		break;
	case 0x3dd:
		hires_mode = data;
		break;
	case 0x3df:
		if((page & 0xb8) != (data & 0xb8)) {
			int page1 = data >> 3;
			int page2 = (data & 0x80) ? (page1 + 1) : page1;
			d_mem->set_memory_rw(0xb8000, 0xbbfff, vram + 0x4000 * (page1 & 7));
			d_mem->set_memory_rw(0xbc000, 0xbffff, vram + 0x4000 * (page2 & 7));
		}
		page = data;
		break;
	}
}

uint32 DISPLAY::read_io8(uint32 addr)
{
	switch(addr) {
	case 0x1ff:
		return 0x69;//0x7f;
	case 0x3da:
		vgarray_num = -1; // okay ???
		return status;
	case 0x3dd:
		return hires_mode;
	case 0x3df:
		return page;
	}
	return 0xff;
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_DISPLAY_ENABLE) {
		if(data & mask) {
			status |= 0x11;
		} else {
			status &= ~0x11;
		}
	} else if(id == SIG_DISPLAY_VBLANK) {
		if(data & mask) {
			status &= ~0x08;
		} else {
			status |= 0x08;
		}
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void DISPLAY::draw_screen()
{
	int mode1 = vgarray[0];
	int mode2 = vgarray[3];
	int screen_width, screen_height, width;
	
	memset(screen, 0, sizeof(screen));
	
	// change screen size
	if((hires_mode & 3) == 1) {
		screen_width = width = 640;
		screen_height = 400;
	} else {
		screen_width = width = 720;
		screen_height = 512;
	}
	if(!(prev_width == screen_width && prev_height == screen_height)) {
		emu->change_screen_size(screen_width, screen_height, -1, -1, screen_width, screen_height);
		prev_width = screen_width;
		prev_height = screen_height;
		
		// we need to wait until screen manager updates buffer size
		return;
	}
	
	// render screen
	if((hires_mode & 3) == 1) {
		switch(mode1 & 0x1a) {
		case 0x08:
			if(!(mode1 & 1)) {
				width = 320;	// 40column
			}
			draw_alpha();
			break;
		case 0x0a:
			if((mode1 & 4) && (mode2 & 8)) {
				draw_graph_640x200_2col();
			} else if((page & 0xc0) == 0xc0) {
				draw_graph_640x200_4col();
			} else {
				draw_graph_320x200_4col();
				width = 320;
			}
			break;
		case 0x1a:
			if(mode1 & 1) {
				draw_graph_320x200_16col();
				width = 320;
			} else {
				draw_graph_160x200_16col();
				width = 160;
			}
			break;
		}
	} else {
		switch(hires_mode & 3) {
		case 0:
			if(!(mode1 & 1)) {
				width = 360;	// 40column
			}
			draw_alpha();
			break;
		case 2:
			draw_graph_720x512_2col();
			break;
		case 3:
//			draw_graph_360x512_4col();
//			width = 360;
			draw_graph_720x512_2col();
			break;
		}
	}
	
	// copy to real screen
	if((hires_mode & 3) == 1) {
		for(int y = 0; y < 200; y++) {
			scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
			scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
			uint8 *src = screen[y];
			
			if(width == 640) {
				for(int x = 0; x < 640; x++) {
					dest0[x] = palette_pc[src[x]];
				}
			} else if(width == 320) {
				for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
					dest0[x2] = dest0[x2 + 1] = palette_pc[src[x]];
				}
			} else if(width == 160) {
				for(int x = 0, x4 = 0; x < 160; x++, x4 += 4) {
					dest0[x4] = dest0[x4 + 1] = dest0[x4 + 2] = dest0[x4 + 3] = palette_pc[src[x]];
				}
			}
			if(!scanline) {
				memcpy(dest1, dest0, 640 * sizeof(scrntype));
			} else {
				memset(dest1, 0, 640 * sizeof(scrntype));
			}
		}
		emu->screen_skip_line = true;
	} else {
		for(int y = 0; y < 512; y++) {
			scrntype* dest = emu->screen_buffer(y);
			uint8 *src = screen[y];
			
			if(width == 720) {
				for(int x = 0; x < 720; x++) {
					dest[x] = palette_pc[src[x]];
				}
			} else if(width == 360) {
				for(int x = 0, x2 = 0; x < 360; x++, x2 += 2) {
					dest[x2] = dest[x2 + 1] = palette_pc[src[x]];
				}
			}
		}
		emu->screen_skip_line = false;
	}
}

void DISPLAY::draw_alpha()
{
	int src = ((regs[12] << 8) | regs[13]) * 2;
	int cursor = ((regs[8] & 0xc0) == 0xc0) ? -1 : ((regs[14] << 8) | regs[15]) * 2;
	int hz_disp = regs[1];
	int vt_disp = regs[6] & 0x7f;
	int ch_height = (regs[9] & 0x1f) + 1;
	int ymax = (ch_height < 16) ? 8 : 16;
	int shift = (ch_height < 16) ? 0 : 1;
	int bp = regs[10] & 0x60;
	
	uint8 *vram_ptr = vram + (page & 7) * 0x4000;
	uint8 pal_mask = vgarray[1] & 0x0f;
	
	int prev_code = 0;
	
	for(int y = 0; y < vt_disp; y++) {
		int ytop = y * ch_height;
		
		for(int x = 0; x < hz_disp; x++) {
			bool draw_cursor = ((src & 0x3fff) == (cursor & 0x3fff));
			int code = vram_ptr[(src++) & 0x3fff];
			int attr = vram_ptr[(src++) & 0x3fff];
			if(x >= 90) {	// 720dot / 8dot
				continue;
			}
			int fore_color = palette[(attr     ) & pal_mask];
			int back_color = palette[(attr >> 4) & pal_mask];
			if((hires_mode & 3) != 1) {
				// hires
				fore_color = (attr & 4) ? 0 : 7;
				back_color = (attr & 4) ? 7 : 0;
				attr = ((attr & 1) ? 0x80 : 0) | ((attr & 2) ? 8 : 0) | 7;
			}
			int lr = 0, hi, lo;
			
			if(attr & 0x80) {
				// kanji character
				if(attr & 8) {
					// right side
					hi = prev_code;
					lo = code;
					prev_code = 0;
					lr = 1;
				} else {
					// left side
					hi = code;
					lo = vram_ptr[src & 0x3fff];
					prev_code = code;
				}
				
				// shift-jis -> addr
				code = (hi << 8) | lo;
				if(code < 0x9900) {
					code &= 0x1fff;
				} else {
					code &= 0x1ff;
					code |= 0x400;
				}
			}
			
			uint8 *pattern;
			if(ch_height < 16) {
				pattern = &font[code * 8];
			} else {
				pattern = &kanji[code * 32];
			}
			
			for(int l = 0; l < ch_height; l++) {
				if(ytop + l >= 512) {
					break;
				}
				uint8 pat = (l < ymax) ? pattern[(l << shift) | lr] : 0;
				uint8 *dest = &screen[ytop + l][x * 8];
				
				dest[0] = (pat & 0x80) ? fore_color : back_color;
				dest[1] = (pat & 0x40) ? fore_color : back_color;
				dest[2] = (pat & 0x20) ? fore_color : back_color;
				dest[3] = (pat & 0x10) ? fore_color : back_color;
				dest[4] = (pat & 0x08) ? fore_color : back_color;
				dest[5] = (pat & 0x04) ? fore_color : back_color;
				dest[6] = (pat & 0x02) ? fore_color : back_color;
				dest[7] = (pat & 0x01) ? fore_color : back_color;
			}
			
			// draw cursor
			if(draw_cursor) {
				int s = regs[10] & 0x1f;
				int e = regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ch_height; l++) {
						if(ytop + l < 512) {
							memset(&screen[ytop + l][x * 8], 7, 8);
						}
					}
				}
			}
		}
	}
}

void DISPLAY::draw_graph_160x200_16col()
{
	int src = (regs[12] << 8) | regs[13];
	
	uint8 mask = vgarray[1] & 0x0f;
	
	for(int l = 0; l < 2; l++) {
		uint8 *vram_ptr = vram + (page & 7) * 0x4000 + l * 0x2000;
		int src2 = src;
		
		for(int y = 0; y < 200; y += 2) {
			uint8 *dest = screen[y + l];
			
			for(int x = 0; x < 160; x += 2) {
				uint8 pat = vram_ptr[(src2++) & 0x1fff];
				
				dest[x    ] = palette[(pat >> 4) & mask];
				dest[x + 1] = palette[(pat     ) & mask];
			}
		}
	}
}

void DISPLAY::draw_graph_320x200_4col()
{
	int src = (regs[12] << 8) | regs[13];
	
	uint8 mask = vgarray[1] & 3;
	
	for(int l = 0; l < 2; l++) {
		uint8 *vram_ptr = vram + (page & 7) * 0x4000 + l * 0x2000;
		int src2 = src;
		
		for(int y = 0; y < 200; y += 2) {
			uint8 *dest = screen[y + l];
			
			for(int x = 0; x < 320; x += 4) {
				uint8 pat = vram_ptr[(src2++) & 0x1fff];
				
				dest[x    ] = palette[(pat >> 6) & mask];
				dest[x + 1] = palette[(pat >> 4) & mask];
				dest[x + 2] = palette[(pat >> 2) & mask];
				dest[x + 3] = palette[(pat     ) & mask];
			}
		}
	}
}

void DISPLAY::draw_graph_320x200_16col()
{
	int src = (regs[12] << 8) | regs[13];
	
	uint8 mask = vgarray[1] & 0x0f;
	
	for(int l = 0; l < 4; l++) {
		uint8 *vram_ptr = vram + ((page >> 1) & 3) * 0x8000 + l * 0x2000;
		int src2 = src;
		
		for(int y = 0; y < 200; y += 4) {
			uint8 *dest = screen[y + l];
			
			for(int x = 0; x < 320; x += 2) {
				uint8 pat = vram_ptr[(src2++) & 0x1fff];
				
				dest[x    ] = palette[(pat >> 4) & mask];
				dest[x + 1] = palette[(pat     ) & mask];
			}
		}
	}
}

void DISPLAY::draw_graph_640x200_2col()
{
	int src = (regs[12] << 8) | regs[13];
	
	uint8 mask = vgarray[1] & 1;
	
	for(int l = 0; l < 2; l++) {
		uint8 *vram_ptr = vram + (page & 7) * 0x4000 + l * 0x2000;
		int src2 = src;
		
		for(int y = 0; y < 200; y += 2) {
			uint8 *dest = screen[y + l];
			
			for(int x = 0; x < 640; x += 8) {
				uint8 pat = vram_ptr[(src2++) & 0x1fff];
				
				dest[x    ] = palette[(pat >> 7) & mask];
				dest[x + 1] = palette[(pat >> 6) & mask];
				dest[x + 2] = palette[(pat >> 5) & mask];
				dest[x + 3] = palette[(pat >> 4) & mask];
				dest[x + 4] = palette[(pat >> 3) & mask];
				dest[x + 5] = palette[(pat >> 2) & mask];
				dest[x + 6] = palette[(pat >> 1) & mask];
				dest[x + 7] = palette[(pat     ) & mask];
			}
		}
	}
}

void DISPLAY::draw_graph_640x200_4col()
{
	int src = (regs[12] << 8) | regs[13];
	
	uint8 mask = vgarray[1] & 3;
	
	for(int l = 0; l < 4; l++) {
		uint8 *vram_ptr = vram + ((page >> 1) & 3) * 0x8000 + l * 0x2000;
		int src2 = src;
		
		for(int y = 0; y < 200; y += 4) {
			uint8 *dest = screen[y + l];
			
			for(int x = 0; x < 640; x += 8) {
				uint8 pat0 = vram_ptr[(src2++) & 0x1fff];
				uint8 pat1 = vram_ptr[(src2++) & 0x1fff];
				
				dest[x    ] = palette[(((pat0 >> 7) & 1) | ((pat1 >> 6) & 2)) & mask];
				dest[x + 1] = palette[(((pat0 >> 6) & 1) | ((pat1 >> 5) & 2)) & mask];
				dest[x + 2] = palette[(((pat0 >> 5) & 1) | ((pat1 >> 4) & 2)) & mask];
				dest[x + 3] = palette[(((pat0 >> 4) & 1) | ((pat1 >> 3) & 2)) & mask];
				dest[x + 4] = palette[(((pat0 >> 3) & 1) | ((pat1 >> 2) & 2)) & mask];
				dest[x + 5] = palette[(((pat0 >> 2) & 1) | ((pat1 >> 1) & 2)) & mask];
				dest[x + 6] = palette[(((pat0 >> 1) & 1) | ((pat1     ) & 2)) & mask];
				dest[x + 7] = palette[(((pat0 >> 0) & 1) | ((pat1 << 1) & 2)) & mask];
			}
		}
	}
}

void DISPLAY::draw_graph_720x512_2col()
{
	static const uint8 palette2[2] = {0, 7};
	
	int src = (regs[12] << 8) | regs[13];
	
	uint8 mask = 1;//vgarray[1] & 1;
	
	for(int l = 0; l < 2; l++) {
		uint8 *vram_ptr = extvram + l * 0x8000;
		int src2 = src;
		
		for(int y = 0; y < 512; y += 2) {
			uint8 *dest = screen[y + l];
			
			for(int x = 0; x < 720; x += 8) {
				uint8 pat = vram_ptr[(src2++) & 0x7fff];
				
				dest[x    ] = palette2[(pat >> 7) & mask];
				dest[x + 1] = palette2[(pat >> 6) & mask];
				dest[x + 2] = palette2[(pat >> 5) & mask];
				dest[x + 3] = palette2[(pat >> 4) & mask];
				dest[x + 4] = palette2[(pat >> 3) & mask];
				dest[x + 5] = palette2[(pat >> 2) & mask];
				dest[x + 6] = palette2[(pat >> 1) & mask];
				dest[x + 7] = palette2[(pat     ) & mask];
			}
		}
	}
}

void DISPLAY::draw_graph_360x512_4col()
{
	// TODO
}

