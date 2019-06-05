/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ uPD7220 ]
*/

#include <math.h>
#include "upd7220.h"
#include "../ringbuffer.h"

#define EVENT_HSYNC_HFP	0
#define EVENT_HSYNC_HS	1
#define EVENT_HSYNC_HBP	2
#define EVENT_CMD_READY 3

enum {
	CMD_RESET1	= 0x00,
	CMD_RESET2	= 0x01,
//	CMD_STOP2	= 0x05, // Define later
//	CMD_START2	= 0x04,
	CMD_RESET3	= 0x09,
	
	CMD_BCTRL	= 0x0c, // 0C/0D
	CMD_SYNC	= 0x0e, // 0E/0F
	
	CMD_WRITE	= 0x20, // 20-3F
	CMD_DMAW	= 0x24,
	
	CMD_ZOOM	= 0x46,
	CMD_PITCH	= 0x47,
	CMD_CSRW	= 0x49,
	CMD_MASK	= 0x4a,
	CMD_CSRFORM	= 0x4b,
	CMD_VECTW	= 0x4c,
	
	CMD_TEXTE	= 0x68,
	CMD_START	= 0x6b,
	CMD_VECTE	= 0x6c,
	CMD_SLAVE	= 0x6e,
	CMD_MASTER	= 0x6f,
	
	CMD_SCROLL	= 0x70, // 70-7F
	CMD_TEXTW	= 0x78,
	
	CMD_READ	= 0xa0, // A9-BF
	CMD_DMAR	= 0xa4,
	
	CMD_LPEN	= 0xc0,
	CMD_CSRR	= 0xe0,
	/* unknown command (3 params) */
	CMD_UNK_5A	= 0x5a,
	CMD_STOP2	= 0x05,
	CMD_START2	= 0x04,
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

void UPD7220::initialize()
{
	DEVICE::initialize();
	if(osd->check_feature(_T("LINES_PER_FRAME"))) {
		_LINES_PER_FRAME = osd->get_feature_uint32_value(_T("LINES_PER_FRAME"));
	} else {
		_LINES_PER_FRAME = 0;
	}
	_UPD7220_UGLY_PC98_HACK = osd->check_feature(_T("UPD7220_UGLY_PC98_HACK"));
	
	vtotal = 0; //LINES_PER_FRAME;
	v1 = v2 = v3 = 16;
	v4 = _LINES_PER_FRAME - v1 - v2 - v3;
	
	if(osd->check_feature(_T("CHARS_PER_LINE"))) {
		h4 = (int)(osd->get_feature_int_value(_T("CHARS_PER_LINE")));
	} else {
		h4 = width;
	}
	if(osd->check_feature(_T("UPD7220_HORIZ_FREQ"))) {
		_UPD7220_HORIZ_FREQ = (int)(osd->get_feature_int_value(_T("UPD7220_HORIZ_FREQ")));
		horiz_freq = 0;
		next_horiz_freq = _UPD7220_HORIZ_FREQ;
	} else {
		_UPD7220_HORIZ_FREQ = 0;
	}
	if(osd->check_feature(_T("UPD7220_FIXED_PITCH"))) {
		_UPD7220_FIXED_PITCH = (int)(osd->get_feature_int_value(_T("UPD7220_FIXED_PITCH")));
	} else {
		_UPD7220_FIXED_PITCH = 0;
	}
	if(osd->check_feature(_T("UPD7220_A_VERSION"))) {
		_UPD7220_A_VERSION = (int)(osd->get_feature_int_value(_T("UPD7220_A_VERSION")));
	} else {
		_UPD7220_A_VERSION = 0;
	}
	__QC10 = osd->check_feature(_T("_QC10"));
	_UPD7220_MSB_FIRST = osd->check_feature(_T("UPD7220_MSB_FIRST"));
	
	sync_changed = false;
	sync_mask = false;
	vs = hs = 0;
	for(int i = 0; i <= RT_TABLEMAX; i++) {
		rt[i] = (int)((double)(1 << RT_MULBIT) * (1 - sqrt(1 - pow((0.70710678118654 * i) / RT_TABLEMAX, 2))));
	}
	// Design manual p.104 "because FIFO is implemented as a ring buffer".
	fo = new RINGBUFFER(0x10000);
	cmd_fifo = new RINGBUFFER(32); // OK?
	
	vsync = vblank = false;
	hsync = hblank = false;
	master = false;
	pitch = 40;	// 640dot

	event_cmdready = -1;
	wrote_bytes = 0;
	cmd_drawing = false;

	first_load = true;
	before_addr = 0xffffffff;
	cache_val = 0;
	// register events
	register_frame_event(this);
	register_vline_event(this);
}

void UPD7220::release()
{
	fo->release();
	delete fo;
	cmd_fifo->release();
	delete cmd_fifo;
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

		if(_UPD7220_HORIZ_FREQ != 0) {
			horiz_freq = 0;
		}

	}
	if((master) || (sync_mask)) {
		if(vtotal != v1 + v2 + v3 + v4) {
			vtotal = v1 + v2 + v3 + v4;
			set_lines_per_frame(vtotal);
		}
		if(_UPD7220_HORIZ_FREQ != 0) {
			if(horiz_freq != next_horiz_freq && vtotal != 0) {
				horiz_freq = next_horiz_freq;
				out_debug_log("Set lines to %d ,freq to %f \n", vtotal, (double)horiz_freq / (double)vtotal);
				set_frames_per_sec((double)horiz_freq / (double)vtotal);
			}
		}
	}
}

