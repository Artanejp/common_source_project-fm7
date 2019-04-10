/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ uPD7220 ]
*/

#include <math.h>
#include "upd7220.h"
#include "../fifo.h"
// -> See also: upd7220_base.cpp .

#define EVENT_HSYNC_HFP	0
#define EVENT_HSYNC_HS	1
#define EVENT_HSYNC_HBP	2
#define EVENT_CMD_READY 3

enum {
	STAT_DRDY	= 0x01,
	STAT_FULL	= 0x02,
	STAT_EMPTY	= 0x04,
	STAT_DRAW	= 0x08,
	STAT_DMA	= 0x10,
	STAT_VSYNC	= 0x20,
	STAT_BLANK	= 0x40,
	STAT_LPEN	= 0x80,
};


void UPD7220_BASE::initialize()
{
	DEVICE::initialize();
	for(int i = 0; i <= RT_TABLEMAX; i++) {
		rt[i] = (int)((double)(1 << RT_MULBIT) * (1 - sqrt(1 - pow((0.70710678118654 * i) / RT_TABLEMAX, 2))));
	}
	fo = new FIFO(0x10000);
	//cmd_fifo = new FIFO(16);
	
	vsync = vblank = false;
	hsync = hblank = false;
	master = false;
	pitch = 40;	// 640dot

	event_cmdready = -1;
	wrote_bytes = 0;
	cmd_ready = true;
	// -> upd7220.cpp
}

void UPD7220_BASE::release()
{
	fo->release();
	delete fo;
	//cmd_fifo->release();
	//delete cmd_fifo;
}

void UPD7220_BASE::reset()
{
	if(event_cmdready >= 0) cancel_event(this, event_cmdready);
	event_cmdready = -1;
	cmd_reset();
}

void UPD7220_BASE::write_dma_io8(uint32_t addr, uint32_t data)
{
	// for dma access
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		if(low_high) {
			cmd_write_sub(ead * 2 + 1, data & maskh);
			ead += dif;
		} else {
			cmd_write_sub(ead * 2 + 0, data & maskl);
		}
		low_high = !low_high;
		break;
	case 0x10: // low byte
		cmd_write_sub(ead * 2 + 0, data & maskl);
		ead += dif;
		break;
	case 0x18: // high byte
		cmd_write_sub(ead * 2 + 1, data & maskh);
		ead += dif;
		break;
	}
}

uint32_t UPD7220_BASE::read_dma_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	// for dma access
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		if(low_high) {
			val = read_vram(ead * 2 + 1);
			ead += dif;
		} else {
			val = read_vram(ead * 2 + 0);
		}
		low_high = !low_high;
		break;
	case 0x10: // low byte
		val =  read_vram(ead * 2 + 0);
		ead += dif;
		break;
	case 0x18: // high byte
		val =  read_vram(ead * 2 + 1);
		ead += dif;
		break;
	}
	return val;
}

void UPD7220_BASE::write_io8(uint32_t addr, uint32_t data)
{
	// Dummy function
}

uint32_t UPD7220_BASE::read_io8(uint32_t addr)
{
	uint32_t val;
	
	switch(addr & 3) {
	case 0: // status
		val = statreg;
		if(sync[5] & 0x80) {
			val |= vblank ? STAT_BLANK : 0;
		} else {
			val |= hblank ? STAT_BLANK : 0;
		}
		val |= vsync ? STAT_VSYNC : 0;
//		val |= (params_count == 0) ? STAT_EMPTY : 0;
		val |= STAT_EMPTY;
		val |= (params_count == 16) ? STAT_FULL : 0;
		val |= fo->count() ? STAT_DRDY : 0;
		// clear busy stat
		statreg &= ~(STAT_DMA | STAT_DRAW);
		return val;
	case 1: // data
		if(fo->count()) {
			return fo->read();
		}
		return 0xff;
	}
	return 0xff;
}

void UPD7220_BASE::event_pre_frame()
{
	// Dummy func.
}

void UPD7220_BASE::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
	frames_per_sec = new_frames_per_sec;	// note: refer these params given from the event manager
	lines_per_frame = new_lines_per_frame;	// because this device may be slave gdc
	
	// update event clocks
	vs = hs = 0;
}

