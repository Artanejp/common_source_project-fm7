/*
 * FM77AV/FM16β ALU [mb61vh010.cpp]
 *  of Fujitsu MB61VH010/011
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 28, 2015 : Initial
 *
 */

#include "mb61vh010.h"


MB61VH010::MB61VH010(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_emu = parent_emu;
	p_vm = parent_vm;
}

MB61VH010::~MB61VH010()
{
}

uint8 MB61VH010::do_read(uint32 addr, uint32 bank)
{
	uint32 raddr;
	
	if(((1 << bank) & multi_page) != 0) return 0xff;
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = (addr  & 0x7fff) | (0x8000 * bank);
			return target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
		}
	} else {
		raddr = (addr & 0x3fff) | (0x4000 * bank);
		return target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
	}
	return 0xff;
}

uint8 MB61VH010::do_write(uint32 addr, uint32 bank, uint8 data)
{
	uint32 raddr;
	uint8 readdata;

	if(((1 << bank) & multi_page) != 0) return 0xff;
	if((command_reg & 0x40) != 0) { // Calculate before writing
	  	readdata = do_read(addr, bank);
		//readdata = data;
		if((command_reg & 0x20) != 0) { // NAND
			readdata = readdata & cmp_status_reg;
			data = data & (~cmp_status_reg);
		} else { // AND
			readdata = readdata & (~cmp_status_reg);
			data = data & cmp_status_reg;
		}
		readdata = readdata | data;
	} else {
		readdata = data;
	}
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			//raddr = ((addr + (line_addr_offset.w.l << 1)) & 0x7fff) | (0x8000 * bank);
			raddr = (addr & 0x7fff) | (0x8000 * bank);
			target->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, readdata);
		}
	} else {
		//raddr = ((addr + (line_addr_offset.w.l << 1)) & 0x3fff) | (0x4000 * bank);
		raddr = (addr & 0x3fff) | (0x4000 * bank);
		target->write_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS, readdata);
	}
	return readdata;
}


