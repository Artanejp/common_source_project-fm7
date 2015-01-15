/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ vdp ]
*/

#include "vdp.h"
#include "../../fileio.h"

void VDP::initialize()
{
	memset(reg, 0, sizeof(reg));
	cmask = pmask = 0xff;
	bg = 0;
	create_pal();
	create_bg();
	
	// register event to interrupt
	register_vline_event(this);
}

void VDP::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xf5:
		reg[0] = data;
		create_pal();
		break;
	case 0xf6:
		reg[1] = data;
		create_pal();
		break;
	case 0xf7:
		reg[2] = data;
		create_pal();
		break;
	case 0xf8:
		reg[3] = data;
		create_pal();
		break;
	case 0xf9:
		reg[4] = data;
		create_pal();
		break;
	case 0xfa:
		reg[5] = data;
		create_pal();
		break;
	case 0xfb:
		cmask = ((data & 1) ? 4 : 0) | ((data & 2) ? 1 : 0) | ((data & 4) ? 2 : 0); 
		cmask |= cmask << 4;
		create_pal();
		break;
	case 0xfc:
		bg = data;
		create_bg();
		break;
	case 0xfe:
		pmask = data;
		break;
	}
}

void VDP::event_vline(int v, int clock)
{
	// vsync interrupt (not pending ???)
	if(v == 184) {
		d_cpu->set_intr_line(true, false, 0);
	}
}

void VDP::draw_screen()
{
	// create screen
	for(int y = 0, src = 0; y < 184; y++) {
		uint8* dest0 = screen0[y];
		uint8* dest1 = screen1[y];
		
		for(int x = 0; x < 192; x += 8) {
			uint8 p0 = (pmask & 0x01) ? vram[0][src] : 0;
			uint8 p1 = (pmask & 0x02) ? vram[1][src] : 0;
			uint8 p2 = (pmask & 0x04) ? vram[2][src] : 0;
			uint8 p3 = (pmask & 0x08) ? vram[3][src] : 0;
			uint8 p4 = (pmask & 0x10) ? vram[4][src] : 0;
			uint8 p5 = (pmask & 0x20) ? vram[5][src] : 0;
			src++;
			
			dest0[x + 7] = ((p0 & 0x80) >> 7) | ((p1 & 0x80) >> 6) | ((p2 & 0x80) >> 5);
			dest0[x + 6] = ((p0 & 0x40) >> 6) | ((p1 & 0x40) >> 5) | ((p2 & 0x40) >> 4);
			dest0[x + 5] = ((p0 & 0x20) >> 5) | ((p1 & 0x20) >> 4) | ((p2 & 0x20) >> 3);
			dest0[x + 4] = ((p0 & 0x10) >> 4) | ((p1 & 0x10) >> 3) | ((p2 & 0x10) >> 2);
			dest0[x + 3] = ((p0 & 0x08) >> 3) | ((p1 & 0x08) >> 2) | ((p2 & 0x08) >> 1);
			dest0[x + 2] = ((p0 & 0x04) >> 2) | ((p1 & 0x04) >> 1) | ((p2 & 0x04) >> 0);
			dest0[x + 1] = ((p0 & 0x02) >> 1) | ((p1 & 0x02) >> 0) | ((p2 & 0x02) << 1);
			dest0[x + 0] = ((p0 & 0x01) >> 0) | ((p1 & 0x01) << 1) | ((p2 & 0x01) << 2);
			
			dest1[x + 7] = ((p3 & 0x80) >> 7) | ((p4 & 0x80) >> 6) | ((p5 & 0x80) >> 5);
			dest1[x + 6] = ((p3 & 0x40) >> 6) | ((p4 & 0x40) >> 5) | ((p5 & 0x40) >> 4);
			dest1[x + 5] = ((p3 & 0x20) >> 5) | ((p4 & 0x20) >> 4) | ((p5 & 0x20) >> 3);
			dest1[x + 4] = ((p3 & 0x10) >> 4) | ((p4 & 0x10) >> 3) | ((p5 & 0x10) >> 2);
			dest1[x + 3] = ((p3 & 0x08) >> 3) | ((p4 & 0x08) >> 2) | ((p5 & 0x08) >> 1);
			dest1[x + 2] = ((p3 & 0x04) >> 2) | ((p4 & 0x04) >> 1) | ((p5 & 0x04) >> 0);
			dest1[x + 1] = ((p3 & 0x02) >> 1) | ((p4 & 0x02) >> 0) | ((p5 & 0x02) << 1);
			dest1[x + 0] = ((p3 & 0x01) >> 0) | ((p4 & 0x01) << 1) | ((p5 & 0x01) << 2);
		}
	}
	
	// copy to screen buffer
	for(int y = 0; y < 184; y++) {
		scrntype* dest = emu->screen_buffer(y);
		uint8* src0 = screen0[y];
		uint8* src1 = screen1[y];
		
		for(int x = 0; x < 192; x++) {
			dest[x] = palette_pc[src0[x] ? src0[x] : src1[x] ? (src1[x] + 8) : 16];
		}
	}
}

