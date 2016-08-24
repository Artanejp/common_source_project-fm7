/*
	Skelton for retropc emulator

	Origin : MAME HD63484
	Author : Takeda.Toshiya
	Date   : 2009.02.09 -

	[ HD63484 ]
*/

#include "hd63484.h"

#define ADDR_MASK	(vram_size - 1)
#define CCR		regs[0x02 >> 1]
#define MWR1		regs[0xca >> 1]

static const int instruction_length[64] =
{
	 0, 3, 2, 1,
	 0, 0,-1, 2,
	 0, 3, 3, 3,
	 0, 0, 0, 0,
	 0, 1, 2, 2,
	 0, 0, 4, 4,
	 5, 5, 5, 5,
	 5, 5, 5, 5,
	 3, 3, 3, 3,
	 3, 3,-2,-2,
	-2,-2, 2, 4,
	 5, 5, 7, 7,
	 3, 3, 1, 1,
	 2, 2, 2, 2,
	 5, 5, 5, 5,
	 5, 5, 5, 5
};

void HD63484::initialize()
{
	register_vline_event(this);
}

void HD63484::reset()
{
	ch = fifo_ptr = 0;
}

void HD63484::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		write_io16(0, data);
		break;
	}
}

uint32_t HD63484::read_io8(uint32_t addr)
{
	switch(addr & 3) {
	case 0:
		return read_io16(0) & 0xff;
	case 1:
		return read_io16(0) >> 8;
	}
	return 0xff;
}

void HD63484::write_io16(uint32_t addr, uint32_t data)
{
	if(addr & 2) {
		// data
		regs[ch >> 1] = data;
		if(ch & 0x80) {
			// auto increment
			ch = ((ch + 2) & 0x7f) | 0x80;
		}
		if(ch == 0) {
			fifo[fifo_ptr++] = regs[0];
			process_cmd();
		}
	} else {
		// register no
		ch = data & 0xff;
	}
}

uint32_t HD63484::read_io16(uint32_t addr)
{
	if(addr & 2) {
		// data
		if(ch == 0x80) {
			return vpos;
		} else if(ch == 0) {
			return readfifo;
		} else {
			return 0;
		}
	} else {
		// status
		return 0xff22 | (fifo_ptr ? 0 : 1) | (rand() & 4);	// write FIFO ready + command end + (read FIFO ready or read FIFO not ready)
	}
}

void HD63484::event_vline(int v, int clock)
{
	vpos = v;
}

