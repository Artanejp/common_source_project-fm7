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


FMALU::FMALU(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_emu = parent_emu;
	p_vm = parent_vm;
}

FMALU::~FMALU()
{
}

uint8 FMALU::do_read(uint32 addr, uint32 bank)
{
	uint32 raddr;

	if(((1 << bank) & read_signal(SIG_DISPLAY_MULTIPAGE)) != 0) return 0xff;

	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = addr + 0x8000 * bank;
			return target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
		}
		return 0xff;
	} else {
		raddr = addr + 0x4000 * bank;
		return target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
	}
	return 0xff;
}

uint8 FMALU::do_write(uint32 addr, uint32 bank, uint8 data)
{
	uint32 raddr;
	uint8 readdata;
	
	if(((1 << bank) & read_signal(SIG_DISPLAY_MULTIPAGE)) != 0) return 0xff;
	if((command_reg & 0x40) != 0) { // Calculate before writing
		readdata = do_read(addr, bank);
		if((command_reg & 0x20) != 0) { // NAND
			readdata = readdata & cmp_status_reg;
			readdata = readdata | (data & ~cmp_status_reg);
		} else { // AND
			readdata = readdata & ~cmp_status_reg;
			readdata = readdata | (data & cmp_status_reg);
		}
	} else {
		readdata = data;
	}
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = addr + 0x8000 * bank;
			target->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, readdata);
		}
	} else {
		raddr = addr + 0x4000 * bank;
		target->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, readdata);
	}
	return readdata;
}


