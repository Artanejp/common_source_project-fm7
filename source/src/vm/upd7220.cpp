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

#define EVENT_HSYNC_HFP	0
#define EVENT_HSYNC_HS	1
#define EVENT_HSYNC_HBP	2

enum {
	CMD_RESET	= 0x00,
	CMD_SYNC_OFF	= 0x0e,
	CMD_SYNC_ON	= 0x0f,
	CMD_MASTER	= 0x6f,
	CMD_SLAVE	= 0x6e,

	CMD_START_	= 0x6b,
	CMD_START	= 0x0d,
	CMD_STOP_	= 0x05,
	CMD_STOP	= 0x0c,

	CMD_ZOOM	= 0x46,
	CMD_SCROLL	= 0x70,
	CMD_CSRW	= 0x49,
	CMD_CSRFORM	= 0x4b,
	CMD_PITCH	= 0x47,
	CMD_LPEN	= 0xc0,
	CMD_VECTW	= 0x4c,
	CMD_VECTE	= 0x6c,
	CMD_TEXTW	= 0x78,
	CMD_TEXTE	= 0x68,
	CMD_CSRR	= 0xe0,
	CMD_MASK	= 0x4a,

	CMD_WRITE	= 0x20,
	CMD_READ	= 0xa0,
	CMD_DMAR	= 0xa4,
	CMD_DMAW	= 0x24,
	/* unknown command (3 params) */
	CMD_UNK_5A	= 0x5a,
};

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

static const int vectdir[16][4] = {
	{ 0, 1, 1, 0}, { 1, 1, 1,-1}, { 1, 0, 0,-1}, { 1,-1,-1,-1},
	{ 0,-1,-1, 0}, {-1,-1,-1, 1}, {-1, 0, 0, 1}, {-1, 1, 1, 1},
	{ 0, 1, 1, 1}, { 1, 1, 1, 0}, { 1, 0, 1,-1}, { 1,-1, 0,-1},
	{ 0,-1,-1,-1}, {-1,-1,-1, 0}, {-1, 0,-1, 1}, {-1, 1, 0, 1}
};

void UPD7220::initialize()
{
	for(int i = 0; i <= RT_TABLEMAX; i++) {
		rt[i] = (int)((double)(1 << RT_MULBIT) * (1 - sqrt(1 - pow((0.70710678118654 * i) / RT_TABLEMAX, 2))));
	}
	fo = new FIFO(0x10000);
	
	hsync = hblank = false;
	vsync = vblank = false;
	master = false;
	pitch = 40;	// 640dot
	memset(ra, 0, sizeof(ra));
	memset(cs, 0, sizeof(cs));
	
	// initial settings for 1st frame
	vtotal = 0; //LINES_PER_FRAME;
	v1 = v2 = v3 = 16;
	v4 = LINES_PER_FRAME - v1 - v2 - v3;
	h1 = h2 = h3 = 8;
#ifdef CHARS_PER_LINE
	h4 = CHARS_PER_LINE - v1 - v2 - v3;
#else
	h4 = 80;
#endif
	
	sync_changed = false;
	vs = hs = 0;
	
#ifdef UPD7220_HORIZ_FREQ
	horiz_freq = 0;
	next_horiz_freq = UPD7220_HORIZ_FREQ;
#endif
	
	// register events
	register_frame_event(this);
	register_vline_event(this);
}

void UPD7220::release()
{
	fo->release();
	delete fo;
}

void UPD7220::reset()
{
	cmd_reset();
}

void UPD7220::write_dma_io8(uint32_t addr, uint32_t data)
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

uint32_t UPD7220::read_dma_io8(uint32_t addr)
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

void UPD7220::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0: // set parameter
//		this->out_debug_log(_T("\tPARAM = %2x\n"), data);
		if(cmdreg != -1) {
			if(params_count < 16) {
				params[params_count++] = (uint8_t)(data & 0xff);
			}
			check_cmd();
			if(cmdreg == -1) {
				params_count = 0;
			}
		}
		break;
	case 1: // process prev command if not finished
		if(cmdreg != -1) {
			process_cmd();
		}
		// set new command
		cmdreg = data & 0xff;
//		this->out_debug_log(_T("CMDREG = %2x\n"), cmdreg);
		params_count = 0;
		check_cmd();
		break;
	case 2: // set zoom
		zoom = data;
		break;
	case 3: // light pen request
		break;
	}
}

