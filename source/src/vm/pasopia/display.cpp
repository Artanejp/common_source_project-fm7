/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2007.02.08 -

	[ display ]
*/

#include "display.h"
#include "../../fileio.h"

void DISPLAY::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
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
	
	// init pasopia own
	mode = 0;
	cblink = 0;
	
	// register event
	register_frame_event(this);
}

void DISPLAY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x10:
		d_crtc->write_io8(addr, data);
		break;
	case 0x11:
		d_crtc->write_io8(addr, data);
		break;
	}
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	// from 8255-1 port.a
	mode = data;
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void DISPLAY::draw_screen()
{
	if((regs[8] & 0x30) != 0x30) {
		uint16 src = ((regs[12] << 8) | regs[13]) & 0x7ff;
		if((regs[8] & 0xc0) == 0xc0) {
			cursor = -1;
		} else {
			cursor = ((regs[14] << 8) | regs[15]) & 0x7ff;
		}
		
		// render screen
		memset(screen, mode & 7, sizeof(screen));
		
		switch(mode & 0xe0) {
		case 0x00:	// screen 0, wide
			draw_screen0_wide(src);
			break;
		case 0x20:	// screen 0, normal
			draw_screen0_normal(src);
			break;
		case 0x40:	// screen 1, wide
			draw_screen1_wide(src);
			break;
		case 0x60:	// screen 1, normal
			draw_screen1_normal(src);
			break;
		case 0x80:	// screen 2, wide
			draw_screen2_wide(src);
			break;
		case 0xa0:	// screen 2, normal
			draw_screen2_normal(src);
			break;
		case 0xc0:	// screen 1.5, wide
			draw_screen15_wide(src);
			break;
		case 0xe0:	// screen 1.5, normal
			draw_screen15_normal(src);
			break;
		}
	} else {
		memset(screen, 0, sizeof(screen));
	}
	
	// copy to real screen
	uint16 bcol = palette_pc[mode & 7];
	for(int y = 0; y < 200; y++) {
		scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
		scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
		uint8* src = screen[y];
		
		for(int x = 0; x < 640; x++) {
			dest0[x] = palette_pc[src[x] & 7];
		}
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype));
		}
	}
	emu->screen_skip_line = true;
}

#define IS_ATTRIB(d) (((d) & 0xf8) == 0xf8)