void HD63484::process_cmd()
{
	int len = instruction_length[fifo[0] >> 10];
	
	if(len == -1) {
		if(fifo_ptr < 2) {
			return;
		}
		len = fifo[1] + 2;
	} else if(len == -2) {
		if(fifo_ptr < 2) {
			return;
		}
		len = 2 * fifo[1] + 2;
	}
	if(fifo_ptr >= len) {
		if(fifo[0] == 0x400) {
			// ORG
			org = ((fifo[1] & 0xff) << 12) | ((fifo[2] & 0xfff0) >> 4);
			org_dpd = fifo[2] & 0xf;
		} else if((fifo[0] & 0xffe0) == 0x800) {
			// WPR
			if(fifo[0] == 0x800) {
				cl0 = fifo[1];
			} else if(fifo[0] == 0x801) {
				cl1 = fifo[1];
			} else if(fifo[0] == 0x802) {
				ccmp = fifo[1];
			} else if(fifo[0] == 0x803) {
				edg = fifo[1];
			} else if(fifo[0] == 0x804) {
				mask = fifo[1];
			} else if(fifo[0] == 0x805) {
				ppy  = (fifo[1] & 0xf000) >> 12;
				pzcy = (fifo[1] & 0x0f00) >> 8;
				ppx  = (fifo[1] & 0x00f0) >> 4;
				pzcx = (fifo[1] & 0x000f) >> 0;
			} else if(fifo[0] == 0x806) {
				psy  = (fifo[1] & 0xf000) >> 12;
				psx  = (fifo[1] & 0x00f0) >> 4;
			} else if(fifo[0] == 0x807) {
				pey  = (fifo[1] & 0xf000) >> 12;
				pzy  = (fifo[1] & 0x0f00) >> 8;
				pex  = (fifo[1] & 0x00f0) >> 4;
				pzx  = (fifo[1] & 0x000f) >> 0;
			} else if(fifo[0] == 0x808) {
				xmin = fifo[1];
			} else if(fifo[0] == 0x809) {
				ymin = fifo[1];
			} else if(fifo[0] == 0x80a) {
				xmax = fifo[1];
			} else if(fifo[0] == 0x80b) {
				ymax = fifo[1];
			} else if(fifo[0] == 0x80c) {
				rwp = (rwp & 0xfff) | ((fifo[1] & 0xff) << 12);
				rwp_dn = (fifo[1] & 0xc000) >> 14;
			} else if(fifo[0] == 0x80d) {
				rwp = (rwp & 0xff000) | ((fifo[1] & 0xfff0) >> 4);
			} else {
				this->out_debug_log(_T("HD63484: unsupported register\n"));
			}
		} else if((fifo[0] & 0xfff0) == 0x1800) {
			// WPTN
			int start = fifo[0] & 0xf;
			int n = fifo[1];
			for(int i = 0; i < n; i++) {
				pattern[start + i] = fifo[2 + i];
			}
		} else if(fifo[0] == 0x4400) {
			// RD
			readfifo = vram[rwp];
			rwp = (rwp + 1) & ADDR_MASK;
		} else if(fifo[0] == 0x4800) {
			// WT
			vram[rwp] = fifo[1];
			rwp = (rwp + 1) & ADDR_MASK;
		} else if(fifo[0] == 0x5800) {
			// CLR
			doclr16(fifo[0], fifo[1], &rwp, fifo[2], fifo[3]);
			int fifo2 = (int)fifo[2], fifo3 = (int)fifo[3];
			if(fifo2 < 0) fifo2 *= -1;
			if(fifo3 < 0) fifo3 *= -1;
			rwp += (fifo2 + 1) * (fifo3 + 1);
		} else if((fifo[0] & 0xfffc) == 0x5c00) {
			// SCLR
			doclr16(fifo[0], fifo[1], &rwp, fifo[2], fifo[3]);
			int fifo2 = (int)fifo[2], fifo3 = (int)fifo[3];
			if(fifo2 < 0) fifo2 *= -1;
			if(fifo3 < 0) fifo3 *= -1;
			rwp += (fifo2 + 1) * (fifo3 + 1);
		} else if((fifo[0] & 0xf0ff) == 0x6000) {
			// CPY
			docpy16(fifo[0], ((fifo[1] & 0xff) << 12) | ((fifo[2] & 0xfff0) >> 4), &rwp, fifo[3], fifo[4]);
			int fifo2 = (int)fifo[2], fifo3 = (int)fifo[3];
			if(fifo2 < 0) fifo2 *= -1;
			if(fifo3 < 0) fifo3 *= -1;
			rwp += (fifo2 + 1) * (fifo3 + 1);
		} else if((fifo[0] & 0xf0fc) == 0x7000) {
			// SCPY
			docpy16(fifo[0], ((fifo[1] & 0xff) << 12) | ((fifo[2] & 0xfff0) >> 4), &rwp, fifo[3], fifo[4]);
			int fifo2 = (int)fifo[2], fifo3 = (int)fifo[3];
			if(fifo2 < 0) fifo2 *= -1;
			if(fifo3 < 0) fifo3 *= -1;
			rwp += (fifo2 + 1) * (fifo3 + 1);
		} else if(fifo[0] == 0x8000) {
			// AMOVE
			cpx = fifo[1];
			cpy = fifo[2];
		} else if(fifo[0] == 0x8400) {
			// RMOVE
			cpx += (int16_t)fifo[1];
			cpy += (int16_t)fifo[2];
		} else if((fifo[0] & 0xff00) == 0x8800) {
			// ALINE
			line(cpx, cpy, fifo[1], fifo[2], fifo[0] & 0xff);
			cpx = (int16_t)fifo[1];
			cpy = (int16_t)fifo[2];
		} else if((fifo[0] & 0xff00) == 0x8c00) {
			// RLINE
			line(cpx, cpy, cpx + (int16_t)fifo[1], cpy + (int16_t)fifo[2], fifo[0] & 0xff);
			cpx += (int16_t)fifo[1];
			cpy += (int16_t)fifo[2];
		} else if((fifo[0] & 0xfff8) == 0x9000) {
			// ARCT
			line(cpx, cpy, (int16_t)fifo[1], cpy, fifo[0] & 0xff);
			line((int16_t)fifo[1], cpy, (int16_t)fifo[1], (int16_t)fifo[2], fifo[0] & 0xff);
			line((int16_t)fifo[1], (int16_t)fifo[2], cpx, (int16_t)fifo[2], fifo[0] & 0xff);
			line(cpx, (int16_t)fifo[2], cpx, cpy, fifo[0] & 0xff);
			cpx = (int16_t)fifo[1];
			cpy = (int16_t)fifo[2];
		} else if((fifo[0] & 0xfff8) == 0x9400) {
			// RRCT
			line(cpx, cpy, cpx + (int16_t)fifo[1], cpy, fifo[0] & 0xff);
			line(cpx + (int16_t)fifo[1], cpy, cpx + (int16_t)fifo[1], cpy + (int16_t)fifo[2], fifo[0] & 0xff);
			line(cpx + (int16_t)fifo[1], cpy + (int16_t)fifo[2], cpx, cpy + (int16_t)fifo[2], fifo[0] & 0xff);
			line(cpx, cpy + (int16_t)fifo[2], cpx, cpy, fifo[0] & 0xff);
			cpx += (int16_t)fifo[1];
			cpy += (int16_t)fifo[2];
		} else if((fifo[0] & 0xfff8) == 0xa400) {
			// RPLG  added
			int sx = cpx;
			int sy = cpy;
			for(int nseg = 0; nseg < fifo[1]; nseg++) {
				int ex = sx + (int16_t)fifo[2 + nseg * 2];
				int ey = sy + (int16_t)fifo[2 + nseg * 2 + 1];
				line(sx, sy, ex, ey, fifo[0] & 7);
				sx = ex;
				sy = ey;
			}
			line(sx, sy, cpx, cpy, fifo[0] & 7);
		} else if((fifo[0] & 0xfff8) == 0xc000) {
			// AFRCT
			int16_t pcx = fifo[1], pcy = fifo[2];
			int16_t ax = pcx - cpx, ay = pcy - cpy;
			int16_t xx = cpx, yy = cpy;
			for(;;) {
				for(;;) {
					dot(xx, yy, fifo[0] & 7, cl0);
					if(ax == 0) {
						break;
					}
					if(ax > 0) {
						xx++;
						ax--;
					} else {
						xx--;
						ax++;
					}
				}
				ax = pcx - cpx;
				if(pcy < cpy) {
					yy--;
					xx -= ax;
					if(ay == 0) {
						break;
					}
					ay++;
				} else {
					yy++;
					xx -= ax;
					if(ay == 0) {
						break;
					}
					ay--;
				}
			}
		} else if((fifo[0] & 0xfff8) == 0xc400) {
			// RFRCT
			line(cpx, cpy, cpx + (int16_t)fifo[1], cpy, fifo[0] & 0xff);
			line(cpx + fifo[1], cpy, cpx + fifo[1], cpy + fifo[2], fifo[0] & 0xff);
			line(cpx + fifo[1], cpy + fifo[2], cpx, cpy + fifo[2], fifo[0] & 0xff);
			line(cpx, cpy + fifo[2], cpx, cpy, fifo[0] & 0xff);
			cpx=cpx + (int16_t)fifo[1];
			cpy=cpy + (int16_t)fifo[2];
		} else if(fifo[0] == 0xc800) {
			// PAINT
			paint(cpx, cpy, cl0);
		} else if((fifo[0] & 0xfff8) == 0xcc00) {
			// DOT
			dot(cpx, cpy, fifo[0] & 0xff, cl0);
		} else if((fifo[0] & 0xf000) == 0xd000) {
			// PTN
			ptn(fifo[0], psx, psy, pex - psx, pey - psy);
			if(!(fifo[0] & 0x800)) {
				switch(fifo[0] & 0x700) {
				case 0x000:
					if(pey - psy > 0) {
						cpy += pey - psy;
					} else {
						cpy -= pey - psy;
					}
					break;
				case 0x100:
					// missing
					break;
				case 0x200:
					if(pey - psy > 0) {
						cpx += pey - psy;
					} else {
						cpx -= pey - psy;
					}
					break;
				case 0x300:
					// missing
					break;
				case 0x400:
					if(pey - psy > 0) {
						cpy -= pey - psy;
					} else {
						cpy += pey - psy;
					}
					break;
				case 0x500:
					// missing
					break;
				case 0x600:
					if(pey - psy > 0) {
						cpx -= pey - psy;
					} else {
						cpx += pey - psy;
					}
					break;
				case 0x700:
					// missing
					break;
				}
			} else {
				// missing
			}
		} else if((fifo[0] & 0xf018) == 0xe000) {
			// AGCPY
			agcpy(fifo[0], (int16_t)fifo[1], (int16_t)fifo[2], cpx, cpy, fifo[3], fifo[4]);
			switch(fifo[0] & 0x700) {
			case 0x000:
				if(fifo[4] > 0) {
					cpy += fifo[4];
				} else {
					cpy -= fifo[4];
				}
				break;
			case 0x100:
				if(fifo[4] > 0) {
					cpy -= fifo[4];
				} else {
					cpy += fifo[4];
				}
				break;
			case 0x200:
				if(fifo[4] > 0) {
					cpy += fifo[4];
				} else {
					cpy -= fifo[4];
				}
				break;
			case 0x300:
				if(fifo[4] > 0) {
					cpy -= fifo[4];
				} else {
					cpy += fifo[4];
				}
				break;
			case 0x400:
				if(fifo[3] > 0) {
					cpx += fifo[3];
				} else {
					cpx -= fifo[3];
				}
				break;
			case 0x500:
				if(fifo[3] > 0) {
					cpx += fifo[3];
				} else {
					cpx -= fifo[3];
				}
				break;
			case 0x600:
				if(fifo[3] > 0) {
					cpx -= fifo[3];
				} else {
					cpx += fifo[3];
				}
				break;
			case 0x700:
				if(fifo[3] > 0) {
					cpx -= fifo[3];
				} else {
					cpx += fifo[3];
				}
				break;
			}
		} else {
			this->out_debug_log(_T("unsupported command\n"));
		}
		fifo_ptr = 0;
	}
}