void UPD7220::reset()
{
	if(event_cmdready >= 0) cancel_event(this, event_cmdready);
	event_cmdready = -1;
	cmd_reset();
}

void UPD7220::check_cmd()
{
	// check fifo buffer and process command if enough params in fifo
	switch(cmdreg) {
	case CMD_RESET1:
		cmd_reset();
		break;
	case CMD_RESET2:
		cmd_reset2();
		break;
	case CMD_RESET3:
		cmd_reset3();
		break;
	case CMD_SYNC + 0:
	case CMD_SYNC + 1:
		//if(cmd_fifo->count() > 7) {
			cmd_sync();
		//}
		break;
	case CMD_MASTER:
		cmd_master();
		break;
	case CMD_SLAVE:
		cmd_slave();
		break;
	case CMD_START:
		cmd_start();
		break;
	case CMD_STOP2:
		cmd_stop();
		sync_mask = true;
		break;
	case CMD_BCTRL + 0:
		cmd_stop();
		sync_mask = false;
		break;
	case CMD_START2:
	case CMD_BCTRL + 1:
		cmd_start();
		sync_mask = true;
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
		//if(cmd_fifo->count() > 10) {
			cmd_vectw();
		//}
		break;
	case CMD_VECTE:
		cmd_drawing = true;
		cmd_vecte();
		if(cmdreg < 0) register_event_wait_cmd(wrote_bytes); // OK?
		break;
	case CMD_TEXTE:
		cmd_drawing = true;
		cmd_texte();
		if(cmdreg < 0) register_event_wait_cmd(wrote_bytes); // OK?
		break;
	case CMD_CSRW:
		//if(cmd_fifo->count() > 2) {
			cmd_csrw();
		//}
		break;
	case CMD_CSRR:
		cmd_drawing = true;
		cmd_csrr();
		register_event_wait_cmd(wrote_bytes); // OK?
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
		cmd_drawing = true;
		cmd_write();
		register_event_wait_cmd(wrote_bytes);
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
		cmd_drawing = true;
		cmd_read();
		register_event_wait_cmd(wrote_bytes);
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
		// ToDo: Wait for DMA cycle
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
		cmd_drawing = true;
		cmd_unk_5a();
		register_event_wait_cmd(wrote_bytes); // OK?
		break;
	}
}

void UPD7220::process_cmd()
{
	switch(cmdreg) {
	case CMD_RESET1:
		cmd_reset();
		break;
	case CMD_RESET2:
		cmd_reset2();
		break;
	case CMD_RESET3:
		cmd_reset3();
		break;
	case CMD_SYNC + 0:
	case CMD_SYNC + 1:	
		cmd_sync();
		break;
	case CMD_CSRFORM:
		cmd_csrform();
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
		update_vect();
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
			//reset_vect();
		}
		cmdreg = -1;
		cmd_write_done = false;
		break;
	}
}

