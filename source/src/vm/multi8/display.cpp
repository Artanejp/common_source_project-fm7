/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

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
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	
	// initialize
	for(int i = 0; i < 7; i++) {
		pal[i] = i;
	}
	text_wide = true;
	text_color = false;
	graph_color = 0xfe;
	graph_page = 7;
	cblink = 0;
	blink = false;
	
	// register event
	register_frame_event(this);
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	uint8_t mask;
	
	switch(addr & 0xff) {
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
		pal[addr & 7] = data & 7;
		mask = 1 << (addr & 7);
		if(data & 7) {
			graph_color |= mask;
		} else {
			graph_color &= ~mask;
		}
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
		return pal[addr & 7];
	}
	return 0xff;
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_DISPLAY_I8255_B) {
		data &= mask;
		graph_page = data & 7;
		text_color = ((data & 0x80) == 0);
		text_wide = ((data & 0x40) == 0);
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void DISPLAY::draw_screen()
{
	if((regs[8] & 0x30) != 0x30) {
		if((regs[8] & 0xc0) == 0xc0) {
			cursor = -1;
		} else {
			cursor = ((regs[14] << 8) | regs[15]) & 0x7ff;
		}
		
		// render screen
		if(graph_color) {
			draw_graph_color();
		} else {
			draw_graph_mono();
		}
		if(text_wide) {
			draw_text_wide();
		} else {
			draw_text_normal();
		}
	} else {
		memset(screen, 0, sizeof(screen));
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
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}

void DISPLAY::draw_graph_color()
{
	uint16_t src = ((regs[12] << 11) | (regs[13] << 3)) & 0x3fff;
	
	for(int y = 0; y < 200; y ++) {
		for(int x = 0; x < 80; x++) {
			uint8_t b = vram_b[src];
			uint8_t r = vram_r[src];
			uint8_t g = vram_g[src];
			uint8_t* d = &screen[y][x << 3];
			
			d[0] = pal[((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5)];
			d[1] = pal[((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4)];
			d[2] = pal[((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3)];
			d[3] = pal[((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2)];
			d[4] = pal[((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1)];
			d[5] = pal[((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0)];
			d[6] = pal[((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1)];
			d[7] = pal[((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2)];
			
			src = (src + 1) & 0x3fff;
		}
	}
}

void DISPLAY::draw_graph_mono()
{
	uint16_t src = ((regs[12] << 11) | (regs[13] << 3)) & 0x3fff;
	
	for(int y = 0; y < 200; y ++) {
		for(int x = 0; x < 80; x++) {
			uint8_t p1 = (graph_page & 1) ? vram_b[src] : 0;
			uint8_t p2 = (graph_page & 2) ? vram_r[src] : 0;
			uint8_t p3 = (graph_page & 4) ? vram_g[src] : 0;
			uint8_t pat = p1 | p2 | p3;
			uint8_t* d = &screen[y][x << 3];
			
			d[0] = (pat & 0x80) ? 7 : 0;
			d[1] = (pat & 0x40) ? 7 : 0;
			d[2] = (pat & 0x20) ? 7 : 0;
			d[3] = (pat & 0x10) ? 7 : 0;
			d[4] = (pat & 0x08) ? 7 : 0;
			d[5] = (pat & 0x04) ? 7 : 0;
			d[6] = (pat & 0x02) ? 7 : 0;
			d[7] = (pat & 0x01) ? 7 : 0;
			
			src = (src + 1) & 0x3fff;
		}
	}
}

void DISPLAY::draw_text_wide()
{
	uint16_t src = ((regs[12] << 8) | regs[13]) & 0x7ff;
	int hz = (regs[1] <= 80) ? regs[1] : 80;
	int vt = (regs[6] <= 25) ? regs[6] : 25;
	int ht = ((regs[9] <= 9) ? regs[9] : 9) + 1;
	uint8_t bp = regs[10] & 0x60;
	
	for(int y = 0; y < vt; y++) {
		for(int x = 0; x < hz; x += 2) {
			uint8_t code = vram_t[src];
			uint8_t attr = vram_a[src];
			
			// check attribute
			bool secret = ((attr & 8) || ((attr & 0x10) && blink));
			bool reverse = ((attr & 0x20) != 0);
			uint8_t col = text_color ? (attr & 7) : 7;
			
			// draw pattern
			for(int l = 0; l < 8; l++) {
				uint8_t pat = font[(code << 3) + l];
				pat = secret ? 0 : reverse ? ~pat : pat;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d = &screen[yy][x << 3];
				
				d[ 0] = (pat & 0x80) ? col : d[ 0];
				d[ 1] = (pat & 0x80) ? col : d[ 1];
				d[ 2] = (pat & 0x40) ? col : d[ 2];
				d[ 3] = (pat & 0x40) ? col : d[ 3];
				d[ 4] = (pat & 0x20) ? col : d[ 4];
				d[ 5] = (pat & 0x20) ? col : d[ 5];
				d[ 6] = (pat & 0x10) ? col : d[ 6];
				d[ 7] = (pat & 0x10) ? col : d[ 7];
				d[ 8] = (pat & 0x08) ? col : d[ 8];
				d[ 9] = (pat & 0x08) ? col : d[ 9];
				d[10] = (pat & 0x04) ? col : d[10];
				d[11] = (pat & 0x04) ? col : d[11];
				d[12] = (pat & 0x02) ? col : d[12];
				d[13] = (pat & 0x02) ? col : d[13];
				d[14] = (pat & 0x01) ? col : d[14];
				d[15] = (pat & 0x01) ? col : d[15];
			}
			// draw cursor
			if(src == cursor) {
				int s = regs[10] & 0x1f;
				int e = regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ht; l++) {
						int yy = y * ht + l;
						if(yy < 200) {
							memset(&screen[yy][x << 3], 7, 16);
						}
					}
				}
			}
			src = (src + 2) & 0x7ff;
		}
	}
}

void DISPLAY::draw_text_normal()
{
	uint16_t src = ((regs[12] << 8) | regs[13]) & 0x7ff;
	int hz = (regs[1] <= 80) ? regs[1] : 80;
	int vt = (regs[6] <= 25) ? regs[6] : 25;
	int ht = ((regs[9] <= 9) ? regs[9] : 9) + 1;
	uint8_t bp = regs[10] & 0x60;
	
	for(int y = 0; y < vt; y++) {
		for(int x = 0; x < hz; x++) {
			uint8_t code = vram_t[src];
			uint8_t attr = vram_a[src];
			
			// check attribute
			bool secret = ((attr & 8) || ((attr & 0x10) && blink));
			bool reverse = ((attr & 0x20) != 0);
			uint8_t col = text_color ? (attr & 7) : 7;
			
			// draw pattern
			for(int l = 0; l < 8; l++) {
				uint8_t pat = font[(code << 3) + l];
				pat = secret ? 0 : reverse ? ~pat : pat;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* d = &screen[yy][x << 3];
				
				d[0] = (pat & 0x80) ? col : d[0];
				d[1] = (pat & 0x40) ? col : d[1];
				d[2] = (pat & 0x20) ? col : d[2];
				d[3] = (pat & 0x10) ? col : d[3];
				d[4] = (pat & 0x08) ? col : d[4];
				d[5] = (pat & 0x04) ? col : d[5];
				d[6] = (pat & 0x02) ? col : d[6];
				d[7] = (pat & 0x01) ? col : d[7];
			}
			// draw cursor
			if(src == cursor) {
				int s = regs[10] & 0x1f;
				int e = regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ht; l++) {
						int yy = y * ht + l;
						if(yy < 200) {
							memset(&screen[yy][x << 3], 7, 8);
						}
					}
				}
			}
			src = (src + 1) & 0x7ff;
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
	state_fio->StateArray(pal, sizeof(pal), 1);
	state_fio->StateValue(text_wide);
	state_fio->StateValue(text_color);
	state_fio->StateValue(graph_color);
	state_fio->StateValue(graph_page);
	state_fio->StateValue(cursor);
	state_fio->StateValue(cblink);
	state_fio->StateValue(hsync);
	state_fio->StateValue(vsync);
	state_fio->StateValue(display);
	state_fio->StateValue(blink);
	return true;
}