uint32_t UPD7220::read_io8(uint32_t addr)
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

void UPD7220::event_pre_frame()
{
	if(sync_changed) {
		// calc vsync/hblank timing
		v1  = sync[5] & 0x3f;		// VFP
		v2  = sync[2] >> 5;		// VS
		v2 += (sync[3] & 0x03) << 3;
		v3  = sync[7] >> 2;		// VBP
		v4  = sync[6];			// L/F
		v4 += (sync[7] & 0x03) << 8;
		
		h1  = (sync[3] >> 2) + 1;	// HFP
		h2  = (sync[2] & 0x1f) + 1;	// HS
		h3  = (sync[4] & 0x3f) + 1;	// HBP
		h4  = sync[1] + 2;		// C/R
		
		sync_changed = false;
		vs = hs = 0;
		vtotal = 0;
#ifdef UPD7220_HORIZ_FREQ
		horiz_freq = 0;
#endif
	}
	if(master) {
		if(vtotal != v1 + v2 + v3 + v4) {
			vtotal = v1 + v2 + v3 + v4;
			set_lines_per_frame(vtotal);
		}
#ifdef UPD7220_HORIZ_FREQ
		if(horiz_freq != next_horiz_freq && vtotal != 0) {
			horiz_freq = next_horiz_freq;
			set_frames_per_sec((double)horiz_freq / (double)vtotal);
		}
#endif
	}
}

void UPD7220::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
	frames_per_sec = new_frames_per_sec;	// note: refer these params given from the event manager
	lines_per_frame = new_lines_per_frame;	// because this device may be slave gdc
	
	// update event clocks
	vs = hs = 0;
}

void UPD7220::event_frame()
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

void UPD7220::event_vline(int v, int clock)
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

void UPD7220::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HSYNC_HFP) {
		hsync = true;
	} else if(event_id == EVENT_HSYNC_HS) {
		hsync = false;
	} else if(event_id == EVENT_HSYNC_HBP) {
		hblank = false;
	}
}

uint32_t UPD7220::cursor_addr(uint32_t mask)
{
	if((cs[0] & 0x80) && ((cs[1] & 0x20) || (blink_cursor < blink_rate * 2))) {
		return (ead << 1) & mask;
	}
	return -1;
}

int UPD7220::cursor_top()
{
	return cs[1] & 0x1f;
}

int UPD7220::cursor_bottom()
{
	return cs[2] >> 3;
}

// command process

