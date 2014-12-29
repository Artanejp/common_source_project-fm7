/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.18-

	[ display ]
*/

#include "display.h"
#include "../../fileio.h"

void DISPLAY::initialize()
{
	scanline = config.scan_line;
	
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// create cg pattern
	for(int i = 0; i < 256; i++) {
		cg[i * 8 + 0] = cg[i * 8 + 1] = ((i & 1) ? 0xf0 : 0) | ((i & 0x10) ? 0xf : 0);
		cg[i * 8 + 2] = cg[i * 8 + 3] = ((i & 2) ? 0xf0 : 0) | ((i & 0x20) ? 0xf : 0);
		cg[i * 8 + 4] = cg[i * 8 + 5] = ((i & 4) ? 0xf0 : 0) | ((i & 0x40) ? 0xf : 0);
		cg[i * 8 + 6] = cg[i * 8 + 7] = ((i & 8) ? 0xf0 : 0) | ((i & 0x80) ? 0xf : 0);
	}
	
	// initialize
	vram_addr = 0;
	chr = wide = true;
	cblink = 0;
	
	// register event
	register_frame_event(this);
}

void DISPLAY::update_config()
{
	scanline = config.scan_line;
}

void DISPLAY::write_io8(uint32 addr, uint32 data)
{
	// $01: vram data
	vram[vram_addr] = data;
}

uint32 DISPLAY::read_io8(uint32 addr)
{
	// $01: vram data
	return vram[vram_addr];
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_DISPLAY_ADDR_L) {
		vram_addr = (vram_addr & 0x700) | (data & 0xff);
	} else if(id == SIG_DISPLAY_ADDR_H) {
		vram_addr = (vram_addr & 0xff) | ((data & 7) << 8);
	} else if(id == SIG_DISPLAY_MODE) {
		chr = ((data & 0x40) == 0);
		wide = ((data & 0x80) != 0);
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

void DISPLAY::draw_screen()
{
	memset(screen, 0, sizeof(screen));
	if((regs[8] & 0x30) != 0x30) {
		if((regs[8] & 0xc0) == 0xc0) {
			cursor = -1;
		} else {
			cursor = ((regs[14] << 8) | regs[15]) & 0x7ff;
		}
		
		// render screen
		if(wide) {
			draw_40column();
		} else {
			draw_80column();
		}
	}
	
	// copy to real screen
	scrntype col = RGB_COLOR(255, 255, 255);
	for(int y = 0; y < 200; y++) {
		scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
		scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
		uint8* src = screen[y];
		
		for(int x = 0; x < 640; x++) {
			dest0[x] = src[x] ? col : 0;
		}
		if(scanline) {
			memset(dest1, 0, 640 * sizeof(scrntype));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype));
		}
	}
	emu->screen_skip_line = true;
}

void DISPLAY::draw_40column()
{
	uint16 src = ((regs[12] << 8) | regs[13]) & 0x7ff;
	int hz = (regs[1] <= 40) ? regs[1] : 40;
	int vt = (regs[6] <= 25) ? regs[6] : 25;
	int ht = ((regs[9] <= 11) ? regs[9] : 11) + 1;
	uint8 bp = regs[10] & 0x60;
	uint8* pattern = chr ? font : cg;
	
	for(int y = 0; y < vt; y++) {
		for(int x = 0; x < hz; x++) {
			// draw pattern
			uint8 code = vram[src];
			for(int l = 0; l < 8; l++) {
				uint8 pat = pattern[(code << 3) + l];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8* d = &screen[yy][x << 4];
				
				d[ 0] = d[ 1] = pat & 0x80;
				d[ 2] = d[ 3] = pat & 0x40;
				d[ 4] = d[ 5] = pat & 0x20;
				d[ 6] = d[ 7] = pat & 0x10;
				d[ 8] = d[ 9] = pat & 0x08;
				d[10] = d[11] = pat & 0x04;
				d[12] = d[13] = pat & 0x02;
				d[14] = d[15] = pat & 0x01;
			}
			// draw cursor
			if(src == cursor) {
				int s = regs[10] & 0x1f;
				int e = regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ht; l++) {
						int yy = y * ht + l;
						if(yy < 200) {
							memset(&screen[yy][x << 4], 1, 16);
						}
					}
				}
			}
			src = (src + 1) & 0x7ff;
		}
	}
}

void DISPLAY::draw_80column()
{
	uint16 src = ((regs[12] << 8) | regs[13]) & 0x7ff;
	int hz = (regs[1] <= 80) ? regs[1] : 80;
	int vt = (regs[6] <= 25) ? regs[6] : 25;
	int ht = ((regs[9] <= 11) ? regs[9] : 11) + 1;
	uint8 bp = regs[10] & 0x60;
	uint8* pattern = chr ? font : cg;
	
	for(int y = 0; y < vt; y++) {
		for(int x = 0; x < hz; x++) {
			// draw pattern
			uint8 code = vram[src];
			for(int l = 0; l < 8; l++) {
				uint8 pat = pattern[(code << 3) + l];
				int yy = y * ht + l;
				if(yy >= 200) {
					break;
				}
				uint8* d = &screen[yy][x << 3];
				
				d[0] = pat & 0x80;
				d[1] = pat & 0x40;
				d[2] = pat & 0x20;
				d[3] = pat & 0x10;
				d[4] = pat & 0x08;
				d[5] = pat & 0x04;
				d[6] = pat & 0x02;
				d[7] = pat & 0x01;
			}
			// draw cursor
			if(src == cursor) {
				int s = regs[10] & 0x1f;
				int e = regs[11] & 0x1f;
				if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
					for(int l = s; l <= e && l < ht; l++) {
						int yy = y * ht + l;
						if(yy < 200) {
							memset(&screen[yy][x << 3], 1, 8);
						}
					}
				}
			}
			src = (src + 1) & 0x7ff;
		}
	}
}