void HD63484::doclr16(int opcode, uint16_t fill, int *dst, int _ax, int _ay)
{
	int ax = _ax, ay = _ay;
	
	for(;;) {
		for(;;) {
			switch(opcode & 3) {
			case 0:
				vram[*dst & ADDR_MASK] = fill;
				break;
			case 1:
				vram[*dst & ADDR_MASK] |= fill;
				break;
			case 2:
				vram[*dst & ADDR_MASK] &= fill;
				break;
			case 3:
				vram[*dst & ADDR_MASK] ^= fill;
				break;
			}
			if(ax == 0) {
				break;
			}
			if(ax > 0) {
				*dst = (*dst + 1) & ADDR_MASK;
				ax--;
			} else {
				*dst = (*dst - 1) & ADDR_MASK;
				ax++;
			}
		}
		ax = _ax;
		if(_ay < 0) {
			*dst = (*dst + (MWR1 & 0xfff) - ax) & ADDR_MASK;
			if(ay == 0) {
				break;
			}
			ay++;
		} else {
			*dst = (*dst - (MWR1 & 0xfff) - ax) & ADDR_MASK;
			if(ay == 0) {
				break;
			}
			ay--;
		}
	}
}

void HD63484::docpy16(int opcode, int src, int *dst, int _ax, int _ay)
{
	int dstep1, dstep2;
	int ax = _ax, ay = _ay;
	
	switch(opcode & 0x700) {
	case 0x000: dstep1 =  1; dstep2 = -1 * (MWR1 & 0xfff) - ax * dstep1; break;
	case 0x100: dstep1 =  1; dstep2 =      (MWR1 & 0xfff) - ax * dstep1; break;
	case 0x200: dstep1 = -1; dstep2 = -1 * (MWR1 & 0xfff) + ax * dstep1; break;
	case 0x300: dstep1 = -1; dstep2 =      (MWR1 & 0xfff) + ax * dstep1; break;
	case 0x400: dstep1 = -1 * (MWR1 & 0xfff); dstep2 =  1 - ay * dstep1; break;
	case 0x500: dstep1 =      (MWR1 & 0xfff); dstep2 =  1 - ay * dstep1; break;
	case 0x600: dstep1 = -1 * (MWR1 & 0xfff); dstep2 = -1 + ay * dstep1; break;
	case 0x700: dstep1 =      (MWR1 & 0xfff); dstep2 = -1 + ay * dstep1; break;
	}
	for(;;) {
		for(;;) {
			switch(opcode & 7) {
			case 0:
				vram[*dst] = vram[src];
				break;
			case 1:
				vram[*dst] |= vram[src];
				break;
			case 2:
				vram[*dst] &= vram[src];
				break;
			case 3:
				vram[*dst] ^= vram[src];
				break;
			case 4:
				if(vram[*dst] == (ccmp & 0xff)) {
					vram[*dst] = vram[src];
				}
				break;
			case 5:
				if(vram[*dst] != (ccmp & 0xff)) {
					vram[*dst] = vram[src];
				}
				break;
			case 6:
				if(vram[*dst] < vram[src]) {
					vram[*dst] = vram[src];
				}
				break;
			case 7:
				if(vram[*dst] > vram[src]) {
					vram[*dst] = vram[src];
				}
				break;
			}
			if(opcode & 0x800) {
				if(ay == 0) {
					break;
				}
				if(_ay > 0) {
					src = (src - (MWR1 & 0xfff)) & ADDR_MASK;
					*dst = (*dst + dstep1) & ADDR_MASK;
					ay--;
				} else {
					src = (src + (MWR1 & 0xfff)) & ADDR_MASK;
					*dst = (*dst + dstep1) & ADDR_MASK;
					ay++;
				}
			} else {
				if(ax == 0) {
					break;
				}
				if(ax > 0) {
					src = (src + 1) & ADDR_MASK;
					*dst = (*dst + dstep1) & ADDR_MASK;
					ax--;
				} else {
					src = (src - 1) & ADDR_MASK;
					*dst = (*dst + dstep1) & ADDR_MASK;
					ax++;
				}
			}
		}
		if(opcode & 0x800) {
			ay = _ay;
			if(_ax < 0) {
				src = (src - 1 + ay * (MWR1 & 0xfff)) & ADDR_MASK;
				*dst = (*dst + dstep2) & ADDR_MASK;
				if(ax == 0) {
					break;
				}
				ax++;
			} else {
				src = (src + 1 - ay * (MWR1 & 0xfff)) & ADDR_MASK;
				*dst = (*dst + dstep2) & ADDR_MASK;
				if(ax == 0) {
					break;
				}
				ax--;
			}
		} else {
			ax = _ax;
			if(_ay < 0) {
				src = (src + (MWR1 & 0xfff) - ax) & ADDR_MASK;
				*dst = (*dst + dstep2) & ADDR_MASK;
				if(ay == 0) {
					break;
				}
				ay++;
			} else {
				src = (src - (MWR1 & 0xfff) - ax) & ADDR_MASK;
				*dst = (*dst + dstep2) & ADDR_MASK;
				if(ay == 0) {
					break;
				}
				ay--;
			}
		}
	}
}