uint8 FMALU::do_pset(uint32 addr)
{
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
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00;
		}
		srcdata = srcdata & mask_reg;
		srcdata = srcdata | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_blank(uint32 addr)
{
	uint32 i;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		srcdata = srcdata & mask_reg;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_or(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;
	
	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = srcdata; // srcdata | 0x00
		} else {
			bitmask = 0xff; // srcdata | 0xff
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_and(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00; // srcdata & 0x00
		} else {
			bitmask = srcdata; // srcdata & 0xff;
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_xor(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask = srcdata ^ 0x00;
		} else {
			bitmask = srcdata ^ 0xff;
		}
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_not(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		bitmask = ~srcdata;
		
		bitmask = bitmask & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}


uint8 FMALU::do_tilepaint(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		bitmask = tile_reg[i] & ~mask_reg;
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 FMALU::do_compare(uint32 addr)
{
	uint32 offset = 0x4000;
	uint8 r, g, b, t;
	uint8 disables = ~bank_disable_reg & 0x0f;
	uint8 tmpcol;
	int i;
	int j;
	if(is_400line) offset = 0x8000;

	b = do_read(addr, 0);
	r = do_read(addr, 1);
	g = do_read(addr, 2);
	if(planes >= 4) {
		t = do_read(addr, 3);
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
			if((cmp_color_data[j] & 0x80) != 0) continue;
			if((cmp_color_data[j] & disables) == tmpcol) {
				cmp_status_reg = cmp_status_reg | (1 << i);
				break;
			}
		}
	}
	return 0xff;
}

uint8 FMALU::do_alucmds(uint32 addr)
{
	if(((command_reg & 0x40) != 0) && ((command_reg & 0x07) != 7)) do_compare(addr); 
	//printf("ALU: CMD %02x ADDR=%08x\n", command_reg, addr);
	switch(command_reg & 0x07) {
		case 0:
			return do_pset(addr);
			break;
		case 1:
			return do_blank(addr);
			break;
		case 2:
			return do_or(addr);
			break;
		case 3:
			return do_and(addr);
			break;
		case 4:
			return do_xor(addr);
			break;
		case 5:
			return do_not(addr);
			break;
		case 6:
			return do_tilepaint(addr);
			break;
		case 7:
			return do_compare(addr);
			break;
	}
	return 0xff;
}

void FMALU::do_line(void)
{
	int x_begin = line_xbegin.w.l;
	int x_end = line_xend.w.l;
	int y_begin = line_ybegin.w.l;
	int y_end = line_yend.w.l;
	uint32 total_bytes;
	int xx, yy;
	int delta;
	int tmp;
	int width, height;
	int count;
	uint8 tmp8a, tmp8b;
	pair line_style;
	bool direction = false;
	uint8 lmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
	uint8 rmask[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	double usec;
	is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	screen_width = target->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	screen_height = target->read_signal(SIG_DISPLAY_Y_HEIGHT);

	//if((command_reg & 0x80) == 0) return;
	// SWAP positions by X.
	if(x_begin > x_end) {
		tmp = x_end;
		x_end = x_begin;
		x_begin = tmp;
		
		tmp = y_end;
		y_end = y_begin;
		y_begin = tmp;
	}
	printf("ALU: write line (%d, %d) - (%d, %d)\n", x_begin, y_begin,
		x_end, y_end);
	
	width = x_end - x_begin;
	height = y_end - y_begin;
	if(height >= 0) {
		direction = true;
	} else {
		height = -height;
		direction = false;
	}
	// Clipping
#if 0
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
#endif
	// DO LINE
	
	//line_style = line_pattern;
	line_style.w.l = 0xffff;
	if(width == 0) { // VERTICAL
		if(height == 0) {
			return; // NOP?
		}
		busy_flag = true;
		if(y_end < y_begin) {
			tmp = y_end;
			y_end = y_begin;
			y_begin = tmp;
		}
		total_bytes = height;
		for(yy = y_begin; yy <= y_end; yy++) {
			put_dot(x_begin, yy, line_style.b.h & 0x80);
			tmp8a = (line_style.b.h & 0x80) >> 7;
			tmp8b = (line_style.b.l & 0x80) >> 7;
			line_style.b.h = (line_style.b.h << 1) | tmp8b;
			line_style.b.l = (line_style.b.l << 1) | tmp8a;
		}
	} else if(height == 0) { // HORIZONAL
		if(width == 0) {
			return; // NOP?
		}
		busy_flag = true;
		total_bytes = ((x_begin & 0x07) == 0) ? 1 : 0;
		total_bytes = total_bytes + (x_end >> 3) - (x_begin >> 3);
		for(xx = x_begin; xx <= x_end; xx++) {
			put_dot(xx, y_begin, line_style.b.h & 0x80);
			tmp8a = (line_style.b.h & 0x80) >> 7;
			tmp8b = (line_style.b.l & 0x80) >> 7;
			line_style.b.h = (line_style.b.h << 1) | tmp8b;
			line_style.b.l = (line_style.b.l << 1) | tmp8a;
		}
	} else if(height == width) {
		if(direction) {
			delta = 1;
		} else {
			delta = -1;
		}
		yy = y_begin;
		busy_flag = true;
		total_bytes = (height / 8) + ((height % 8) == 0) ? 0 : 1;
		for(xx = x_begin; xx <= x_end; xx++) {
			put_dot(xx, yy, line_style.b.h & 0x80);
			tmp8a = (line_style.b.h & 0x80) >> 7;
			tmp8b = (line_style.b.l & 0x80) >> 7;
			line_style.b.h = (line_style.b.h << 1) | tmp8b;
			line_style.b.l = (line_style.b.l << 1) | tmp8a;
			yy += delta;
		}
	} else if(height > width) {
		delta = (256 * width) / height;
		count = 0;
		busy_flag = true;
		total_bytes = (height / 8) + ((height % 8) == 0) ? 0 : 1;
		if(direction) {
			xx = x_begin;
			for(yy = y_begin; yy <= y_end; yy++){
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line_style.b.h & 0x80) >> 7;
				tmp8b = (line_style.b.l & 0x80) >> 7;
				line_style.b.h = (line_style.b.h << 1) | tmp8b;
				line_style.b.l = (line_style.b.l << 1) | tmp8a;
				count += delta;
				if(count >= 256) {
					count -= 256;
					xx++;
				}
			}
		} else {
			xx = x_begin;
			for(yy = y_begin; yy >= y_end; yy--){
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line_style.b.h & 0x80) >> 7;
				tmp8b = (line_style.b.l & 0x80) >> 7;
				line_style.b.h = (line_style.b.h << 1) | tmp8b;
				line_style.b.l = (line_style.b.l << 1) | tmp8a;
				count += delta;
				if(count >= 256) {
					count -= 256;
					xx++;
				}
			}
		}		  
	} else {
		busy_flag = true;
		total_bytes = (width / 8) + ((width % 8) == 0) ? 0 : 1;
		delta = (256 * height) / width;
		count = 0;
		if(direction) {
			yy = y_begin;
			for(xx = x_begin; xx <= x_end; xx++){
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line_style.b.h & 0x80) >> 7;
				tmp8b = (line_style.b.l & 0x80) >> 7;
				line_style.b.h = (line_style.b.h << 1) | tmp8b;
				line_style.b.l = (line_style.b.l << 1) | tmp8a;
				count += delta;
				if(count >= 256) {
					count -= 256;
					yy++;
				}
			}
		} else {
			yy = y_begin;
			for(xx = x_begin; xx <= x_end; xx++){
				put_dot(xx, yy, line_style.b.h & 0x80);
				tmp8a = (line_style.b.h & 0x80) >> 7;
				tmp8b = (line_style.b.l & 0x80) >> 7;
				line_style.b.h = (line_style.b.h << 1) | tmp8b;
				line_style.b.l = (line_style.b.l << 1) | tmp8a;
				count += delta;
				if(count >= 256) {
					count -= 256;
					yy--;
				}
			}
		}		  
	  }
	  
	usec = (double)total_bytes / 16.0;
	if(usec >= 1.0) {
		register_event(this, EVENT_FMALU_BUSY_OFF, usec, false, &eventid_busy) ;
	} else {
       		busy_flag = false;
	}
}

void FMALU::put_dot(int x, int y, uint8 dot)
{
	uint16 addr;
	uint32 bank_offset = target->read_signal(SIG_DISPLAY_BANK_OFFSET);
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint8 mask;
	
	addr = ((y * screen_width) >> 3) + (x >> 3);
	addr = addr + line_addr_offset.w.l;
	addr = addr & 0x7fff;
	if(!is_400line) addr = addr & 0x3fff;
	
	if((dot & 0x80) != 0) {
		mask = mask_reg;
	  	mask_reg &= ~vmask[x & 7];
	  	do_alucmds((uint32) addr);
	  	mask_reg = mask;
	} else {
		mask = mask_reg;
	  	mask_reg |= vmask[x & 7];
	  	do_alucmds((uint32) addr);
	  	mask_reg = mask;
	}
	//do_pset(addr);
}

void FMALU::write_data8(uint32 id, uint32 data)
{
	//printf("ALU: ADDR=%02x DATA=%02x\n", id, data);
	if(id == ALU_CMDREG) {
		command_reg = data;
		return;
	}
	//if((command_reg & 0x80) == 0) return;
	switch(id) {
		case ALU_LOGICAL_COLOR:
			color_reg = data;
			break;
		case ALU_WRITE_MASKREG:
			mask_reg = data;
			break;
		case ALU_BANK_DISABLE:
			bank_disable_reg = data;
			break;
		case ALU_TILEPAINT_B:
			tile_reg[0] = data;
			break;
		case ALU_TILEPAINT_R:
			tile_reg[1] = data;
			break;
		case ALU_TILEPAINT_G:
			tile_reg[2] = data;
			break;
		case ALU_TILEPAINT_L:
			tile_reg[3] = data;
			break;
		case ALU_OFFSET_REG_HIGH:
			is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
			if(is_400line) {
				line_addr_offset.b.h = data & 0x3f;
			} else {
				line_addr_offset.b.h = data & 0x1f;
			}
			break;
		case ALU_OFFSET_REG_LO:
			line_addr_offset.b.l = data;
			break;
		case ALU_LINEPATTERN_REG_HIGH:
			line_pattern.b.h = data;
			break;
		case ALU_LINEPATTERN_REG_LO:
			line_pattern.b.l = data;
			break;
		case ALU_LINEPOS_START_X_HIGH:
			line_xbegin.b.h = data;
			break;
		case ALU_LINEPOS_START_X_LOW:  
			line_xbegin.b.l = data;
			break;
		case ALU_LINEPOS_START_Y_HIGH:
			line_ybegin.b.h = data;
			break;
		case ALU_LINEPOS_START_Y_LOW:  
			line_ybegin.b.l = data;
			break;
		case ALU_LINEPOS_END_X_HIGH:
			line_xend.b.h = data;
			break;
		case ALU_LINEPOS_END_X_LOW:  
			line_xend.b.l = data;
			break;
		case ALU_LINEPOS_END_Y_HIGH:
			line_yend.b.h = data;
			break;
		case ALU_LINEPOS_END_Y_LOW:
			line_yend.b.l = data;
			do_line();
			break;
		default:
			if((id >= ALU_CMPDATA_REG + 0) && (id < ALU_CMPDATA_REG + 8)) {
				cmp_color_data[id - ALU_CMPDATA_REG] = data;
			} else 	if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0x18000))) {
				is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
				planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
				do_alucmds(id - ALU_WRITE_PROXY);
			}
			break;
	}
}

