/*
 * FM77AV/FM16β ALU [77av_alu.cpp]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 28, 2015 : Initial
 *
 */

#include "77av_alu.h"


uint8 FMALU::do_read(uint32 addr, uint32 bank, bool is_400line)
{
	uint32 raddr;

	if(((1 << bank) & read_signal(SIG_DISPLAY_MULTIPAGE)) != 0) return 0xff;

	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = addr + 0x8000 * bank;
			return memory->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
		}
		return 0xff;
	} else {
		raddr = addr + 0x4000 * bank;
		return memory->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
	}
	return 0xff;
}

void FMALU::do_write(uint32 addr, uint32 bank, uint8 data, bool is_400line)
{
	uint32 raddr;

	if(((1 << bank) & read_signal(SIG_DISPLAY_MULTIPAGE)) != 0) return;

	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = addr + 0x8000 * bank;
			memory->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, data);
		}
	} else {
		raddr = addr + 0x4000 * bank;
		memory->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, data);
	}
	return;
}


void FMALU::do_pset(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint32 raddr = addr;  // Use banked ram.
	uint32 offset = 0x4000;
	uint8 bitmask = ~mask_reg;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00;
		}
		srcdata = srcdata & mask_reg;
		srcdata = srcdata | bitmask;
		do_write(addr, i, srcdata, is_400line);
	}
	    
}

void FMALU::do_blank(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		srcdata = srcdata & mask_reg;
		do_write(addr, i, srcdata, is_400line);
	}
	    
}

void FMALU::do_or(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			raddr = raddr + offset;
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		if((color_reg & (1 << i)) == 0) {
			bitmask = srcdata; // srcdata | 0x00
		} else {
			bitmask = 0xff; // srcdata | 0xff
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata, is_400line);
	}
	    
}

void FMALU::do_and(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00; // srcdata & 0x00
		} else {
			bitmask = srcdata; // srcdata & 0xff;
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata, is_400line);
	}
	    
}

void FMALU::do_xor(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		if((color_reg & (1 << i)) == 0) {
			bitmask = srcdata ^ 0x00;
		} else {
			bitmask = srcdata ^ 0xff;
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata, is_400line);
	}
	    
}

void FMALU::do_not(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		bitmask = ~srcdata;
		
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata, is_400line);
	}
}


void FMALU::do_tilepaint(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i, is_400line);
		bitmask = tile_reg[i] & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata, is_400line);
	}
}

void FMALU::do_compare(uint32 addr)
{
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	uint8  planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	uint32 offset = 0x4000;
	uint8 r, g, b, t;
	uint8 disables = ~bank_disable_reg & 0x0f;
	uint8 tmpcol;
	int i;
	int j;
	if(is_400line) offset = 0x8000;

	b = do_read(addr, 0, is_400line);
	r = do_read(addr, 1, is_400line);
	g = do_read(addr, 2, is_400line);
	if(planes >= 4) {
		t = do_read(addr, 3, is_400line);
	} else {
		disables = disables & 0x07;
	}
	cmp_status_reg = 0x00;
	for(i = 7; i >= 0; i--) {
		tmpcol  = (b & 0x80) ? 1 : 0;
		tmpcol |= (r & 0x80) ? 2 : 0;
		tmpcol |= (g & 0x80) ? 4 : 0;
		if(planes >= 4) {
			tmpcol |= (t & 0x80) ? 8 : 0;
		}
		tmpcol = tmpcol & disables;
		for(j = 0; j < 8; j++) {
			if((cmp_status_data[j] & 0x80) != 0) continue;
			if((cmp_status_data[j] & disables) == tmpcol) {
				cmp_status_reg = cmp_status_reg | (1 << i);
				break;
			}
		}
	}
}

void FMALU::do_alucmds(uint32 addr)
{
	if(((command_reg & 0x40) != 0) && ((command_reg & 0x07) != 7)) do_compare(addr); 
	switch(command_reg & 0x07) {
		case 0:
			do_pset(addr);
			break;
		case 1:
			do_blank(addr);
			break;
		case 2:
			do_or(addr);
			break;
		case 3:
			do_and(addr);
			break;
		case 4:
			do_xor(addr);
			break;
		case 5:
			do_not(addr);
			break;
		case 6:
			do_tilepaint(addr);
			break;
		case 7:
			do_compare(addr);
			break;
	}
}