void UPD7220::cmd_vecte()
{
	dx = ((ead % pitch) << 4) | (dad & 0x0f);
	dy = ead / pitch;
	wrote_bytes = 1;

	// execute command
//	pattern = ra[8] | (ra[9] << 8);
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
	if(_UPD7220_UGLY_PC98_HACK) {
		dx = (((ead & 0x3fff)% pitch) << 4) | (dad & 0x0f);
		dy = (ead & 0x3fff) / pitch;
	} else {
		dx = ((ead % pitch) << 4) | (dad & 0x0f);
		dy = ead / pitch;
	}

	wrote_bytes = 1;
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

void UPD7220::cmd_pitch()
{
	if(!(cmd_fifo->empty())) {
		wrote_bytes = 1;
		uint8_t tmp = (uint8_t)(cmd_fifo->read() & 0xff);
		if(_UPD7220_FIXED_PITCH == 0) {
			pitch = ((sync[4] & 0x40) != 0) ? tmp + 256 : tmp;
		}
		cmdreg = -1;
	}
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
	if(__QC10) {
		if(dir == 0 && sy == 40) {
			sy = 640; // ugly patch
		}
	}
//	this->out_debug_log(_T("\tTEXT: dx=%d,dy=%d,sx=%d,sy=%d\n"), dx, dy, sx, sy);
	int index = 15;
	int ead_bak = ead;
	int dad_bak = dad;
	
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
				start_pset();
				while(mulx--) {
					draw_pset(cx, cy);
					cx += vx1;
					cy += vy1;
				}
				finish_pset();
			}
			dx += vx2;
			dy += vy2;
		}
		index = ((index - 1) & 7) | 8;
	}
//	ead = ead_bak;
//	dad = dad_bak;
//	ead = (dx >> 4) + dy * pitch;
//	dad = dx & 0x0f;
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
		//this->out_debug_log(_T("\tPARAM = %2x\n"), data);
		//if(cmd_ready) { // OK?
		if(cmdreg != -1) {
			cmd_fifo->write(data & 0xff);
			check_cmd();
			if(cmdreg == -1) {
				cmd_fifo->clear(); // OK?
			}
			//	}
		}
		break;
	case 1: // process prev command if not finished
		//if(cmd_ready) {
		if(cmdreg != -1) {
			process_cmd();
		}
			// set new command
			// reset pointers, then enqueue command.
		cs_ptr = 0;
		sync_ptr = 0;
		vectw_ptr = 0;
		csrw_ptr = 0;
		cmdreg = (int)data & 0xff;
		//this->out_debug_log(_T("CMDREG = %2x\n"), cmdreg);
//		params_count = 0;
		cmd_fifo->clear(); // OK?
		check_cmd();
		if(cmdreg == -1) {
//			cmd_fifo->clear(); // OK?
		}
//		}
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
		val |= (cmd_drawing) ? STAT_DRAW : 0;
		val |= (cmd_fifo->empty()) ? STAT_EMPTY : 0;
		//val |= STAT_EMPTY;
		val |= (cmd_fifo->full()) ? STAT_FULL : 0;
		val |= (!(fo->empty())) ? STAT_DRDY : 0;
		// clear busy stat
		statreg &= ~(STAT_DMA | STAT_DRAW);
		return val;
	case 1: // data
		if(!(fo->empty())) {
			return fo->read();
		}
		return 0xff;
	}
	return 0xff;
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

void UPD7220::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_UPD7220_CLOCK_FREQ:
		clock_freq = data; // Must be Hz.
		break;
	case SIG_UPD7220_EXT_VSYNC:
		//ToDo:Implement this."0" makes reset some paramaters.
		break;
	}
}

uint32_t UPD7220::read_signal(int ch)
{
	switch(ch) {
	case SIG_UPD7220_CLOCK_FREQ:
		return clock_freq;
		break;
	case SIG_UPD7220_WIDTH_BYTES:
		return (width < 0) ? 0 : width;
		break;
	case SIG_UPD7220_HEIGHT:
		return (height < 0) ? 0 : height;
		break;
	case SIG_UPD7220_PITCH:
		return (pitch < 0) ? 0 : pitch;
		break;
	}
	return 0;
}
		