uint8 MB61VH010::do_pset(uint32 addr)
{
	int i;
	uint32 raddr = addr;  // Use banked ram.
	uint8 bitmask;
	uint8 srcdata;
	int planes_b = planes;
	if(planes_b >= 4) planes_b = 4;
	for(i = 0; i < planes_b; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		if((color_reg & (1 << i)) == 0) {
			bitmask = 0x00;
		} else {
			bitmask = 0xff;
		}
		
		srcdata = do_read(addr, i);
		bitmask = bitmask & (~mask_reg);
		srcdata = srcdata & mask_reg;
		srcdata = srcdata | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 MB61VH010::do_blank(uint32 addr)
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

uint8 MB61VH010::do_or(uint32 addr)
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

uint8 MB61VH010::do_and(uint32 addr)
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

uint8 MB61VH010::do_xor(uint32 addr)
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

uint8 MB61VH010::do_not(uint32 addr)
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


uint8 MB61VH010::do_tilepaint(uint32 addr)
{
	uint32 i;
	uint8 bitmask;
	uint8 srcdata;
	//printf("Tilepaint CMD=%02x, ADDR=%04x Planes=%d, disable=%d, tile_reg=(%02x %02x %02x %02x)\n",
	//      command_reg, addr, planes, bank_disable_reg, tile_reg[0], tile_reg[1], tile_reg[2], tile_reg[3]);
	if(planes > 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		bitmask = tile_reg[i] & (~mask_reg);
		srcdata = (srcdata & mask_reg) | bitmask;
		do_write(addr, i, srcdata);
	}
	return 0xff;
}

uint8 MB61VH010::do_compare(uint32 addr)
{
	uint32 offset = 0x4000;
	uint8 r, g, b;
	uint8 disables = ~bank_disable_reg;
	//uint8 disables = bank_disable_reg;
	uint8 tmpcol;
	uint8 tmp_stat = 0;
	uint8 cmp_reg_bak[8];
	int k;
	int i;
	int j;
	
	disables = disables & 0x07;
	k = 0;
	for(j = 0; j < 8; j++) {
		if((cmp_color_data[j] & 0x80) == 0) {
			cmp_reg_bak[k] = cmp_color_data[j] & disables;
			k++;
		}
	}
	cmp_status_reg = 0x00;
	if(k <= 0) return 0xff;
	b = r = g = 0;
	b = do_read(addr, 0);
	r = do_read(addr, 1);
	g = do_read(addr, 2);
	for(i = 0; i < 8; i++) {
		tmp_stat <<= 1;
		tmpcol  = (b & 0x80) >> 7;
		tmpcol  = tmpcol | ((r & 0x80) >> 6);
		tmpcol  = tmpcol | ((g & 0x80) >> 5);
		//tmpcol |= ((t & 0x80) != 0) ? 8 : 0;
		tmpcol = tmpcol & disables;
		for(j = 0; j < k; j++) {
			if(cmp_reg_bak[j] == tmpcol) {
				tmp_stat = tmp_stat | 0x01;
				goto _l0;
			}
		}
_l0:	   
		b <<= 1;
		r <<= 1;
		g <<= 1;
	}
	cmp_status_reg = tmp_stat;
	return 0xff;
}

void MB61VH010::do_alucmds_dmyread(uint32 addr)
{
	if(!is_400line) {
		addr = addr & 0x3fff;
	} else {
		if(addr >= 0x8000) {
			mask_reg = 0xff;
			return;
		}
		addr = addr & 0x7fff;
	}
	//printf("ALU DMYREAD: CMD %02x ADDR=%04x CMP[]=", command_reg, addr);
	//for(i = 0; i < 8; i++) printf("[%02x]", cmp_color_data[i]);
	if((command_reg & 0x80) == 0) {
	  //printf("\n");
		return;
	}
	//if(((command_reg & 0x40) != 0) && ((command_reg & 0x07) != 7)) do_compare(addr);
	if((command_reg & 0x40) != 0) do_compare(addr);
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
	//printf("ALU DMYREAD ADDR=%04x, CMD=%02x CMP STATUS=%02x\n", addr, command_reg, cmp_status_reg);
}  

uint8 MB61VH010::do_alucmds(uint32 addr)
{
	if(!is_400line) {
		addr = addr & 0x3fff;
	} else {
		if(addr >= 0x8000) {
			mask_reg = 0xff;
			return 0xff;
		}
		addr = addr & 0x7fff;
	}
	//if(((command_reg & 0x40) != 0) && ((command_reg & 0x07) != 7)) do_compare(addr);
	if((command_reg & 0x40) != 0) do_compare(addr);
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

void MB61VH010::do_line(void)
{
	int x_begin = line_xbegin.w.l;
	int x_end = line_xend.w.l;
	int y_begin = line_ybegin.w.l;
	int y_end = line_yend.w.l;
	int cpx_t = x_begin;
	int cpy_t = y_begin;
	int ax = x_end - x_begin;
	int ay = y_end - y_begin;
	int diff = 0;
	int count = 0;
	int xcount;
	int ycount;
	uint8 mask_bak = mask_reg;
	uint16 tmp8a;
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	//printf("Line: (%d,%d) - (%d,%d) CMD=%02x\n", x_begin, y_begin, x_end, y_end, command_reg);  
	double usec;
	bool lastflag = false;
	
	//is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	screen_width = target->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	screen_height = target->read_signal(SIG_DISPLAY_Y_HEIGHT);

	//if((command_reg & 0x80) == 0) return;
	oldaddr = 0xffffffff;
	alu_addr = 0xffffffff;

	
	line_style = line_pattern;
	busy_flag = true;
	total_bytes = 1;
	
	//mask_reg = 0xff & ~vmask[x_begin & 7];
	mask_reg = 0xff;
	if((line_style.b.h & 0x80) != 0) {
	  	mask_reg &= ~vmask[cpx_t & 7];
        }
	tmp8a = (line_style.w.l & 0x8000) >> 15;
	line_style.w.l = (line_style.w.l << 1) | tmp8a;
	xcount = abs(ax);
	ycount = abs(ay);
	if(xcount >= ycount) {
		if(xcount != 0) {
			//if(ycount != 0) {
				diff = ((ycount  + 1) * 1024) / xcount;
			//}
			for(; xcount >= 0; xcount-- ) {
				lastflag = put_dot(cpx_t, cpy_t);
				count += diff;
				if(count > 1024) {
					if(ay > 0) {
						lastflag = put_dot(cpx_t, cpy_t + 1);
					} else if(ax < 0) {
						lastflag = put_dot(cpx_t, cpy_t - 1);
					}
					if(ay < 0) {
						cpy_t--;
					} else {
						cpy_t++;
					}
					count -= 1024;
				}
				if(ax > 0) {
					cpx_t++;
				} else if(ax < 0) {
					cpx_t--;
				}
			}
		} else { // ax = ay = 0
			lastflag = put_dot(cpx_t, cpy_t);
			total_bytes++;
		}
	} else { // (abs(ax) < abs(ay)
		//if(xcount != 0) {
			diff = ((xcount + 1) * 1024) / ycount;
		//}
		for(; ycount >= 0; ycount--) {
			lastflag = put_dot(cpx_t, cpy_t);
			count += diff;
			if(count > 1024) {
				if(ax > 0) {
					lastflag = put_dot(cpx_t + 1, cpy_t);
				} else if(ay < 0) {
					lastflag = put_dot(cpx_t - 1, cpy_t);
				} 				  
				if(ax < 0) {
					cpx_t--;
				} else if(ax > 0) {
					cpx_t++;
				}
				count -= 1024;
				total_bytes++;
			}
			if(ay > 0) {
				cpy_t++;
			} else {
				cpy_t--;
			}
			total_bytes++;
		}
	}

	if(!lastflag) total_bytes++;
	do_alucmds(alu_addr);

	if(total_bytes > 8) { // Over 0.5us
		usec = (double)total_bytes / 16.0;
		if(eventid_busy < 0) register_event(this, EVENT_MB61VH010_BUSY_OFF, usec, false, &eventid_busy) ;
	} else {
		busy_flag = false;
	}
	//mask_reg = mask_bak;
	//line_pattern = line_style;
}

bool MB61VH010::put_dot(int x, int y)
{
	uint8 vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint16 tmp8a;
	bool flag = false;
	
	if((x < 0) || (y < 0)) return flag;
	if((x >= screen_width) || (y >= screen_height)) return flag;
	if((command_reg & 0x80) == 0) return flag;
	
	alu_addr = ((y * screen_width) >> 3) + (x >> 3);
	alu_addr = alu_addr + (line_addr_offset.w.l << 1);
	alu_addr = alu_addr & 0x7fff;
	if(!is_400line) alu_addr = alu_addr & 0x3fff;
	if(oldaddr == 0xffffffff) oldaddr = alu_addr;
	
  	if(oldaddr != alu_addr) {
		do_alucmds(oldaddr);
		mask_reg = 0xff;
		oldaddr = alu_addr;
		flag = true;
		total_bytes++;
	}
	
	if((line_style.b.h & 0x80) != 0) {
	  	mask_reg &= ~vmask[x & 7];
        }
	tmp8a = (line_style.w.l & 0x8000) >> 15;
	line_style.w.l = (line_style.w.l << 1) | tmp8a;
	return flag;
}

void MB61VH010::write_data8(uint32 id, uint32 data)
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
			line_addr_offset.b.h = data;
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
			if((id >= (ALU_CMPDATA_REG + 0)) && (id < (ALU_CMPDATA_REG + 8))) {
				cmp_color_data[id - ALU_CMPDATA_REG] = data;
			} else 	if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0x18000))) {
			  //is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
				do_alucmds_dmyread(id - ALU_WRITE_PROXY);
			}
			break;
	}
}