int HD63484::org_first_pixel(int _org_dpd)
{
	int gbm = (CCR & 0x700) >> 8;
	
	switch(gbm) {
	case 0:
		return (_org_dpd & 0xf);
	case 1:
		return (_org_dpd & 0xf) >> 1;
	case 2:
		return (_org_dpd & 0xf) >> 2;
	case 3:
		return (_org_dpd & 0xf) >> 3;
	case 4:
		return 0;
	}
	this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
	return 0;
}

void HD63484::dot(int x, int y, int opm, uint16_t color)
{
	int dst, x_int, x_mod, bpp;
	uint16_t color_shifted, bitmask, bitmask_shifted;
	
	x += org_first_pixel(org_dpd);
	
	switch((CCR & 0x700) >> 8) {
	case 0:
		bpp = 1;
		bitmask = 0x0001;
		break;
	case 1:
		bpp = 2;
		bitmask = 0x0003;
		break;
	case 2:
		bpp = 4;
		bitmask = 0x000f;
		break;
	case 3:
		bpp = 8;
		bitmask = 0x00ff;
		break;
	case 4:
		bpp = 16;
		bitmask = 0xffff;
		break;
	default:
		this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
		bpp = 0;
		bitmask = 0x0000;
	}
	if(x >= 0) {
		x_int = x / (16 / bpp);
		x_mod = x % (16 / bpp);
	} else {
		x_int = x / (16 / bpp);
		x_mod = -1 * (x % (16 / bpp));
		if(x_mod) {
			x_int--;
			x_mod = (16 / bpp) - x_mod;
		}
	}
	
	color &= bitmask;
	bitmask_shifted = bitmask << (x_mod * bpp);
	color_shifted = color << (x_mod * bpp);
	dst = (org + x_int - y * (MWR1 & 0xfff)) & ADDR_MASK;
	
	switch(opm) {
	case 0:
		vram[dst] = (vram[dst] & ~bitmask_shifted) | color_shifted;
		break;
	case 1:
		vram[dst] = vram[dst] | color_shifted;
		break;
	case 2:
		vram[dst] = vram[dst] & ((vram[dst] & ~bitmask_shifted) | color_shifted);
		break;
	case 3:
		vram[dst] = vram[dst] ^ color_shifted;
		break;
	case 4:
		if(get_pixel(x, y) == (ccmp & bitmask)) {
			vram[dst] = (vram[dst] & ~bitmask_shifted) | color_shifted;
		}
		break;
	case 5:
		if(get_pixel(x, y) != (ccmp & bitmask)) {
			vram[dst] = (vram[dst] & ~bitmask_shifted) | color_shifted;
		}
		break;
	case 6:
		if(get_pixel(x, y) < (cl0 & bitmask)) {
			vram[dst] = (vram[dst] & ~bitmask_shifted) | color_shifted;
		}
		break;
	case 7:
		if(get_pixel(x, y) > (cl0 & bitmask)) {
			vram[dst] = (vram[dst] & ~bitmask_shifted) | color_shifted;
		}
		break;
	}
}

