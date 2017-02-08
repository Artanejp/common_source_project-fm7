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

uint8_t MB61VH010::do_read(uint32_t addr, uint32_t bank)
{
	uint32_t raddr;
	
	if(((1 << bank) & multi_page) != 0) return 0xff;
	if(is_400line) {
		if((addr & 0xffff) < 0x8000) {
			raddr = (addr  & 0x7fff) | (0x8000 * bank);
			return target->read_data8(raddr + direct_access_offset);
		}
	} else {
		raddr = (addr & 0x3fff) | (0x4000 * bank);
		return target->read_data8(raddr + direct_access_offset);
	}
	return 0xff;
}

void MB61VH010::do_write(uint32_t addr, uint32_t bank, uint8_t data)
{
	uint32_t raddr;
	uint8_t readdata;

	if(((1 << bank) & multi_page) != 0) return;
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
			raddr = (addr & 0x7fff) | (0x8000 * bank);
			target->write_data8(raddr + direct_access_offset, readdata);
		}
	} else {
		raddr = (addr & 0x3fff) | (0x4000 * bank);
		target->write_data8(raddr + direct_access_offset, readdata);
	}
	return;
}


void MB61VH010::do_pset(uint32_t addr)
{
	int i;
	uint32_t raddr = addr;  // Use banked ram.
	uint8_t bitmask[4] = {0};
	uint8_t srcdata[4] = {0};
	uint8_t mask_p[4] = {mask_reg, mask_reg, mask_reg, mask_reg};
	int planes_b = planes;
	
	if(planes_b >= 4) planes_b = 4;
	for(i = 0; i < planes_b; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		if((color_reg & (1 << i)) == 0) {
			bitmask[i] = 0x00;
		} else {
			bitmask[i] = ~mask_reg;
		}
		//srcdata[i] = do_read(addr, i) & mask_reg;
		srcdata[i] = do_read(addr, i);
	}
	srcdata[0] = (srcdata[0] & mask_p[0]) | bitmask[0];
	srcdata[1] = (srcdata[1] & mask_p[1]) | bitmask[1];
	srcdata[2] = (srcdata[2] & mask_p[2]) | bitmask[2];
	srcdata[3] = (srcdata[3] & mask_p[3]) | bitmask[3];

	for(i = 0; i < planes_b; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		do_write(addr, i, srcdata[i]);
	}
	return;
}

void MB61VH010::do_blank(uint32_t addr)
{
	uint32_t i;
	uint8_t srcdata;

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata = do_read(addr, i);
		srcdata = srcdata & mask_reg;
		do_write(addr, i, srcdata);
	}
	return;
}

void MB61VH010::do_or(uint32_t addr)
{
	uint32_t i;
	uint8_t bitmask[4] = {0};
	uint8_t srcdata[4] = {0};
	uint8_t mask_p[4] = {mask_reg, mask_reg, mask_reg, mask_reg};
	uint8_t mask_n[4] = {(uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg};
	
	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata[i] = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask[i] = srcdata[i]; // srcdata | 0x00
		} else {
			bitmask[i] = 0xff; // srcdata | 0xff
		}
		//bitmask = bitmask & ~mask_reg;
		//srcdata = (srcdata & mask_reg) | bitmask;
		//do_write(addr, i, srcdata);
	}
	srcdata[0] = (srcdata[0] & mask_p[0]) | (bitmask[0] & mask_n[0]);
	srcdata[1] = (srcdata[1] & mask_p[1]) | (bitmask[1] & mask_n[1]);
	srcdata[2] = (srcdata[2] & mask_p[2]) | (bitmask[2] & mask_n[2]);
	srcdata[3] = (srcdata[3] & mask_p[3]) | (bitmask[3] & mask_n[3]);
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		do_write(addr, i, srcdata[i]);
	}
	return;
}