void UPD7220_BASE::event_frame()
{
	if(vs == 0) {
		vfp = (int)((double)lines_per_frame * (double)(v1          ) / (double)(v1 + v2 + v3 + v4) + 0.5);
		vs  = (int)((double)lines_per_frame * (double)(v1 + v2     ) / (double)(v1 + v2 + v3 + v4) + 0.5);
		vbp = (int)((double)lines_per_frame * (double)(v1 + v2 + v3) / (double)(v1 + v2 + v3 + v4) + 0.5);
		hfp = (int)((double)cpu_clocks * (double)(h1          ) / frames_per_sec / (double)lines_per_frame / (double)(h1 + h2 + h3 + h4) + 0.5);
		hs  = (int)((double)cpu_clocks * (double)(h1 + h2     ) / frames_per_sec / (double)lines_per_frame / (double)(h1 + h2 + h3 + h4) + 0.5);
		hbp = (int)((double)cpu_clocks * (double)(h1 + h2 + h3) / frames_per_sec / (double)lines_per_frame / (double)(h1 + h2 + h3 + h4) + 0.5);
	}
	if(++blink_cursor >= blink_rate * 4) {
		blink_cursor = 0;
	}
	if(++blink_attr >= blink_rate * 2) {
		blink_attr = 0;
	}
}

void UPD7220_BASE::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_UPD7220_CLOCK_FREQ:
		clock_freq = data; // Must be Hz.
		break;
	}
}

uint32_t UPD7220_BASE::read_signal(int ch)
{
	switch(ch) {
	case SIG_UPD7220_CLOCK_FREQ:
		return clock_freq;
		break;
	}
	return 0;
}
		

void UPD7220_BASE::event_vline(int v, int clock)
{
	if(v == 0) {
		vblank = true;
	} else if(v == vfp) {
		write_signals(&outputs_vsync, 0xffffffff);
		vsync = true;
	} else if(v == vs) {
		write_signals(&outputs_vsync, 0);
		vsync = false;
	} else if(v == vbp) {
		vblank = false;
 	}
	hblank = true;
	register_event_by_clock(this, EVENT_HSYNC_HFP, hfp, false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_HS,  hs,  false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_HBP, hbp, false, NULL);
}

void UPD7220_BASE::register_event_wait_cmd(uint32_t bytes)
{
	if(event_cmdready >= 0) cancel_event(this, event_cmdready);
	event_cmdready = -1;
	double usec = (1.0e6 * (double)bytes) / (double)clock_freq;
	if(usec < 1.0) {
		cmd_ready = true;
		params_count = 0;
	} else {
		register_event(this, EVENT_CMD_READY, usec, false, &event_cmdready);
	}
}

				   
void UPD7220_BASE::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HSYNC_HFP) {
		hsync = true;
	} else if(event_id == EVENT_HSYNC_HS) {
		hsync = false;
	} else if(event_id == EVENT_HSYNC_HBP) {
		hblank = false;
	} else if(event_id == EVENT_CMD_READY) {
		event_cmdready = -1;
		cmd_ready = true;
		params_count = 0;
	}
}

uint32_t UPD7220_BASE::cursor_addr(uint32_t mask)
{
	if((cs[0] & 0x80) && ((cs[1] & 0x20) || (blink_cursor < blink_rate * 2))) {
		return (ead << 1) & mask;
	}
	return -1;
}

int UPD7220_BASE::cursor_top()
{
	return cs[1] & 0x1f;
}

int UPD7220_BASE::cursor_bottom()
{
	return cs[2] >> 3;
}

// command process

void UPD7220_BASE::cmd_reset()
{
	// init gdc params
	sync[6] = 0x90;
	sync[7] = 0x01;
	zoom = zr = zw = 0;
	ra[0] = ra[1] = ra[2] = 0;
	ra[3] = 0x1e; /*0x19;*/
	cs[0] = cs[1] = cs[2] = 0;
	ead = dad = 0;
	maskl = maskh = 0xff;
	mod = 0;
	blink_cursor = 0;
	blink_attr = 0;
	blink_rate = 16;
	
	// init fifo
	params_count = 0;
	cmd_ready = true;
	fo->clear();
	
	// stop display and drawing
	start = false;
	statreg = 0;
	cmdreg = -1;
	cmd_write_done = false;
}