void UPD7220::event_vline(int v, int clock)
{
	if(v == 0) {
		vblank = true;
		write_signals(&outputs_vblank, 0xffffffff);
	} else if(v == vfp) {
		write_signals(&outputs_vsync, 0xffffffff);
		vsync = true;
	} else if(v == vs) {
		write_signals(&outputs_vsync, 0);
		vsync = false;
	} else if(v == vbp) {
		vblank = false;
		write_signals(&outputs_vblank, 0x00000000);
 	}
	hblank = true;
	register_event_by_clock(this, EVENT_HSYNC_HFP, hfp, false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_HS,  hs,  false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_HBP, hbp, false, NULL);
}

void UPD7220::register_event_wait_cmd(uint32_t bytes)
{
	if(event_cmdready >= 0) cancel_event(this, event_cmdready);
	event_cmdready = -1;
	// BY uPD7220 GDC Design manual; Clock divided by 2 at internal logic.
	double usec = (1.0e6  * 2.0 * (double)bytes) / (double)clock_freq;
	if(usec < 1.0) {
	    cmd_drawing = false;
	} else {
		register_event(this, EVENT_CMD_READY, usec, false, &event_cmdready);
	}
}

				   
void UPD7220::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HSYNC_HFP) {
		hsync = true;
	} else if(event_id == EVENT_HSYNC_HS) {
		hsync = false;
	} else if(event_id == EVENT_HSYNC_HBP) {
		hblank = false;
	} else if(event_id == EVENT_CMD_READY) {
		event_cmdready = -1;
		cmd_drawing = false;
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

void UPD7220::cmd_reset()
{
	finish_pset();
	// init gdc params
	sync[6] = 0x90;
	sync[7] = 0x01;
	sync_ptr = 0;
	zoom = zr = zw = 0;
	ra[0] = ra[1] = ra[2] = 0;
	ra[3] = 0x1e; /*0x19;*/
	cs[0] = cs[1] = cs[2] = 0;
	cs_ptr = 0;
	ead = dad = 0;
	wg = false;
	maskl = maskh = 0xff;
	mod = 0;
	blink_cursor = 0;
	blink_attr = 0;
	blink_rate = 16;
	
	// init fifo
	cmd_drawing = false;
	fo->clear();
	cmd_fifo->clear();
	
	// stop display and drawing
	start = false;
	statreg = 0;
	cmdreg = -1;
	cmd_write_done = false;
	cmd_drawing = false;
	sync_mask = false;
	write_signals(&outputs_vsync, 0x00000000);
	write_signals(&outputs_vblank, 0xffffffff);
}
void UPD7220::cmd_reset2()
{
	cmd_reset();
	if(_UPD7220_A_VERSION > 0) {
		sync_mask = true;
	}
}

void UPD7220::cmd_reset3()
{
	cmd_reset();
	if(_UPD7220_A_VERSION > 0) {
		sync_mask = true;
		start = true;
	}
}

void UPD7220::cmd_sync()
{
	start = ((cmdreg & 1) != 0);
	int len = cmd_fifo->count();
	wrote_bytes = (len >= 8) ? 8 : len;
	if(sync_ptr >= 8) sync_ptr = 0;
	for(int i = 0; i < 8 && i < len; i++) {
		uint8_t dat = (uint8_t)(cmd_fifo->read() & 0xff); 
		if(sync[sync_ptr] != dat) {
			sync[sync_ptr] = dat;
			sync_changed = true;
		}
		sync_ptr++;
		if(sync_ptr >= 8) {
			sync_ptr = 0;
			cmdreg = -1;
		}
	}
	//cmdreg = -1;
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
	sync_mask = true;
	cmdreg = -1;
}

void UPD7220::cmd_stop()
{
	start = false;
	cmdreg = -1;
}

void UPD7220::cmd_zoom()
{
	wrote_bytes = 0;
	if(cmd_fifo->count() >= 1) {
		wrote_bytes = 1;
		uint8_t tmp = (uint8_t)(cmd_fifo->read() & 0xff);
		zr = tmp >> 4;
		zw = tmp & 0x0f;
		cmdreg = -1;
	}
}

void UPD7220::cmd_scroll()
{
	if(cmd_fifo->count() >= 1) {
		ra[cmdreg & 0x0f] = (uint8_t)(cmd_fifo->read() & 0xff);
		wrote_bytes = 1;
		if(cmdreg < 0x7f) {
			cmdreg++;
			//cmd_fifo->clear(); // OK?
		} else {
			cmdreg = -1;
		}
	}
}

void UPD7220::cmd_csrform()
{
	int len = cmd_fifo->count();
	if(len > 3) len = 3;
	wrote_bytes = len;
	cs_ptr = cs_ptr % 3;
	for(int i = 0; i < len; i++) {
		cs[cs_ptr++] = (uint8_t)(cmd_fifo->read() & 0xff);
		if(cs_ptr > 2) {
			cmdreg = -1;
			cs_ptr = 0;
		}
	}
	blink_rate = (cs[1] >> 6) | ((cs[2] & 7) << 2);
}

void UPD7220::cmd_lpen()
{
	wrote_bytes = 3;
	fo->write(lad & 0xff);
	fo->write((lad >> 8) & 0xff);
	fo->write((lad >> 16) & 0xff);
	cmdreg = -1;
}

void UPD7220::cmd_vectw()
{
	int len = cmd_fifo->count();
	if(len > 11) len = 11;
	wrote_bytes += len;
	for(int i = 0; i < 11 && i < len; i++) {
		vect[vectw_ptr++] = (uint8_t)(cmd_fifo->read() & 0xff);
		if(vectw_ptr >= 11) {
			vectw_ptr = 0;
			cmdreg = -1;
			update_vect();
			break;
		}
//		this->out_debug_log(_T("\tVECT[%d] = %2x\n"), i, vect[i]);
	}
	update_vect();

//	cmdreg = -1;
}


void UPD7220::cmd_csrw()
{
#if 0
	if(!(cmd_fifo->empty())) {

		ead = cmd_fifo->read() & 0xff;
		wrote_bytes = 1;
		if(!(cmd_fifo->empty())) {
			ead |= (cmd_fifo->read() & 0xff) << 8;
			wrote_bytes = 2;
			if(!(cmd_fifo->empty())) {
				ead |= (cmd_fifo->read() & 0xff) << 16;
				wrote_bytes = 3;
				cmdreg = -1;
			}
		}
		dad = (ead >> 20) & 0x0f;
		ead &= 0x3ffff;
	}
#else
	csrw_ptr = csrw_ptr % 3;
	if(!(cmd_fifo->empty())) {
		if(csrw_ptr == 0) {
			ead = 0;
			dad = 0;
			wg = false;
		}
		int len = cmd_fifo->count();
		if(len > 3) len = 3;
		for(int i = 0; i < len; i++) {
			int nmask = ~(0xff << (csrw_ptr * 8));
			int val = cmd_fifo->read() & 0xff;
			val = val << (csrw_ptr * 8);
			
			ead = ead & nmask;
			ead = ead | val;
			csrw_ptr++;
			if(csrw_ptr >= 3) {
				csrw_ptr = 0;
				cmdreg = -1;
				break;
			}
		}
		dad = (ead >> 20) & 0x0f;
		wg = ((ead & 0x080000) != 0) ? true : false;
		ead &= 0x3ffff;
	}
#endif
}

void UPD7220::cmd_csrr()
{
	wrote_bytes = 5;
	fo->write(ead & 0xff);
	fo->write((ead >> 8) & 0xff);
	fo->write((ead >> 16) & 0x03);
	fo->write(dad & 0xff);
	fo->write((dad >> 8) & 0xff);
	cmdreg = -1;
}

void UPD7220::cmd_mask()
{
	if(cmd_fifo->count() > 2) {
		maskl = (cmd_fifo->read() & 0xff);
		maskh = (cmd_fifo->read() & 0xff);
		cmdreg = -1;
	}
}

void UPD7220::cmd_write()
{
	mod = cmdreg & 3;
	wrote_bytes = 0;
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		if(cmd_fifo->count() > 1) {
			uint8_t l = (uint8_t)(cmd_fifo->read()) & maskl;
			uint8_t h = (uint8_t)(cmd_fifo->read()) & maskh;
			wrote_bytes = 2;
			for(int i = 0; i < dc + 1; i++) {
				cmd_write_sub(ead * 2 + 0, l);
				cmd_write_sub(ead * 2 + 1, h);
				ead += dif;
				if(!(cmd_fifo->empty())) {
					l = (uint8_t)(cmd_fifo->read()) & maskl;
					h = (uint8_t)(cmd_fifo->read()) & maskh;
				}
			}
			wrote_bytes += ((dc + 1) * 2 * 2);
			cmd_write_done = true;
			//cmd_fifo->clear(); // OK?
		}
		break;
	case 0x10: // low byte
		if(cmd_fifo->count() > 0) {
			wrote_bytes = 1;
			uint8_t l = (uint8_t)(cmd_fifo->read()) & maskl;
			for(int i = 0; i < dc + 1; i++) {
				cmd_write_sub(ead * 2 + 0, l);
				ead += dif;
				if(!(cmd_fifo->empty())) {
					l = (uint8_t)(cmd_fifo->read()) & maskl;
				}
			}
			wrote_bytes += ((dc + 1) * 2);
			cmd_write_done = true;
			//cmd_fifo->clear(); // OK?
		}
		break;
	case 0x18: // high byte
		if(cmd_fifo->count() > 0) {
			wrote_bytes = 1;
			uint8_t h = (uint8_t)(cmd_fifo->read()) & maskh;
			for(int i = 0; i < dc + 1; i++) {
				cmd_write_sub(ead * 2 + 1, h);
				if(!(cmd_fifo->empty())) {
					h = (uint8_t)(cmd_fifo->read()) & maskh;
				}
				ead += dif;
			}
			wrote_bytes += ((dc + 1) * 2);
			cmd_write_done = true;
			//cmd_fifo->clear(); // OK?
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
	wrote_bytes = 0;
	switch(cmdreg & 0x18) {
	case 0x00: // low and high
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 0));
			fo->write(read_vram(ead * 2 + 1));
			ead += dif;
		}
		wrote_bytes = dc * 2 * 2;
		break;
	case 0x10: // low byte
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 0));
			ead += dif;
		}
		wrote_bytes = dc * 2;
		break;
	case 0x18: // high byte
		for(int i = 0; i < dc; i++) {
			fo->write(read_vram(ead * 2 + 1));
			ead += dif;
		}
		wrote_bytes = dc * 2;
		break;
	default: // invalid
		break;
	}