void UPD7220::check_cmd()
{
	// check fifo buffer and process command if enough params in fifo
	switch(cmdreg & 0xff) {
	case CMD_RESET:
		cmd_reset();
		break;
	case CMD_SYNC_OFF:
		if(params_count > 7) {
			cmd_sync(false);
		}
		break;
	case CMD_SYNC_ON:
		if(params_count > 7) {
			cmd_sync(true);
		}
		break;
	case CMD_MASTER:
		cmd_master();
		break;
	case CMD_SLAVE:
		cmd_slave();
		break;
	case CMD_START_:
	case CMD_START:
		cmd_start();
		break;
	case CMD_STOP_:
	case CMD_STOP:
		cmd_stop();
		break;
	case CMD_ZOOM:
		cmd_zoom();
		break;
	case CMD_SCROLL + 0:
	case CMD_SCROLL + 1:
	case CMD_SCROLL + 2:
	case CMD_SCROLL + 3:
	case CMD_SCROLL + 4:
	case CMD_SCROLL + 5:
	case CMD_SCROLL + 6:
	case CMD_SCROLL + 7:
	case CMD_TEXTW + 0:
	case CMD_TEXTW + 1:
	case CMD_TEXTW + 2:
	case CMD_TEXTW + 3:
	case CMD_TEXTW + 4:
	case CMD_TEXTW + 5:
	case CMD_TEXTW + 6:
	case CMD_TEXTW + 7:
		cmd_scroll();
		break;
	case CMD_CSRFORM:
		cmd_csrform();
		break;
	case CMD_PITCH:
		cmd_pitch();
		break;
	case CMD_LPEN:
		cmd_lpen();
		break;
	case CMD_VECTW:
		if(params_count > 10) {
			cmd_vectw();
		}
		break;
	case CMD_VECTE:
		cmd_vecte();
		break;
	case CMD_TEXTE:
		cmd_texte();
		break;
	case CMD_CSRW:
		cmd_csrw();
		break;
	case CMD_CSRR:
		cmd_csrr();
		break;
	case CMD_MASK:
		cmd_mask();
		break;
	case CMD_WRITE + 0x00:
	case CMD_WRITE + 0x01:
	case CMD_WRITE + 0x02:
	case CMD_WRITE + 0x03:
	case CMD_WRITE + 0x08:
	case CMD_WRITE + 0x09:
	case CMD_WRITE + 0x0a:
	case CMD_WRITE + 0x0b:
	case CMD_WRITE + 0x10:
	case CMD_WRITE + 0x11:
	case CMD_WRITE + 0x12:
	case CMD_WRITE + 0x13:
	case CMD_WRITE + 0x18:
	case CMD_WRITE + 0x19:
	case CMD_WRITE + 0x1a:
	case CMD_WRITE + 0x1b:
		cmd_write();
		break;
	case CMD_READ + 0x00:
	case CMD_READ + 0x01:
	case CMD_READ + 0x02:
	case CMD_READ + 0x03:
	case CMD_READ + 0x08:
	case CMD_READ + 0x09:
	case CMD_READ + 0x0a:
	case CMD_READ + 0x0b:
	case CMD_READ + 0x10:
	case CMD_READ + 0x11:
	case CMD_READ + 0x12:
	case CMD_READ + 0x13:
	case CMD_READ + 0x18:
	case CMD_READ + 0x19:
	case CMD_READ + 0x1a:
	case CMD_READ + 0x1b:
		cmd_read();
		break;
	case CMD_DMAW + 0x00:
	case CMD_DMAW + 0x01:
	case CMD_DMAW + 0x02:
	case CMD_DMAW + 0x03:
	case CMD_DMAW + 0x08:
	case CMD_DMAW + 0x09:
	case CMD_DMAW + 0x0a:
	case CMD_DMAW + 0x0b:
	case CMD_DMAW + 0x10:
	case CMD_DMAW + 0x11:
	case CMD_DMAW + 0x12:
	case CMD_DMAW + 0x13:
	case CMD_DMAW + 0x18:
	case CMD_DMAW + 0x19:
	case CMD_DMAW + 0x1a:
	case CMD_DMAW + 0x1b:
		cmd_dmaw();
		break;
	case CMD_DMAR + 0x00:
	case CMD_DMAR + 0x01:
	case CMD_DMAR + 0x02:
	case CMD_DMAR + 0x03:
	case CMD_DMAR + 0x08:
	case CMD_DMAR + 0x09:
	case CMD_DMAR + 0x0a:
	case CMD_DMAR + 0x0b:
	case CMD_DMAR + 0x10:
	case CMD_DMAR + 0x11:
	case CMD_DMAR + 0x12:
	case CMD_DMAR + 0x13:
	case CMD_DMAR + 0x18:
	case CMD_DMAR + 0x19:
	case CMD_DMAR + 0x1a:
	case CMD_DMAR + 0x1b:
		cmd_dmar();
		break;
	case CMD_UNK_5A:
		cmd_unk_5a();
		break;
	}
}

void UPD7220::process_cmd()
{
	switch(cmdreg & 0xff) {
	case CMD_RESET:
		cmd_reset();
		break;
	case CMD_SYNC_OFF:
		cmd_sync(false);
		break;
	case CMD_SYNC_ON:
		cmd_sync(true);
		break;
	case CMD_SCROLL + 0:
	case CMD_SCROLL + 1:
	case CMD_SCROLL + 2:
	case CMD_SCROLL + 3:
	case CMD_SCROLL + 4:
	case CMD_SCROLL + 5:
	case CMD_SCROLL + 6:
	case CMD_SCROLL + 7:
	case CMD_TEXTW + 0:
	case CMD_TEXTW + 1:
	case CMD_TEXTW + 2:
	case CMD_TEXTW + 3:
	case CMD_TEXTW + 4:
	case CMD_TEXTW + 5:
	case CMD_TEXTW + 6:
	case CMD_TEXTW + 7:
		cmd_scroll();
		break;
	case CMD_VECTW:
		cmd_vectw();
		break;
	case CMD_CSRW:
		cmd_csrw();
		break;
	case CMD_WRITE + 0x00:
	case CMD_WRITE + 0x01:
	case CMD_WRITE + 0x02:
	case CMD_WRITE + 0x03:
	case CMD_WRITE + 0x08:
	case CMD_WRITE + 0x09:
	case CMD_WRITE + 0x0a:
	case CMD_WRITE + 0x0b:
	case CMD_WRITE + 0x10:
	case CMD_WRITE + 0x11:
	case CMD_WRITE + 0x12:
	case CMD_WRITE + 0x13:
	case CMD_WRITE + 0x18:
	case CMD_WRITE + 0x19:
	case CMD_WRITE + 0x1a:
	case CMD_WRITE + 0x1b:
		if(cmd_write_done) {
			reset_vect();
		}
		cmdreg = -1;
		cmd_write_done = false;
		break;
	}
}