void UPD7220_BASE::cmd_sync()
{
	start = ((cmdreg & 1) != 0);
	for(int i = 0; i < 8 && i < params_count; i++) {
		if(sync[i] != params[i]) {
			sync[i] = params[i];
			sync_changed = true;
		}
	}
	cmdreg = -1;
}

void UPD7220_BASE::cmd_master()
{
	master = true;
	cmdreg = -1;
}

void UPD7220_BASE::cmd_slave()
{
	master = false;
	cmdreg = -1;
}

void UPD7220_BASE::cmd_start()
{
	start = true;
	cmdreg = -1;
}

void UPD7220_BASE::cmd_stop()
{
	start = false;
	cmdreg = -1;
}

void UPD7220_BASE::cmd_zoom()
{
	if(params_count > 0) {
		uint8_t tmp = params[0];
		zr = tmp >> 4;
		zw = tmp & 0x0f;
		cmdreg = -1;
	}
}

void UPD7220_BASE::cmd_scroll()
{
	if(params_count > 0) {
		ra[cmdreg & 0x0f] = params[0];
		if(cmdreg < 0x7f) {
			cmdreg++;
			params_count = 0;
		} else {
			cmdreg = -1;
		}
	}
}

void UPD7220_BASE::cmd_csrform()
{
	for(int i = 0; i < params_count; i++) {
		cs[i] = params[i];
	}
	if(params_count > 2) {
		cmdreg = -1;
	}
	blink_rate = (cs[1] >> 6) | ((cs[2] & 7) << 2);
}

void UPD7220_BASE::cmd_lpen()
{
	fo->write(lad & 0xff);
	fo->write((lad >> 8) & 0xff);
	fo->write((lad >> 16) & 0xff);
	cmdreg = -1;
}

void UPD7220_BASE::cmd_vectw()
{
	for(int i = 0; i < 11 && i < params_count; i++) {
		vect[i] = params[i];
//		this->out_debug_log(_T("\tVECT[%d] = %2x\n"), i, vect[i]);
	}
	update_vect();
	cmdreg = -1;
}


void UPD7220_BASE::cmd_csrw()
{
	if(params_count > 0) {
		ead = params[0];
		if(params_count > 1) {
			ead |= params[1] << 8;
			if(params_count > 2) {
				ead |= params[2] << 16;
				cmdreg = -1;
			}
		}
		dad = (ead >> 20) & 0x0f;
		ead &= 0x3ffff;
	}
}

void UPD7220_BASE::cmd_csrr()
{
	fo->write(ead & 0xff);
	fo->write((ead >> 8) & 0xff);
	fo->write((ead >> 16) & 0x03);
	fo->write(dad & 0xff);
	fo->write((dad >> 8) & 0xff);
	cmdreg = -1;
}

void UPD7220_BASE::cmd_mask()
{
	if(params_count > 1) {
		maskl = params[0];
		maskh = params[1];
		cmdreg = -1;
	}
}

void UPD7220_BASE::cmd_write()
{
	mod = cmdreg & 3;
	wrote_bytes = 0;
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		if(params_count > 1) {
			uint8_t l = params[0] & maskl;
			uint8_t h = params[1] & maskh;
			for(int i = 0; i < dc + 1; i++) {
				cmd_write_sub(ead * 2 + 0, l);
				cmd_write_sub(ead * 2 + 1, h);
				ead += dif;
			}
			wrote_bytes = (dc + 1) * 2;
			cmd_write_done = true;
			params_count = 0;
		}
		break;
	case 0x10: // low byte
		if(params_count > 0) {
			uint8_t l = params[0] & maskl;
			for(int i = 0; i < dc + 1; i++) {
				cmd_write_sub(ead * 2 + 0, l);
				ead += dif;
			}
			wrote_bytes = dc + 1;
			cmd_write_done = true;
			params_count = 0;
		}
		break;
	case 0x18: // high byte
		if(params_count > 0) {
			uint8_t h = params[0] & maskh;
			for(int i = 0; i < dc + 1; i++) {
				cmd_write_sub(ead * 2 + 1, h);
				ead += dif;
			}
			wrote_bytes = dc + 1;
			cmd_write_done = true;
			params_count = 0;
		}
		break;
	default: // invalid
		cmdreg = -1;
		break;
	}
}

