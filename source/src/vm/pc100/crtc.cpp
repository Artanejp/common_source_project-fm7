/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.14 -

	[ crtc ]
*/

#include "crtc.h"
#include "../i8259.h"

void CRTC::initialize()
{
	// init vram
	memset(vram, 0, sizeof(vram));
	
	// init bit control
	shift = 0;
	maskl = maskh = busl = bush = 0;
	
	// init vram plane
	write_plane = 1;
	read_plane = 0;
	
	// init pallete
	for(int i = 1; i < 16; i++) {
		palette[i] = 0x1ff;
		update_palette(i);
	}
	palette[0] = 0;
	update_palette(0);
	
	// register event
	register_vline_event(this);
}

void CRTC::event_vline(int v, int clock)
{
	if(v == 512) {
		d_pic->write_signal(SIG_I8259_IR4, 1, 1);
	}
}

void CRTC::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x30:
		shift = data & 0x0f;
		break;
	case 0x38:
		sel = data;
		break;
	case 0x3a:
		regs[sel & 7] = data;
		break;
	case 0x3c:
		vs = (vs & 0xff00) | data;
		break;
	case 0x3e:
		vs = (vs & 0xff) | (data << 8);
		break;
	case 0x40:
	case 0x42:
	case 0x44:
	case 0x46:
	case 0x48:
	case 0x4a:
	case 0x4c:
	case 0x4e:
	case 0x50:
	case 0x52:
	case 0x54:
	case 0x56:
	case 0x58:
	case 0x5a:
	case 0x5c:
	case 0x5e:
		palette[(addr >> 1) & 0x0f] = (palette[(addr >> 1) & 0x0f] & 0xff00) | data;
		update_palette((addr >> 1) & 0x0f);
		break;
	case 0x41:
	case 0x43:
	case 0x45:
	case 0x47:
	case 0x49:
	case 0x4b:
	case 0x4d:
	case 0x4f:
	case 0x51:
	case 0x53:
	case 0x55:
	case 0x57:
	case 0x59:
	case 0x5b:
	case 0x5d:
	case 0x5f:
		palette[(addr >> 1) & 0x0f] = (palette[(addr >> 1) & 0x0f] & 0xff) | (data << 8);
		update_palette((addr >> 1) & 0x0f);
		break;
	case 0x60:
		cmd = (cmd & 0xff00) | data;
		break;
	case 0x61:
		cmd = (cmd & 0xff) | (data << 8);
		break;
	}
}

uint32 CRTC::read_io8(uint32 addr)
{
	uint32 val = 0xff;
	
	switch(addr & 0x3ff) {
	case 0x30:
		return shift;
	case 0x38:
		return sel;
	case 0x3a:
		return regs[sel & 7];
	case 0x3c:
		return vs & 0xff;
	case 0x3e:
		return vs >> 8;
	case 0x40:
	case 0x42:
	case 0x44:
	case 0x46:
	case 0x48:
	case 0x4a:
	case 0x4c:
	case 0x4e:
	case 0x50:
	case 0x52:
	case 0x54:
	case 0x56:
	case 0x58:
	case 0x5a:
	case 0x5c:
	case 0x5e:
		return palette[(addr >> 1) & 0x0f] & 0xff;
	case 0x41:
	case 0x43:
	case 0x45:
	case 0x47:
	case 0x49:
	case 0x4b:
	case 0x4d:
	case 0x4f:
	case 0x51:
	case 0x53:
	case 0x55:
	case 0x57:
	case 0x59:
	case 0x5b:
	case 0x5d:
	case 0x5f:
		return palette[(addr >> 1) & 0x0f] >> 8;
	case 0x60:
		return cmd & 0xff;
	case 0x61:
		return cmd >> 8;
	}
	return 0xff;
}

void CRTC::write_memory_mapped_io8(uint32 addr, uint32 data)
{
	if(addr & 1) {
		bush = data;
	} else {
		busl = data;
	}
	uint32 bus = busl | (bush << 8) | (busl << 16) | (bush << 24);
	bus >>= shift;
	
	if(addr & 1) {
		uint32 h = (bus >> 8) & 0xff;
		for(int pl = 0; pl < 4; pl++) {
			if(write_plane & (1 << pl)) {
				int ofsh = (addr & 0x1ffff) | (0x20000 * pl);
				vram[ofsh] = (vram[ofsh] & maskh) | (h & ~maskh);
			}
		}
	} else {
		uint32 l = bus & 0xff;
		for(int pl = 0; pl < 4; pl++) {
			if(write_plane & (1 << pl)) {
				int ofsl = (addr & 0x1ffff) | (0x20000 * pl);
				vram[ofsl] = (vram[ofsl] & maskl) | (l & ~maskl);
			}
		}
	}
}

uint32 CRTC::read_memory_mapped_io8(uint32 addr)
{
	return vram[(addr & 0x1ffff) | (0x20000 * read_plane)];
}

void CRTC::write_memory_mapped_io16(uint32 addr, uint32 data)
{
	busl = (addr & 1) ? (data >> 8) : (data & 0xff);
	bush = (addr & 1) ? (data & 0xff) : (data >> 8);
	uint32 bus = busl | (bush << 8) | (busl << 16) | (bush << 24);
	bus >>= shift;
	uint32 l = bus & 0xff;
	uint32 h = (bus >> 8) & 0xff;
	
	for(int pl = 0; pl < 4; pl++) {
		if(write_plane & (1 << pl)) {
			int ofsl = ((addr & 1 ? (addr + 1) : addr) & 0x1ffff) | (0x20000 * pl);
			int ofsh = ((addr & 1 ? addr : (addr + 1)) & 0x1ffff) | (0x20000 * pl);
			vram[ofsl] = (vram[ofsl] & maskl) | (l & ~maskl);
			vram[ofsh] = (vram[ofsh] & maskh) | (h & ~maskh);
		}
	}
}