void UPD7220::cmd_reset()
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
	fo->clear();
	
	// stop display and drawing
	start = false;
	statreg = 0;
	cmdreg = -1;
	cmd_write_done = false;
}

void UPD7220::cmd_sync(bool flag)
{
	for(int i = 0; i < 8 && i < params_count; i++) {
		if(sync[i] != params[i]) {
			sync[i] = params[i];
			sync_changed = true;
		}
#ifndef UPD7220_FIXED_PITCH
		if(i == 1) {
			pitch = params[1] + 2;
		}
#endif
	}
	start = flag;
	cmdreg = -1;
}

void UPD7220::cmd_master()
{
	master = true;
	cmdreg = -1;
}

void UPD7220::cmd_slave()
{
	master = false;
	cmdreg = -1;
}

void UPD7220::cmd_start()
{
	start = true;
	cmdreg = -1;
}

void UPD7220::cmd_stop()
{
	start = false;
	cmdreg = -1;
}

void UPD7220::cmd_zoom()
{
	if(params_count > 0) {
		uint8_t tmp = params[0];
		zr = tmp >> 4;
		zw = tmp & 0x0f;
		cmdreg = -1;
	}
}

void UPD7220::cmd_scroll()
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

void UPD7220::cmd_csrform()
{
	for(int i = 0; i < params_count; i++) {
		cs[i] = params[i];
	}
	if(params_count > 2) {
		cmdreg = -1;
	}
	blink_rate = (cs[1] >> 6) | ((cs[2] & 7) << 2);
}

void UPD7220::cmd_pitch()
{
	if(params_count > 0) {
#ifndef UPD7220_FIXED_PITCH
		pitch = params[0];
#endif
		cmdreg = -1;
	}
}

void UPD7220::cmd_lpen()
{
	fo->write(lad & 0xff);
	fo->write((lad >> 8) & 0xff);
	fo->write((lad >> 16) & 0xff);
	cmdreg = -1;
}

void UPD7220::cmd_vectw()
{
	for(int i = 0; i < 11 && i < params_count; i++) {
		vect[i] = params[i];
//		this->out_debug_log(_T("\tVECT[%d] = %2x\n"), i, vect[i]);
	}
	update_vect();
	cmdreg = -1;
}

void UPD7220::cmd_vecte()
{
	uint32_t ead2 = ead;
	
	if(plane_size) {
		plane = ead2 / (plane_size >> 1);
		ead2 %= (plane_size >> 1);
	}
	dx = ((ead2 % pitch) << 4) | (dad & 0x0f);
	dy = ead2 / pitch;
	
	// execute command
	if(!(vect[0] & 0x78)) {
		pattern = ra[8] | (ra[9] << 8);
		draw_pset(dx, dy);
	}
	if(vect[0] & 0x08) {
		draw_vectl();
	}
	if(vect[0] & 0x10) {
		draw_vectt();
	}
	if(vect[0] & 0x20) {
		draw_vectc();
	}
	if(vect[0] & 0x40) {
		draw_vectr();
	}
	reset_vect();
	statreg |= STAT_DRAW;
	cmdreg = -1;
}