//	reset_vect();
	cmdreg = -1;
}

void UPD7220::cmd_dmaw()
{
	mod = cmdreg & 3;
	low_high = false;
	write_signals(&outputs_drq, 0xffffffff);
//	reset_vect();
	statreg |= STAT_DMA;
	cmdreg = -1;
}

void UPD7220::cmd_dmar()
{
	mod = cmdreg & 3;
	low_high = false;
	write_signals(&outputs_drq, 0xffffffff);
//	reset_vect();
	statreg |= STAT_DMA;
	cmdreg = -1;
}

void UPD7220::cmd_unk_5a()
{
	wrote_bytes = 0;
	int len = cmd_fifo->count();
	if(len > 2) {
		wrote_bytes = len;
		cmdreg = -1;
		cmd_fifo->clear();
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
// 		uint8_t mask = (addr & 1) ? (vram_data_mask >> 8) : (vram_data_mask & 0xff);
		uint8_t mask = 0xff;
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
	dgd = ((vect[2] & 0x40) != 0) ? true : false;
//	if(dc & 0x2000) dc = -(dc & 0x1fff);
//	if(d & 0x2000)  d = -(d & 0x1fff); 
//	if(d2 & 0x2000) d2 = -(d2 & 0x1fff);
//	if(d1 & 0x2000) d1 = -(d1 & 0x1fff);
//	if(dc & 0x2000) dc = -(dc & 0x1fff);
//	if(dm & 0x2000) dm = -(dm & 0x1fff);
}

void UPD7220::reset_vect()
{
	vect[ 1] = 0x00;
	vect[ 2] = 0x00;
	vect[ 3] = 0x08;
	vect[ 4] = 0x00;
	vect[ 5] = 0x08;
	vect[ 6] = 0x00;
	vect[ 7] = 0xff;
	vect[ 8] = 0xff;
	vect[ 9] = 0xff;
	vect[10] = 0xff;
	vectw_ptr = 0;
	update_vect();
}



void UPD7220::draw_vectl()
{
	pattern = ra[8] | (ra[9] << 8);
	
	//out_debug_log(_T("DRAW VECTL: X=%d Y=%d to DC=%d D1=%d DIR=%d PATTERN=%04x MODE=%d\n"), dx, dy, dc, d1, dir, pattern, mod);
	dc = dc & 0x3fff;
	if(dc) {
		int x = dx;
		int y = dy;
		int step;
		int stepx = 0;
		int stepy = 0;
		int stepn = 0;
		
		step = (((d1 << 12) / dc)) >> 1;
		//stepn = step & ((1 << 14) - 1);
		start_pset();
		switch(dir) {
		case 0:
			for(int i = 0; i <= dc; i++) {
				//stepx = (int)((((d1 * i) / dc) + 1) >> 1);
				stepx += step;
				draw_pset_diff(x + (stepx >> 12), y++);
			}
			break;
		case 1:
			for(int i = 0; i <= dc; i++) {
				//stepy = (int)((((d1 * i) / dc) + 1) >> 1);
				stepy += step;
				draw_pset_diff(x++, y + (stepy >> 12));
			}
			break;
		case 2:
			for(int i = 0; i <= dc; i++) {
				//stepy = (int)((((d1 * i) / dc) + 1) >> 1);
				stepy += step;
				draw_pset_diff(x++, y - (stepy >> 12));
				//draw_pset_diff(x++, y - step);
			}
			break;
		case 3:
			for(int i = 0; i <= dc; i++) {
				//stepx = (int)((((d1 * i) / dc) + 1) >> 1);
				stepx += step;
				draw_pset_diff(x + (stepx >> 12), y--);
			}
			break;
		case 4:
			for(int i = 0; i <= dc; i++) {
				//stepx = (int)((((d1 * i) / dc) + 1) >> 1);
				stepx += step;
				draw_pset_diff(x - (stepx >> 12), y--);
			}
			break;
		case 5:
			for(int i = 0; i <= dc; i++) {
//				stepy = (int)((((d1 * i) / dc) + 1) >> 1);
				stepy += step;
				draw_pset_diff(x--, y - (stepy >> 12));
			}
			break;
		case 6:
			for(int i = 0; i <= dc; i++) {
//				stepy = (int)((((d1 * i) / dc) + 1) >> 1);
				stepy += step;
				draw_pset_diff(x--, y + (stepy >> 12));
			}
			break;
		case 7:
			for(int i = 0; i <= dc; i++) {
				//stepx = (int)((((d1 * i) / dc) + 1) >> 1);
				stepx += step;
				draw_pset_diff(x - (stepx >> 12), y++);
			}
			break;
		}
		finish_pset();
	} else {
		start_pset();
		draw_pset_diff(dx, dy);
		finish_pset();
		wrote_bytes++;
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
	int nwidth = width << 3;
	pattern = 0xffff;
	
	int ead_bak = ead;
	int dad_bak = dad;
	while(muly--) {
		int cx = dx;
		int cy = dy;
		int xrem = d;
		while(xrem--) {
			int mulx = zw + 1;
			if(draw & 1) {
				draw >>= 1;
				draw |= 0x8000;
				start_pset();
				while(mulx--) {
					draw_pset(cx, cy);
					cx += vx1;
					cy += vy1;
				}
				cx += (vx1 * mulx);
				cy += (vy1 * mulx);
				mulx = 0;
				finish_pset();
			} else {
				draw >>= 1;
				//while(mulx--) {
				//	cx += vx1;
				//	cy += vy1;
				//}
				cx += (vx1 * mulx);
				cy += (vy1 * mulx);
				mulx = 0;
			}
		}
	_abort:
		dx += vx2;
		dy += vy2;
	}
//	ead = (dx >> 4) + dy * pitch;
//	dad = dx & 0x0f;
}

void UPD7220::draw_vectc()
{
	int m = (d * 10000 + 14141) / 14142;
	int t = (dc > m) ? m : dc;
	pattern = ra[8] | (ra[9] << 8);
	int xbak = dx;
	int ybak = dy;
	if(m) {
		start_pset();
		switch(dir) {
		case 0:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx + s), (dy + i));
			}
			break;
		case 1:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx + i), (dy + s));
			}
			break;
		case 2:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx + i), (dy - s));
			}
			break;
		case 3:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx + s), (dy - i));
			}
			break;
		case 4:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx - s), (dy - i));
			}
			break;
		case 5:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				draw_pset_diff((dx - i), (dy - s));
			}
			break;
		case 6:
			start_pset();
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx - i), (dy + s));
			}
			break;
		case 7:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				draw_pset_diff((dx - s), (dy + i));
			}
			break;
		}
		finish_pset();
	} else {
		start_pset();
		draw_pset_diff(dx, dy);
		finish_pset();
		//wrote_bytes++;
	}
}