void UPD7220_BASE::cmd_read()
{
	mod = cmdreg & 3;
	wrote_bytes = 0;
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 0));
			fo->write(read_vram(ead * 2 + 1));
			ead += dif;
		}
		wrote_bytes = (dc + 1) * 2;
		break;
	case 0x10: // low byte
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 0));
			ead += dif;
		}
		wrote_bytes = dc + 1;
		break;
	case 0x18: // high byte
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 1));
			ead += dif;
		}
		wrote_bytes = dc + 1;
		break;
	default: // invalid
		break;
	}
	reset_vect();
	cmdreg = -1;
}

void UPD7220_BASE::cmd_dmaw()
{
	mod = cmdreg & 3;
	low_high = false;
	write_signals(&outputs_drq, 0xffffffff);
	reset_vect();
//	statreg |= STAT_DMA;
	cmdreg = -1;
}

void UPD7220_BASE::cmd_dmar()
{
	mod = cmdreg & 3;
	low_high = false;
	write_signals(&outputs_drq, 0xffffffff);
	reset_vect();
//	statreg |= STAT_DMA;
	cmdreg = -1;
}

void UPD7220_BASE::cmd_unk_5a()
{
	if(params_count > 2) {
		cmdreg = -1;
	}
}

// command sub

void UPD7220_BASE::cmd_write_sub(uint32_t addr, uint8_t data)
{
	switch(mod) {
	case 0: // replace
		write_vram(addr, data);
		break;
	case 1: // complement
		write_vram(addr, read_vram(addr) ^ data);
		break;
	case 2: // reset
		write_vram(addr, read_vram(addr) & ~data);
		break;
	case 3: // set
		write_vram(addr, read_vram(addr) | data);
		break;
	}
}

void UPD7220_BASE::write_vram(uint32_t addr, uint8_t data)
{
	if(addr < vram_size) {
		if(vram != NULL) {
			vram[addr] = data;
		} else if(d_vram_bus != NULL) {
			d_vram_bus->write_dma_io8(addr, data);
		}
	}
}

uint8_t UPD7220_BASE::read_vram(uint32_t addr)
{
	if(addr < vram_size) {
 		uint8_t mask = (addr & 1) ? (vram_data_mask >> 8) : (vram_data_mask & 0xff);
		if(vram != NULL) {
			return (vram[addr] & mask) | ~mask;
		} else if(d_vram_bus != NULL) {
			return (d_vram_bus->read_dma_io8(addr) & mask) | ~mask;
		}
 	}
	return 0xff;
}

void UPD7220_BASE::update_vect()
{
	dir = vect[0] & 7;
	dif = vectdir[dir][0] + vectdir[dir][1] * pitch;
	sl = vect[0] & 0x80;
	dc = (vect[1] | (vect[ 2] << 8)) & 0x3fff;
	d  = (vect[3] | (vect[ 4] << 8)) & 0x3fff;
	d2 = (vect[5] | (vect[ 6] << 8)) & 0x3fff;
	d1 = (vect[7] | (vect[ 8] << 8)) & 0x3fff;
	dm = (vect[9] | (vect[10] << 8)) & 0x3fff;
}

void UPD7220_BASE::reset_vect()
{
	vect[ 1] = 0x00;
	vect[ 2] = 0x00;
	vect[ 3] = 0x08;
	vect[ 4] = 0x00;
	vect[ 5] = 0x08;
	vect[ 6] = 0x00;
	vect[ 7] = 0x00;
	vect[ 8] = 0x00;
	vect[ 9] = 0x00;
	vect[10] = 0x00;
	update_vect();
}