void MB61VH010::do_and(uint32_t addr)
{
	uint32_t i;
	uint8_t bitmask[4] = {0};
	uint8_t srcdata[4] = {0};
	uint8_t mask_p[4] = {mask_reg, mask_reg, mask_reg, mask_reg};
	uint8_t mask_n[4] = {(uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg};

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata[i] = do_read(addr, i);
		if((color_reg & (1 << i)) == 0) {
			bitmask[i] = 0x00; // srcdata & 0x00
		} else {
			bitmask[i] = srcdata[i]; // srcdata & 0xff;
		}
		//bitmask = bitmask & ~mask_reg;
		//srcdata = (srcdata & mask_reg) | bitmask;
		//do_write(addr, i, srcdata);
	}
	srcdata[0] = (srcdata[0] & mask_p[0]) | (bitmask[0] & mask_n[0]);
	srcdata[1] = (srcdata[1] & mask_p[1]) | (bitmask[1] & mask_n[1]);
	srcdata[2] = (srcdata[2] & mask_p[2]) | (bitmask[2] & mask_n[2]);
	srcdata[3] = (srcdata[3] & mask_p[3]) | (bitmask[3] & mask_n[3]);
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		do_write(addr, i, srcdata[i]);
	}
	return;
}

void MB61VH010::do_xor(uint32_t addr)
{
	uint32_t i;
	//uint8_t bitmask;
	//uint8_t srcdata;
	uint8_t bitmask[4] = {0};
	uint8_t srcdata[4] = {0};
	uint8_t mask_p[4] = {mask_reg, mask_reg, mask_reg, mask_reg};
	uint8_t mask_n[4] = {(uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg};

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		if((color_reg & (1 << i)) == 0) {
			bitmask[i] = 0x00;
		} else {
			bitmask[i] = 0xff;
		}
		
		srcdata[i] = do_read(addr, i);
		//bitmask = bitmask ^ srcdata;
		//bitmask = bitmask & (~mask_reg);
		//srcdata = srcdata & mask_reg;
		//srcdata = srcdata | bitmask;
		//do_write(addr, i, srcdata);
	}
	bitmask[0] = bitmask[0] ^ srcdata[0];
	bitmask[1] = bitmask[1] ^ srcdata[1];
	bitmask[2] = bitmask[2] ^ srcdata[2];
	bitmask[3] = bitmask[3] ^ srcdata[3];
	
	srcdata[0] = (srcdata[0] & mask_p[0]) | (bitmask[0] & mask_n[0]);
	srcdata[1] = (srcdata[1] & mask_p[1]) | (bitmask[1] & mask_n[1]);
	srcdata[2] = (srcdata[2] & mask_p[2]) | (bitmask[2] & mask_n[2]);
	srcdata[3] = (srcdata[3] & mask_p[3]) | (bitmask[3] & mask_n[3]);
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		do_write(addr, i, srcdata[i]);
	}
	return;
}

void MB61VH010::do_not(uint32_t addr)
{
	uint32_t i;
	uint8_t bitmask[4] = {0};
	uint8_t srcdata[4] = {0};
	uint8_t mask_p[4] = {mask_reg, mask_reg, mask_reg, mask_reg};
	uint8_t mask_n[4] = {(uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg};

	if(planes >= 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata[i] = do_read(addr, i);
		bitmask[i] = ~srcdata[i];
		//bitmask = bitmask & ~mask_reg;
		//srcdata = (srcdata & mask_reg) | bitmask;
		//do_write(addr, i, srcdata);
	}
	srcdata[0] = (srcdata[0] & mask_p[0]) | (bitmask[0] & mask_n[0]);
	srcdata[1] = (srcdata[1] & mask_p[1]) | (bitmask[1] & mask_n[1]);
	srcdata[2] = (srcdata[2] & mask_p[2]) | (bitmask[2] & mask_n[2]);
	srcdata[3] = (srcdata[3] & mask_p[3]) | (bitmask[3] & mask_n[3]);
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		do_write(addr, i, srcdata[i]);
	}
	return;
}