int HD63484::get_pixel(int x, int y)
{
	int dst, x_int, x_mod, bpp;
	uint16_t bitmask, bitmask_shifted;
	
	switch((CCR & 0x700) >> 8) {
	case 0:
		bpp = 1;
		bitmask = 0x0001;
		break;
	case 1:
		bpp = 2;
		bitmask = 0x0003;
		break;
	case 2:
		bpp = 4;
		bitmask = 0x000f;
		break;
	case 3:
		bpp = 8;
		bitmask = 0x00ff;
		break;
	case 4:
		bpp = 16;
		bitmask = 0xffff;
		break;
	default:
		this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
		bpp = 0;
		bitmask = 0x0000;
	}
	if(x >= 0) {
		x_int = x / (16 / bpp);
		x_mod = x % (16 / bpp);
	} else {
		x_int = x / (16 / bpp);
		x_mod = -1 * (x % (16 / bpp));
		if(x_mod) {
			x_int--;
			x_mod = (16 / bpp) - x_mod;
		}
	}
	bitmask_shifted = bitmask << (x_mod * bpp);
	dst = (org + x_int - y * (MWR1 & 0xfff)) & ADDR_MASK;
	
	return ((vram[dst] & bitmask_shifted) >> (x_mod * bpp));
}

int HD63484::get_pixel_ptn(int x, int y)
{
	int dst, x_int, x_mod, bpp;
	uint16_t bitmask, bitmask_shifted;
	
	bpp = 1;
	bitmask = 1;
	
	if(x >= 0) {
		x_int = x / (16 / bpp);
		x_mod = x % (16 / bpp);
	} else {
		x_int = x / (16 / bpp);
		x_mod = -1 * (x % (16 / bpp));
		if(x_mod) {
			x_int--;
			x_mod = (16 / bpp) - x_mod;
		}
	}
	bitmask_shifted = bitmask << (x_mod * bpp);
	dst = (x_int + y * 1);
	
	if((pattern[dst] & bitmask_shifted) >> (x_mod * bpp)) {
		return 1;
	} else {
		return 0;
	}
}