void UPD7220::draw_vectr()
{
	int vx1 = vectdir[dir][0];
	int vy1 = vectdir[dir][1];
	int vx2 = vectdir[dir][2];
	int vy2 = vectdir[dir][3];
	int nwidth = width <<3;
	int xbak = dx;
	int ybak = dy;
	pattern = ra[8] | (ra[9] << 8);
//	out_debug_log(_T("DRAW VECTR: X=%d Y=%d to DC=%d DIR=%d PATTERN=%04x MODE=%d VX1=%d VY1=%d VX2=%d VY2=%d\n"), dx, dy, dc, dir, pattern, mod, vx1, vy1, vx2, vy2);
	start_pset();
	for(int c = 0; c < d; c++) {
		draw_pset_diff(dx, dy);
		dx += vx1;
		dy += vy1;
	}
	for(int c = 0; c < d2; c++) {
		draw_pset_diff(dx, dy);
		dx += vx2;
		dy += vy2;
	}
	for(int c = 0; c < d; c++) {
		draw_pset_diff(dx, dy);
		dx -= vx1;
		dy -= vy1;
	}
	for(int c = 0; c < d2; c++) {
		draw_pset_diff(dx, dy);
		dx -= vx2;
		dy -= vy2;
	}
	
	finish_pset();
//	dx = xbak;
//	dy = ybak;

//	ead = (dx >> 4) + dy * pitch;
//	dad = dx & 0x0f;
}