void MB61VH010::do_tilepaint(uint32_t addr)
{
	uint32_t i;
	uint8_t bitmask[4] = {0};
	uint8_t srcdata[4] = {0};
	uint8_t mask_p[4] = {mask_reg, mask_reg, mask_reg, mask_reg};
	uint8_t mask_n[4] = {(uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg, (uint8_t)~mask_reg};

	if(planes > 4) planes = 4;
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		srcdata[i] = do_read(addr, i);
		bitmask[i] = tile_reg[i];
		//bitmask = tile_reg[i] & (~mask_reg);
		//srcdata = (srcdata & mask_reg) | bitmask;
		//do_write(addr, i, srcdata);
	}
	srcdata[0] = (srcdata[0] & mask_p[0]) | (bitmask[0] & mask_n[0]);
	srcdata[1] = (srcdata[1] & mask_p[1]) | (bitmask[1] & mask_n[1]);
	srcdata[2] = (srcdata[2] & mask_p[2]) | (bitmask[2] & mask_n[2]);
	srcdata[3] = (srcdata[3] & mask_p[3]) | (bitmask[3] & mask_n[3]);
	for(i = 0; i < planes; i++) {
		if((bank_disable_reg & (1 << i)) != 0) {
			continue;
		}
		do_write(addr, i, srcdata[i]);
	}
	return;
}

void MB61VH010::do_compare(uint32_t addr)
{
	uint32_t offset = 0x4000;
	uint8_t r, g, b;
	uint8_t disables = ~bank_disable_reg;

	uint8_t tmpcol;
	uint8_t tmp_stat = 0;
	uint8_t cmp_reg_bak[8];
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
	if(k <= 0) return;
	b = do_read(addr, 0);
	r = do_read(addr, 1);
	g = do_read(addr, 2);
	for(i = 0; i < 8; i++) {
		tmp_stat <<= 1;
		tmpcol  = (b & 0x80) >> 7;
		tmpcol  = tmpcol | ((r & 0x80) >> 6);
		tmpcol  = tmpcol | ((g & 0x80) >> 5);
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
	return;
}

void MB61VH010::do_alucmds_dmyread(uint32_t addr)
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
	if((command_reg & 0x80) == 0) {
		return;
	}
	//busy_flag = true;
	//cmp_status_reg = 0x00;
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
	//printf("ALU DMYREAD ADDR=%04x, CMD=%02x CMP STATUS=%02x DISABLE=%01x\n", addr, command_reg, cmp_status_reg, bank_disable_reg);
	//if(eventid_busy >= 0) cancel_event(this, eventid_busy) ;
	//register_event(this, EVENT_MB61VH010_BUSY_OFF, 1.0 / 16.0, false, &eventid_busy) ;
}  

void MB61VH010::do_alucmds(uint32_t addr)
{
	if(addr >= 0x8000) {
		mask_reg = 0xff;
		return;
	}

	//cmp_status_reg = 0xff;
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
	//printf("ALU CMDS ADDR=%04x, CMD=%02x CMP STATUS=%02x\n", addr, command_reg, cmp_status_reg);
	return;
}