void HD63484::agcpy(int opcode, int src_x, int src_y, int dst_x, int dst_y, int16_t _ax, int16_t _ay)
{
	int dst_step1_x, dst_step1_y, dst_step2_x, dst_step2_y;
	int src_step1_x, src_step1_y, src_step2_x, src_step2_y;
	int ax = _ax;
	int ay = _ay;
	int ax_neg = (_ax >= 0) ? 1 : -1;
	int ay_neg = (_ay >= 0) ? 1 : -1;
	int xxs = src_x;
	int yys = src_y;
	int xxd = dst_x;
	int yyd = dst_y;
	
	if(opcode & 0x800) {
		switch(opcode & 0x700) {
		case 0x000: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ay_neg * ay; dst_step2_y =  1; break;
		case 0x100: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ay_neg * ay; dst_step2_y = -1; break;
		case 0x200: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ay_neg * ay; dst_step2_y =  1; break;
		case 0x300: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ay_neg * ay; dst_step2_y = -1; break;
		case 0x400: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x =  1; dst_step2_y = -ay_neg * ay; break;
		case 0x500: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x =  1; dst_step2_y =  ay_neg * ay; break;
		case 0x600: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x = -1; dst_step2_y = -ay_neg * ay; break;
		case 0x700: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x = -1; dst_step2_y =  ay_neg * ay; break;
		}
		src_step1_x = 0;
		src_step1_y = (_ay >= 0) ? 1 : -1;
		src_step2_x = (_ax >= 0) ? 1 : -1;
		src_step2_y = -ay;
	} else {
		switch(opcode & 0x700) {
		case 0x000: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ax_neg * ax; dst_step2_y =  1; break;
		case 0x100: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ax_neg * ax; dst_step2_y = -1; break;
		case 0x200: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ax_neg * ax; dst_step2_y =  1; break;
		case 0x300: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ax_neg * ax; dst_step2_y = -1; break;
		case 0x400: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x =  1; dst_step2_y =  ax_neg * ax; break;
		case 0x500: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x =  1; dst_step2_y = -ax_neg * ax; break;
		case 0x600: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x = -1; dst_step2_y =  ax_neg * ax; break;
		case 0x700: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x = -1; dst_step2_y = -ax_neg * ax; break;
		}
		src_step1_x = (_ax >= 0) ? 1 : -1;
		src_step1_y = 0;
		src_step2_x = -ax;
		src_step2_y = (_ay >= 0) ? 1 : -1;
	}
	for(;;) {
		for(;;) {
			dot(xxd, yyd, opcode & 7, get_pixel(xxs, yys));
			if(opcode & 0x800) {
				if(ay == 0) {
					break;
				}
				if(_ay > 0) {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay--;
				} else {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay++;
				}
			} else {
				if(ax == 0) {
					break;
				}
				if(ax > 0) {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax--;
				} else {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax++;
				}
			}
		}
		if(opcode & 0x800) {
			ay = _ay;
			if(_ax < 0) {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ax == 0) {
					break;
				}
				ax++;
			} else {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ax == 0) {
					break;
				}
				ax--;
			}
		} else {
			ax = _ax;
			if(_ay < 0) {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ay == 0) {
					break;
				}
				ay++;
			} else {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ay == 0) {
					break;
				}
				ay--;
			}
		}
	}
}

