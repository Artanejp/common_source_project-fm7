/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ uPD7220 ]
*/

#include <math.h>
#include "vm.h"
#include "../emu.h"
#include "upd7220.h"
#include "../fifo.h"

// -> See also: upd7220_base.cpp .
enum {
	CMD_RESET	= 0x00,
	CMD_SYNC	= 0x0e,
	CMD_SLAVE	= 0x6e,
	CMD_MASTER	= 0x6f,
	CMD_START	= 0x6b,
	CMD_BCTRL	= 0x0c,
	CMD_ZOOM	= 0x46,
	CMD_SCROLL	= 0x70,
	CMD_CSRFORM	= 0x4b,
	CMD_PITCH	= 0x47,
	CMD_LPEN	= 0xc0,
	CMD_VECTW	= 0x4c,
	CMD_VECTE	= 0x6c,
	CMD_TEXTW	= 0x78,
	CMD_TEXTE	= 0x68,
	CMD_CSRW	= 0x49,
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

void UPD7220::initialize()
{
	UPD7220_BASE::initialize();
	// initial settings for 1st frame
	vtotal = 0; //LINES_PER_FRAME;
	v1 = v2 = v3 = 16;
	v4 = LINES_PER_FRAME - v1 - v2 - v3;
#ifdef CHARS_PER_LINE
	h4 = CHARS_PER_LINE - v1 - v2 - v3;
#else
	h4 = width;
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

void UPD7220::check_cmd()
{
	// check fifo buffer and process command if enough params in fifo
	switch(cmdreg) {
	case CMD_RESET:
		cmd_reset();
		register_event_wait_cmd(0);
		break;
	case CMD_SYNC + 0:
	case CMD_SYNC + 1:
		if(params_count > 7) {
			cmd_sync();
			register_event_wait_cmd(0); // OK?
		}
		break;
	case CMD_MASTER:
		cmd_master();
			register_event_wait_cmd(0); // OK?
		break;
	case CMD_SLAVE:
		cmd_slave();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_START:
		cmd_start();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_BCTRL + 0:
		cmd_stop();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_BCTRL + 1:
		cmd_start();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_ZOOM:
		cmd_zoom();
		register_event_wait_cmd(0); // OK?
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
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_CSRFORM:
		cmd_csrform();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_PITCH:
		cmd_pitch();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_LPEN:
		cmd_lpen();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_VECTW:
		if(params_count > 10) {
			cmd_vectw();
			register_event_wait_cmd(0); // OK?
		}
		break;
	case CMD_VECTE:
		cmd_vecte();
		register_event_wait_cmd(wrote_bytes); // OK?
		break;
	case CMD_TEXTE:
		cmd_texte();
		register_event_wait_cmd(wrote_bytes); // OK?
		break;
	case CMD_CSRW:
		cmd_csrw();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_CSRR:
		cmd_csrr();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_MASK:
		cmd_mask();
		register_event_wait_cmd(0); // OK?
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
		cmd_dmaw();
		register_event_wait_cmd(0); // OK?
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
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_UNK_5A:
		cmd_unk_5a();
		register_event_wait_cmd(0); // OK?
		break;
	}
}

void UPD7220::process_cmd()
{
	switch(cmdreg) {
	case CMD_RESET:
		cmd_reset();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_SYNC + 0:
	case CMD_SYNC + 1:
		cmd_sync();
		register_event_wait_cmd(0); // OK?
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
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_VECTW:
		cmd_vectw();
		register_event_wait_cmd(0); // OK?
		break;
	case CMD_CSRW:
		cmd_csrw();
		register_event_wait_cmd(0); // OK?
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

void UPD7220::cmd_vecte()
{
	dx = ((ead % pitch) << 4) | (dad & 0x0f);
	dy = ead / pitch;
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
	dx = ((ead % pitch) << 4) | (dad & 0x0f);
	dy = ead / pitch;

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
	if(params_count > 0) {
#ifndef UPD7220_FIXED_PITCH
		pitch = params[0];
#endif
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
	dad = dx & 0x0f;
}

void UPD7220::draw_pset(int x, int y)
{
	uint16_t dot = pattern & 1;
	pattern = (pattern >> 1) | (dot << 15);
	uint32_t addr = y * width + (x >> 3);
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

void UPD7220::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0: // set parameter
//		this->out_debug_log(_T("\tPARAM = %2x\n"), data);
		if(cmd_ready) {
		//if(cmdreg != -1) {
			if(params_count < 16) {
				params[params_count++] = (uint8_t)(data & 0xff);
			}
			check_cmd();
			//if(cmdreg == -1) {
			//	params_count = 0;
			//}
		//}
		}
		break;
	case 1: // process prev command if not finished
		if(cmd_ready) {
			if(cmdreg != -1) {
				process_cmd();
			}
			// set new command
			cmdreg = (uint8_t)(data & 0xff);
//		this->out_debug_log(_T("CMDREG = %2x\n"), cmdreg);
//		params_count = 0;
			check_cmd();
		}
		break;
	case 2: // set zoom
		zoom = data;
		break;
	case 3: // light pen request
		break;
	}
}

// draw

void UPD7220::draw_vectl()
{
	pattern = ra[8] | (ra[9] << 8);
	
	if(dc) {
		int x = dx;
		int y = dy;
		int xbak = dy;
		int ybak = dy;
		int nwidth = width << 3;
		switch(dir) {
		case 0:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((x + step) >= nwidth) break; // ToDo: High RESO.
				if(y >= height) break;
				draw_pset(x + step, y++);
				wrote_bytes++;
			}
			break;
		case 1:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((y + step) >= height) break; // ToDo: High RESO.
				if(x >= nwidth) break;
				draw_pset(x++, y + step);
				if((x & 7) == 0) wrote_bytes++;
				if((ybak & 0xff8) != ((y + step) & 0xff8)) wrote_bytes++;
				ybak = y + step;
			}
			break;
		case 2:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((y - step) < 0) break; // ToDo: High RESO.
				if(x >= nwidth) break;
				draw_pset(x++, y - step);
				if((x & 7) == 0) wrote_bytes++;
				if((ybak & 0xff8) != ((y - step) & 0xff8)) wrote_bytes++;
				ybak = y - step;
			}
			break;
		case 3:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((x + step) >= nwidth) break; // ToDo: High RESO.
				if(y < 0) break;
				draw_pset(x + step, y--);
				wrote_bytes++;
			}
			break;
		case 4:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((x - step) < 0) break; // ToDo: High RESO.
				if(y < 0) break;
				draw_pset(x - step, y--);
				wrote_bytes++;
			}
			break;
		case 5:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((y - step) < 0) break; // ToDo: High RESO.
				if(x < 0) break;
				draw_pset(x--, y - step);
				if((x & 7) == 7) wrote_bytes++;
				if((ybak & 0xff8) != ((y - step) & 0xff8)) wrote_bytes++;
				ybak = y - step;
			}
			break;
		case 6:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((y + step) >= height) break; // ToDo: High RESO.
				if(x < 0) break;
				draw_pset(x--, y + step);
				if((x & 7) == 7) wrote_bytes++;
				if((ybak & 0xff8) != ((y + step) & 0xff8)) wrote_bytes++;
				ybak = y + step;
			}
			break;
		case 7:
			for(int i = 0; i <= dc; i++) {
				int step = (int)((((d1 * i) / dc) + 1) >> 1);
				if((x - step) < 0) break; // ToDo: High RESO.
				if(y >= height) break;
				draw_pset(x - step, y++);
				wrote_bytes++;
			}
			break;
		}
	} else {
		draw_pset(dx, dy);
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
	
	while(muly--) {
		int cx = dx;
		int cy = dy;
		int xrem = d;
		int xbak = cx;
		int ybak = cy;
		while(xrem--) {
			int mulx = zw + 1;
			if(draw & 1) {
				draw >>= 1;
				draw |= 0x8000;
				while(mulx--) {
					draw_pset(cx, cy);
					if(cx >= nwidth) goto _abort;
					if(cy >= height) goto _abort;
					if(cx < 0) goto _abort;
					if(cy < 0) goto _abort;
					cx += vx1;
					cy += vy1;
					if((cx & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
					if(cy != ybak) wrote_bytes++;
					xbak = cx;
					ybak = cy;
				}
			} else {
				draw >>= 1;
				while(mulx--) {
					if(cx >= nwidth) goto _abort;
					if(cy >= height) goto _abort;
					if(cx < 0) goto _abort;
					if(cy < 0) goto _abort;
					cx += vx1;
					cy += vy1;
					if((cx & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
					if(cy != ybak) wrote_bytes++;
					xbak = cx;
					ybak = cy;
				}
			}
		}
	_abort:
		dx += vx2;
		dy += vy2;
	}
	ead = (dx >> 4) + dy * pitch;
	dad = dx & 0x0f;
}

void UPD7220::draw_vectc()
{
	int m = (d * 10000 + 14141) / 14142;
	int t = (dc > m) ? m : dc;
	pattern = ra[8] | (ra[9] << 8);
	int xbak = dx;
	int ybak = dy;
	int nwidth = width << 3;
	if(m) {
		switch(dir) {
		case 0:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx + s) >= nwidth) break;
				if((dy + i) >= height) break;
				if((dx + s) < 0) break;
				if((dy + i) < 0) break;
				
				draw_pset((dx + s), (dy + i));
				
				if(((dx + s) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy + i) != ybak) wrote_bytes++;
				xbak = dx + s;
				ybak = dy + i;
			}
			break;
		case 1:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx + i) >= nwidth) break;
				if((dy + s) >= height) break;
				if((dx + i) < 0) break;
				if((dy + s) < 0) break;
				
				draw_pset((dx + i), (dy + s));
				
				if(((dx + i) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy + s) != ybak) wrote_bytes++;
				xbak = dx + i;
				ybak = dy + s;
			}
			break;
		case 2:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx + i) >= nwidth) break;
				if((dy - s) >= height) break;
				if((dx + i) < 0) break;
				if((dy - s) < 0) break;
				
				draw_pset((dx + i), (dy - s));

				if(((dx + i) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy - s) != ybak) wrote_bytes++;
				xbak = dx + i;
				ybak = dy - s;
			}
			break;
		case 3:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx + s) >= nwidth) break;
				if((dy - i) >= height) break;
				if((dx + s) < 0) break;
				if((dy - i) < 0) break;
				
				draw_pset((dx + s), (dy - i));

				if(((dx + s) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy - i) != ybak) wrote_bytes++;
				xbak = dx + s;
				ybak = dy - i;
			}
			break;
		case 4:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;
				
				if((dx - s) >= nwidth) break;
				if((dy - i) >= height) break;
				if((dx - s) < 0) break;
				if((dy - i) < 0) break;
				
				draw_pset((dx - s), (dy - i));
				
				if(((dx - s) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy - i) != ybak) wrote_bytes++;
				xbak = dx - s;
				ybak = dy - i;
			}
			break;
		case 5:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx - i) >= nwidth) break;
				if((dy - s) >= height) break;
				if((dx - i) < 0) break;
				if((dy - s) < 0) break;
				
				draw_pset((dx - i), (dy - s));
				
				if(((dx - i) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy - s) != ybak) wrote_bytes++;
				xbak = dx - i;
				ybak = dy - s;
			}
			break;
		case 6:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx - i) >= nwidth) break;
				if((dy + s) >= height) break;
				if((dx - i) < 0) break;
				if((dy + s) < 0) break;
				
				draw_pset((dx - i), (dy + s));

				if(((dx - i) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy + s) != ybak) wrote_bytes++;
				xbak = dx - i;
				ybak = dy + s;
			}
			break;
		case 7:
			for(int i = dm; i <= t; i++) {
				int s = (rt[(i << RT_TABLEBIT) / m] * d);
				s = (s + (1 << (RT_MULBIT - 1))) >> RT_MULBIT;

				if((dx - s) >= nwidth) break;
				if((dy + i) >= height) break;
				if((dx - s) < 0) break;
				if((dy + i) < 0) break;
				
				draw_pset((dx - s), (dy + i));
				
				if(((dx - s) & 0xff8) != (xbak & 0xff8)) wrote_bytes++;
				if((dy + i) != ybak) wrote_bytes++;
				xbak = dx - s;
				ybak = dy + i;
			}
			break;
		}
	} else {
		draw_pset(dx, dy);
		wrote_bytes++;
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
	
	for(int i = 0; i < d; i++) {
		if(dx >= nwidth) break;
		if(dy >= height) break;
		if(dx < 0) break;
		if(dy < 0) break;
		
		draw_pset(dx, dy);
		
		dx += vx1;
		dy += vy1;

		if((xbak & 0xff8) != (dx & 0xff8)) wrote_bytes++;
		if(ybak != dy) wrote_bytes++;
		xbak = dx;
		ybak = dy;
	}
	for(int i = 0; i < d2; i++) {
		if(dx >= nwidth) break;
		if(dy >= height) break;
		if(dx < 0) break;
		if(dy < 0) break;
		
		draw_pset(dx, dy);
		dx += vx2;
		dy += vy2;
		
		if((xbak & 0xff8) != (dx & 0xff8)) wrote_bytes++;
		if(ybak != dy) wrote_bytes++;
		xbak = dx;
		ybak = dy;
	}
	for(int i = 0; i < d; i++) {
		if(dx >= nwidth) break;
		if(dy >= height) break;
		if(dx < 0) break;
		if(dy < 0) break;
		
		draw_pset(dx, dy);
		dx -= vx1;
		dy -= vy1;
		
		if((xbak & 0xff8) != (dx & 0xff8)) wrote_bytes++;
		if(ybak != dy) wrote_bytes++;
		xbak = dx;
		ybak = dy;
	}
	for(int i = 0; i < d2; i++) {
		if(dx >= nwidth) break;
		if(dy >= height) break;
		if(dx < 0) break;
		if(dy < 0) break;
		
		draw_pset(dx, dy);
		dx -= vx2;
		dy -= vy2;
		
		if((xbak & 0xff8) != (dx & 0xff8)) wrote_bytes++;
		if(ybak != dy) wrote_bytes++;
		xbak = dx;
		ybak = dy;
	}
	ead = (dx >> 4) + dy * pitch;
	dad = dx & 0x0f;
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
	state_fio->StateValue(dir);
	state_fio->StateValue(dif);
	state_fio->StateValue(sl);
	state_fio->StateValue(dc);
	state_fio->StateValue(d);
	state_fio->StateValue(d2);
	state_fio->StateValue(d1);
	state_fio->StateValue(dm);
	state_fio->StateValue(pattern);

	state_fio->StateValue(clock_freq);
	state_fio->StateValue(wrote_bytes);
	state_fio->StateValue(cmd_ready);
	state_fio->StateValue(event_cmdready);

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