void VDP::create_pal()
{
	// create palette
	for(int i = 0; i < 8; i++) {
		uint8 tmp = ((i & 1) ? reg[0] : 0) | ((i & 2) ? reg[1] : 0) | ((i & 4) ? reg[2] : 0);
		if(tmp & cmask) {
			tmp &= cmask;
		}
		uint16 r = ((tmp & 0x11) == 0x11) ? 255 : ((tmp & 0x11) == 0x1) ? 127 : 0;
		uint16 g = ((tmp & 0x22) == 0x22) ? 255 : ((tmp & 0x22) == 0x2) ? 127 : 0;
		uint16 b = ((tmp & 0x44) == 0x44) ? 255 : ((tmp & 0x44) == 0x4) ? 127 : 0;
		
		palette_pc[i] = RGB_COLOR(r, g, b);
	}
	for(int i = 8; i < 16; i++) {
		uint8 tmp = ((i & 1) ? reg[3] : 0) | ((i & 2) ? reg[4] : 0) | ((i & 4) ? reg[5] : 0);
		if(tmp & cmask) {
			tmp &= cmask;
		}
		uint16 r = ((tmp & 0x11) == 0x11) ? 255 : ((tmp & 0x11) == 0x1) ? 127 : 0;
		uint16 g = ((tmp & 0x22) == 0x22) ? 255 : ((tmp & 0x22) == 0x2) ? 127 : 0;
		uint16 b = ((tmp & 0x44) == 0x44) ? 255 : ((tmp & 0x44) == 0x4) ? 127 : 0;
		
		palette_pc[i] = RGB_COLOR(r, g, b);
	}
}

void VDP::create_bg()
{
	// create bg palette
	uint16 r = ((bg & 0x11) == 0x11) ? 255 : ((bg & 0x11) == 0x1) ? 127 : 0;
	uint16 g = ((bg & 0x22) == 0x22) ? 255 : ((bg & 0x22) == 0x2) ? 127 : 0;
	uint16 b = ((bg & 0x44) == 0x44) ? 255 : ((bg & 0x44) == 0x4) ? 127 : 0;
	
	palette_pc[16] = RGB_COLOR(r, g, b);
}

#define STATE_VERSION	1

void VDP::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(palette_pc, sizeof(palette_pc), 1);
	state_fio->Fwrite(reg, sizeof(reg), 1);
	state_fio->FputUint8(bg);
	state_fio->FputUint8(cmask);
	state_fio->FputUint8(pmask);
}

bool VDP::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(palette_pc, sizeof(palette_pc), 1);
	state_fio->Fread(reg, sizeof(reg), 1);
	bg = state_fio->FgetUint8();
	cmask = state_fio->FgetUint8();
	pmask = state_fio->FgetUint8();
	return true;
}