uint32 FMALU::read_data8(uint32 id)
{
  //if((command_reg & 0x80) == 0) return 0xff;
  
	switch(id) {
		case ALU_CMDREG:
			return (uint32)command_reg;
			break;
		case ALU_LOGICAL_COLOR:
			return (uint32)color_reg;
			break;
		case ALU_WRITE_MASKREG:
			return (uint32)mask_reg;
			break;
		case ALU_CMP_STATUS_REG:
			return (uint32)cmp_status_reg;
			break;
		case ALU_BANK_DISABLE:
			return (uint32)bank_disable_reg;
			break;
		default:
			if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0x18000))) {
				return do_alucmds(id - ALU_WRITE_PROXY);
			}
			return 0xffffffff;
			break;
	}
}

uint32 FMALU::read_signal(int id)
{
	uint32 val = 0x00000000;
	switch(id) {
		case SIG_ALU_BUSYSTAT:
			if(busy_flag) val = 0xffffffff;
			break;
	}
	return val;
}

void FMALU::event_callback(int event_id, int err)
{
	switch(event_id) {
		case EVENT_FMALU_BUSY_ON:
			busy_flag = true;
			if(eventid_busy >= 0) cancel_event(this, eventid_busy);
			eventid_busy = -1;
			break;
		case EVENT_FMALU_BUSY_OFF:
			busy_flag = false;
			eventid_busy = -1;
			break;
	}
}

void FMALU::initialize(void)
{
	busy_flag = false;
	eventid_busy = -1;
}

void FMALU::reset(void)
{
	int i;
	busy_flag = false;
	if(eventid_busy >= 0) cancel_event(this, eventid_busy);
	eventid_busy = -1;
	
  	command_reg = 0;        // D410 (RW)
	color_reg = 0;          // D411 (RW)
	mask_reg = 0;           // D412 (RW)
	cmp_status_reg = 0;     // D413 (RO)
	for(i = 0; i < 8; i++) cmp_color_data[i] = 0; // D413-D41A (WO)
	bank_disable_reg = 0;   // D41B (RW)
	for(i = 0; i < 4; i++) tile_reg[i] = 0;        // D41C-D41F (WO)
	
	line_addr_offset.d = 0; // D420-D421 (WO)
	line_pattern.d = 0;     // D422-D423 (WO)
	line_xbegin.d = 0;      // D424-D425 (WO)
	line_ybegin.d = 0;      // D426-D427 (WO)
	line_xend.d = 0;        // D428-D429 (WO)
	line_yend.d = 0;        // D42A-D42B (WO)
	
	planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	
	screen_width = target->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	screen_height = target->read_signal(SIG_DISPLAY_Y_HEIGHT);
}