void UPD7220::cmd_texte()
{
	uint32_t ead2 = ead;
	
	if(plane_size) {
		plane = ead2 / (plane_size >> 1);
		ead2 %= (plane_size >> 1);
	}
	dx = ((ead2 % pitch) << 4) | (dad & 0x0f);
	dy = ead2 / pitch;
	
	// execute command
	if(!(vect[0] & 0x78)) {
		pattern = ra[8] | (ra[9] << 8);
		draw_pset(dx, dy);
	}
	if(vect[0] & 0x08) {
		draw_vectl();
	}
	if(vect[0] & 0x10) {
		draw_text();
	}
	if(vect[0] & 0x20) {
		draw_vectc();
	}
	if(vect[0] & 0x40) {
		draw_vectr();
	}
	reset_vect();
	statreg |= STAT_DRAW;
	cmdreg = -1;
}

void UPD7220::cmd_csrw()
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

void UPD7220::cmd_csrr()
{
	fo->write(ead & 0xff);
	fo->write((ead >> 8) & 0xff);
	fo->write((ead >> 16) & 0x03);
	fo->write(dad & 0xff);
	fo->write((dad >> 8) & 0xff);
	cmdreg = -1;
}

void UPD7220::cmd_mask()
{
	if(params_count > 1) {
		maskl = params[0];
		maskh = params[1];
		cmdreg = -1;
	}
}

void UPD7220::cmd_write()
{
	mod = cmdreg & 3;
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
			cmd_write_done = true;
			params_count = 0;
		}
		break;
	default: // invalid
		cmdreg = -1;
		break;
	}
}

void UPD7220::cmd_read()
{
	mod = cmdreg & 3;
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 0));
			fo->write(read_vram(ead * 2 + 1));
			ead += dif;
		}
		break;
	case 0x10: // low byte
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 0));
			ead += dif;
		}
		break;
	case 0x18: // high byte
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 1));
			ead += dif;
		}
		break;
	default: // invalid
		break;
	}
	reset_vect();
	cmdreg = -1;
}

void UPD7220::cmd_dmaw()
{
	mod = cmdreg & 3;
	low_high = false;
	write_signals(&outputs_drq, 0xffffffff);
	reset_vect();
//	statreg |= STAT_DMA;
	cmdreg = -1;
}

void UPD7220::cmd_dmar()
{
	mod = cmdreg & 3;
	low_high = false;
	write_signals(&outputs_drq, 0xffffffff);
	reset_vect();
//	statreg |= STAT_DMA;
	cmdreg = -1;
}

void UPD7220::cmd_unk_5a()
{
	if(params_count > 2) {
		cmdreg = -1;
	}
}

// command sub

void UPD7220::cmd_write_sub(uint32_t addr, uint8_t data)
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

void UPD7220::write_vram(uint32_t addr, uint8_t data)
{
	if(addr < vram_size) {
		if(vram != NULL) {
			vram[addr] = data;
		} else if(d_vram_bus != NULL) {
			d_vram_bus->write_dma_io8(addr, data);
		}
	}
}

uint8_t UPD7220::read_vram(uint32_t addr)
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

void UPD7220::update_vect()
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

void UPD7220::reset_vect()
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

// draw

void UPD7220::draw_vectl()
{
	pattern = ra[8] | (ra[9] << 8);
	
	if(dc) {
		int x = dx;
		int y = dy;
		
		switch(dir) {
		case 0:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x + step, y++);
			}
			break;
		case 1:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x++, y + step);
			}
			break;
		case 2:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x++, y - step);
			}
			break;
		case 3:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x + step, y--);
			}
			break;
		case 4:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x - step, y--);
			}
			break;
		case 5:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x--, y - step);
			}
			break;
		case 6:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x--, y + step);
			}
			break;
		case 7:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				draw_pset(x - step, y++);
			}
			break;
		}
	} else {
		draw_pset(dx, dy);
	}
}