uint32 MB61VH010::read_data8(uint32 id)
{
	uint32 raddr;  
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
				uint32 dmydata;
				raddr = (id - ALU_WRITE_PROXY) & 0xffff;
				//is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
				if(is_400line) raddr = raddr & 0x7fff;
				dmydata = target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
				do_alucmds_dmyread(id - ALU_WRITE_PROXY);
				dmydata = target->read_data8(raddr + DISPLAY_VRAM_DIRECT_ACCESS);
				return dmydata;
			}
			return 0xffffffff;
			break;
	}
}

uint32 MB61VH010::read_signal(int id)
{
	uint32 val = 0x00000000;
	switch(id) {
		case SIG_ALU_BUSYSTAT:
			if(busy_flag) val = 0xffffffff;
			break;
	}
	return val;
}

void MB61VH010::write_signal(int id, uint32 data, uint32 mask)
{
	bool flag = ((data & mask) != 0);
	switch(id) {
		case SIG_ALU_400LINE:
			is_400line = flag;
			break;
		case SIG_ALU_MULTIPAGE:
			multi_page = (data & mask) & 0x07;
			break;
	}
}

void MB61VH010::event_callback(int event_id, int err)
{
	switch(event_id) {
		case EVENT_MB61VH010_BUSY_ON:
			busy_flag = true;
			if(eventid_busy >= 0) cancel_event(this, eventid_busy);
			eventid_busy = -1;
			break;
		case EVENT_MB61VH010_BUSY_OFF:
			busy_flag = false;
			eventid_busy = -1;
			break;
	}
}

void MB61VH010::initialize(void)
{
	busy_flag = false;
	is_400line = false;
	eventid_busy = -1;
	multi_page = 0x00;
}

void MB61VH010::reset(void)
{
	int i;
	busy_flag = false;
	if(eventid_busy >= 0) cancel_event(this, eventid_busy);
	eventid_busy = -1;
	
  	command_reg = 0;        // D410 (RW)
	color_reg = 0;          // D411 (RW)
	mask_reg = 0;           // D412 (RW)
	cmp_status_reg = 0;     // D413 (RO)
	for(i = 0; i < 8; i++) cmp_color_data[i] = 0x80; // D413-D41A (WO)
	bank_disable_reg = 0;   // D41B (RW)
	for(i = 0; i < 4; i++) tile_reg[i] = 0;        // D41C-D41F (WO)
	
	line_addr_offset.d = 0; // D420-D421 (WO)
	line_pattern.d = 0;     // D422-D423 (WO)
	line_xbegin.d = 0;      // D424-D425 (WO)
	line_ybegin.d = 0;      // D426-D427 (WO)
	line_xend.d = 0;        // D428-D429 (WO)
	line_yend.d = 0;        // D42A-D42B (WO)

	oldaddr = 0xffffffff;
	
	planes = target->read_signal(SIG_DISPLAY_PLANES) & 0x07;
	if(planes >= 4) planes = 4;
	//is_400line = (target->read_signal(SIG_DISPLAY_MODE_IS_400LINE) != 0) ? true : false;
	
	screen_width = target->read_signal(SIG_DISPLAY_X_WIDTH) * 8;
	screen_height = target->read_signal(SIG_DISPLAY_Y_HEIGHT);
}