void MB61VH010::do_line(void)
{
	uint32_t x_begin = line_xbegin.w.l;
	uint32_t x_end = line_xend.w.l;
	uint32_t y_begin = line_ybegin.w.l;
	uint32_t y_end = line_yend.w.l;
	int cpx_t = (int)x_begin;
	int cpy_t = (int)y_begin;
	int ax = (int)x_end - (int)x_begin;
	int ay = (int)y_end - (int)y_begin;
	int diff = 0;
	int diff8 = 0;
	int count = 0;
	int xcount;
	int ycount;
	bool updated = false;
	uint16_t tmp8a;
	uint8_t vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	double usec;
	oldaddr = 0xffffffff;
	alu_addr = 0xffffffff;
	line_style = line_pattern;
	busy_flag = true;
	total_bytes = 0;
	
	mask_reg = 0xff;
	if ((line_style.b.h & 0x80) != 0) {
		mask_reg &= ~vmask[cpx_t & 7];
	}
	xcount = abs(ax);
	ycount = abs(ay);
	//this->out_debug_log("LINE: (%d,%d)-(%d,%d) CMD=%02x STYLE=%04x, \n", x_begin, y_begin, x_end , y_end, command_reg, line_style.w.l); 
	if(ycount == 0) {
		if((cpy_t < 0) || (cpy_t >= 512)) goto _finish;
		if(xcount == 0) {
			updated = put_dot(cpx_t, cpy_t);
			goto _finish;
		}
		if(ax > 0) {
			int left = x_end - cpx_t;
			if((cpx_t & 0x07) != 7) total_bytes = 1;
			for(; cpx_t <= (int)x_end;) {
				if((cpx_t & 7) == 0) {
					if(left >= 8) {
						updated = put_dot8(cpx_t, cpy_t);
						cpx_t += 8;
						left -= 8;
						total_bytes++;
						continue;
					}
				}
				{
					updated = put_dot(cpx_t, cpy_t);
					cpx_t++;
					left--;
					if((cpx_t & 0x07) == 7) total_bytes++;
				}
			}
		} else {
			int left = cpx_t - x_end;
			if((cpx_t & 0x07) != 0) total_bytes = 1;
			for(; cpx_t >= (int)x_end;) {
				if(cpx_t < 0) break; // Comment out for Amnork.
				if((cpx_t & 7) == 7) {
					if(left >= 8) {
						updated = put_dot8(cpx_t, cpy_t);
						cpx_t -= 8;
						left -= 8;
						total_bytes++;
						continue;
					}
				}
				{
					if((cpx_t & 7) == 0) total_bytes++;
					updated = put_dot(cpx_t, cpy_t);
					left--;
					cpx_t--;
				}
			}
		}
	} else if(xcount == 0) {
		//if((cpx_t < 0) || (cpx_t >= screen_width)) goto _finish; // Okay?
		if(cpx_t < 0) goto _finish; // Okay?
		if(ay > 0) {
			for(; cpy_t <= (int)y_end; cpy_t++) {
				if(cpy_t >= 512) break;
				updated = put_dot(cpx_t, cpy_t);
				total_bytes++;
			}
		} else {
			for(; cpy_t  >= (int)y_end; cpy_t--) {
				if(cpy_t < 0) break;
				updated = put_dot(cpx_t, cpy_t);
				total_bytes++;
			}
		}
	} else if(xcount > ycount) { // (abs(ax) > abs(ay)
		diff = (ycount * 32768) / xcount;
		diff8 = diff << 3;
		if(ax < 0) {
			if((cpx_t & 0x07) != 0) total_bytes = 1;
		} else {
			if((cpx_t & 0x07) == 0) total_bytes = 1;
		}
		for(; xcount >= 0; ) {
			if((diff8 + count) <= 16384) {
				if(ax > 0) {
					if((cpx_t & 0x07) == 0) {
						if(xcount >= 8) {
							updated = put_dot8(cpx_t, cpy_t);
							total_bytes++;
							count += diff8;
							xcount -= 8;
							cpx_t += 8;
							continue;
						}
					}
				} else { // ax < 0
					if((cpx_t & 0x07) == 7) {
						if(xcount >= 8) {
							updated = put_dot8(cpx_t, cpy_t);
							total_bytes++;
							count += diff8;
							xcount -= 8;
							cpx_t -= 8;
							if(cpx_t < 0) break;
							continue;
						}
					}
				}
			}
			updated = put_dot(cpx_t, cpy_t);
			count += diff;
			if(count > 16384) {
				if(ay < 0) {
					cpy_t--;
					if(cpy_t < 0) break;
				} else {
					cpy_t++;
					if(cpy_t >= 512) break;
				}
				total_bytes++;
				count -= 32768;
			}
			if(ax > 0) {
				cpx_t++;
				if((cpx_t & 0x07) == 0) total_bytes++;
			} else if(ax < 0) {
				if((cpx_t & 0x07) == 0) total_bytes++;
				cpx_t--;
				if(cpx_t < 0) break; // Comment out for Amnork.
			}
			xcount--;
		}
	} else if(xcount == ycount) { // (abs(ax) == abs(ay)
		diff = (ycount * 32768) / xcount;
		if(ax < 0) {
			if((cpx_t & 0x07) != 0) total_bytes = 1;
		} else {
			if((cpx_t & 0x07) == 0) total_bytes = 1;
		}
		for(; xcount >= 0; xcount-- ) {
			updated = put_dot(cpx_t, cpy_t);
			if(ay < 0) {
				cpy_t--;
				if(cpy_t < 0) break;
			} else {
				cpy_t++;
				if(cpy_t >= 512) break;
			}
			total_bytes++;
			if(ax > 0) {
				cpx_t++;
			} else if(ax < 0) {
				cpx_t--;
				if(cpx_t < 0) break; // Comment out for Amnork.
			}
		}
	} else { // (abs(ax) < abs(ay)
		diff = (xcount  * 32768) / ycount;
		for(; ycount >= 0; ycount--) {
			updated = put_dot(cpx_t, cpy_t);
			total_bytes++;
			count += diff;
			if(count > 16384) {
				if(ax < 0) {
					cpx_t--;
					if(cpx_t < 0) break; // Comment out for Amnork.
				} else if(ax > 0) {
					cpx_t++;
				}
				count -= 32768;
			}
			if(ay > 0) {
				cpy_t++;
				if(cpy_t >= 512) {
					break;
				}
			} else {
				cpy_t--;
				if(cpy_t < 0) {
					break;
				}
			}
		}
	}
	// last byte
	do_alucmds(alu_addr);
	total_bytes++;
_finish:   
	//if(total_bytes == 0) total_bytes = 1;
	mask_reg = 0xff;
	if(total_bytes >= 8) { // Only greater than 8bytes.
		usec = (double)total_bytes / 16.0;
		if(eventid_busy >= 0) cancel_event(this, eventid_busy) ;
		register_event(this, EVENT_MB61VH010_BUSY_OFF, usec, false, &eventid_busy);
	} else {
		busy_flag = false;
	}
}