void DISPLAY::draw_screen0_normal(uint16 src)
{
	// screen 0, normal char (80chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	
	for(int y = 0; y < 200; y += 8) {
		uint8 c_t = IS_ATTRIB(vram[src_t]) ? (vram[src_t] & 7) : 7;
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			uint8 code = vram[src_t];
			if(IS_ATTRIB(code)) {
				c_t = code & 7;
			}
			uint8* font_base = &font[code << 3];
			
			for(int l = 0; l < 8; l++) {
				uint8 p = font_base[l];
				uint8* d = &screen[y + l][x << 3];
				
				d[0] = (p & 0x80) ? c_t : c_b;
				d[1] = (p & 0x40) ? c_t : c_b;
				d[2] = (p & 0x20) ? c_t : c_b;
				d[3] = (p & 0x10) ? c_t : c_b;
				d[4] = (p & 0x08) ? c_t : c_b;
				d[5] = (p & 0x04) ? c_t : c_b;
				d[6] = (p & 0x02) ? c_t : c_b;
				d[7] = (p & 0x01) ? c_t : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 3], 7, 8);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen0_wide(uint16 src)
{
	// screen 0, wide char (36chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	
	for(int y = 0; y < 192; y += 8) {
		uint8 c_t = IS_ATTRIB(vram[src_t]) ? (vram[src_t] & 7) : 7;
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			uint8 code = vram[src_t];
			if(IS_ATTRIB(code)) {
				c_t = code & 7;
			}
			uint8* font_base = &font[code << 3];
			
			for(int l = 0; l < 8; l++) {
				uint8 p = font_base[l];
				uint8* d = &screen[y + l][x << 4];
				
				d[ 0] = d[ 1] = (p & 0x80) ? c_t : c_b;
				d[ 2] = d[ 3] = (p & 0x40) ? c_t : c_b;
				d[ 4] = d[ 5] = (p & 0x20) ? c_t : c_b;
				d[ 6] = d[ 7] = (p & 0x10) ? c_t : c_b;
				d[ 8] = d[ 9] = (p & 0x08) ? c_t : c_b;
				d[10] = d[11] = (p & 0x04) ? c_t : c_b;
				d[12] = d[13] = (p & 0x02) ? c_t : c_b;
				d[14] = d[15] = (p & 0x01) ? c_t : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 4], 7, 16);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen1_normal(uint16 src)
{
	// screen 1, normal char (80chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	uint8 c_t[8] = {7, 7, 7, 7, 7, 7, 7, 7};
	
	for(int y = 0; y < 200; y += 8) {
		// character data is set for every other line in scren 1
		for(int i = 0; i < 8; i += 2) {
			uint8 t = vram[src_t + (i * 0x800)];
			if(IS_ATTRIB(t)) {
				c_t[i] = c_t[i + 1] = t & 7;
			}
		}
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			bool is_graph[8];
			uint8 attr_t[8];
			uint8 code[8];
			for(int i = 0; i < 8; i += 2) {
				uint16 t = (src_t + (i * 0x800)) & 0x3fff;
				is_graph[i] = is_graph[i + 1] = (attr[t] != 0);
				attr_t[i] = attr_t[i + 1] = attr[t];
				code[i] = code[i + 1] = vram[t];
			}
			
			for(int l = 0; l < 8; l++) {
				// change line color if vram data is text and is attribute character
				// note: check only first line
				uint8 code_t = code[l];
				if(!is_graph[l] && IS_ATTRIB(code_t)) {
					c_t[l] = code_t & 7;
				}
				uint8* font_base = &font[code_t << 3];
				uint8 c_l = c_t[l], c_r = c_t[l], p = font_base[l];
				if(is_graph[l]) {
					if(attr_t[l]) {
						p = code_t;
						c_l = (p >> 4) & 7;
						c_r = p & 7;
						p = (p & 0xf0 ? 0xf0 : 0) | (p & 0x0f ? 0x0f : 0);
					} else {
						p = 0;
					}
				}
				uint8* d = &screen[y + l][x << 3];
				
				d[0] = (p & 0x80) ? c_l : c_b;
				d[1] = (p & 0x40) ? c_l : c_b;
				d[2] = (p & 0x20) ? c_l : c_b;
				d[3] = (p & 0x10) ? c_l : c_b;
				d[4] = (p & 0x08) ? c_r : c_b;
				d[5] = (p & 0x04) ? c_r : c_b;
				d[6] = (p & 0x02) ? c_r : c_b;
				d[7] = (p & 0x01) ? c_r : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 3], 7, 8);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen1_wide(uint16 src)
{
	// screen 1, wide char (36chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	uint8 c_t[8] = {7, 7, 7, 7, 7, 7, 7, 7};
	
	for(int y = 0; y < 192; y += 8) {
		// character data is set for every other line in scren 1
		for(int i = 0; i < 8; i += 2) {
			uint8 t = vram[src_t + (i * 0x800)];
			if(IS_ATTRIB(t)) {
				c_t[i] = c_t[i + 1] = t & 7;
			}
		}
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			bool is_graph[8];
			uint8 attr_t[8];
			uint8 code[8];
			for(int i = 0; i < 8; i += 2) {
				uint16 t = (src_t + (i * 0x800)) & 0x3fff;
				is_graph[i] = is_graph[i + 1] = (attr[t] != 0);
				attr_t[i] = attr_t[i + 1] = attr[t];
				code[i] = code[i + 1] = vram[t];
			}
			
			for(int l = 0; l < 8; l++) {
				// change line color if vram data is text and is attribute character
				// note: check only first line
				uint8 code_t = code[l];
				if(!is_graph[l] && IS_ATTRIB(code_t)) {
					c_t[l] = code_t & 7;
				}
				uint8* font_base = &font[code_t << 3];
				uint8 c_l = c_t[l], c_r = c_t[l], p = font_base[l];
				if(is_graph[l]) {
					if(attr_t[l]) {
						p = code_t;
						c_l = (p >> 4) & 7;
						c_r = p & 7;
						p = (p & 0xf0 ? 0xf0 : 0) | (p & 0x0f ? 0x0f : 0);
					} else {
						p = 0;
					}
				}
				uint8* d = &screen[y + l][x << 4];
				
				d[ 0] = d[ 1] = (p & 0x80) ? c_l : c_b;
				d[ 2] = d[ 3] = (p & 0x40) ? c_l : c_b;
				d[ 4] = d[ 5] = (p & 0x20) ? c_l : c_b;
				d[ 6] = d[ 7] = (p & 0x10) ? c_l : c_b;
				d[ 8] = d[ 9] = (p & 0x08) ? c_r : c_b;
				d[10] = d[11] = (p & 0x04) ? c_r : c_b;
				d[12] = d[13] = (p & 0x02) ? c_r : c_b;
				d[14] = d[15] = (p & 0x01) ? c_r : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 4], 7, 16);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen2_normal(uint16 src)
{
	// screen 2, normal char (80chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	uint8 c_t[8] = {7, 7, 7, 7, 7, 7, 7, 7};
	
	for(int y = 0; y < 200; y += 8) {
		// character data is set for every line in scren 2
		for(int i = 0; i < 8; i++) {
			uint8 t = vram[src_t + (i * 0x800)];
			if(IS_ATTRIB(t)) {
				c_t[i] = t & 7;
			}
		}
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			uint16 src_g = src_t;
			bool is_graph[8];
			uint8 code[8];
			for(int i = 0; i < 8; i++) {
				uint16 t = (src_t + (i * 0x800)) & 0x3fff;
				is_graph[i] = (attr[t] != 0);
				code[i] = vram[t];
			}
			
			for(int l = 0; l < 8; l++) {
				// change line color if vram data is text and is attribute character
				uint8 code_t = code[l];
				if(!is_graph[l] && (0xf8 <= code_t) && (code_t <= 0xff)) {
					c_t[l] = code_t & 7;
				}
				uint8 c_l = c_t[l], c_r = c_t[l];
				
				uint8* font_base = &font[code_t << 3];
				uint8 p = is_graph[l] ? (attr[src_g] ? vram[src_g] : 0) : font_base[l];
				src_g = (src_g + 0x800) & 0x3fff;
				uint8 c_p = is_graph[l] ? 7 : c_t[l];
				uint8* d = &screen[y + l][x << 3];
				
				d[0] = (p & 0x80) ? c_p : c_b;
				d[1] = (p & 0x40) ? c_p : c_b;
				d[2] = (p & 0x20) ? c_p : c_b;
				d[3] = (p & 0x10) ? c_p : c_b;
				d[4] = (p & 0x08) ? c_p : c_b;
				d[5] = (p & 0x04) ? c_p : c_b;
				d[6] = (p & 0x02) ? c_p : c_b;
				d[7] = (p & 0x01) ? c_p : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 3], 7, 8);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen2_wide(uint16 src)
{
	// screen 0, wide char (36chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	uint8 c_t[8] = {7, 7, 7, 7, 7, 7, 7, 7};
	
	for(int y = 0; y < 192; y += 8) {
		// character data is set for every line in scren 2
		for(int i = 0; i < 8; i++) {
			uint8 t = vram[src_t + (i * 0x800)];
			if(IS_ATTRIB(t)) {
				c_t[i] = t & 7;
			}
		}
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			uint16 src_g = src_t;
			bool is_graph[8];
			uint8 code[8];
			for(int i = 0; i < 8; i++) {
				uint16 t = (src_t + (i * 0x800)) & 0x3fff;
				is_graph[i] = (attr[t] != 0);
				code[i] = vram[t];
			}
			for(int l = 0; l < 8; l++) {
				// change line color if vram data is text and is attribute character
				uint8 code_t = code[l];
				if(!is_graph[l] && IS_ATTRIB(code_t)) {
					c_t[l] = code_t & 7;
				}
				uint8 c_l = c_t[l], c_r = c_t[l];
				
				uint8* font_base = &font[code_t << 3];
				uint8 p = is_graph[l] ? (attr[src_g] ? vram[src_g] : 0) : font_base[l];
				src_g = (src_g + 0x800) & 0x3fff;
				uint8 c_p = is_graph[l] ? 7 : c_t[l];
				uint8* d = &screen[y + l][x << 4];
				
				d[ 0] = d[ 1] = (p & 0x80) ? c_p : c_b;
				d[ 2] = d[ 3] = (p & 0x40) ? c_p : c_b;
				d[ 4] = d[ 5] = (p & 0x20) ? c_p : c_b;
				d[ 6] = d[ 7] = (p & 0x10) ? c_p : c_b;
				d[ 8] = d[ 9] = (p & 0x08) ? c_p : c_b;
				d[10] = d[11] = (p & 0x04) ? c_p : c_b;
				d[12] = d[13] = (p & 0x02) ? c_p : c_b;
				d[14] = d[15] = (p & 0x01) ? c_p : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 4], 7, 16);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen15_normal(uint16 src)
{
	// screen 2, normal char (80chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	uint8 c_t[8] = {7, 7, 7, 7, 7, 7, 7, 7};
	
	for(int y = 0; y < 200; y += 8) {
		// character data is set for every line in scren 1.5
		for(int i = 0; i < 8; i++) {
			uint8 t = vram[src_t + (i * 0x800)];
			if(IS_ATTRIB(t)) {
				c_t[i] = t & 7;
			}
		}
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			bool is_graph[8];
			uint8 attr_t[8];
			uint8 code[8];
			for(int i = 0; i < 8; i++) {
				uint16 t = (src_t + (i * 0x800)) & 0x3fff;
				is_graph[i] = (attr[t] != 0);
				attr_t[i] = attr[t];
				code[i] = vram[t];
			}
			for(int l = 0; l < 8; l++) {
				// change line color if vram data is text and is attribute character
				uint8 code_t = code[l];
				if(!is_graph[l] && IS_ATTRIB(code_t)) {
					c_t[l] = code_t & 7;
				}
				uint8* font_base = &font[code_t << 3];
				uint8 c_l = c_t[l], c_r = c_t[l], p = font_base[l];
				if(is_graph[l]) {
					if(attr_t[l]) {
						p = code_t;
						c_l = (p >> 4) & 7;
						c_r = p & 7;
						p = (p & 0xf0 ? 0xf0 : 0) | (p & 0x0f ? 0x0f : 0);
					} else {
						p = 0;
					}
				}
				uint8* d = &screen[y + l][x << 3];
				
				d[0] = (p & 0x80) ? c_l : c_b;
				d[1] = (p & 0x40) ? c_l : c_b;
				d[2] = (p & 0x20) ? c_l : c_b;
				d[3] = (p & 0x10) ? c_l : c_b;
				d[4] = (p & 0x08) ? c_r : c_b;
				d[5] = (p & 0x04) ? c_r : c_b;
				d[6] = (p & 0x02) ? c_r : c_b;
				d[7] = (p & 0x01) ? c_r : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 3], 7, 8);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

void DISPLAY::draw_screen15_wide(uint16 src)
{
	// screen 0, normal char (80chars)
	uint16 src_t = src & 0x7ff;
	uint8 c_b = mode & 7;
	int width = regs[1] - 1;
	uint8 c_t[8] = {7, 7, 7, 7, 7, 7, 7, 7};
	
	for(int y = 0; y < 192; y += 8) {
		// character data is set for every line in scren 1.5
		for(int i = 0; i < 8; i++) {
			uint8 t = vram[src_t + (i * 0x800)];
			if(IS_ATTRIB(t)) {
				c_t[i] = t & 7;
			}
		}
		src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		
		for(int x = 0; x < width; x++) {
			bool is_graph[8];
			uint8 attr_t[8];
			uint8 code[8];
			for(int i = 0; i < 8; i++) {
				uint16 t = (src_t + (i * 0x800)) & 0x3fff;
				is_graph[i] = (attr[t] != 0);
				attr_t[i] = attr[t];
				code[i] = vram[t];
			}
			
			for(int l = 0; l < 8; l++) {
				// change line color if vram data is text and is attribute character
				uint8 code_t = code[l];
				if(!is_graph[l] && IS_ATTRIB(code_t)) {
					c_t[l] = code_t & 7;
				}
				uint8* font_base = &font[code_t << 3];
				uint8 c_l = c_t[l], c_r = c_t[l], p = font_base[l];
				if(is_graph[l]) {
					if(attr_t[l]) {
						p = code_t;
						c_l = (p >> 4) & 7;
						c_r = p & 7;
						p = (p & 0xf0 ? 0xf0 : 0) | (p & 0x0f ? 0x0f : 0);
					} else {
						p = 0;
					}
				}
				uint8* d = &screen[y + l][x << 4];
				
				d[ 0] = d[ 1] = (p & 0x80) ? c_l : c_b;
				d[ 2] = d[ 3] = (p & 0x40) ? c_l : c_b;
				d[ 4] = d[ 5] = (p & 0x20) ? c_l : c_b;
				d[ 6] = d[ 7] = (p & 0x10) ? c_l : c_b;
				d[ 8] = d[ 9] = (p & 0x08) ? c_r : c_b;
				d[10] = d[11] = (p & 0x04) ? c_r : c_b;
				d[12] = d[13] = (p & 0x02) ? c_r : c_b;
				d[14] = d[15] = (p & 0x01) ? c_r : c_b;
			}
			if(src_t == cursor) {
				int bp = regs[10] & 0x60;
				if((bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int i = (regs[10] & 7); i < 8; i++) {
						memset(&screen[y + i][x << 4], 7, 16);
					}
				}
			}
			src_t = (src_t & 0x3800) | ((src_t + 1) & 0x7ff);
		}
	}
}

#define STATE_VERSION	1

void DISPLAY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(mode);
	state_fio->FputUint16(cursor);
	state_fio->FputUint16(cblink);
}

bool DISPLAY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	mode = state_fio->FgetUint8();
	cursor = state_fio->FgetUint16();
	cblink = state_fio->FgetUint16();
	return true;
}