void HD63484::ptn(int opcode, int src_x, int src_y, int16_t _ax, int16_t _ay)
{
	int dst_step1_x = 0, dst_step1_y = 0, dst_step2_x = 0, dst_step2_y = 0;
	int src_step1_x = 1, src_step1_y = 0, src_step2_x = -_ax, src_step2_y = 1;
	int ax = _ax;
	int ay = _ay;
	int ax_neg = (_ax >= 0) ? 1 : -1;
	int ay_neg = (_ay >= 0) ? 1 : -1;
	int xxs = src_x;
	int yys = src_y;
	int xxd = cpx;
	int yyd = cpy;
	int getpixel;
	
	if(opcode & 0x800) {
		this->out_debug_log(_T("HD63484 ptn not supported\n"));
	} else {
		switch(opcode & 0x700) {
			case 0x000: dst_step1_x =  1; dst_step1_y =  0; dst_step2_x = -ax_neg * ax; dst_step2_y =  1; break;
			case 0x100: this->out_debug_log(_T("HD63484 ptn not supported\n")); break;
			case 0x200: dst_step1_x =  0; dst_step1_y =  1; dst_step2_x = -1; dst_step2_y = -ax_neg * ax; break;
			case 0x300: this->out_debug_log(_T("HD63484 ptn not supported\n")); break;
			case 0x400: dst_step1_x = -1; dst_step1_y =  0; dst_step2_x =  ax_neg * ax; dst_step2_y = -1; break;
			case 0x500: this->out_debug_log(_T("HD63484 ptn not supported\n")); break;
			case 0x600: dst_step1_x =  0; dst_step1_y = -1; dst_step2_x =  1; dst_step2_y =  ax_neg * ax; break;
			case 0x700: this->out_debug_log(_T("HD63484 ptn not supported\n")); break;
		}
	}
	for(;;) {
		for(;;) {
			getpixel = get_pixel_ptn(xxs, yys);
			switch((opcode >> 3) & 3) {
			case 0:
				if(getpixel) {
					dot(xxd, yyd, opcode & 7, cl1);
				} else {
					dot(xxd, yyd, opcode & 7, cl0);
				}
				break;
			case 1:
				if(getpixel) {
					dot(xxd, yyd, opcode & 7, cl1);
				}
				break;
			case 2:
				if(getpixel == 0) {
					dot(xxd, yyd, opcode & 7, cl0);
				}
				break;
			case 3:
				this->out_debug_log(_T("HD63484 ptn not supported\n"));
				break;
			}
			if(opcode & 0x800) {
				if(ay == 0) {
					break;
				}
				if(_ay > 0) {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay--;
				} else {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ay++;
				}
			} else {
				if(ax == 0) {
					break;
				}
				if(ax > 0) {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax--;
				} else {
					xxs += src_step1_x;
					yys += src_step1_y;
					xxd += dst_step1_x;
					yyd += dst_step1_y;
					ax++;
				}
			}
		}
		if(opcode & 0x800) {
			ay = _ay;
			if(_ax < 0) {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ax == 0) break;
				ax++;
			} else {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ax == 0) break;
				ax--;
			}
		} else {
			ax = _ax;
			if(_ay < 0) {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ay == 0) break;
				ay++;
			} else {
				xxs += src_step2_x;
				yys += src_step2_y;
				xxd += dst_step2_x;
				yyd += dst_step2_y;
				if(ay == 0) break;
				ay--;
			}
		}
	}
}