#define STATE_VERSION	9

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
	state_fio->StateValue(wg);
	
	state_fio->StateValue(maskl);
	state_fio->StateValue(maskh);
	state_fio->StateValue(mod);
	state_fio->StateValue(hsync);
	state_fio->StateValue(hblank);
	state_fio->StateValue(vsync);
	state_fio->StateValue(vblank);
	state_fio->StateValue(start);
	state_fio->StateValue(sync_mask);

	state_fio->StateValue(blink_cursor);
	state_fio->StateValue(blink_attr);
	state_fio->StateValue(blink_rate);
	state_fio->StateValue(low_high);
	state_fio->StateValue(cmd_write_done);
	state_fio->StateValue(cpu_clocks);
	if(_UPD7220_HORIZ_FREQ != 0) {
		state_fio->StateValue(horiz_freq);
		state_fio->StateValue(next_horiz_freq);
	}
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(lines_per_frame);
	if(!fo->process_state((void *)state_fio, loading)) {
 		return false;
 	}
	state_fio->StateArray(rt, sizeof(rt), 1);
	state_fio->StateValue(dx);
	state_fio->StateValue(dy);
	state_fio->StateValue(dir);
	state_fio->StateValue(dif);
	state_fio->StateValue(sl);
	state_fio->StateValue(dc);
	state_fio->StateValue(d);
	state_fio->StateValue(d2);
	state_fio->StateValue(d1);
	state_fio->StateValue(dm);
	state_fio->StateValue(pattern);
	state_fio->StateValue(dgd);

	state_fio->StateValue(clock_freq);
	state_fio->StateValue(wrote_bytes);
	state_fio->StateValue(cmd_drawing);
	state_fio->StateValue(event_cmdready);
	if(!cmd_fifo->process_state((void *)state_fio, loading)) {
 		return false;
 	}
	state_fio->StateValue(first_load);
	state_fio->StateValue(before_addr);
	state_fio->StateValue(cache_val);
	state_fio->StateValue(cs_ptr);
	state_fio->StateValue(sync_ptr);
	state_fio->StateValue(vectw_ptr);
	state_fio->StateValue(csrw_ptr);
 	// post process
	if(loading && master) {
 		// force update timing
 		vtotal = 0;
		if(_UPD7220_HORIZ_FREQ != 0) {
			horiz_freq = 0;
		}
	}
	return true;
}




