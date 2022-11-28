/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ display ]
*/

#include "display.h"

// SL:	0-6,74

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
#ifdef TYPE_SL
	palette_pc[0] = RGB_COLOR(160, 168, 160);
	palette_pc[1] = RGB_COLOR(48, 56, 16);
#else
	palette_pc[0] = RGB_COLOR(55, 15, 0);
	palette_pc[1] = RGB_COLOR(220, 60, 0);
#endif
	
	cblink = 0;
	register_frame_event(this);
}

void DISPLAY::reset()
{
	status = 4;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x3d8:
		mode = data;
		break;
	case 0x3d9:
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x3da:
		return status;
	}
	return 0xff;
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
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
	int width = 320, height = 200;
	
	memset(screen, 0, sizeof(screen));
	
	// render screen
	switch(mode & 0x1f) {
	case 0x0c:
	case 0x08:
		// 40x25
		draw_alpha();
		break;
	case 0x0d:
	case 0x09:
		// 80x25
		draw_alpha();
		width = 640;
		break;
	case 0x0e:
	case 0x0a:
		// 320x200 mono/color
		draw_graph_320x200();
		width = 640;
		height = 400;
		break;
	case 0x1e:
		if((regs[9] & 0x1f) == 3) {
			// 640x400 mono
			draw_graph_640x400();
			width = 640;
			height = 400;
		} else {
			// 640x200 mono
			draw_graph_640x200();
			width = 640;
		}
		break;
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(height);
	
	if(height == 200) {
		// 320x200 or 640x200
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
			uint8_t *src = screen[y];
			
			if(width == 320) {
				for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
					dest0[x2] = dest0[x2 + 1] = palette_pc[src[x]];
				}
			} else {
				for(int x = 0; x < 640; x++) {
					dest0[x] = palette_pc[src[x]];
				}
			}
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	} else {
		// 640x400
		for(int y = 0; y < 400; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t *src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest[x] = palette_pc[src[x]];
			}
		}
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
	
	for(int y = 0; y < vt_disp; y++) {
		int ytop = y * ch_height;
		
		for(int x = 0; x < hz_disp; x++) {
			bool draw_cursor = ((src & 0x7fff) == (cursor & 0x7fff));
			int code = vram[(src++) & 0x7fff];
			int attr = vram[(src++) & 0x7fff];
			if(x >= 80) {
				continue;
			}
			uint8_t *pattern = &font[code * 8];
			uint8_t fore_color = (attr & 0x07) ? 1 : 0;
			uint8_t back_color = (attr & 0x70) ? 1 : 0;
			
			for(int l = 0; l < ch_height; l++) {
				if(ytop + l >= 200) {
					break;
				}
				uint8_t pat = (l < 8) ? pattern[l] : 0;
				uint8_t *dest = &screen[ytop + l][x * 8];
				
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
						if(ytop + l < 200) {
							memset(&screen[ytop + l][x * 8], 7, 8);
						}
					}
				}
			}
		}
	}
}

void DISPLAY::draw_graph_320x200()
{
	// mode 4,5
	static const uint8_t dots[4][4] = {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 1}, {1, 1, 1, 1}};
	int src = (regs[12] << 8) | regs[13];
	
	for(int l = 0; l < 2; l++) {
		int src2 = src + l * 0x2000;
		
		for(int y = 0; y < 200; y += 2) {
			uint8_t *dest0 = screen[(y + l) * 2];
			uint8_t *dest1 = screen[(y + l) * 2 + 1];
			
			for(int x = 0; x < 640; x += 8) {
				uint8_t pat = vram[(src2++) & 0x7fff];
				int c0 = (pat >> 6) & 3;
				int c1 = (pat >> 4) & 3;
				int c2 = (pat >> 2) & 3;
				int c3 = (pat     ) & 3;
				
				dest0[x    ] = dots[c0][0];
				dest0[x + 1] = dots[c0][1];
				dest0[x + 2] = dots[c1][0];
				dest0[x + 3] = dots[c1][1];
				dest0[x + 4] = dots[c2][0];
				dest0[x + 5] = dots[c2][1];
				dest0[x + 6] = dots[c3][0];
				dest0[x + 7] = dots[c3][1];
				dest1[x    ] = dots[c0][2];
				dest1[x + 1] = dots[c0][3];
				dest1[x + 2] = dots[c1][2];
				dest1[x + 3] = dots[c1][3];
				dest1[x + 4] = dots[c2][2];
				dest1[x + 5] = dots[c2][3];
				dest1[x + 6] = dots[c3][2];
				dest1[x + 7] = dots[c3][3];
			}
		}
	}
}

void DISPLAY::draw_graph_640x200()
{
	// mode 6
	int src = (regs[12] << 8) | regs[13];
	
	for(int l = 0; l < 2; l++) {
		int src2 = src + l * 0x2000;
		
		for(int y = 0; y < 200; y += 2) {
			uint8_t *dest = screen[y + l];
			
			for(int x = 0; x < 640; x += 8) {
				uint8_t pat = vram[(src2++) & 0x7fff];
				
				dest[x    ] = (pat >> 7) & 1;
				dest[x + 1] = (pat >> 6) & 1;
				dest[x + 2] = (pat >> 5) & 1;
				dest[x + 3] = (pat >> 4) & 1;
				dest[x + 4] = (pat >> 3) & 1;
				dest[x + 5] = (pat >> 2) & 1;
				dest[x + 6] = (pat >> 1) & 1;
				dest[x + 7] = (pat     ) & 1;
			}
		}
	}
}

void DISPLAY::draw_graph_640x400()
{
	// mode 74
	int src = (regs[12] << 8) | regs[13];
	
	for(int l = 0; l < 4; l++) {
		int src2 = src + l * 0x2000;
		
		for(int y = 0; y < 400; y += 4) {
			uint8_t *dest = screen[y + l];
			
			for(int x = 0; x < 640; x += 8) {
				uint8_t pat = vram[(src2++) & 0x7fff];
				
				dest[x    ] = (pat >> 7) & 1;
				dest[x + 1] = (pat >> 6) & 1;
				dest[x + 2] = (pat >> 5) & 1;
				dest[x + 3] = (pat >> 4) & 1;
				dest[x + 4] = (pat >> 3) & 1;
				dest[x + 5] = (pat >> 2) & 1;
				dest[x + 6] = (pat >> 1) & 1;
				dest[x + 7] = (pat     ) & 1;
			}
		}
	}
}