uint32 CRTC::read_memory_mapped_io16(uint32 addr)
{
	uint32 val = read_memory_mapped_io8(addr);
	val |= read_memory_mapped_io8(addr + 1) << 8;
	return val;
}

void CRTC::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CRTC_BITMASK_LOW) {
		// $18: 8255 PA
		maskl = data & 0xff;
	} else if(id == SIG_CRTC_BITMASK_HIGH) {
		// $1A: 8255 PB
		maskh = data & 0xff;
	} else if(id == SIG_CRTC_VRAM_PLANE) {
		// $1C: 8255 PC
		write_plane = data & 0x0f;
		read_plane = (data >> 4) & 3;
	}
}

void CRTC::draw_screen()
{
	// display region
	int hd = (regs[2] >> 1) & 0x3f;
	int vd = (regs[6] >> 1) & 0x3f;
	int hs = (int)(int8)((regs[0] & 0x40) ? (regs[0] | 0x80) : (regs[0] & 0x3f));
//	int hs = (int)(int8)regs[0];
	int vs_tmp = (int)(int16)((vs & 0x400) ? (vs | 0xf800) : (vs & 0x3ff));
	int sa = (hs + hd + 1) * 2 + (vs_tmp + vd) * 0x80;
//	int sa = (hs + hd + 1) * 2 + ((vs & 0x3ff) + vd) * 0x80;
	
	if(cmd != 0xffff) {
		// mono
		scrntype col = RGB_COLOR(255, 255, 255);
		for(int y = 0; y < 512; y++) {
			int ptr = sa & 0x1ffff;
			sa += 0x80;
			scrntype *dest = emu->screen_buffer(y);
			
			for(int x = 0; x < 720; x += 8) {
				uint8 pat = vram[ptr];
				ptr = (ptr + 1) & 0x1ffff;
				
				dest[x + 0] = pat & 0x01 ? col : 0;
				dest[x + 1] = pat & 0x02 ? col : 0;
				dest[x + 2] = pat & 0x04 ? col : 0;
				dest[x + 3] = pat & 0x08 ? col : 0;
				dest[x + 4] = pat & 0x10 ? col : 0;
				dest[x + 5] = pat & 0x20 ? col : 0;
				dest[x + 6] = pat & 0x40 ? col : 0;
				dest[x + 7] = pat & 0x80 ? col : 0;
			}
		}
	} else {
		// color
		for(int y = 0; y < 512; y++) {
			int ptr = sa & 0x1ffff;
			sa += 0x80;
			scrntype *dest = emu->screen_buffer(y);
			
			for(int x = 0; x < 720; x += 8) {
				uint8 p0 = vram[0x00000 | ptr];
				uint8 p1 = vram[0x20000 | ptr];
				uint8 p2 = vram[0x40000 | ptr];
				uint8 p3 = vram[0x60000 | ptr];
				ptr = (ptr + 1) & 0x1ffff;
				
				dest[x + 0] = palette_pc[((p0 & 0x01) << 0) | ((p1 & 0x01) << 1) | ((p2 & 0x01) << 2) | ((p3 & 0x01) << 3)];
				dest[x + 1] = palette_pc[((p0 & 0x02) >> 1) | ((p1 & 0x02) << 0) | ((p2 & 0x02) << 1) | ((p3 & 0x02) << 2)];
				dest[x + 2] = palette_pc[((p0 & 0x04) >> 2) | ((p1 & 0x04) >> 1) | ((p2 & 0x04) << 0) | ((p3 & 0x04) << 1)];
				dest[x + 3] = palette_pc[((p0 & 0x08) >> 3) | ((p1 & 0x08) >> 2) | ((p2 & 0x08) >> 1) | ((p3 & 0x08) << 0)];
				dest[x + 4] = palette_pc[((p0 & 0x10) >> 4) | ((p1 & 0x10) >> 3) | ((p2 & 0x10) >> 2) | ((p3 & 0x10) >> 1)];
				dest[x + 5] = palette_pc[((p0 & 0x20) >> 5) | ((p1 & 0x20) >> 4) | ((p2 & 0x20) >> 3) | ((p3 & 0x20) >> 2)];
				dest[x + 6] = palette_pc[((p0 & 0x40) >> 6) | ((p1 & 0x40) >> 5) | ((p2 & 0x40) >> 4) | ((p3 & 0x40) >> 3)];
				dest[x + 7] = palette_pc[((p0 & 0x80) >> 7) | ((p1 & 0x80) >> 6) | ((p2 & 0x80) >> 5) | ((p3 & 0x80) >> 4)];
			}
		}
	}
	emu->screen_skip_line = false;
}

void CRTC::update_palette(int num)
{
	int r = (palette[num] >> 0) & 7;
	int g = (palette[num] >> 3) & 7;
	int b = (palette[num] >> 6) & 7;
	palette_pc[num] = RGB_COLOR(r << 5, g << 5, b << 5);
}