void FMALU::do_line(void)
{
	int x_begin = xbegin_w.l;
	int x_end = xend.w.l;
	int y_begin = ybegin_w.l;
	int y_end = yend.w.l;
	uint32 total_bytes;
	int xx, yy;
	int tmp;
	int width, height;
	uint8 tmp8a, tmp8b;
	pair line_style;
	bool direction = false;
	uint32 screen_width = memory->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	uint32 screen_height = memory->read_signal(SIG_DISPLAY_Y_HEIGHT) * screen_width;
	uint8 lmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
	uint8 rmask[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	

	// SWAP positions by X.
	if(x_begin > x_end) {
		tmp = x_end;
		x_end = x_begin;
		x_begin = tmp;
		
		tmp = y_end;
		y_end = y_begin;
		y_begin = tmp;
	}
	
	width = x_end - x_begin;
	height = y_end - y_begin;
	if(height >= 0) direction = true;

	// Clipping
	if(x_end >= screen_width) {
		int tmpwidth = (screen_width - 1) - x_begin;
		double ratio = (double)tmpwidth / (double)width;
		x_end = screen_width - 1;
		if(direction) {
			y_end = y_begin + (int)(ratio * (double)height);
		} else {
			y_begin = y_end + (int)(ratio * (double)height);
		}
		width = x_end - x_begin;
		height = y_end - y_begin;
	}
	if(x_begin < 0) {
		int tmpwidth = x_end;
		double ratio = (double)tmpwidth / (double)width;
		x_begin = 0;
		if(direction) {
			y_end = y_begin + (int)(ratio * (double)height);
		} else {
			y_begin = y_end + (int)(ratio * (double)height);
		}
		width = x_end - x_begin;
		height = y_end - y_begin;
	}
	if(direction) {
		if(y_end >= screen_height) {
			int tmpheight = (screen_height - 1) - y_begin;
			double ratio = (double)tmpheight / (double)height;
			y_end = screen_height - 1;
			x_end = x_begin + (int)(ratio * (double)width);
			width = x_end - x_begin;
			height = y_end - y_begin;
		}
		if(y_begin < 0) {
			int tmpheight = y_end;
			double ratio = (double)tmpheight / (double)height;
			y_begin = 0;
			x_end = x_begin + (int)(ratio * (double)width);
			width = x_end - x_begin;
			height = y_end - y_begin;
		}
	} else {
		if(y_begin >= screen_height) {
			int tmpheight = (screen_height - 1) - y_end;
			double ratio = (double)tmpheight / (double)height;
			y_begin = screen_height - 1;
			x_end = x_begin + (int)(ratio * (double)width);
			width = x_end - x_begin;
			height = y_end - y_begin;
		}
		if(y_end < 0) {
			int tmpheight = y_begin;
			double ratio = (double)tmpheight / (double)height;
			y_end = 0;
			x_end = x_begin + (int)(ratio * (double)width);
			width = x_end - x_begin;
			height = y_end - y_begin;
		}
	}
	// DO LINE
	line_style = line_pattern;
	if(width == 0) { // VERTICAL
		if(height == 0) {
			return; // NOP?
		}
		if(y_end < y_begin) {
			tmp = y_end;
			y_end = y_begin;
			y_begin = tmp;
		}
		total_bytes = height;
		for(yy = y_begin; yy <= y_end; yy++) {
			put_dot(x_begin, yy, line_style.b.h & 0x80);
			tmp8a = (line.style.b.h & 0x80) >> 7;
			tmp8b = (line.style.b.l & 0x80) >> 7;
			line.style.b.h = (line.style.b.h << 1) | tmp8b;
			line.style.b.l = (line.style.b.l << 1) | tmp8a;
		}
	} else if(height == 0) { // HORIZONAL
		if(width == 0) {
			return; // NOP?
		}
		total_bytes = ((x_begin & 0x07) == 0) ? 1 : 0;
		total_bytes = total_bytes + (x_end >> 3) - (x_begin >> 3);
		for(xx = x_begin; xx <= x_end; xx++) {
			put_dot(xx, y_begin, line_style.b.h & 0x80);
			tmp8a = (line.style.b.h & 0x80) >> 7;
			tmp8b = (line.style.b.l & 0x80) >> 7;
			line.style.b.h = (line.style.b.h << 1) | tmp8b;
			line.style.b.l = (line.style.b.l << 1) | tmp8a;
		}
	} else if(direction) {
		if(height > width) { //
			double ratio = (double)width / (double)height;
			double pratio = 0;
			total_bytes = ((x_begin & 0x07) == 0) ? 1 : 0;
			total_bytes = total_bytes + (x_end >> 3) - (x_begin >> 3);
			
			xx = x_begin;
			for(yy = y_begin; yy <= y_end; yy++) {
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line.style.b.h & 0x80) >> 7;
				tmp8b = (line.style.b.l & 0x80) >> 7;
				line.style.b.h = (line.style.b.h << 1) | tmp8b;
				line.style.b.l = (line.style.b.l << 1) | tmp8a;
				pratio = pratio + ratio;
				if(pratio >= 1.0) {
					pratio -= 1.0;
					xx++;
					put_dot(xx, yy, line_style.b.h & 0x80);
					tmp8a = (line.style.b.h & 0x80) >> 7;
					tmp8b = (line.style.b.l & 0x80) >> 7;
					line.style.b.h = (line.style.b.h << 1) | tmp8b;
					line.style.b.l = (line.style.b.l << 1) | tmp8a;
				}
			}
		} else { // height < width
			double ratio = (double)height / (double)width;
			double pratio = 0;
			total_bytes = ((x_begin & 0x07) == 0) ? 1 : 0;
			total_bytes = total_bytes + (x_end >> 3) - (x_begin >> 3);
			
			yy = y_begin;
			for(xx = x_begin; xx <= x_end; xx++) {
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line.style.b.h & 0x80) >> 7;
				tmp8b = (line.style.b.l & 0x80) >> 7;
				line.style.b.h = (line.style.b.h << 1) | tmp8b;
				line.style.b.l = (line.style.b.l << 1) | tmp8a;
				pratio = pratio + ratio;
				if(pratio >= 1.0) {
					pratio -= 1.0;
					yy++;
					put_dot(xx, yy, line_style.b.h & 0x80);
					tmp8a = (line.style.b.h & 0x80) >> 7;
					tmp8b = (line.style.b.l & 0x80) >> 7;
					line.style.b.h = (line.style.b.h << 1) | tmp8b;
					line.style.b.l = (line.style.b.l << 1) | tmp8a;
				}
			}
		}
	} else { // HEIGHT is negative
		height = -height;
		if(height > width) { //
			double ratio = (double)width / (double)height;
			double pratio = 0;
			total_bytes = ((x_begin & 0x07) == 0) ? 1 : 0;
			total_bytes = total_bytes + (x_end >> 3) - (x_begin >> 3);
			
			xx = x_begin;
			for(yy = y_begin; yy >= y_end; yy--) {
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line.style.b.h & 0x80) >> 7;
				tmp8b = (line.style.b.l & 0x80) >> 7;
				line.style.b.h = (line.style.b.h << 1) | tmp8b;
				line.style.b.l = (line.style.b.l << 1) | tmp8a;
				pratio = pratio + ratio;
				if(pratio >= 1.0) {
					pratio -= 1.0;
					xx++;
					put_dot(xx, yy, line_style.b.h & 0x80);
					tmp8a = (line.style.b.h & 0x80) >> 7;
					tmp8b = (line.style.b.l & 0x80) >> 7;
					line.style.b.h = (line.style.b.h << 1) | tmp8b;
					line.style.b.l = (line.style.b.l << 1) | tmp8a;
				}
			}
		} else { // height < width
			double ratio = (double)height / (double)width;
			double pratio = 0;
			total_bytes = ((x_begin & 0x07) == 0) ? 1 : 0;
			total_bytes = total_bytes + (x_end >> 3) - (x_begin >> 3);
			
			yy = y_begin;
			for(xx = x_begin; xx <= x_end; xx++) {
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line.style.b.h & 0x80) >> 7;
				tmp8b = (line.style.b.l & 0x80) >> 7;
				line.style.b.h = (line.style.b.h << 1) | tmp8b;
				line.style.b.l = (line.style.b.l << 1) | tmp8a;
				pratio = pratio + ratio;
				if(pratio >= 1.0) {
					pratio -= 1.0;
					yy--;
					put_dot(xx, yy, line_style.b.h & 0x80);
					tmp8a = (line.style.b.h & 0x80) >> 7;
					tmp8b = (line.style.b.l & 0x80) >> 7;
					line.style.b.h = (line.style.b.h << 1) | tmp8b;
					line.style.b.l = (line.style.b.l << 1) | tmp8a;
				}
			}
		}
	}	  
}

void FMALU::put_dot(int x, int y, uint8 dot)
{
	uint16 addr;
	uint32 width = memory->read_signal(SIG_DISPLAY_X_WIDTH);
	uint32 height = memory->read_signal(SIG_DISPLAY_Y_HEIGHT);
	uint32 bank_offset = memory->read_signal(SIG_DISPLAY_BANK_OFFSET);
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint8 mask;
	bool is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	
	if((command_reg & 0x80) == 0) return;
	addr = y * width + (x >> 3);
	addr = addr + line_addr_offset.w.l;
	addr = addr & 0x7fff;
	if(!is_400line) addr = addr & 0x3fff;
	
	if((dot & 0x80) != 0) {
		mask = mask_reg;
		mask_reg &= ~vmask[x & 7];
		do_alucmds((uint32) addr);
		mask_reg = mask;
	} else {
		do_alucmds((uint32) addr);
	}	  
}