void HD63484::line(int16_t sx, int16_t sy, int16_t ex, int16_t ey, int16_t col)
{
	int cpx_t = sx;
	int cpy_t = sy;
	int16_t ax = ex - sx;
	int16_t ay = ey - sy;
	
	if(abs(ax) >= abs(ay)) {
		while(ax) {
			dot(cpx_t, cpy_t, col & 7, cl0);
			if(ax > 0) {
				cpx_t++;
				ax--;
			} else {
				cpx_t--;
				ax++;
			}
			cpy_t = sy + ay * (cpx_t - sx) / (ex - sx);
		}
	} else {
		while(ay) {
			dot(cpx_t, cpy_t, col & 7, cl0);
			if(ay > 0) {
				cpy_t++;
				ay--;
			} else {
				cpy_t--;
				ay++;
			}
			cpx_t = sx + ax * (cpy_t - sy) / (ey - sy);
		}
	}
}

void HD63484::paint(int sx, int sy, int col)
{
	int getpixel;
	dot(sx, sy, 0, col);
	
	getpixel = get_pixel(sx + 1, sy);
	switch((CCR & 0x700) >> 8) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
		break;
	case 3:
		getpixel = (getpixel << 8) | (getpixel << 0);
		break;
	case 4:
		break;
	default:
		this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
	}
	if((getpixel != col) && (getpixel != edg)) {
		sx++;
		paint(sx, sy, col);
		sx--;
	}
	
	getpixel = get_pixel(sx - 1, sy);
	switch((CCR & 0x700) >> 8) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
		break;
	case 3:
		getpixel = (getpixel << 8) | (getpixel << 0);
		break;
	case 4:
		break;
	default:
		this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
	}
	if((getpixel != col) && (getpixel != edg)) {
		sx--;
		paint(sx, sy, col);
		sx++;
	}
	
	getpixel = get_pixel(sx, sy + 1);
	switch((CCR & 0x700) >> 8) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
		break;
	case 3:
		getpixel = (getpixel << 8) | (getpixel << 0);
		break;
	case 4:
		break;
	default:
		this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
	}
	if((getpixel != col) && (getpixel != edg)) {
		sy++;
		paint(sx, sy, col);
		sy--;
	}
	
	getpixel = get_pixel(sx, sy - 1);
	switch((CCR & 0x700) >> 8) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		getpixel = (getpixel << 12) | (getpixel << 8) | (getpixel << 4) | (getpixel << 0);
		break;
	case 3:
		getpixel = (getpixel << 8) | (getpixel << 0);
		break;
	case 4:
		break;
	default:
		this->out_debug_log(_T("HD63484 graphic bit mode not supported\n"));
	}
	if((getpixel != col) && (getpixel != edg)) {
		sy--;
		paint(sx, sy, col);
		sy++;
	}
}

