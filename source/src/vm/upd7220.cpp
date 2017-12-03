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
	vtotal = LINES_PER_FRAME;
	v1 = v2 = v3 = 16;
	v4 = vtotal - v1 - v2 - v3;
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
		if(horiz_freq != next_horiz_freq) {
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
		break;
	case CMD_SYNC + 0:
	case CMD_SYNC + 1:
		if(params_count > 7) {
			cmd_sync();
		}
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
	case CMD_BCTRL + 0:
		cmd_stop();
		break;
	case CMD_BCTRL + 1:
		cmd_start();
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
	switch(cmdreg) {
	case CMD_RESET:
		cmd_reset();
		break;
	case CMD_SYNC + 0:
	case CMD_SYNC + 1:
		cmd_sync();
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

void UPD7220::cmd_vecte()
{
	dx = ((ead % pitch) << 4) | (dad & 0x0f);
	dy = ead / pitch;
	
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
		cmdreg = (uint8_t)(data & 0xff);
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
	dad = dx & 0x0f;
}

#define STATE_VERSION	2

void UPD7220::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(cmdreg);
	state_fio->FputUint8(statreg);
	state_fio->Fwrite(sync, sizeof(sync), 1);
	state_fio->FputInt32(vtotal);
	state_fio->FputInt32(vfp);
	state_fio->FputInt32(vs);
	state_fio->FputInt32(vbp);
	state_fio->FputInt32(v1);
	state_fio->FputInt32(v2);
	state_fio->FputInt32(v3);
	state_fio->FputInt32(v4);
	state_fio->FputInt32(hfp);
	state_fio->FputInt32(hs);
	state_fio->FputInt32(hbp);
	state_fio->FputInt32(h1);
	state_fio->FputInt32(h2);
	state_fio->FputInt32(h3);
	state_fio->FputInt32(h4);
	state_fio->FputBool(sync_changed);
	state_fio->FputBool(master);
	state_fio->FputUint8(zoom);
	state_fio->FputUint8(zr);
	state_fio->FputUint8(zw);
	state_fio->Fwrite(ra, sizeof(ra), 1);
	state_fio->Fwrite(cs, sizeof(cs), 1);
	state_fio->FputUint8(pitch);
	state_fio->FputUint32(lad);
	state_fio->Fwrite(vect, sizeof(vect), 1);
	state_fio->FputInt32(ead);
	state_fio->FputInt32(dad);
	state_fio->FputUint8(maskl);
	state_fio->FputUint8(maskh);
	state_fio->FputUint8(mod);
	state_fio->FputBool(hsync);
	state_fio->FputBool(hblank);
	state_fio->FputBool(vsync);
	state_fio->FputBool(vblank);
	state_fio->FputBool(start);
	state_fio->FputInt32(blink_cursor);
	state_fio->FputInt32(blink_attr);
	state_fio->FputInt32(blink_rate);
	state_fio->FputBool(low_high);
	state_fio->FputBool(cmd_write_done);
	state_fio->FputInt32(cpu_clocks);
#ifdef UPD7220_HORIZ_FREQ
	state_fio->FputInt32(horiz_freq);
	state_fio->FputInt32(next_horiz_freq);
#endif
	state_fio->FputDouble(frames_per_sec);
	state_fio->FputInt32(lines_per_frame);
	state_fio->Fwrite(params, sizeof(params), 1);
	state_fio->FputInt32(params_count);
	fo->save_state((void *)state_fio);
	state_fio->Fwrite(rt, sizeof(rt), 1);
	state_fio->FputInt32(dx);
	state_fio->FputInt32(dy);
	state_fio->FputInt32(dir);
	state_fio->FputInt32(dif);
	state_fio->FputInt32(sl);
	state_fio->FputInt32(dc);
	state_fio->FputInt32(d);
	state_fio->FputInt32(d2);
	state_fio->FputInt32(d1);
	state_fio->FputInt32(dm);
	state_fio->FputUint16(pattern);
}

bool UPD7220::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	cmdreg = state_fio->FgetInt32();
	statreg = state_fio->FgetUint8();
	state_fio->Fread(sync, sizeof(sync), 1);
	vtotal = state_fio->FgetInt32();
	vfp = state_fio->FgetInt32();
	vs = state_fio->FgetInt32();
	vbp = state_fio->FgetInt32();
	v1 = state_fio->FgetInt32();
	v2 = state_fio->FgetInt32();
	v3 = state_fio->FgetInt32();
	v4 = state_fio->FgetInt32();
	hfp = state_fio->FgetInt32();
	hs = state_fio->FgetInt32();
	hbp = state_fio->FgetInt32();
	h1 = state_fio->FgetInt32();
	h2 = state_fio->FgetInt32();
	h3 = state_fio->FgetInt32();
	h4 = state_fio->FgetInt32();
	sync_changed = state_fio->FgetBool();
	master = state_fio->FgetBool();
	zoom = state_fio->FgetUint8();
	zr = state_fio->FgetUint8();
	zw = state_fio->FgetUint8();
	state_fio->Fread(ra, sizeof(ra), 1);
	state_fio->Fread(cs, sizeof(cs), 1);
	pitch = state_fio->FgetUint8();
	lad = state_fio->FgetUint32();
	state_fio->Fread(vect, sizeof(vect), 1);
	ead = state_fio->FgetInt32();
	dad = state_fio->FgetInt32();
	maskl = state_fio->FgetUint8();
	maskh = state_fio->FgetUint8();
	mod = state_fio->FgetUint8();
	hsync = state_fio->FgetBool();
	hblank = state_fio->FgetBool();
	vsync = state_fio->FgetBool();
	vblank = state_fio->FgetBool();
	start = state_fio->FgetBool();
	blink_cursor = state_fio->FgetInt32();
	blink_attr = state_fio->FgetInt32();
	blink_rate = state_fio->FgetInt32();
	low_high = state_fio->FgetBool();
	cmd_write_done = state_fio->FgetBool();
	cpu_clocks = state_fio->FgetInt32();
#ifdef UPD7220_HORIZ_FREQ
	horiz_freq = state_fio->FgetInt32();
	next_horiz_freq = state_fio->FgetInt32();
#endif
	frames_per_sec = state_fio->FgetDouble();
	lines_per_frame = state_fio->FgetInt32();
	state_fio->Fread(params, sizeof(params), 1);
	params_count = state_fio->FgetInt32();
	if(!fo->load_state((void *)state_fio)) {
		return false;
	}
	state_fio->Fread(rt, sizeof(rt), 1);
	dx = state_fio->FgetInt32();
	dy = state_fio->FgetInt32();
	dir = state_fio->FgetInt32();
	dif = state_fio->FgetInt32();
	sl = state_fio->FgetInt32();
	dc = state_fio->FgetInt32();
	d = state_fio->FgetInt32();
	d2 = state_fio->FgetInt32();
	d1 = state_fio->FgetInt32();
	dm = state_fio->FgetInt32();
	pattern = state_fio->FgetUint16();
	return true;
}