void UPD7220::draw_vectt()
{
	uint16_t draw = ra[8] | (ra[9] << 8);
	if(sl) {
		// reverse
		draw = (draw & 0x0001 ? 0x8000 : 0) | (draw & 0x0002 ? 0x4000 : 0) | 
		       (draw & 0x0004 ? 0x2000 : 0) | (draw & 0x0008 ? 0x1000 : 0) | 
		       (draw & 0x0010 ? 0x0800 : 0) | (draw & 0x0020 ? 0x0400 : 0) | 
		       (draw & 0x0040 ? 0x0200 : 0) | (draw & 0x0080 ? 0x0100 : 0) | 
		       (draw & 0x0100 ? 0x0080 : 0) | (draw & 0x0200 ? 0x0040 : 0) | 
		       (draw & 0x0400 ? 0x0020 : 0) | (draw & 0x0800 ? 0x0010 : 0) | 
		       (draw & 0x1000 ? 0x0008 : 0) | (draw & 0x2000 ? 0x0004 : 0) | 
		       (draw & 0x8000 ? 0x0002 : 0) | (draw & 0x8000 ? 0x0001 : 0);
	}
	int vx1 = vectdir[dir][0];
	int vy1 = vectdir[dir][1];
	int vx2 = vectdir[dir][2];
	int vy2 = vectdir[dir][3];
	int muly = zw + 1;
	pattern = 0xffff;
	
	while(muly--) {
		int cx = dx;
		int cy = dy;
		int xrem = d;
		while(xrem--) {
			int mulx = zw + 1;
			if(draw & 1) {
				draw >>= 1;
				draw |= 0x8000;
				while(mulx--) {
					draw_pset(cx, cy);
					cx += vx1;
					cy += vy1;
				}
			} else {
				draw >>= 1;
				while(mulx--) {
					cx += vx1;
					cy += vy1;
				}
			}
		}
		dx += vx2;
		dy += vy2;
	}
	ead = (dx >> 4) + dy * pitch;
	if(plane_size) {
		ead += (plane_size >> 1) * plane;
	}
	dad = dx & 0x0f;
}

void UPD7220::draw_vectc()
{
	int m = (d * 10000 + 14141) / 14142;
	int t = (dc > m) ? m : dc;
	pattern = ra[8] | (ra[9] << 8);
	
	if(m) {
		switch(dir) {
		case 0:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx + s), (dy + i));
			}
			break;
		case 1:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx + i), (dy + s));
			}
			break;
		case 2:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx + i), (dy - s));
			}
			break;
		case 3:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx + s), (dy - i));
			}
			break;
		case 4:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx - s), (dy - i));
			}
			break;
		case 5:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx - i), (dy - s));
			}
			break;
		case 6:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx - i), (dy + s));
			}
			break;
		case 7:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset((dx - s), (dy + i));
			}
			break;
		}
	} else {
		draw_pset(dx, dy);
	}
}

void UPD7220::draw_vectr()
{
	int vx1 = vectdir[dir][0];
	int vy1 = vectdir[dir][1];
	int vx2 = vectdir[dir][2];
	int vy2 = vectdir[dir][3];
	pattern = ra[8] | (ra[9] << 8);
	
	for(int i = 0; i < d; i++) {
		draw_pset(dx, dy);
		dx += vx1;
		dy += vy1;
	}
	for(int i = 0; i < d2; i++) {
		draw_pset(dx, dy);
		dx += vx2;
		dy += vy2;
	}
	for(int i = 0; i < d; i++) {
		draw_pset(dx, dy);
		dx -= vx1;
		dy -= vy1;
	}
	for(int i = 0; i < d2; i++) {
		draw_pset(dx, dy);
		dx -= vx2;
		dy -= vy2;
	}
	ead = (dx >> 4) + dy * pitch;
	if(plane_size) {
		ead += (plane_size >> 1) * plane;
	}
	dad = dx & 0x0f;
}

void UPD7220::draw_text()
{
	int dir2 = dir + (sl ? 8 : 0);
	int vx1 = vectdir[dir2][0];
	int vy1 = vectdir[dir2][1];
	int vx2 = vectdir[dir2][2];
	int vy2 = vectdir[dir2][3];
	int sx = d;
	int sy = dc + 1;
#ifdef _QC10
	if(dir == 0 && sy == 40) {
		sy = 640; // ugly patch
	}
#endif
//	this->out_debug_log(_T("\tTEXT: dx=%d,dy=%d,sx=%d,sy=%d\n"), dx, dy, sx, sy);
	int index = 15;
	
	while(sy--) {
		int muly = zw + 1;
		while(muly--) {
			int cx = dx;
			int cy = dy;
			uint8_t bit = ra[index];
			int xrem = sx;
			while(xrem--) {
				pattern = (bit & 1) ? 0xffff : 0;
				bit = (bit >> 1) | ((bit & 1) ? 0x80 : 0);
				int mulx = zw + 1;
				while(mulx--) {
					draw_pset(cx, cy);
					cx += vx1;
					cy += vy1;
				}
			}
			dx += vx2;
			dy += vy2;
		}
		index = ((index - 1) & 7) | 8;
	}
	ead = (dx >> 4) + dy * pitch;
	if(plane_size) {
		ead += (plane_size >> 1) * plane;
	}
	dad = dx & 0x0f;
}

