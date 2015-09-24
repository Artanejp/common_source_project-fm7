/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ sub ]
*/

#include "sub.h"
#include "main.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void SUB::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// create pc palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram_chr, 0, sizeof(vram_chr));
	memset(vram_gfx, 0, sizeof(vram_gfx));
	
	SET_BANK(0x0000, 0x1fff, wdmy, ipl);
	SET_BANK(0x2000, 0x27ff, common, common);
	SET_BANK(0x2800, 0x3fff, wdmy, rdmy);
	SET_BANK(0x4000, 0x7fff, ram, ram);
	SET_BANK(0x8000, 0xffff, wdmy, rdmy);
	
	register_frame_event(this);
}

void SUB::reset()
{
	memset(disp, 0, sizeof(disp));
	cblink = 0;
}

void SUB::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 SUB::read_data8(uint32 addr)
{
	addr &= 0xffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void SUB::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xf0) {
	case 0x00:	// mz3500sm p.18,77
//		emu->out_debug_log("SUB->MAIN\tINT0=1\n");
		d_main->write_signal(SIG_MAIN_INT0, 1, 1);
		break;
	case 0x50:	// mz3500sm p.28
		if((addr & 0x0f) == 0x0d) {
			disp[5] = data;
		} else {
			disp[addr & 0x0f] = data;
		}
		break;
	}
}

uint32 SUB::read_io8(uint32 addr)
{
	switch(addr & 0xf0) {
	case 0x00:	// mz3500sm p.18,77
//		emu->out_debug_log("SUB->MAIN\tINT0=1\n");
		d_main->write_signal(SIG_MAIN_INT0, 1, 1);
		break;
	case 0x50:	// mz3500sm p.28
		if((addr & 0x0f) == 0x0d) {
			return disp[5];
		} else {
			return disp[addr & 0x0f];
		}
	}
	return 0xff;
}

void SUB::event_frame()
{
	cblink++;
}

void SUB::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_SUB_PIO_PM) {
		pm = ((data & mask) != 0);
	}
}

// display

void SUB::draw_screen()
{
	memset(screen_chr, 0, sizeof(screen_chr));
	memset(screen_gfx, 0, sizeof(screen_gfx));
	
	// mz3500sm p.28
	if(disp[0] & 1) {
		draw_chr();
	}
	if(disp[1] & 7) {
		draw_gfx();
	}
	uint8 back = disp[3] & 7;
	
	// copy to pc screen
	for(int y = 0; y <400; y++) {
		scrntype* dest = emu->screen_buffer(y);
		uint8* src_chr = screen_chr[y];
		uint8* src_gfx = screen_gfx[y];
		
		for(int x = 0; x < 640; x++) {
			dest[x] = palette_pc[(src_chr[x] ? (src_chr[x] & 7) : src_gfx[x] ? (src_gfx[x] & 7) : back)];
		}
	}
	emu->screen_skip_line = false;
}

void SUB::draw_chr()
{
	// mz3500sm p.28
	int width = (disp[5] & 2) ? 1 : 2;	// 80/40 columns
	int height = (disp[7] & 1) ? 20 : 16;	// 20/16 dots/ine
	int ymax = (disp[7] & 1) ? 20 : 25;	// 20/25 lines
	
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32 ra = ra_chr[4 * i];
		ra |= ra_chr[4 * i + 1] << 8;
		ra |= ra_chr[4 * i + 2] << 16;
		ra |= ra_chr[4 * i + 3] << 24;
		int src = ra & 0x1fff;
		int len = ((ra >> 20) & 0x3ff) / height;
		int caddr = ((cs_chr[0] & 0x80) && ((cs_chr[1] & 0x20) || !(cblink & 0x10))) ? (*ead_chr & 0x1fff) : -1;
		
		for(int y = ytop; y < (ytop + len) && y < ymax; y++) {
			for(int x = 0; x < 80; x += width) {
				bool cursor = (src == caddr);
				uint8 code = vram_chr[(src * 2 + 0) & 0xfff];	// low byte  : code
				uint8 attr = vram_chr[(src * 2 + 1) & 0xfff];	// high byte : attr
				src++;
				
				// mz3500sm p.31
				// bit3: blink
				// bit2: reverse or green
				// bit1: vertical line or red
				// bit0: horizontal line or blue
				
				uint8 color;
				bool reverse, blink;
				
				if(disp[4] & 1) {
					// color
					color = attr & 7;
					reverse = false;
				} else {
					// monocrhome
					color = 7;
					reverse = ((attr & 4) != 0);
				}
				blink = ((attr & 8) != 0 && (cblink & 0x10) != 0);
				reverse = (reverse != blink);
				
				uint8* pattern = &font[0x1000 | (code << 4)];
				
				// NOTE: need to consider 200 line mode
				
				for(int l = 0; l < 16; l++) {
					int yy = y * height + l;
					if(yy >= 400) {
						break;
					}
					uint8 pat = reverse ? ~pattern[l] : pattern[l];
					uint8 *dest = &screen_chr[yy][x << 3];
					
					if(width == 1) {
						// 8dots (80columns)
						dest[0] = (pat & 0x80) ? color : 0;
						dest[1] = (pat & 0x40) ? color : 0;
						dest[2] = (pat & 0x20) ? color : 0;
						dest[3] = (pat & 0x10) ? color : 0;
						dest[4] = (pat & 0x08) ? color : 0;
						dest[5] = (pat & 0x04) ? color : 0;
						dest[6] = (pat & 0x02) ? color : 0;
						dest[7] = (pat & 0x01) ? color : 0;
					} else {
						// 16dots (40columns)
						dest[ 0] = dest[ 1] = (pat & 0x80) ? color : 0;
						dest[ 2] = dest[ 3] = (pat & 0x40) ? color : 0;
						dest[ 4] = dest[ 5] = (pat & 0x20) ? color : 0;
						dest[ 6] = dest[ 7] = (pat & 0x10) ? color : 0;
						dest[ 8] = dest[ 9] = (pat & 0x08) ? color : 0;
						dest[10] = dest[11] = (pat & 0x04) ? color : 0;
						dest[12] = dest[13] = (pat & 0x02) ? color : 0;
						dest[14] = dest[15] = (pat & 0x01) ? color : 0;
					}
				}
				if(cursor) {
					int top = cs_chr[1] & 0x1f, bottom = cs_chr[2] >> 3;
					for(int l = top; l < bottom && l < height; l++) {
						int yy = y * height + l;
						if(yy >= 400) {
							break;
						}
						memset(&screen_chr[yy][x << 3], 7, width * 8);	// always white ???
					}
				}
			}
		}
		ytop += len;
	}
}

void SUB::draw_gfx()
{
}

#define STATE_VERSION	1

void SUB::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(vram_chr, sizeof(vram_chr), 1);
	state_fio->Fwrite(vram_gfx, sizeof(vram_gfx), 1);
	state_fio->Fwrite(disp, sizeof(disp), 1);
	state_fio->FputBool(pm);
	state_fio->FputInt32(cblink);
}

bool SUB::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	state_fio->Fread(vram_chr, sizeof(vram_chr), 1);
	state_fio->Fread(vram_gfx, sizeof(vram_gfx), 1);
	state_fio->Fread(disp, sizeof(disp), 1);
	pm = state_fio->FgetBool();
	cblink = state_fio->FgetInt32();
	return true;
}