inline bool MB61VH010::put_dot(int x, int y)
{
	uint8_t vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint16_t tmp16a;
	uint8_t tmp8a;
	uint8_t mask8;
	
	bool updated;
   
	if((command_reg & 0x80) == 0) return false; // Not compare.
	if((x < 0) || (y < 0)) {
		return false; // Lower address
	}
   
	//if(y >= (int)screen_height) return; // Workaround of overflow
	
	alu_addr = (y * screen_width + x)  >> 3;
	alu_addr = alu_addr + line_addr_offset.w.l;
	if(!is_400line) {
		alu_addr = alu_addr & 0x3fff;
	} else {
		alu_addr = alu_addr & 0x7fff;
	}
	
	mask8 = ~vmask[x & 7];
	updated = false;
	tmp8a = line_style.b.h & 0x80;
	
  	if(oldaddr != alu_addr) {
		if(oldaddr == 0xffffffff) {
			if(tmp8a != 0) {
				mask_reg &= mask8;
			}
			oldaddr = alu_addr;
		}
		do_alucmds(oldaddr);
		mask_reg = 0xff;
		oldaddr = alu_addr;
		updated = true;
	}
	if(tmp8a != 0) {
	  	mask_reg &= mask8;
	}
	line_style.w.l <<= 1;
	if(tmp8a != 0) line_style.w.l |= 0x01; 
	return updated;
}

inline bool MB61VH010::put_dot8(int x, int y)
{
	uint8_t vmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	uint8_t tmp8a;
	int xx;
	uint16_t tmp16a;
	bool updated;
   
	if((command_reg & 0x80) == 0) return false; // Not compare.
	if((x < 0) || (y < 0)) {
		return false; // Lower address
	}
   
	//if(y >= (int)screen_height) return; // Workaround of overflow
	
	alu_addr = (y * screen_width + x)  >> 3;
	alu_addr = alu_addr + line_addr_offset.w.l;
	if(!is_400line) {
		alu_addr = alu_addr & 0x3fff;
	} else {
		alu_addr = alu_addr & 0x7fff;
	}
	updated = false;
	if(oldaddr != alu_addr) {
		if(oldaddr == 0xffffffff) {
			if((line_style.b.h & 0x80) != 0) {
				mask_reg &= ~vmask[x & 7];
			}
			oldaddr = alu_addr;
		}
		do_alucmds(oldaddr);
		mask_reg = 0xff;
		oldaddr = alu_addr;
		updated = true;
	}
	tmp8a = line_style.b.h;
	mask_reg = mask_reg & ~tmp8a;
	tmp8a = line_style.b.l;
	line_style.b.l = line_style.b.h;
	line_style.b.h = tmp8a;
	return updated;
}

