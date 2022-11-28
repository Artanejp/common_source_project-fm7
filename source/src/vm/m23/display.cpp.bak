/*
	SORD M23 Emulator 'Emu23'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ display ]
*/

#include "display.h"

void DISPLAY::initialize()
{
	// load rom image
	memset(font, 0x00, sizeof(font));
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path("FONT.ROM"), FILEIO_READ_BINARY)) {
		fio->Fread(font, 0x800, 1);
		memcpy(font + 0x800, font, 0x800);
		fio->Fread(font + 0x800, 0x800, 1);
		fio->Fclose();
	}
	delete fio;
	
	// create pc palette
	for(int i = 1; i < 8; i++) {
		if(config.monitor_type == 1) {
			palette_pc[i] = RGB_COLOR(0, 255, 0);
		} else {
			palette_pc[i] = RGB_COLOR((i & 1) ? 255 : 0, (i & 2) ? 255 : 0, (i & 4) ? 255 : 0);
		}
	}
	palette_pc[0] = 0;
	
	// initialize
	vd_control = 1;
	
	// register event
	register_frame_event(this);
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xd7:
		return vd_control;
	}
	return 0xff;
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xd7:
		// bit1: 0=25lines,1=20lines
		// bit2: 0=Text,1=Graph
		// bit4: R
		// bit5: G
		// bit6: B
		vd_control = data;
		break;
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void DISPLAY::draw_screen()
{
	memset(screen, (vd_control >> 4) & 7, sizeof(screen));
	
	if((regs[8] & 0x30) != 0x30 && (vd_control & 1) != 0) {
		if(vd_control & 4) {
			//draw_graph();
		} else {
			draw_text();
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
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
		}
	}
	emu->screen_skip_line(true);
}

void DISPLAY::draw_text()
{
	uint16_t src = ((regs[12] << 8) | regs[13]) & 0x7ff;
	int hz = (regs[1] <= 80) ? regs[1] : 80;
	int vt = (regs[6] <= 25) ? regs[6] : 25;
	int ht = ((regs[9] <= 9) ? regs[9] : 9) + 1;
	uint8_t bp = regs[10] & 0x60;
	int cursor = -1;
	
	if((regs[8] & 0xc0) != 0xc0) {
		cursor = ((regs[14] << 8) | regs[15]) & 0x7ff;
	}
	for(int y = 0; y < vt; y++) {
		for(int x = 0; x < hz; x++) {
			uint8_t code = vram_t[src];
			uint8_t attr = vram_a[src];
			
			// check attribute
			bool reverse = ((attr & 0x01) != 0);
			bool under_line = ((attr & 0x04) != 0);
			bool ascii = ((attr & 0x08) != 0);
			uint8_t color = (attr >> 4) & 7;
			uint8_t pattern;
			
			// draw pattern
			for(int l = 0; l < ht; l++) {
				if(under_line && l == ((vd_control & 2) ? 8 : 7)) {
					pattern = 0xff;
				} else {
					pattern = (l < 8) ? font[(ascii ? 0x800 : 0) | ((code << 3) + l)] : 0;
				}
				pattern = reverse ? ~pattern : pattern;
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8_t* dest = &screen[yy][x << 3];
				
				dest[0] = (pattern & 0x80) ? color : dest[0];
				dest[1] = (pattern & 0x40) ? color : dest[1];
				dest[2] = (pattern & 0x20) ? color : dest[2];
				dest[3] = (pattern & 0x10) ? color : dest[3];
				dest[4] = (pattern & 0x08) ? color : dest[4];
				dest[5] = (pattern & 0x04) ? color : dest[5];
				dest[6] = (pattern & 0x02) ? color : dest[6];
				dest[7] = (pattern & 0x01) ? color : dest[7];
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
	state_fio->StateValue(cblink);
	state_fio->StateValue(hsync);
	state_fio->StateValue(vsync);
	state_fio->StateValue(display);
	state_fio->StateValue(blink);
	state_fio->StateValue(vd_control);
	return true;
}