void UPD7220::draw_pset(int x, int y)
{
	uint16_t dot = pattern & 1;
	pattern = (pattern >> 1) | (dot << 15);
	uint32_t addr = y * (pitch << 1) + (x >> 3);
	if(plane_size) {
		addr += plane_size * plane;
	}
#ifdef UPD7220_MSB_FIRST
	uint8_t bit = 0x80 >> (x & 7);
#else
	uint8_t bit = 1 << (x & 7);
#endif
	uint8_t cur = read_vram(addr);
	
	switch(mod) {
	case 0: // replace
		write_vram(addr, (cur & ~bit) | (dot ? bit : 0));
		break;
	case 1: // complement
		write_vram(addr, (cur & ~bit) | ((cur ^ (dot ? 0xff : 0)) & bit));
		break;
	case 2: // reset
		write_vram(addr, cur & (dot ? ~bit : 0xff));
		break;
	case 3: // set
		write_vram(addr, cur | (dot ? bit : 0));
		break;
	}
}

#define STATE_VERSION	3

bool UPD7220::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(cmdreg);
	state_fio->StateValue(statreg);
	state_fio->StateArray(sync, sizeof(sync), 1);
	state_fio->StateValue(vtotal);
	state_fio->StateValue(vfp);
	state_fio->StateValue(vs);
	state_fio->StateValue(vbp);
	state_fio->StateValue(v1);
	state_fio->StateValue(v2);
	state_fio->StateValue(v3);
	state_fio->StateValue(v4);
	state_fio->StateValue(hfp);
	state_fio->StateValue(hs);
	state_fio->StateValue(hbp);
	state_fio->StateValue(h1);
	state_fio->StateValue(h2);
	state_fio->StateValue(h3);
	state_fio->StateValue(h4);
	state_fio->StateValue(sync_changed);
	state_fio->StateValue(master);
	state_fio->StateValue(zoom);
	state_fio->StateValue(zr);
	state_fio->StateValue(zw);
	state_fio->StateArray(ra, sizeof(ra), 1);
	state_fio->StateArray(cs, sizeof(cs), 1);
	state_fio->StateValue(pitch);
	state_fio->StateValue(lad);
	state_fio->StateArray(vect, sizeof(vect), 1);
	state_fio->StateValue(ead);
	state_fio->StateValue(dad);
	state_fio->StateValue(maskl);
	state_fio->StateValue(maskh);
	state_fio->StateValue(mod);
	state_fio->StateValue(hsync);
	state_fio->StateValue(hblank);
	state_fio->StateValue(vsync);
	state_fio->StateValue(vblank);
	state_fio->StateValue(start);
	state_fio->StateValue(blink_cursor);
	state_fio->StateValue(blink_attr);
	state_fio->StateValue(blink_rate);
	state_fio->StateValue(low_high);
	state_fio->StateValue(cmd_write_done);
	state_fio->StateValue(cpu_clocks);
#ifdef UPD7220_HORIZ_FREQ
	state_fio->StateValue(horiz_freq);
	state_fio->StateValue(next_horiz_freq);
#endif
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(lines_per_frame);
	state_fio->StateArray(params, sizeof(params), 1);
	state_fio->StateValue(params_count);
	if(!fo->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateArray(rt, sizeof(rt), 1);
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(plane);
	state_fio->StateValue(dir);
	state_fio->StateValue(dif);
	state_fio->StateValue(sl);
	state_fio->StateValue(dc);
	state_fio->StateValue(d);
	state_fio->StateValue(d2);
	state_fio->StateValue(d1);
	state_fio->StateValue(dm);
	state_fio->StateValue(pattern);
	
	// post process
	if(loading && master) {
		// force update timing
		vtotal = 0;
#ifdef UPD7220_HORIZ_FREQ
		horiz_freq = 0;
#endif
	}
	return true;
}