void MB61VH010::write_data8(uint32_t id, uint32_t data)
{
	//this->out_debug_log("ALU: ADDR=%02x DATA=%02x\n", id, data);
	if(id == ALU_CMDREG) {
		command_reg = data;
		return;
	}
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
			line_addr_offset.b.h &= 0x01;
			line_addr_offset.b.h = line_addr_offset.b.h | ((data << 1) & 0x3e);
			break;
		case ALU_OFFSET_REG_LO:
			line_addr_offset.b.l = data << 1;
			line_addr_offset.b.h &= 0xfe;
			line_addr_offset.b.h = line_addr_offset.b.h | ((data >> 7) & 0x01);
			break;
		case ALU_LINEPATTERN_REG_HIGH:
			line_pattern.b.h = data;
			break;
		case ALU_LINEPATTERN_REG_LO:
			line_pattern.b.l = data;
			break;
		case ALU_LINEPOS_START_X_HIGH:
			line_xbegin.b.h = data & 0x03;
			break;
		case ALU_LINEPOS_START_X_LOW:  
			line_xbegin.b.l = data;
			break;
		case ALU_LINEPOS_START_Y_HIGH:
			line_ybegin.b.h = data & 0x01;
			break;
		case ALU_LINEPOS_START_Y_LOW:  
			line_ybegin.b.l = data;
			break;
		case ALU_LINEPOS_END_X_HIGH:
			line_xend.b.h = data & 0x03;
			break;
		case ALU_LINEPOS_END_X_LOW:  
			line_xend.b.l = data;
			break;
		case ALU_LINEPOS_END_Y_HIGH:
			line_yend.b.h = data & 0x01;
			break;
		case ALU_LINEPOS_END_Y_LOW:
			line_yend.b.l = data;
			do_line();
			break;
		default:
			if((id >= (ALU_CMPDATA_REG + 0)) && (id < (ALU_CMPDATA_REG + 8))) {
				cmp_color_data[id - ALU_CMPDATA_REG] = data;
			} else 	if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0xc000))) {
				uint32_t raddr = id - ALU_WRITE_PROXY;
				if(is_400line) {
					if(raddr >= 0x8000) return;
				} else {
					raddr = raddr & 0x3fff;
				}
				do_alucmds_dmyread(raddr);
			}
			break;
	}
}

uint32_t MB61VH010::read_data8(uint32_t id)
{
	uint32_t raddr;  
	switch(id) {
		case ALU_CMDREG:
			return (uint32_t)command_reg;
			break;
		case ALU_LOGICAL_COLOR:
			return (uint32_t)color_reg;
			break;
		case ALU_WRITE_MASKREG:
			return (uint32_t)mask_reg;
			break;
		case ALU_CMP_STATUS_REG:
			return (uint32_t)cmp_status_reg;
			break;
		case ALU_BANK_DISABLE:
			return (uint32_t)bank_disable_reg;
			break;
		default:
			if((id >= ALU_WRITE_PROXY) && (id < (ALU_WRITE_PROXY + 0xc000))) {
				uint32_t dmydata;
				raddr = id - ALU_WRITE_PROXY;
				if(is_400line) {
					if(raddr >= 0x8000) return 0xffffffff;
				} else {
					raddr = raddr & 0x3fff;
				}
				do_alucmds_dmyread(raddr);
				raddr = (id - ALU_WRITE_PROXY) % 0xc000;
				dmydata = target->read_data8(raddr + direct_access_offset);
				return dmydata;
			}
			return 0xffffffff;
			break;
	}
}

uint32_t MB61VH010::read_signal(int id)
{
	uint32_t val = 0x00000000;
	switch(id) {
		case SIG_ALU_BUSYSTAT:
			if(busy_flag) val = 0xffffffff;
			break;
		case SIG_ALU_ENABLED:
			if((command_reg & 0x80) != 0) val = 0xffffffff;
			break;
	}
	return val;
}

void MB61VH010::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool flag = ((data & mask) != 0);
	switch(id) {
		case SIG_ALU_400LINE:
			is_400line = flag;
			break;
		case SIG_ALU_MULTIPAGE:
			multi_page = (data & mask) & 0x07;
			break;
		case SIG_ALU_PLANES:
			planes = (data & mask) & 0x07;
			if(planes >= 4) planes = 4;
			break;
		case SIG_ALU_X_WIDTH:
			screen_width = (data << 3) & 0x3ff;
			break;
		case SIG_ALU_Y_HEIGHT:
			screen_height = data & 0x3ff;
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
	int i;
	busy_flag = false;
	is_400line = false;
	eventid_busy = -1;
	multi_page = 0x00;
	planes = 3;
	screen_width = 640;
	screen_height = 200;
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
	
	if(planes >= 4) planes = 4;
}

#define STATE_VERSION 1
void MB61VH010::save_state(FILEIO *state_fio)
{
	int i;
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	this->out_debug_log("Save State: MB61VH010 : id=%d ver=%d\n", this_device_id, STATE_VERSION);

	{ // V1
		state_fio->FputUint8(command_reg);
		state_fio->FputUint8(color_reg);
		state_fio->FputUint8(mask_reg);
		state_fio->FputUint8(cmp_status_reg);
		for(i = 0; i < 8; i++) 	state_fio->FputUint8(cmp_color_data[i]);
		state_fio->FputUint8(bank_disable_reg);
		for(i = 0; i < 4; i++) 	state_fio->FputUint8(tile_reg[i]);
		state_fio->FputUint8(multi_page);
		
		state_fio->FputUint32_BE(line_addr_offset.d);
		state_fio->FputUint16_BE(line_pattern.w.l);
		state_fio->FputUint16_BE(line_xbegin.w.l);
		state_fio->FputUint16_BE(line_ybegin.w.l);
		state_fio->FputUint16_BE(line_xend.w.l);
		state_fio->FputUint16_BE(line_yend.w.l);
		
		state_fio->FputBool(busy_flag);
		state_fio->FputInt32_BE(eventid_busy);

		state_fio->FputUint32_BE(total_bytes);
		state_fio->FputUint32_BE(oldaddr);
		state_fio->FputUint32_BE(alu_addr);

		state_fio->FputUint32_BE(planes);
		state_fio->FputBool(is_400line);
		state_fio->FputUint32_BE(screen_width);
		state_fio->FputUint32_BE(screen_height);

		state_fio->FputUint16_BE(line_style.w.l);
	}
   
}

bool MB61VH010::load_state(FILEIO *state_fio)
{
	uint32_t version = state_fio->FgetUint32();
	int i;
	this->out_debug_log("Load State: MB61VH010 : id=%d ver=%d\n", this_device_id, version);
	if(this_device_id != state_fio->FgetInt32()) return false;
	if(version >= 1) {
		command_reg = state_fio->FgetUint8();
		color_reg = state_fio->FgetUint8();
		mask_reg = state_fio->FgetUint8();
		cmp_status_reg = state_fio->FgetUint8();
		for(i = 0; i < 8; i++) 	cmp_color_data[i] = state_fio->FgetUint8();
		bank_disable_reg = state_fio->FgetUint8();
		for(i = 0; i < 4; i++) 	tile_reg[i] = state_fio->FgetUint8();
		multi_page = state_fio->FgetUint8();

		line_addr_offset.d = state_fio->FgetUint32_BE();
		line_pattern.d = 0;
		line_xbegin.d = 0;
		line_ybegin.d = 0;
		line_xend.d = 0;
		line_yend.d = 0;
	   
		line_pattern.w.l = state_fio->FgetUint16_BE();
		line_xbegin.w.l = state_fio->FgetUint16_BE();
		line_ybegin.w.l = state_fio->FgetUint16_BE();
		line_xend.w.l = state_fio->FgetUint16_BE();
		line_yend.w.l = state_fio->FgetUint16_BE();

		busy_flag = state_fio->FgetBool();
		eventid_busy = state_fio->FgetInt32_BE();
		
		total_bytes = state_fio->FgetUint32_BE();
		oldaddr = state_fio->FgetUint32_BE();
		alu_addr = state_fio->FgetUint32_BE();

		planes = state_fio->FgetUint32_BE();
		is_400line = state_fio->FgetBool();
		screen_width = state_fio->FgetUint32_BE();
		screen_height = state_fio->FgetUint32_BE();

		line_style.d = 0;
		line_style.w.l = state_fio->FgetUint16_BE();
	}
	if(version != STATE_VERSION) return false;
	return true;
}
