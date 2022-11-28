#define USE_CMDTIME

// from "v99x8.c" of Zodiac

/*
 * Copyright (c) 2000-2002 SASAKI Shunsuke (eruchan@users.sourceforge.net).
 * Copyright (c) 2001-2002 The Zodiac project.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//#include "../../config.h"

#include <string.h>

//#include "../zodiac.h"
#include "v99x8.h"
v99x8_t v99x8;

#ifdef USE_CMDTIME
void cmdtime_set(int m);
void cmdtime_chk(void);
static int cmdtime_t;
static int cmdtime_m;
#endif
static int latch1;
static int latch2;

// from "md.h.in" of Zodiac
#if defined(_MSC_VER)
#	define __inline__ __forceinline
#elif defined(_MWERKS_)
#	define __inline__ inline
#elif !defined(__GNUC__)
#	define __inline__
#endif

#define MD_LITTLE

//#include "z80.h"	/* XXX interrupt */
#define Z80_NOINT 0xffff
#define Z80_INT 0x0038
//#define Z80_NMI 0x0066

#define	MD_BPP 32
#define md_maprgb15(R,G,B) RGB_COLOR(((R) << 3), ((G) << 3), ((B) << 3))
#define md_video_pixbytes(n) (n*32/8)
#define md_refresh_sync() (TRUE)
#define	md_video_defaultopt() (0)
int md_video_pitch(void);	// the length of a row of pixels in bytes
uint8_t *md_video_lockline(int x, int y, int w, int h);
void md_video_unlockline(void) {}
void md_video_update(int n, /*md_video_rect_t*/void *rp);
void md_video_fill(int x, int y, int w, int h, uint32_t c);
typedef uint32_t md_pixel_t;

// from "md_depend.h" of Zodiac
typedef struct
{
	int width, height;
	int bpp;
	int option;
}	md_video_mode_t;
//typedef	SDL_Rect md_video_rect_t;
//typedef	SDL_Surface md_video_surface_t;
typedef struct
{
	//md_video_surface_t *screen;
	int w, h;					/* 要求したサイズ */
}	md_video_t;


static int vram_addr;
static int vram_page;

static bool f_out3;
static bool f_mode;
bool f_scr;


extern void v99x8_command(int m);	/* v99x8_internal.h ?? */
extern void v99x8_cputovdp(int m);


#define XXX_V99X8_ID 0


/*
0: 000, 10 xx TMS 
1: 000, 00 xx TMS 
2: 001, 00 xx TMS 
3: 000, 01 xx TMS 

x: 010, 10 xx     
4: 010, 00 xx     
5: 011, 00 xx     1 
6: 100, 00 xx     2 
7: 101, 00 xx I   1 
8: 111, 00 xx I   0 

a: 111, 00 11     0
c: 111, 00 01     0
*/


/*
normal, sc7/8

00000h: 00000h
00001h: 10000h
00002h: 00001h
00003h: 10000h
0fffeh: 07fffh
0ffffh: 17fffh
10000h: 08000h
10001h: 08001h

*/

static void v99x8_interleave(void)
{
#if 0
	static uint8_t *vram = NULL;
	uint8_t *p;
	size_t size;

	size = v99x8.pages * 0x4000;

	if (vram == NULL)
		vram = (uint8_t *)malloc(size);

	p = vram;
	vram = v99x8.vram;
	v99x8.vram = p;

	if (v99x8.mode.f_interleave)
	{
		int a, b, c;

		a = 0;
		b = size / 2;
		c = 0;

		while (c < (int)size)
		{
			v99x8.vram[c]     = vram[a++];
			v99x8.vram[c + 1] = vram[b++];
			c += 2;
		}
	} else
	{
		int a, b, c;

		a = 0;
		b = size / 2;
		c = 0;

		while (c < (int)size)
		{
			v99x8.vram[a++] = vram[c];
			v99x8.vram[b++] = vram[c + 1];
			c += 2;
		}
	}
#endif
}

static void v99x8_mode_set(int n)
{
	v99x8_screen_mode_t mode[] =
	{
		{TRUE, FALSE, 0, 0},  /* V99X8_SCREEN_0 */
		{TRUE, FALSE, 0, 0},  /* V99X8_SCREEN_1 */
		{TRUE, FALSE, 0, 0},  /* V99X8_SCREEN_2 */
		{TRUE, FALSE, 0, 0},  /* V99X8_SCREEN_3 */

		{FALSE, FALSE, 0, 0}, /* V99X8_SCREEN_4 */
		{FALSE, FALSE, 256, 1}, /* V99X8_SCREEN_5 */
		{FALSE, FALSE, 512, 2}, /* V99X8_SCREEN_6 */
		{FALSE, TRUE, 512, 1},  /* V99X8_SCREEN_7 */

		{FALSE, TRUE, 256, 0},  /* V99X8_SCREEN_8 */
		{FALSE, FALSE, 0, 0}, /* V99X8_SCREEN_X */
		{FALSE, TRUE, 256, 0},  /* V99X8_SCREEN_A */
		{FALSE, TRUE, 256, 0}   /* V99X8_SCREEN_C */
	};

	if (n != V99X8_SCREEN_8 || (v99x8.ctrl[25] & 0x08) != 0x08)
		v99x8.scr = n;
	else
	{
		if ((v99x8.ctrl[25] & 0x10) == 0x10)
			v99x8.scr = V99X8_SCREEN_A;
		else
			v99x8.scr = V99X8_SCREEN_C;
	}

	memcpy(&v99x8.mode, &mode[v99x8.scr], sizeof(v99x8_screen_mode_t));
}

static void v99x8_update(void)
{
	int mapping[] =
	{
		V99X8_SCREEN_1, V99X8_SCREEN_2, V99X8_SCREEN_4, V99X8_SCREEN_5,
		V99X8_SCREEN_6, V99X8_SCREEN_7, V99X8_SCREEN_IGN, V99X8_SCREEN_8,
		V99X8_SCREEN_3, V99X8_SCREEN_IGN, V99X8_SCREEN_IGN, V99X8_SCREEN_IGN,
		V99X8_SCREEN_IGN, V99X8_SCREEN_IGN, V99X8_SCREEN_IGN, V99X8_SCREEN_IGN,
		V99X8_SCREEN_0, V99X8_SCREEN_IGN, V99X8_SCREEN_X
	};

	bool f;
	int a;

	if (!f_mode)
		return;

	f = v99x8.mode.f_interleave;

	a = ((v99x8.ctrl[0] & 0x0e) >> 1) | (v99x8.ctrl[1] & 0x18);
	if (a >= sizeof(mapping) / sizeof(*mapping) || 
	    mapping[a] == V99X8_SCREEN_IGN)
	{
		return;
	}

	v99x8_mode_set(mapping[a]);

	if (f != v99x8.mode.f_interleave)
		v99x8_interleave();

	f_mode = FALSE;
}


static void v99x8_ctrl_init(void)
{
	memset(v99x8.ctrl, 0, sizeof(v99x8.ctrl));
}

void v99x8_ctrl(int n, uint8_t m)
{
/* printf ("v99x8_ctrl %2d <= %02x\n", n, m); */

	if (n >= V99X8_NREG)
	{
		n = V99X8_NREG - 1;
	}

	switch(n)
	{
	case 0:
		if (((m ^ v99x8.ctrl[0]) & 0x0e) != 0)
		{
			f_mode = TRUE;
			f_scr = TRUE;
		}
		break;

	case 1:
		if (((m ^ v99x8.ctrl[1]) & 0x18) != 0)
		{
			f_mode = TRUE;
			f_scr = TRUE;
		}
		break;

	case 2:
	case 3:
	case 4:
	case 10:
		f_scr = TRUE;
		break;

	case 7:
		v99x8.col_fg = m >> 4;
		v99x8.col_bg = m & 0x0f;
		break;

	case 14:
		m &= v99x8.pages - 1;
		vram_page = (int)m << 14;
		break;

	case 15:
		if (m >= V99X8_NSTAT)
			m = V99X8_NSTAT - 1;
		break;

	case 16:
		m &= 0x0f;
		break;

	case 17:
		f_out3 = !(m & 0x80);
		m &= 0x3f;
		break;

    case 44: 
    	v99x8_update();
		v99x8_cputovdp(m);
    	break;
    case 46: 
    	v99x8_update();
		v99x8_command(m);
    	break;

    /* XXX

    */
    case 6:
	m &= 0x3f;
	break;

    case 11:
	m &= 0x03;
	break;
	}

	v99x8.ctrl[n] = m;
}


static void vram_incaddr(void)
{
	vram_addr = (vram_addr + 1) & 0x3fff;
	if (vram_addr == 0 && !v99x8.mode.f_tms)
		v99x8_ctrl(14, v99x8.ctrl[14] + 1);
}

uint8_t vram_read(int addr)
{
	return v99x8.vram[addr];
}

void vram_write(int addr, uint8_t n)
{
	v99x8.vram[addr] = n;
}


uint8_t v99x8_in_0(void)	/* VRAM read */
{
	int n;

	v99x8_update();

	n = vram_read(vram_addr + vram_page);
	vram_incaddr();
	return n;
}

void v99x8_out_0(uint8_t n)	/* VRAM write */
{
	v99x8_update();

	vram_write(vram_addr + vram_page, n);
	vram_incaddr();
}

uint8_t v99x8_in_1(void)	/* status in */
{
	int n;
#if 0
	int a, b;
#endif

	v99x8_update();

	n = v99x8.status[v99x8.ctrl[15]];

/*
if (z80.ivec!=Z80_NOINT)
	{
printf("* IFF:%d H:%d V:%d\n", z80.IFF&1, (v99x8.ctrl[0]&0x10), (v99x8.ctrl[1]&0x20));
	}
	z80_intreq(Z80_NOINT);
	VKey=1;
*/

	switch(v99x8.ctrl[15])
	{
	case 0:
		v99x8.status[0] &= ~0xa0;
		break;
	case 1:
		v99x8.status[1] &= ~0x01;
		break;
	case 7:
/*		v99x8.status[7] = v99x8.ctrl[44] = v99x8_vdptocpu(); */
		break;

	case 2:
#if 0
		context_timelock();
		a = context.hz / 60 / 262;
		b = (context.time_cycle % a) * 100 / a;
		context_timeunlock();

		if (b > 73)
			n |= 0x20;
//		else
#endif
//			n &= ~0x20;
		break;
	}

	return n;
}

void	v99x8_out_1(uint8_t n)	/* ctrl out */
{
	//static int latch = -1;

	if (latch1 == -1)
	{
		latch1 = n;
	} else
	{
		if (n & 0x80)
		{
			if ((n & 0x40) == 0)
				v99x8_ctrl(n & 0x3f, latch1);
		} else
		{
/* ??? read/write の区別 */
			vram_addr = ((int)(n & 0x3f) << 8) + latch1;
		}
		latch1 = -1;
	}
}

void v99x8_out_2(uint8_t n)	/* palette out */
{
	//static int latch = -1;

	if (latch2 == -1)
	{
		latch2 = n;
	} else
	{
		int a;

		a = v99x8.ctrl[16];
		v99x8_pallete_set(a, (latch2 & 0x70) >> 4, n & 0x07, latch2 & 0x07);
		v99x8_ctrl(16, a + 1);

		latch2 = -1;
	}
}

void v99x8_out_3(uint8_t n)	/* ctrl out */
{
	if (v99x8.ctrl[17] != 17)
		v99x8_ctrl(v99x8.ctrl[17], n);

	if (f_out3)
		v99x8_ctrl(17, v99x8.ctrl[17] + 1);
}


static void v99x8_status_init(void)
{
	memset(v99x8.status, 0, sizeof(v99x8.status));
	v99x8.status[0] = 0x9f;
	v99x8.status[1] = XXX_V99X8_ID << 1;
	v99x8.status[2] = 0xac;
}

static void v99x8_vram_init(void)
{
	//v99x8.vram = (uint8_t *)malloc(v99x8.pages * 0x4000);
	memset(v99x8.vram, 0xff, v99x8.pages * 0x4000);
}

void v99x8_init(void)
{
/* ---- */

	vram_addr = 0;
	vram_page = 0;
	f_out3 = FALSE;
 	f_scr = TRUE;
	f_mode = FALSE;


/* ---- */

	v99x8.scanline = 0;
	v99x8.n_scanlines = 262;
	v99x8.pages = 8;

	v99x8_ctrl_init();
	v99x8_status_init();

/*	v99x8.f_zoom = FALSE; */

	v99x8_vram_init();

	v99x8_mode_set(V99X8_SCREEN_1);

	v99x8.col_fg = 0;
	v99x8.col_bg = 0;

	v99x8_refresh_init();
}

static bool flag_frame = FALSE;

int V99X8::hsync(int v/*void*/)
{
	int a, line;

	v99x8.scanline = v;	// is this OK???
	v99x8.status[2] ^= 0x20;

	if (v99x8.scanline < 226)
	{
		if (flag_frame)
		{
			if (0 == v99x8.scanline) v99x8_refresh_clear(); // added by umaiboux
			line = v99x8.scanline + (((v99x8.ctrl[18] >> 4) + 8) & 0x0f) - 8 - 7;
			if (v99x8.ctrl[9] & 0x80)
				a = 212;
			else
				a = 192, line -= 10;

			if (v99x8.ctrl[1] & 0x40 && line >= 0 && line < a)
			{
				v99x8_update();

				switch(v99x8.scr)
				{
				case V99X8_SCREEN_0: v99x8_refresh_sc0(line, 1); break;
				case V99X8_SCREEN_1: v99x8_refresh_sc1(line, 1); break;
				case V99X8_SCREEN_2: v99x8_refresh_sc4(line, 1); break;
				case V99X8_SCREEN_3: v99x8_refresh_sc3(line, 1); break;
				case V99X8_SCREEN_4: v99x8_refresh_sc4(line, 1); break;
				case V99X8_SCREEN_5: v99x8_refresh_sc5(line, 1); break;
				case V99X8_SCREEN_6: v99x8_refresh_sc6(line, 1); break;
				case V99X8_SCREEN_7: v99x8_refresh_sc7(line, 1); break;
				case V99X8_SCREEN_8: v99x8_refresh_sc8(line, 1); break;
				case V99X8_SCREEN_A: v99x8_refresh_scc(line, 1); break;
				case V99X8_SCREEN_C: v99x8_refresh_scc(line, 1); break;
				case V99X8_SCREEN_X: v99x8_refresh_scx(line, 1); break;
				}
			}
		}

		if (((v99x8.scanline + v99x8.ctrl[23] 
		    - ((v99x8.ctrl[9] & 0x80) ? 8 : 18)) & 0xff) == v99x8.ctrl[19])
		{
			if (v99x8.ctrl[0] & 0x10)
			{
				v99x8.status[1] |= 0x01; /* H-sync */
				z80_intreq(Z80_INT);
			}
		} else
		{
			if (!(v99x8.ctrl[0] & 0x10))
				v99x8.status[1] &= ~0x01;   /* ?? H-sync off*/
		}
	} else
	{
		switch(v99x8.scanline)
		{
		case 234:
			if (flag_frame)
				v99x8_refresh_screen();

			flag_frame = md_refresh_sync();

			v99x8.status[2] |= 0x40;    /* VBlank on */

			v99x8.status[1] &= ~0x01;   /* ?? H-sync off*/
			z80_intreq(Z80_NOINT);      /* ?? H-sync を clear */

/* XXX sprite check */
			break;

		case 237:
/*			v99x8.status[1] &= ~0x01; */  /* ?? H-sync off*/
			if (v99x8.ctrl[1] & 0x20)
			{
				v99x8.status[0] |= 0x80;    /* V-sync int */
				z80_intreq(Z80_INT);
			}
			break;

		case 261:
			v99x8.status[2] &= ~0x40;   /* VBlank off */
			v99x8.status[0] &= ~0x40;   /* 5sprit off */
			v99x8.status[0] &= ~0x80;   /* Vsync off */
			z80_intreq(Z80_NOINT);      /* ?? V-sync を clear */

			/*if (flag_frame)
				v99x8_refresh_clear();*/ // deleted by umaiboux
		}


/* NTSC timing

  3/  3: sync signal
 13/ 13: top erase
 26/ 16: top border
192/212: line
 25/ 15: bottom border

  3/  3: bottom erase

*/
	}
	return v99x8.scanline = (v99x8.scanline + 1) % v99x8.n_scanlines;
}



// from "v99x8_command.c" of Zodiac

/*
 * Copyright (c) 2000-2002 SASAKI Shunsuke (eruchan@users.sourceforge.net).
 * Copyright (c) 2001-2002 The Zodiac project.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//#include "../../config.h"

//#include <string.h>
#include <stdlib.h>

//#include "../zodiac.h"
#include "v99x8.h"

/* #define YAN_V99X8_TEST 1 */	/* XXX */

typedef struct
{
	int xbytes;
	int xmask, xshift;
	int ymask, yshift;

	int sx, sy, dx, dy, nx, ny;
	int lop;

	uint8_t *src, *dst;
} vcom_t;

vcom_t vcom;

typedef struct
{
	int sx, sy, ex, ey;
	int x, y;
	int xsize;
} r44_t;

static r44_t r44;

#if 1	/* XXX */
typedef struct
{
	int npix;
	int xmask;
	int mask;
	int pmask[4];
	int lshift[4];
	int rshift[4];
} pixmask_t;

static pixmask_t pixmasks[4] = {
	{2, 0x01, 0x0f, {0x0f, 0xf0, 0x0f, 0xf0}, {4, 0, 4, 0}, {4, 0, 4, 0}},
	{4, 0x03, 0x03, {0x3f, 0xcf, 0xf3, 0xfc}, {6, 4, 2, 0}, {6, 4, 2, 0}},
	{2, 0x01, 0x0f, {0x0f, 0xf0, 0x0f, 0xf0}, {4, 0, 4, 0}, {4, 0, 4, 0}},
	{1, 0x00, 0xff, {0x00, 0x00, 0x00, 0x00}, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

static pixmask_t pixmask;
#endif

static int getshift(int n)
{
	int i;

	for (i = 0; n & (1 << i); i++)
		;
	return i;
}

#define vcom_getsx() vcom.sx = (v99x8.ctrl[32] + (v99x8.ctrl[33] << 8)) & vcom.xmask
#define vcom_getsy() vcom.sy = (v99x8.ctrl[34] + (v99x8.ctrl[35] << 8)) & vcom.ymask
#define vcom_getdx() vcom.dx = (v99x8.ctrl[36] + (v99x8.ctrl[37] << 8)) & vcom.xmask
#define vcom_getdy() vcom.dy = (v99x8.ctrl[38] + (v99x8.ctrl[39] << 8)) & vcom.ymask
#define vcom_getnx() vcom.nx = ((v99x8.ctrl[40] + (v99x8.ctrl[41] << 8) - 1) & 511) + 1
#define vcom_getny() vcom.ny = ((v99x8.ctrl[42] + (v99x8.ctrl[43] << 8) - 1) & 1023) + 1

static void vcom_lpset(int x, int y, int clr);
static uint8_t vcom_point(int x, int y);

static void vcom_set(int base, int n)
{
	v99x8.ctrl[base]     = n & 0xff;
	v99x8.ctrl[base + 1] = n >> 8;
}

#define vcom_setsy(n) vcom_set(34, n)
#define vcom_setdy(n) vcom_set(38, n)
#define vcom_setny(n) vcom_set(42, n)

#define vcom_vram(x, y) (&v99x8.vram[((x) >> vcom.xshift) + ((y) << vcom.yshift)])

static int vcom_chksum(uint8_t *d, int n)
{
	uint8_t *p;
	int sum = 0;

	p = d;
	while(n--) {
		sum += *p;
		p++;
	}
	return sum;
}

#define vcom_dtcopy(d, s, nbytes) { \
	uint8_t *p1, *p2;             \
	int n;                      \
	p1 = (s);                   \
	p2 = (d);                   \
	n = (nbytes);               \
	while(n--)                  \
		*p2++ = *p1++;      \
}

static int vcom_canonaddr(void)
{
	int ny;

	if (v99x8.ctrl[45] & 0x04)  /* Direction to left */
	{
		vcom.sx -= vcom.nx;
		vcom.dx -= vcom.nx;
	}
	vcom.sx = max(vcom.sx, 0);
	vcom.nx = min(vcom.nx, vcom.xmask + 1 - vcom.sx);
	vcom.dx = max(vcom.dx, 0);
	vcom.nx = min(vcom.nx, vcom.xmask + 1 - vcom.dx);

	ny = vcom.ny;
	if ((v99x8.ctrl[45] & 0x08) == 0)   /* Direction to down */
	{
		ny = min(ny, vcom.ymask + 1 - vcom.sy);
		ny = min(ny, vcom.ymask + 1 - vcom.dy);
	} else
	{
		ny = min(ny, vcom.sy + 1);
		ny = min(ny, vcom.dy + 1);
		ny = 0 - ny;
	}

/* printf("can %d->%d *(%d,%d)\n", vcom.sx, vcom.dx, vcom.nx, ny); */

	vcom.src = vcom_vram(vcom.sx, vcom.sy);
	vcom.dst = vcom_vram(vcom.dx, vcom.dy);
	vcom.nx >>= vcom.xshift;

	return ny;
}

static void vcom_hcopy(uint8_t *dst, uint8_t *src, int nx, int ny)
{
	if (ny < 0)
	{
		while (ny++ < 0)
		{
/* printf("sum1: %d\n", vcom_chksum(src, nx)); */
			/* memmove(dst, src, nx); */
			vcom_dtcopy(dst, src, nx);
/* printf("sum2: %d\n", vcom_chksum(dst, nx)); */
			src -= vcom.xbytes;
			dst -= vcom.xbytes;
		}
	} else
	{
		while (ny--)
		{
/* printf("sum1: %d\n", vcom_chksum(src, nx)); */
			/* memmove(dst, src, nx); */
			vcom_dtcopy(dst, src, nx);
/* printf("sum2: %d\n", vcom_chksum(dst, nx)); */
			src += vcom.xbytes;
			dst += vcom.xbytes;
		}
	}
}

static void ymmm(void)
{
	int n;

	vcom_getdx();
	vcom_getsy();
	vcom.sx = vcom.dx;
	vcom_getdy();
	vcom.nx = 512;
	vcom_getny();

/* printf("ymmm: (%d,%d) %d*%d\n", vcom.sx, vcom.sy, vcom.dy, vcom.ny); */
	n = vcom_canonaddr();
	vcom_hcopy(vcom.dst, vcom.src, vcom.nx, n);

	vcom_setsy(vcom.sy + n);
	vcom_setdy(vcom.dy + n);

	if (vcom.ny != abs(n))
		vcom_setny(vcom.ny - abs(n));
}

static void hmmm(void)
{
	int n;

	vcom_getsx();
	vcom_getsy();
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();

/* printf("hmmm: (%d,%d)->(%d,%d) *(%d,%d)\n", vcom.sx, vcom.sy, vcom.dx, vcom.dy, vcom.nx, vcom.ny); */
	n = vcom_canonaddr();
	vcom_hcopy(vcom.dst, vcom.src, vcom.nx, n);

	vcom_setsy(vcom.sy + n);
	vcom_setdy(vcom.dy + n);

	if (vcom.ny != abs(n))
		vcom_setny(vcom.ny - abs(n));
}

static void hmmv(void)
{
	int n, ny, clr;

	vcom.sx = 0;
	vcom.sy = 0;
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();
	clr = v99x8.ctrl[44];

	ny = n = vcom_canonaddr();
	if (n < 0)
	{
		while (ny++ < 0)
		{
			memset(vcom.dst, clr, vcom.nx);
			vcom.dst -= vcom.xbytes;
		}
	} else
	{
		while (ny--)
		{
			memset(vcom.dst, clr, vcom.nx);
			vcom.dst += vcom.xbytes;
		}
	}

	vcom_setsy(vcom.sy + n);
	vcom_setdy(vcom.dy + n);

	if (vcom.ny != abs(n))
		vcom_setny(vcom.ny - abs(n));
}

#define vcom_lset(dc, sc)                           \
switch(vcom.lop)                                    \
{                                                   \
	case 0x0: (dc)  = (sc); break;                  \
	case 0x1: (dc) &= (sc); break;                  \
	case 0x2: (dc) |= (sc); break;                  \
	case 0x3: (dc) ^= (sc); break;                  \
	case 0x4: (dc) =~ (sc); break;                  \
	case 0x8: if ((sc) != 0) (dc)  = (sc); break;   \
	case 0x9: if ((sc) != 0) (dc) &= (sc); break;   \
	case 0xa: if ((sc) != 0) (dc) |= (sc); break;   \
	case 0xb: if ((sc) != 0) (dc) ^= (sc); break;   \
	case 0xc: if ((sc) != 0) (dc) =~ (sc); break;   \
}

/*
	dc =  sc;
	dc &= sc;
	dc |= sc;
	dc ^= sc;
	dc =  ~sc;
*/

static void vcom_lmove(uint8_t *dst, uint8_t *src, int n)
{
#if 0
	if (v99x8.ctrl[45] & 0x04)  /* Direction to left */
#endif

	while(n>0)
	{
		vcom_lset(*dst, *src);
		++dst, ++src;
		--n;
	}
}

static void vcom_lcopy(uint8_t *dst, uint8_t *src, int nx, int ny)
{
	if (ny < 0)
	{
		while (ny++ < 0)
		{
			vcom_lmove(dst, src, nx);
			src -= vcom.xbytes;
			dst -= vcom.xbytes;
		}
	} else
	{
		while (ny--)
		{
			vcom_lmove(dst, src, nx);
			src += vcom.xbytes;
			dst += vcom.xbytes;
		}
	}
}

/* 
  XXX
	これはあくまでその場しのぎなもの。
	memo:
		srcのデータをbufにバイトコピーし、dstの先頭ｐｉｘｅｌ位置にあうようにビットシフトし、
		先端と終端のデータをdstと合成して、それをdstへlogopを考慮しつつ、バイトコピ
		ーすればいいのか？

		で、SCREEN8の時や、srcとdstのpixel位置が同じなら、シフト処理をしないよう
		にする、と。

		コストはかかるけど、1pixずつちまちまコピーするよりかは速そうな気がする。

		その場合 vcom_lset にも手直しが必要になると(T付き論理演算部分)。
		だけど memmove の件のように、一旦bufにセーブすると、VRAM壊しちゃうかも。

		とすると…下みたいにちまちまコピーしてくしかないのかなぁ…
*/
#ifdef YAN_V99X8_TEST
static void vcom_lbcopy(int sx, int sy, int dx, int dy, int nx, int ny)
{
	int osx, odx, onx;

	nx <<= vcom.xshift;
	osx = sx;
	odx = dx;
	onx = nx;

	if (ny < 0)
	{
		while (ny++ < 0)
		{
			while (nx-- > -1)
			{
				vcom_lpset(dx, dy, (int)vcom_point(sx, sy));
				sx++;
				dx++;
			}
			sy--;
			dy--;
			sx = osx;
			dx = odx;
			nx = onx;
		}
	} else
	{
		while (ny--)
		{
			while (nx-- > -1)
			{
				vcom_lpset(dx, dy, (int)vcom_point(sx, sy));
				sx++;
				dx++;
			}
			sy++;
			dy++;
			sx = osx;
			dx = odx;
			nx = onx;
		}
	}
}
#endif /* YAN_V99X8_TEST */


void vcom_lputc(int m)  /* XXX 左方向、上方向のテストはしてません */
{
	uint8_t *dst;
	int dot;

	dst = vcom_vram(r44.x, r44.y);

#if 1	/* XXX */
	{
		int pixn;

		m &= pixmask.mask;
		pixn = r44.x & pixmask.xmask;
		dot = (*dst >> ((1 - pixn) * pixmask.npix)) & pixmask.mask;
		vcom_lset(dot, m);
		*dst = (*dst & pixmask.pmask[pixn]) | (dot << pixmask.lshift[pixn]);
	}
#else
	switch (v99x8.scr)
	{
	case V99X8_SCREEN_5:
	case V99X8_SCREEN_7:
		m &= 0x0f;
		dot = (*dst >> ((1 - (r44.x & 1)) * 4)) & 0x0f;
		vcom_lset(dot, m);
		if ((r44.x & 1) == 0)
			*dst = (*dst & 0x0f) | (dot << 4);
		else
			*dst = (*dst & 0xf0) | dot;
		break;

	case V99X8_SCREEN_6:
		m &= 0x03;
		dot = (*dst >> ((1 - (r44.x & 3)) * 2)) & 0x03;
		vcom_lset(dot, m);
		switch (r44.x & 0x03)
		{
		case 0:
			*dst = (*dst & 0x3f) | (dot << 6);
			break;
		case 1:
			*dst = (*dst & 0xcf) | (dot << 4);
			break;
		case 2:
			*dst = (*dst & 0xf3) | (dot << 2);
			break;
		case 3:
			*dst = (*dst & 0xfc) | dot;
		}
		break;

	case V99X8_SCREEN_8:
		vcom_lset(*dst, m & 0xff);
		break;

	}
#endif

	if (r44.sx <= r44.ex)
	{
		if (++r44.x >= r44.ex)
		{
			r44.x = r44.sx;
			if (r44.sy < r44.ey)
			{
				if (++r44.y >= r44.ey)
					v99x8.status[2] &= ~0x01;
			} else
			{
				if (--r44.y < r44.ey)
					v99x8.status[2] &= ~0x01;
			}
		}
	} else
	{
		if (--r44.x < r44.ex)
		{
			r44.x = r44.sx;
			if (r44.sy <= r44.ey)
			{
				if (++r44.y > r44.ey)
					v99x8.status[2] &= ~0x01;
			} else
			{
				if (--r44.y < r44.ey)
					v99x8.status[2] &= ~0x01;
			}
		}
	}
}


void vcom_hputc(int m) /* XXX 左方向、上方向のテストはしてません */
{
	*vcom_vram(r44.x, r44.y) = m;

	if (r44.sx <= r44.ex)
	{
		r44.x += 1 << vcom.xshift;

		if (r44.x >= r44.ex)
		{
			r44.x = r44.sx;
			if (r44.sy <= r44.ey)
			{
				if ((++r44.y) >= r44.ey)
					v99x8.status[2] &= ~0x01;
			} else
			{
				if ((--r44.y) <= r44.ey)
					v99x8.status[2] &= ~0x01;
			}
		}
	} else
	{
		r44.x -= 1 << vcom.xshift;

		if (r44.x <= r44.ex)
		{
			r44.x = r44.sx;
			if (r44.sy <= r44.ey)
			{
				if ((++r44.y) >= r44.ey)
					v99x8.status[2] &= ~0x01;
			} else
			{
				if ((--r44.y) <= r44.ey)
					v99x8.status[2] &= ~0x01;
			}
		}
	}
}


void v99x8_cputovdp(int m)
{
	if ((v99x8.status[2] & 0x01) == 0)
		return;

	switch(v99x8.ctrl[46] >> 4)
	{
	case 0xb:
		vcom_lputc(m);
		break;
	case 0xf:
		vcom_hputc(m);
		break;
	}
}

static void lmmc(void)
{
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();


	r44.sx = r44.x = vcom.dx;
	r44.sy = r44.y = vcom.dy;
	r44.ex = vcom.dx + vcom.nx;
	r44.ey = vcom.dy + vcom.ny;

	vcom_lputc(v99x8.ctrl[44]);
}


static void lmcm(void);

static void stop(void);


static void vcom_lpset(int x,int y, int clr)
{
	uint8_t *dst;
	int src_dot, dst_dot;

	dst = vcom_vram(x, y);

#if 1	/* XXX */
	{
		int pixn;

		pixn = x & pixmask.xmask;
		src_dot = clr & pixmask.mask;
		dst_dot = (*dst >> pixmask.rshift[pixn]) & pixmask.mask;
		vcom_lset(dst_dot, src_dot);
		*dst = (*dst & pixmask.pmask[pixn]) | (dst_dot << pixmask.lshift[pixn]);
	}
#else
	switch(v99x8.scr)
	{
	case V99X8_SCREEN_5:
	case V99X8_SCREEN_7:
		src_dot = clr & 0x0f;
		dst_dot = (*dst >> ((1 - (x & 1)) * 4)) & 0x0f;
		vcom_lset(dst_dot, src_dot);
		if ((x & 1) == 0)
			*dst = (*dst & 0x0f) | (dst_dot << 4);
		else
			*dst = (*dst & 0xf0) | dst_dot;
		break;

	case V99X8_SCREEN_6:
		src_dot = clr & 0x03;
		dst_dot = (*dst >> ((1 - (r44.x & 3)) * 2)) & 0x03;
		vcom_lset(dst_dot, src_dot);
		switch(x & 0x03)
		{
		case 0:
			*dst = (*dst & 0x3f) | (src_dot << 6);
			break;
		case 1:
			*dst = (*dst & 0xcf) | (src_dot << 4);
			break;
		case 2:
			*dst = (*dst & 0xf3) | (src_dot << 2);
			break;
		case 3:
			*dst = (*dst & 0xfc) | src_dot;
		}
		break;

	case V99X8_SCREEN_8:
		vcom_lset(*dst, clr & 0xff);
		break;
	}
#endif
}


static void line(void)
{
	int i, j, x, y;
	int maj, min;

	vcom_getdx();
	vcom_getdy();
	maj = (v99x8.ctrl[40] + (v99x8.ctrl[41] << 8)) & 1023;
	min = (v99x8.ctrl[42] + (v99x8.ctrl[43] << 8)) & 511;

	/* XXX */
	maj = (maj == 0) ? (1) : (maj);

	for (i = 0; i <= maj; i++)
	{
		j = (i * min) / maj;
		if (v99x8.ctrl[45] & 0x01)
		{
			y = vcom.dy + ((v99x8.ctrl[45] & 0x08) ? -i : i);
			x = vcom.dx + ((v99x8.ctrl[45] & 0x04) ? -j : j);
		} else
		{
			x = vcom.dx + ((v99x8.ctrl[45] & 0x04) ? -i : i);
			y = vcom.dy + ((v99x8.ctrl[45] & 0x08) ? -j : j);
		}
		vcom_lpset(x, y, v99x8.ctrl[44]);
	}
}

static uint8_t vcom_point(int x, int y)
{
	uint8_t clr = *vcom_vram(x, y);

#if 1	/* XXX */
	{
		int pixn;

		pixn = x & pixmask.xmask;
		clr = (clr >> pixmask.rshift[pixn]) & pixmask.mask;
	}
#else
	switch (v99x8.scr)
	{
	case V99X8_SCREEN_5:
	case V99X8_SCREEN_7:
		clr = (clr >> (4 * (x & 1))) & 0x0F;
		break;
	case V99X8_SCREEN_6:
		clr = (clr >> (2 * (x & 3))) & 0x03;
		break;
	}
#endif

	return clr;
}

static void srch(void)
{
	int i;

	vcom_getsx();
	vcom_getsy();

	i = 0;
	v99x8.status[2] &= ~0x10;
	while ((0 <= (vcom.sx + i)) && ((vcom.sx + i) <= vcom.xmask))
	{
		uint8_t clr = vcom_point(vcom.sx + i, vcom.sy);
		if (v99x8.ctrl[45] & 0x02)
		{
			if (clr != v99x8.ctrl[44])
			{
				v99x8.status[2] |= 0x10;
				break;
			}
		} else
		{
			if (clr == v99x8.ctrl[44])
			{
				v99x8.status[2] |= 0x10;
				break;
			}
		}
		i = (v99x8.ctrl[45] & 0x04) ? (i - 1) : (i + 1);
	}

	if (v99x8.status[2] & 0x10)
	{
		v99x8.status[8] = (vcom.sx + i) & 0xff;
		v99x8.status[9] = (((vcom.sx + i) >> 8) & 0x03) | 0xfc;
	}
}


static void pset(void)
{
	vcom_getdx();
	vcom_getdy();
	vcom_lpset(vcom.dx, vcom.dy, v99x8.ctrl[44]);
}


static void point(void)
{
	vcom_getsx();
	vcom_getsy();
	v99x8.status[7] = vcom_point(vcom.sx, vcom.sy);
}

static void hmmc(void)
{
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();


	r44.sx = r44.x = vcom.dx & ~1;
	r44.sy = r44.y = vcom.dy;
	r44.ex = (vcom.dx + vcom.nx) & ~1;
	r44.ey = vcom.dy + vcom.ny;
	r44.xsize = vcom.nx & ~1;

	vcom_hputc(v99x8.ctrl[44]);
}

static void lmmm(void)
{
	int n;

	vcom_getsx();
	vcom_getsy();
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();

/* printf("hmmm: (%d,%d)->(%d,%d) *(%d,%d)\n", vcom.sx, vcom.sy, vcom.dx, vcom.dy, vcom.nx, vcom.ny); */
	n = vcom_canonaddr();
#if YAN_V99X8_TEST	/* XXX */
	vcom_lbcopy(vcom.sx, vcom.sy, vcom.dx, vcom.dy, vcom.nx, n);
#else
	vcom_lcopy(vcom.dst, vcom.src, vcom.nx, n);
#endif
	vcom_setsy(vcom.sy + n);
	vcom_setdy(vcom.dy + n);

	if (vcom.ny != abs(n))
		vcom_setny(vcom.ny - abs(n));
}


static void lmmm_(void);


static void vcom_lfill(uint8_t *p, int clr, int n)
{
	if (n == 0)     /* バイト数単位で処理をおこなうため */
		n = 1;      /* 1dotが無視されてしまう対応 */

	switch(vcom.lop)
	{
	case 0x8:
		if (clr == 0)
			break;
	case 0x0:
		while (n-- > 0)
			*p++ = clr;
		break;

	case 0x9:
		if (clr == 0)
			break;
	case 0x1:
		while (n-- > 0)
			*p++ &= clr;
		break;

	case 0xa:
		if (clr == 0)
			break;
	case 0x2:
		while (n-- > 0)
			*p++ |= clr;
		break;

	case 0xb:
		if (clr == 0)
			break;
	case 0x3:
		while (n-- > 0)
			*p++ ^= clr;
		break;

	case 0xc:
		if (clr == 0)
			break;
	case 0x4:
		while (n-- > 0)
			*p++ = ~clr;
		break;
	}

}

static void lmmv(void)   /* XXX byte 単位で処理してるのは手抜き(^^; */
{
	int n, ny, clr;

	vcom.sx = 0;
	vcom.sy = 0;
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();
	clr = v99x8.ctrl[44];
	switch (vcom.xshift)
	{
	case 2:
		clr |= clr << 2;
	case 1:
		clr |= clr << 4;
	}

	ny = n = vcom_canonaddr();
	if (n < 0)
	{
		while (ny++ < 0)
		{
			vcom_lfill(vcom.dst, clr, vcom.nx);
			vcom.dst -= vcom.xbytes;
		}
	} else
	{
		while (ny--)
		{
			vcom_lfill(vcom.dst, clr, vcom.nx);
			vcom.dst += vcom.xbytes;
		}
	}

	vcom_setsy(vcom.sy + n);
	vcom_setdy(vcom.dy + n);

	if (vcom.ny != abs(n))
		vcom_setny(vcom.ny - abs(n));
}


static void lmmv_(void);


void v99x8_command(int m)
{
	if (v99x8.mode.xsize == 0) /* XXX */
		return;

	vcom.xbytes = v99x8.mode.xsize >> v99x8.mode.xshift;
	vcom.xmask  = v99x8.mode.xsize - 1;
	vcom.xshift = v99x8.mode.xshift;

	vcom.ymask  = v99x8.pages * 0x4000 / vcom.xbytes - 1;
	vcom.yshift = getshift(vcom.xbytes - 1);

#if 1	/* XXX */
	pixmask = pixmasks[(v99x8.scr - V99X8_SCREEN_5) & 0x03];
#endif

	v99x8.status[2] |= 0x01;
#ifdef USE_CMDTIME
	cmdtime_set(m);
#endif


	vcom.lop = m & 0xf;

	switch(m >> 4)
	{
	case 0xf:	hmmc(); break;
	case 0xe:	ymmm(); break;
	case 0xd:	hmmm(); break;
	case 0xc:	hmmv(); break;
	case 0xb:	lmmc(); break;
	case 0xa:	lmcm(); break;
	case 0x9:	lmmm_(); break;
	case 0x8:	lmmv_(); break;
	case 0x7:	line(); break;
	case 0x6:	srch(); break;
	case 0x5:	pset(); break;
	case 0x4:	point(); break;
	case 0x0:	stop(); break;
	}

	v99x8.ctrl[46] &= 0x0f;
#ifdef USE_CMDTIME
	if (((m >> 4) != 0xb) && ((m >> 4) != 0xf) && (0 == cmdtime_t)) v99x8.status[2] &= ~0x01;
	//cmdtime_chk();
#else
	if ((m >> 4) != 0xb && (m >> 4) != 0xf)
		v99x8.status[2] &= ~0x01;
#endif
}

// from "v99x8_refresh.c" of Zodiac

/*
 * Copyright (c) 2000-2002 SASAKI Shunsuke (eruchan@users.sourceforge.net).
 * Copyright (c) 2001-2002 The Zodiac project.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//#include "../../config.h"

#include <stdlib.h>
//#include <string.h>

//#include "../zodiac.h"
#include "v99x8.h"

extern bool f_scr;


#ifdef MD_LITTLE
#	define UINT32_FROM16(n1, n2) ((n2) << 16 | (n1))
#	define UINT8_FROM4(n1, n2) ((n1) << 4 | (n2))
#else
#	define	UINT32_FROM16(n1, n2)	((n1) << 16 | (n2))
#	define	UINT8_FROM4(n1, n2)	((n2) << 4 | (n1))
#endif


typedef struct
{
	int width, height;
	int bpp;

} v99x8_refresh_t;

static v99x8_refresh_t v99x8_refresh;


  /* pallete */

typedef struct
{
	bool flag;
	uint8_t r, g, b;
	uint32_t color;
} v99x8_pallete_t;

static v99x8_pallete_t pal[16 + 1];
static uint32_t pal_8[256];
static uint32_t pal_m[256];

static bool f_pal;


void v99x8_pallete_set(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
	if (n == 0)
		n = 16;

	pal[n].r = r * 31 / 7;
	pal[n].g = g * 31 / 7;
	pal[n].b = b * 31 / 7;
	pal[n].flag = TRUE;

	f_pal = TRUE;
}

static int col_bg = 0;

static void v99x8_pallete_update(void)
{
	int a;
	int i, j;

	if (f_pal)
		for (i = 1; i < 17; ++i)
			if (pal[i].flag)
				pal[i].color = md_maprgb15(pal[i].r, pal[i].g, pal[i].b);

	a = (v99x8.col_bg == 0 || (v99x8.ctrl[8] & 0x20)) ? 16 : v99x8.col_bg;
	if (col_bg != a || pal[a].flag)
	{
		f_pal = TRUE;
		col_bg = a;
		memcpy(&pal[0], &pal[a], sizeof(v99x8_pallete_t));
		pal[0].flag = TRUE;
	}

	if (!f_pal)
		return;

	if (!v99x8.f_zoom)
	{
		for (i = 0; i < 16; ++i)
		{
			for (j = 0; j < 16; ++j)
			{
				if (!pal[i].flag && !pal[j].flag)
					continue;

				a = UINT8_FROM4(i, j);

				if (i == j)
				{
					pal_m[a] = pal[i].color;
				} else
				{
					pal_m[a] = md_maprgb15((pal[i].r >> 1) + (pal[j].r >> 1),
					                       (pal[i].g >> 1) + (pal[j].g >> 1),
					                       (pal[i].b >> 1) + (pal[j].b >> 1));
				}
			}
		}
	}
	f_pal = FALSE;
	for (i = 0; i < 17; ++i)
		pal[i].flag = FALSE;
}

static void v99x8_pallete_init(void)
{
	static const uint8_t inipal[16][3] =
	{
		{0, 0, 0}, {0, 0, 0}, {1, 6, 1}, {3, 7, 3}, 
		{1, 1, 7}, {2, 3, 7}, {5, 1, 1}, {2, 6, 7}, 
		{7, 1, 1}, {7, 3, 3}, {6, 6, 1}, {6, 6, 4}, 
		{1, 4, 1}, {6, 2, 5}, {5, 5, 5}, {7, 7, 7}
	};

	uint32_t pal_black;
	int i;

	memset(pal, 0, sizeof(pal));

	pal_black = md_maprgb15(0, 0, 0);
	for (i = 0; i < 256; ++i)
	{
	             /* GGGRRRBB */
		/* i & 0x1c, (i >> 3) & 0x1c, (i & 3) << 3); */
		pal_8[i] = md_maprgb15((i & 0x1c) * 31 / (7 << 2),
		                       (i & 0xe0) * 31 / (7 << 5),
		                       (i & 3) * 31 / 3);
		pal_m[i] = pal_black;
	}

	for(i = 0; i < 16; i++)
		v99x8_pallete_set(i, inipal[i][0], inipal[i][1], inipal[i][2]);
}




#define	V99X8_WIDTH  (256 + 15)
#define	V99X8_HEIGHT (212 + 15)


static uint8_t tbl_yjk_b[32 * 64 * 64], tbl_yjk_rg[62 + 32];
static uint8_t blackbuf[256];      /* sprite 非表示用バッファ */


void v99x8_refresh_init(void)
{
	int i;
	md_video_mode_t	mode;

	v99x8_refresh.width  = V99X8_WIDTH;
	v99x8_refresh.height = V99X8_HEIGHT;

	if (v99x8.f_zoom)
	{
		v99x8_refresh.width *= 2;
		v99x8_refresh.height *= 2;
	}

	mode.width  = v99x8_refresh.width;
	mode.height = v99x8_refresh.height;

#ifdef	MD_BPP
	mode.bpp = v99x8_refresh.bpp = MD_BPP;
#else
	mode.bpp = v99x8_refresh.bpp = 16;
#endif

	mode.option	= md_video_defaultopt();

#if 0
	md_video_fixmode(&mode);
	if (!md_video_setmode(v99x8_refresh.width, v99x8_refresh.height, &mode, NULL))
		exit(EXIT_FAILURE);

	v99x8_refresh.bpp = md_video_bpp();
#endif

	v99x8_pallete_init();

	for (i = 0; i < 32; ++i)
	{
		int	n;

		int	j;
		for (j = 0; j < 64; ++j)
		{
			int	k;
			for (k = 0; k < 64; ++k)
			{
				n = (i * 5 - ((j & 0x20) ? j - 64 : j) * 2
				     - ((k & 0x20) ? k - 64 : k)) >> 2;
				if (n > 31)
					n = 31;
				if (n < 0)
					n = 0;

				tbl_yjk_b[(((j + 32) & 63) << 11)
				    + (((k + 32) & 63) << 5) + i] = n;
			}
		}

		tbl_yjk_rg[i] = 0;
		tbl_yjk_rg[i + 32] = i;
		tbl_yjk_rg[i + 64] = 31;
	}

/*
y5:k3 (low)
y5:k3 (high)
y5:j3 
y5:j3 

r: y + j
g: y + k
b: (5Y + 2J + K) >> 2
*/
	memset(blackbuf, 0, sizeof(blackbuf));
}

void V99X8::v99x8_refresh_screen(void)
{
	md_video_update(0, NULL);
}

void V99X8::v99x8_refresh_clear(void)
{
	md_video_fill(0, 0, v99x8_refresh.width, v99x8_refresh.height, pal[v99x8.col_bg].color);
}

uint8_t *V99X8::v99x8_refresh_start(int x, int w, int h)
{
	int a;

	v99x8_pallete_update();

	a = v99x8.f_zoom ? 2 : 1;

	return md_video_lockline((((7 - v99x8.ctrl[18]) & 0x0f) + x) * a
	                         , v99x8.scanline * a
	                         , w * a, h * a);
}

static __inline__ void v99x8_refresh_stop(void)
{
	md_video_unlockline();
}


#ifdef MD_BPP

#	if MD_BPP != 2

#		define pixel_put(pb, n, px) *((md_pixel_t *)(pb) + (n)) = (px)

#	else

/*

#		ifdef MD_LITTLE
#			define pixel_put(pb, n, px) *((uint8_t *)(pb) + (n) / 4)  \
			                              |= (px) << ((3 - (n)) * 2)
#		else
#			define pixel_put(pb, n, px) *((uint8_t *)(pb) + (n) / 4)  \
			                              |= (px) << ((n) * 2)
#		endif

*/

#	endif

#else
static __inline__ void pixel_put(void *pb, int n, uint32_t p1)
{
	uint8_t *p;
	int mask, pix2bpp;

	switch (v99x8_refresh.bpp)
	{
	case 16:
		*((uint16_t *)pb + n) = p1;
		break;
	case 8:
		*((uint8_t *)pb + n) = p1;
		break;
	case 32:
		*((uint32_t *)pb + n) = p1;
		break;
	case 2:
		p = (uint8_t *)pb + (n >> 2);
#ifdef MD_LITTLE
		mask = 0xc0 >> ((n & 3) * 2);
		pix2bpp = p1 << (6 - ((n & 3) * 2));
#else
		mask = 0x03 << ((n & 3) * 2);
		pix2bpp = p1 << ((n & 3) * 2);

#endif
		*p = (*p & ~mask) | (pix2bpp & mask);
		break;

/* XXX 2bpp 対応方法
 *
 *  1. まず対応する 2bit を & で 0 クリアする
 *  2. 次に、2bit を | で加える
 *
 * という手順が必要。対応は後日にて.... --Ｌ
 */

	}
}
#endif




static uint8_t sbuf[32 + 256 + 16];

typedef struct
{
	uint8_t y;
	uint8_t x;
	uint8_t n;
	uint8_t a;
} v99x8_sprite_t;



static uint8_t *v99x8_refresh_sprite1(uint8_t y)
{
	v99x8_sprite_t *ptr_a;
	int n, size;
	int i;
	uint8_t a, c, *ptr_g, *ptr_s, *tbl_sp;
	bool cf;

/*	if (v99x8.ctrl[8] & 0x02)
		return blackbuf; */

/*	tbl_sp = &v99x8.vram[(int)v99x8.ctrl[6] << 11]; */
	tbl_sp = v99x8.vram + ((int)v99x8.ctrl[6] << 11);

	memset(sbuf + 32, 0, 256);

	size = (v99x8.ctrl[1] & 0x02) ? 16 : 8;
/*	if (SpritesMAG)
		h *= 2; */

	n = 0;
	cf = FALSE;
	ptr_a = (v99x8_sprite_t *)(v99x8.vram + ((int)v99x8.ctrl[11] << 15)
		                                  + ((int)v99x8.ctrl[5] << 7));

	for (i = 0 ;; ++i, ++ptr_a)
	{
		if (i >= 32 || n >= 4 || ptr_a->y == 208)
			break;

		a = (uint8_t)(ptr_a->y - v99x8.ctrl[23]); /* a>256-h? a-=256 */
		if (a >=y || a + size < y)
			continue;

		++n;
		a = y - (a + 1);

		c = ptr_a->a;
		ptr_s = sbuf + ptr_a->x + ((c & 0x80) ? 0 : 32);
		ptr_g = tbl_sp + a
		+ (((v99x8.ctrl[1] & 0x02) ? ptr_a->n & 0xfc : ptr_a->n) << 3);

/*		cf = TRUE; */
		c &= 0x0f;

		if (ptr_s[0] == 0 && (*ptr_g & 0x80))	ptr_s[0] = c;
		if (ptr_s[1] == 0 && (*ptr_g & 0x40))	ptr_s[1] = c;
		if (ptr_s[2] == 0 && (*ptr_g & 0x20))	ptr_s[2] = c;
		if (ptr_s[3] == 0 && (*ptr_g & 0x10))	ptr_s[3] = c;
		if (ptr_s[4] == 0 && (*ptr_g & 0x08))	ptr_s[4] = c;
		if (ptr_s[5] == 0 && (*ptr_g & 0x04))	ptr_s[5] = c;
		if (ptr_s[6] == 0 && (*ptr_g & 0x02))	ptr_s[6] = c;
		if (ptr_s[7] == 0 && (*ptr_g & 0x01))	ptr_s[7] = c;

		if (!(v99x8.ctrl[1] & 0x02))
			continue;

		ptr_s += 8;
		ptr_g += 16;

		if (ptr_s[0] == 0 && (*ptr_g & 0x80))	ptr_s[0] = c;
		if (ptr_s[1] == 0 && (*ptr_g & 0x40))	ptr_s[1] = c;
		if (ptr_s[2] == 0 && (*ptr_g & 0x20))	ptr_s[2] = c;
		if (ptr_s[3] == 0 && (*ptr_g & 0x10))	ptr_s[3] = c;
		if (ptr_s[4] == 0 && (*ptr_g & 0x08))	ptr_s[4] = c;
		if (ptr_s[5] == 0 && (*ptr_g & 0x04))	ptr_s[5] = c;
		if (ptr_s[6] == 0 && (*ptr_g & 0x02))	ptr_s[6] = c;
		if (ptr_s[7] == 0 && (*ptr_g & 0x01))	ptr_s[7] = c;
	}
	return sbuf + 32;
}


static uint8_t *v99x8_refresh_sprite2(uint8_t y)
{
	v99x8_sprite_t *ptr_a;
	int n, size;
	int i;
	uint8_t a, c, *ptr_g, *ptr_c, *ptr_s, *tbl_sp;
	bool cf;

	if (v99x8.ctrl[8] & 0x02)
		return blackbuf;

	tbl_sp = v99x8.vram + ((int)v99x8.ctrl[6] << 11);

	memset(sbuf + 32, 0, 256);

	size = (v99x8.ctrl[1] & 0x02) ? 16 : 8;
/*	if (SpritesMAG)
		h *= 2; */

	n = 0;
	cf = FALSE;

	ptr_c = v99x8.vram + ((int)v99x8.ctrl[11] << 15)
		               + ((int)(v99x8.ctrl[5] & 0xf8) << 7);
	ptr_a = (v99x8_sprite_t *)(ptr_c + 0x200);

	for (i = 0 ;; ++i, ++ptr_a, ptr_c += 16)
	{
		if (i >= 32 || n >= 8 || ptr_a->y == 216)
			break;

		a = (uint8_t)(ptr_a->y - v99x8.ctrl[23]); /* a>256-h? a-=256 */
		if (a >= y || a + size < y)
			continue;

		++n;
		a = y - (a + 1);

		c = ptr_c[a];
		ptr_s = sbuf + ptr_a->x + ((c & 0x80) ? 0 : 32);
		ptr_g = tbl_sp + a
		        + (((v99x8.ctrl[1] & 0x02) ? ptr_a->n & 0xfc : ptr_a->n) << 3);

		if ((c & 0x40) == 0)
		{
			cf = TRUE;
			c &= 0x0f;

			if (ptr_s[0] == 0 && (*ptr_g & 0x80))	ptr_s[0] = c;
			if (ptr_s[1] == 0 && (*ptr_g & 0x40))	ptr_s[1] = c;
			if (ptr_s[2] == 0 && (*ptr_g & 0x20))	ptr_s[2] = c;
			if (ptr_s[3] == 0 && (*ptr_g & 0x10))	ptr_s[3] = c;
			if (ptr_s[4] == 0 && (*ptr_g & 0x08))	ptr_s[4] = c;
			if (ptr_s[5] == 0 && (*ptr_g & 0x04))	ptr_s[5] = c;
			if (ptr_s[6] == 0 && (*ptr_g & 0x02))	ptr_s[6] = c;
			if (ptr_s[7] == 0 && (*ptr_g & 0x01))	ptr_s[7] = c;

			if (!(v99x8.ctrl[1] & 0x02))
				continue;

			ptr_s += 8; ptr_g += 16;
			if (ptr_s[0] == 0 && (*ptr_g & 0x80))	ptr_s[0] = c;
			if (ptr_s[1] == 0 && (*ptr_g & 0x40))	ptr_s[1] = c;
			if (ptr_s[2] == 0 && (*ptr_g & 0x20))	ptr_s[2] = c;
			if (ptr_s[3] == 0 && (*ptr_g & 0x10))	ptr_s[3] = c;
			if (ptr_s[4] == 0 && (*ptr_g & 0x08))	ptr_s[4] = c;
			if (ptr_s[5] == 0 && (*ptr_g & 0x04))	ptr_s[5] = c;
			if (ptr_s[6] == 0 && (*ptr_g & 0x02))	ptr_s[6] = c;
			if (ptr_s[7] == 0 && (*ptr_g & 0x01))	ptr_s[7] = c;
		} else
		{
			if (!cf)
				continue;

			c &= 0x0f;
			if (*ptr_g & 0x80)	ptr_s[0] |= c;
			if (*ptr_g & 0x40)	ptr_s[1] |= c;
			if (*ptr_g & 0x20)	ptr_s[2] |= c;
			if (*ptr_g & 0x10)	ptr_s[3] |= c;
			if (*ptr_g & 0x08)	ptr_s[4] |= c;
			if (*ptr_g & 0x04)	ptr_s[5] |= c;
			if (*ptr_g & 0x02)	ptr_s[6] |= c;
			if (*ptr_g & 0x01)	ptr_s[7] |= c;

			if (!(v99x8.ctrl[1] & 0x02))
				continue;

			ptr_s += 8; ptr_g += 16;
			if (*ptr_g & 0x80)	ptr_s[0] |= c;
			if (*ptr_g & 0x40)	ptr_s[1] |= c;
			if (*ptr_g & 0x20)	ptr_s[2] |= c;
			if (*ptr_g & 0x10)	ptr_s[3] |= c;
			if (*ptr_g & 0x08)	ptr_s[4] |= c;
			if (*ptr_g & 0x04)	ptr_s[5] |= c;
			if (*ptr_g & 0x02)	ptr_s[6] |= c;
			if (*ptr_g & 0x01)	ptr_s[7] |= c;
		}
	}
	return sbuf + 32;
}



void V99X8::v99x8_refresh_sc0(int y, int h)
{
	uint8_t *pbuf;
	uint32_t fg, bg;
	int pp;


	if (f_scr)
	{
		v99x8.tbl_pg = v99x8.vram + (((int)v99x8.ctrl[4] & 0x3f) << 11);
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x7f) << 10);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(8, 240, h);
	pp = md_video_pitch() - md_video_pixbytes(240);

	fg = pal[v99x8.col_fg].color;
	bg = pal[v99x8.col_bg].color;

	while(h-- > 0)
	{
		int i;
		uint8_t *T,*G;

		T = v99x8.tbl_pn + (y >> 3) * 40;
		G = v99x8.tbl_pg + (y & 0x07);
		++y;

		for (i = 0; i < 40; ++i)
		{
			uint8_t	a;

			a = G[(int)*T++ << 3];
			if  (v99x8.f_zoom)
			{
				pixel_put(pbuf, 0, (a & 0x80)? fg : bg);
				pixel_put(pbuf, 1, (a & 0x80)? fg : bg);
				pixel_put(pbuf, 2, (a & 0x40)? fg : bg);
				pixel_put(pbuf, 3, (a & 0x40)? fg : bg);
				pixel_put(pbuf, 4, (a & 0x20)? fg : bg);
				pixel_put(pbuf, 5, (a & 0x20)? fg : bg);
				pixel_put(pbuf, 6, (a & 0x10)? fg : bg);
				pixel_put(pbuf, 7, (a & 0x10)? fg : bg);
				pixel_put(pbuf, 8, (a & 0x08)? fg : bg);
				pixel_put(pbuf, 9, (a & 0x08)? fg : bg);
				pixel_put(pbuf, 10, (a & 0x04)? fg : bg);
				pixel_put(pbuf, 11, (a & 0x04)? fg : bg);
				pbuf += md_video_pixbytes(12);
	  		} else
	  		{
				pixel_put(pbuf, 0, (a & 0x80)? fg : bg);
				pixel_put(pbuf, 1, (a & 0x40)? fg : bg);
				pixel_put(pbuf, 2, (a & 0x20)? fg : bg);
				pixel_put(pbuf, 3, (a & 0x10)? fg : bg);
				pixel_put(pbuf, 4, (a & 0x08)? fg : bg);
				pixel_put(pbuf, 5, (a & 0x04)? fg : bg);
				pbuf += md_video_pixbytes(6);
			}
	  	}
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void V99X8::v99x8_refresh_sc1(int y, int h)
{
	uint8_t *T,*G;
	int i, n;
	int pp;
	uint8_t *pbuf, *sp;

	if (f_scr)
	{
		v99x8.tbl_pg = v99x8.vram + (((int)v99x8.ctrl[4] & 0x3f) << 11);
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x7f) << 10);
		v99x8.tbl_cl = v99x8.vram + (((int)v99x8.ctrl[10] & 0x07) << 14)
		                          + ((int)v99x8.ctrl[3] << 6);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

	while (h-- > 0)
	{
		n = v99x8.ctrl[23] + y;
		G = v99x8.tbl_pg + (n & 0x07);
		T = v99x8.tbl_pn + ((n & 0xF8) << 2);
		sp = v99x8_refresh_sprite1(y++);

		for(i = 0; i < 32; ++i)
		{
			uint8_t a;
			uint8_t n;
			uint32_t fg, bg;

			n = v99x8.tbl_cl[*T >> 3];
			fg = pal[n >> 4].color;
			bg = pal[n & 0x0f].color;

			a = G[(int)*T++ << 3];
			if  (v99x8.f_zoom)
			{
				pixel_put(pbuf, 0, sp[0] ? pal[sp[0]].color : (a & 0x80) ? fg : bg);
				pixel_put(pbuf, 1, sp[0] ? pal[sp[0]].color : (a & 0x80) ? fg : bg);
				pixel_put(pbuf, 2, sp[1] ? pal[sp[1]].color : (a & 0x40) ? fg : bg);
				pixel_put(pbuf, 3, sp[1] ? pal[sp[1]].color : (a & 0x40) ? fg : bg);
				pixel_put(pbuf, 4, sp[2] ? pal[sp[2]].color : (a & 0x20) ? fg : bg);
				pixel_put(pbuf, 5, sp[2] ? pal[sp[2]].color : (a & 0x20) ? fg : bg);
				pixel_put(pbuf, 6, sp[3] ? pal[sp[3]].color : (a & 0x10) ? fg : bg);
				pixel_put(pbuf, 7, sp[3] ? pal[sp[3]].color : (a & 0x10) ? fg : bg);
				pixel_put(pbuf, 8, sp[4] ? pal[sp[4]].color : (a & 0x08) ? fg : bg);
				pixel_put(pbuf, 9, sp[4] ? pal[sp[4]].color : (a & 0x08) ? fg : bg);
				pixel_put(pbuf, 10, sp[5] ? pal[sp[5]].color : (a & 0x04) ? fg : bg);
				pixel_put(pbuf, 11, sp[5] ? pal[sp[5]].color : (a & 0x04) ? fg : bg);
				pixel_put(pbuf, 12, sp[6] ? pal[sp[6]].color : (a & 0x02) ? fg : bg);
				pixel_put(pbuf, 13, sp[6] ? pal[sp[6]].color : (a & 0x02) ? fg : bg);
				pixel_put(pbuf, 14, sp[7] ? pal[sp[7]].color : (a & 0x01) ? fg : bg);
				pixel_put(pbuf, 15, sp[7] ? pal[sp[7]].color : (a & 0x01) ? fg : bg);
				pbuf += md_video_pixbytes(16);
				sp += 8;
			} else
			{
				pixel_put(pbuf, 0, sp[0] ? pal[sp[0]].color : (a & 0x80) ? fg : bg);
				pixel_put(pbuf, 1, sp[1] ? pal[sp[1]].color : (a & 0x40) ? fg : bg);
				pixel_put(pbuf, 2, sp[2] ? pal[sp[2]].color : (a & 0x20) ? fg : bg);
				pixel_put(pbuf, 3, sp[3] ? pal[sp[3]].color : (a & 0x10) ? fg : bg);
				pixel_put(pbuf, 4, sp[4] ? pal[sp[4]].color : (a & 0x08) ? fg : bg);
				pixel_put(pbuf, 5, sp[5] ? pal[sp[5]].color : (a & 0x04) ? fg : bg);
				pixel_put(pbuf, 6, sp[6] ? pal[sp[6]].color : (a & 0x02) ? fg : bg);
				pixel_put(pbuf, 7, sp[7] ? pal[sp[7]].color : (a & 0x01) ? fg : bg);

				pbuf += md_video_pixbytes(8);
				sp += 8;
			}
		}
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void V99X8::v99x8_refresh_sc4(int y, int h)
{
	int i;
	int n;
	uint8_t *T, *PGT, *CLT;
	int pp;
	uint8_t *pbuf, *sp;

	if (f_scr)
	{
		v99x8.tbl_pg = v99x8.vram + (((int)v99x8.ctrl[4] & 0x3c) << 11);
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x7f) << 10);
		v99x8.tbl_cl = v99x8.vram + (((int)v99x8.ctrl[10] & 0x07) << 14)
		                          + (((int)v99x8.ctrl[3] & 0x80) << 6);
		f_scr = FALSE;
	}


	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

	while (h-- > 0)
	{
		n = v99x8.ctrl[23] + y;
		T = v99x8.tbl_pn + ((n & 0xf8) << 2);

		n = ((n & 0xc0) << 5) + (n & 0x07);
		PGT = v99x8.tbl_pg + n;
		CLT = v99x8.tbl_cl + n;

		if (v99x8.scr == V99X8_SCREEN_2)
			sp = v99x8_refresh_sprite1(y++);
		else
			sp = v99x8_refresh_sprite2(y++);

		for (i = 0; i < 32; ++i)
		{
			uint8_t a;
			uint8_t n;
			uint32_t fg, bg;

			n = CLT[*T << 3];
			fg = pal[n >> 4].color;
			bg = pal[n & 0x0f].color;

			a = PGT[(int)*T++ << 3];

			if  (v99x8.f_zoom)
			{
				pixel_put(pbuf, 0, sp[0] ? pal[sp[0]].color : (a & 0x80) ? fg : bg);
				pixel_put(pbuf, 1, sp[0] ? pal[sp[0]].color : (a & 0x80) ? fg : bg);
				pixel_put(pbuf, 2, sp[1] ? pal[sp[1]].color : (a & 0x40) ? fg : bg);
				pixel_put(pbuf, 3, sp[1] ? pal[sp[1]].color : (a & 0x40) ? fg : bg);
				pixel_put(pbuf, 4, sp[2] ? pal[sp[2]].color : (a & 0x20) ? fg : bg);
				pixel_put(pbuf, 5, sp[2] ? pal[sp[2]].color : (a & 0x20) ? fg : bg);
				pixel_put(pbuf, 6, sp[3] ? pal[sp[3]].color : (a & 0x10) ? fg : bg);
				pixel_put(pbuf, 7, sp[3] ? pal[sp[3]].color : (a & 0x10) ? fg : bg);
				pixel_put(pbuf, 8, sp[4] ? pal[sp[4]].color : (a & 0x08) ? fg : bg);
				pixel_put(pbuf, 9, sp[4] ? pal[sp[4]].color : (a & 0x08) ? fg : bg);
				pixel_put(pbuf, 10, sp[5] ? pal[sp[5]].color : (a & 0x04) ? fg : bg);
				pixel_put(pbuf, 11, sp[5] ? pal[sp[5]].color : (a & 0x04) ? fg : bg);
				pixel_put(pbuf, 12, sp[6] ? pal[sp[6]].color : (a & 0x02) ? fg : bg);
				pixel_put(pbuf, 13, sp[6] ? pal[sp[6]].color : (a & 0x02) ? fg : bg);
				pixel_put(pbuf, 14, sp[7] ? pal[sp[7]].color : (a & 0x01) ? fg : bg);
				pixel_put(pbuf, 15, sp[7] ? pal[sp[7]].color : (a & 0x01) ? fg : bg);
				pbuf += md_video_pixbytes(16); sp += 8;
			} else
			{
				pixel_put(pbuf, 0, sp[0] ? pal[sp[0]].color : (a & 0x80)? fg : bg);
				pixel_put(pbuf, 1, sp[1] ? pal[sp[1]].color : (a & 0x40)? fg : bg);
				pixel_put(pbuf, 2, sp[2] ? pal[sp[2]].color : (a & 0x20)? fg : bg);
				pixel_put(pbuf, 3, sp[3] ? pal[sp[3]].color : (a & 0x10)? fg : bg);
				pixel_put(pbuf, 4, sp[4] ? pal[sp[4]].color : (a & 0x08)? fg : bg);
				pixel_put(pbuf, 5, sp[5] ? pal[sp[5]].color : (a & 0x04)? fg : bg);
				pixel_put(pbuf, 6, sp[6] ? pal[sp[6]].color : (a & 0x02)? fg : bg);
				pixel_put(pbuf, 7, sp[7] ? pal[sp[7]].color : (a & 0x01)? fg : bg);

				pbuf += md_video_pixbytes(8); sp += 8;
			}
		}
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void V99X8::v99x8_refresh_sc5(int y, int h)
{
	int i;
	uint8_t *T;
	int pp;
	uint8_t *pbuf, *sp;

	if (f_scr)
	{
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x60) << 10);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

	T = v99x8.tbl_pn + (((y + v99x8.ctrl[23]) & 0xff) << 7);

	while(h-- > 0)
	{
		sp = v99x8_refresh_sprite2(y++);
		for (i = 0; i < 32; ++i)
		{
			if  (v99x8.f_zoom)
			{
				pixel_put(pbuf, 0, pal[sp[0] ? sp[0] : T[0] >> 4].color);
				pixel_put(pbuf, 1, pal[sp[0] ? sp[0] : T[0] >> 4].color);
				pixel_put(pbuf, 2, pal[sp[1] ? sp[1] : T[0] & 0x0f].color);
				pixel_put(pbuf, 3, pal[sp[1] ? sp[1] : T[0] & 0x0f].color);
				pixel_put(pbuf, 4, pal[sp[2] ? sp[2] : T[1] >> 4].color);
				pixel_put(pbuf, 5, pal[sp[2] ? sp[2] : T[1] >> 4].color);
				pixel_put(pbuf, 6, pal[sp[3] ? sp[3] : T[1] & 0x0f].color);
				pixel_put(pbuf, 7, pal[sp[3] ? sp[3] : T[1] & 0x0f].color);
				pixel_put(pbuf, 8, pal[sp[4] ? sp[4] : T[2] >> 4].color);
				pixel_put(pbuf, 9, pal[sp[4] ? sp[4] : T[2] >> 4].color);
				pixel_put(pbuf, 10, pal[sp[5] ? sp[5] : T[2] & 0x0f].color);
				pixel_put(pbuf, 11, pal[sp[5] ? sp[5] : T[2] & 0x0f].color);
				pixel_put(pbuf, 12, pal[sp[6] ? sp[6] : T[3] >> 4].color);
				pixel_put(pbuf, 13, pal[sp[6] ? sp[6] : T[3] >> 4].color);
				pixel_put(pbuf, 14, pal[sp[7] ? sp[7] : T[3] & 0x0f].color);
				pixel_put(pbuf, 15, pal[sp[7] ? sp[7] : T[3] & 0x0f].color);

				pbuf += md_video_pixbytes(16); T += 4; sp += 8;
			} else {
				pixel_put(pbuf, 0, pal[sp[0] ? sp[0] : T[0] >> 4].color);
				pixel_put(pbuf, 1, pal[sp[1] ? sp[1] : T[0] & 0x0f].color);
				pixel_put(pbuf, 2, pal[sp[2] ? sp[2] : T[1] >> 4].color);
				pixel_put(pbuf, 3, pal[sp[3] ? sp[3] : T[1] & 0x0f].color);
				pixel_put(pbuf, 4, pal[sp[4] ? sp[4] : T[2] >> 4].color);
				pixel_put(pbuf, 5, pal[sp[5] ? sp[5] : T[2] & 0x0f].color);
				pixel_put(pbuf, 6, pal[sp[6] ? sp[6] : T[3] >> 4].color);
				pixel_put(pbuf, 7, pal[sp[7] ? sp[7] : T[3] & 0x0f].color);

				pbuf += md_video_pixbytes(8); T += 4; sp += 8;
			}
		}
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void	V99X8::v99x8_refresh_sc6(int y, int h)
{
	int i;
	uint8_t *T;
	int pp;
	uint8_t *pbuf, *sp;

	if (f_scr)
	{
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x60) << 10);
		f_scr = FALSE;
	}


	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

	T = v99x8.tbl_pn + (((y + v99x8.ctrl[23]) & 0xff) << 7);

	while(h-- > 0)
	{
		sp = v99x8_refresh_sprite2(y++);
		for(i = 0; i < 32; ++i)
		{
			if  (v99x8.f_zoom)
			{
				pixel_put(pbuf, 0, pal[sp[0] ? sp[0] : T[0] >> 6].color);
				pixel_put(pbuf, 1, pal[sp[0] ? sp[0] : (T[0] >> 4) & 3].color);
				pixel_put(pbuf, 2, pal[sp[1] ? sp[1] : (T[0] >> 2) & 3].color);
				pixel_put(pbuf, 3, pal[sp[1] ? sp[1] : T[0] & 3].color);
				pixel_put(pbuf, 4, pal[sp[2] ? sp[2] : T[1] >> 6].color);
				pixel_put(pbuf, 5, pal[sp[2] ? sp[2] : (T[1] >> 4) & 3].color);
				pixel_put(pbuf, 6, pal[sp[3] ? sp[3] : (T[1] >> 2) & 3].color);
				pixel_put(pbuf, 7, pal[sp[3] ? sp[3] : T[1] & 3].color);
				pixel_put(pbuf, 8, pal[sp[4] ? sp[4] : T[2] >> 6].color);
				pixel_put(pbuf, 9, pal[sp[4] ? sp[4] : (T[2] >> 4) & 3].color);
				pixel_put(pbuf, 10, pal[sp[5] ? sp[5] : (T[2] >> 2) & 3].color);
				pixel_put(pbuf, 11, pal[sp[5] ? sp[5] : T[2] & 3].color);
				pixel_put(pbuf, 12, pal[sp[6] ? sp[6] : T[3] >> 6].color);
				pixel_put(pbuf, 13, pal[sp[6] ? sp[6] : (T[3] >> 4) & 3].color);
				pixel_put(pbuf, 14, pal[sp[7] ? sp[7] : (T[3] >> 2) & 3].color);
				pixel_put(pbuf, 15, pal[sp[7] ? sp[7] : T[3] & 3].color);
				pbuf += md_video_pixbytes(16); T += 4; sp += 8;
			} else
			{
				pixel_put(pbuf, 0, sp[0] ? pal[sp[0]].color : pal_m[(T[0] >> 6) | (T[0] & 0x30)]);
				pixel_put(pbuf, 1, sp[1] ? pal[sp[1]].color : pal_m[((T[0] & 0x0c) << 2) | (T[0] & 0x03)]);
				pixel_put(pbuf, 2, sp[2] ? pal[sp[2]].color : pal_m[(T[1] >> 6) | (T[1] & 0x30)]);
				pixel_put(pbuf, 3, sp[3] ? pal[sp[3]].color : pal_m[((T[1] & 0x0c) << 2) | (T[1] & 0x03)]);
				pixel_put(pbuf, 4, sp[4] ? pal[sp[4]].color : pal_m[(T[2] >> 6) | (T[2] & 0x30)]);
				pixel_put(pbuf, 5, sp[5] ? pal[sp[5]].color : pal_m[((T[2] & 0x0c) << 2) | (T[2] & 0x03)]);
				pixel_put(pbuf, 6, sp[6] ? pal[sp[6]].color : pal_m[(T[3] >> 6) | (T[3] & 0x30)]);
				pixel_put(pbuf, 7, sp[7] ? pal[sp[7]].color : pal_m[((T[3] & 0x0c) << 2) | (T[3] & 0x03)]);
				pbuf += md_video_pixbytes(8); T += 4; sp += 8;
			}
		}
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void V99X8::v99x8_refresh_sc7(int y, int h)
{
	uint8_t *pbuf, *sp;
	uint8_t *T;
	int pp;
	int i;

	if (f_scr)
	{
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x20) << 11);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

/* printf("%d/%d %d  %d\n", y, v99x8.ctrl[23], (y+v99x8.ctrl[23])&0xff, v99x8.ctrl[19]+1); */
	T = v99x8.tbl_pn + (((y + v99x8.ctrl[23]) & 0xff) << 8);

	while(h-- > 0)
	{
		sp = v99x8_refresh_sprite2(y);
		{
			for (i = 0; i < 32; ++i)
			{
			if  (v99x8.f_zoom)
				{
					pixel_put(pbuf, 0, pal[sp[0] ? sp[0] : T[0] >> 4].color);
					pixel_put(pbuf, 1, pal[sp[0] ? sp[0] : T[0] & 0x0f].color);
					pixel_put(pbuf, 2, pal[sp[1] ? sp[1] : T[1] >> 4].color);
					pixel_put(pbuf, 3, pal[sp[1] ? sp[1] : T[1] & 0x0f].color);
					pixel_put(pbuf, 4, pal[sp[2] ? sp[2] : T[2] >> 4].color);
					pixel_put(pbuf, 5, pal[sp[2] ? sp[2] : T[2] & 0x0f].color);
					pixel_put(pbuf, 6, pal[sp[3] ? sp[3] : T[3] >> 4].color);
					pixel_put(pbuf, 7, pal[sp[3] ? sp[3] : T[3] & 0x0f].color);
					pixel_put(pbuf, 8, pal[sp[4] ? sp[4] : T[4] >> 4].color);
					pixel_put(pbuf, 9, pal[sp[4] ? sp[4] : T[4] & 0x0f].color);
					pixel_put(pbuf, 10, pal[sp[5] ? sp[5] : T[5] >> 4].color);
					pixel_put(pbuf, 11, pal[sp[5] ? sp[5] : T[5] & 0x0f].color);
					pixel_put(pbuf, 12, pal[sp[6] ? sp[6] : T[6] >> 4].color);
					pixel_put(pbuf, 13, pal[sp[6] ? sp[6] : T[6] & 0x0f].color);
					pixel_put(pbuf, 14, pal[sp[7] ? sp[7] : T[7] >> 4].color);
					pixel_put(pbuf, 15, pal[sp[7] ? sp[7] : T[7] & 0x0f].color);
					pbuf += md_video_pixbytes(16); T += 8; sp += 8;
				} else
				{
					pixel_put(pbuf,0, sp[0]? pal[sp[0]].color : pal_m[T[0]]);
					pixel_put(pbuf,1, sp[1]? pal[sp[1]].color : pal_m[T[1]]);
					pixel_put(pbuf,2, sp[2]? pal[sp[2]].color : pal_m[T[2]]);
					pixel_put(pbuf,3, sp[3]? pal[sp[3]].color : pal_m[T[3]]);
					pixel_put(pbuf,4, sp[4]? pal[sp[4]].color : pal_m[T[4]]);
					pixel_put(pbuf,5, sp[5]? pal[sp[5]].color : pal_m[T[5]]);
					pixel_put(pbuf,6, sp[6]? pal[sp[6]].color : pal_m[T[6]]);
					pixel_put(pbuf,7, sp[7]? pal[sp[7]].color : pal_m[T[7]]);

					pbuf += md_video_pixbytes(8); T += 8; sp += 8;
				}
			}
		}
		++y;
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void V99X8::v99x8_refresh_sc8(int y, int h)
{
	uint8_t *pbuf, *sp;
	uint8_t *T;
	int pp;
	int i;
	uint8_t sc8spr[16] =
	{
		0x00,0x02,0x10,0x12,0x80,0x82,0x90,0x92,
		0x49,0x4B,0x59,0x5B,0xC9,0xCB,0xD9,0xDB
	};

	if (f_scr)
	{
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x20) << 11);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

	T = v99x8.tbl_pn + (((y + v99x8.ctrl[23]) & 0xff) << 8);

	while(h-- > 0)
	{
		sp = v99x8_refresh_sprite2(y);
		{
			for (i = 0; i < 32; ++i)
			{
				if  (v99x8.f_zoom)
				{
					pixel_put(pbuf, 0, pal_8[sp[0] ? sc8spr[sp[0]] : T[0]]);
					pixel_put(pbuf, 1, pal_8[sp[0] ? sc8spr[sp[0]] : T[0]]);
					pixel_put(pbuf, 2, pal_8[sp[1] ? sc8spr[sp[1]] : T[1]]);
					pixel_put(pbuf, 3, pal_8[sp[1] ? sc8spr[sp[1]] : T[1]]);
					pixel_put(pbuf, 4, pal_8[sp[2] ? sc8spr[sp[2]] : T[2]]);
					pixel_put(pbuf, 5, pal_8[sp[2] ? sc8spr[sp[2]] : T[2]]);
					pixel_put(pbuf, 6, pal_8[sp[3] ? sc8spr[sp[3]] : T[3]]);
					pixel_put(pbuf, 7, pal_8[sp[3] ? sc8spr[sp[3]] : T[3]]);
					pixel_put(pbuf, 8, pal_8[sp[4] ? sc8spr[sp[4]] : T[4]]);
					pixel_put(pbuf, 9, pal_8[sp[4] ? sc8spr[sp[4]] : T[4]]);
					pixel_put(pbuf, 10, pal_8[sp[5] ? sc8spr[sp[5]] : T[5]]);
					pixel_put(pbuf, 11, pal_8[sp[5] ? sc8spr[sp[5]] : T[5]]);
					pixel_put(pbuf, 12, pal_8[sp[6] ? sc8spr[sp[6]] : T[6]]);
					pixel_put(pbuf, 13, pal_8[sp[6] ? sc8spr[sp[6]] : T[6]]);
					pixel_put(pbuf, 14, pal_8[sp[7] ? sc8spr[sp[7]] : T[7]]);
					pixel_put(pbuf, 15, pal_8[sp[7] ? sc8spr[sp[7]] : T[7]]);
					pbuf += md_video_pixbytes(16); T += 8; sp += 8;
				} else
				{
					pixel_put(pbuf,0, pal_8[sp[0] ? sc8spr[sp[0]] : T[0]]);
					pixel_put(pbuf,1, pal_8[sp[1] ? sc8spr[sp[1]] : T[1]]);
					pixel_put(pbuf,2, pal_8[sp[2] ? sc8spr[sp[2]] : T[2]]);
					pixel_put(pbuf,3, pal_8[sp[3] ? sc8spr[sp[3]] : T[3]]);
					pixel_put(pbuf,4, pal_8[sp[4] ? sc8spr[sp[4]] : T[4]]);
					pixel_put(pbuf,5, pal_8[sp[5] ? sc8spr[sp[5]] : T[5]]);
					pixel_put(pbuf,6, pal_8[sp[6] ? sc8spr[sp[6]] : T[6]]);
					pixel_put(pbuf,7, pal_8[sp[7] ? sc8spr[sp[7]] : T[7]]);
					pbuf += md_video_pixbytes(8); T += 8; sp += 8;
				}
			}
		}
		++y;
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

static __inline__ uint32_t v99x8_refresh_MapYJK(int n, int j, int k, int jk)
{
	return md_maprgb15(tbl_yjk_rg[n + j], tbl_yjk_rg[n + k], tbl_yjk_b [n + jk]);
}


void V99X8::v99x8_refresh_scc(int y, int h)
{
	uint8_t *pbuf, *sp;
	uint8_t *T;
	int pp;
	int i;
	uint8_t sc8spr[16] =
	{
		0x00, 0x02, 0x10, 0x12, 0x80, 0x82, 0x90, 0x92,
		0x49, 0x4B, 0x59, 0x5B, 0xC9, 0xCB, 0xD9, 0xDB
	};

	if (f_scr)
	{
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x20) << 11);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(0, 256, h);
	pp = md_video_pitch() - md_video_pixbytes(256);

	T = v99x8.tbl_pn + (((y + v99x8.ctrl[23]) & 0xff) << 8);

	while(h-- > 0)
	{
		sp = v99x8_refresh_sprite2(y);
		for (i = 0; i < 64; ++i)
		{
			int yjk_k, yjk_j, yjk_jk;

			yjk_k = (((T[0] & 7) | ((T[1] & 7) << 3)) + 32) & 0x3f;
			yjk_j = (((T[2] & 7) | ((T[3] & 7) << 3)) + 32) & 0x3f;

			yjk_jk = (yjk_j << 11) + (yjk_k << 5);

			pixel_put(pbuf, 0, sp[0] ? pal_8[sc8spr[sp[0]]] : v99x8_refresh_MapYJK(T[0] >> 3, yjk_j, yjk_k, yjk_jk));
			pixel_put(pbuf, 1, sp[1] ? pal_8[sc8spr[sp[1]]] : v99x8_refresh_MapYJK(T[1] >> 3, yjk_j, yjk_k, yjk_jk));
			pixel_put(pbuf, 2, sp[2] ? pal_8[sc8spr[sp[2]]] : v99x8_refresh_MapYJK(T[2] >> 3, yjk_j, yjk_k, yjk_jk));
			pixel_put(pbuf, 3, sp[3] ? pal_8[sc8spr[sp[3]]] : v99x8_refresh_MapYJK(T[3] >> 3, yjk_j, yjk_k, yjk_jk));

			pbuf += md_video_pixbytes(4); T += 4; sp += 4;
		}
		++y;
		pbuf += pp;
	}

	v99x8_refresh_stop();
}

void	V99X8::v99x8_refresh_sc2(int y, int h)	{;}
void	V99X8::v99x8_refresh_sc3(int y, int h)	{;}
void	V99X8::v99x8_refresh_sca(int y, int h)	{;} /* sc10/sc11 */
void	V99X8::v99x8_refresh_scx(int y, int h)
{
	uint8_t *pbuf;
	int pp;
	uint32_t fg, bg, m[4];

	if (f_scr)
	{
		v99x8.tbl_pg = v99x8.vram + (((int)v99x8.ctrl[4] & 0x3f) << 11);
		v99x8.tbl_pn = v99x8.vram + (((int)v99x8.ctrl[2] & 0x7c) << 10);
		v99x8.tbl_cl = v99x8.vram + (((int)v99x8.ctrl[10] & 0x07) << 14)
		                          + (((int)v99x8.ctrl[3] & 0xf8) << 6);
		f_scr = FALSE;
	}

	pbuf = v99x8_refresh_start(8, 240, h);
	pp = md_video_pitch() - md_video_pixbytes(240);

	fg = pal[v99x8.col_fg].color;
	bg = pal[v99x8.col_bg].color;
	m[0] = pal_m[(v99x8.col_bg << 4) | v99x8.col_bg];
	m[1] = pal_m[(v99x8.col_bg << 4) | v99x8.col_fg];
	m[2] = pal_m[(v99x8.col_fg << 4) | v99x8.col_bg];
	m[3] = pal_m[(v99x8.col_fg << 4) | v99x8.col_fg];

	while(h-- > 0)
	{
		int i;
		uint8_t *T,*G;

		T = v99x8.tbl_pn + (y >> 3) * 80;
		G = v99x8.tbl_pg + (y & 0x07);
		++y;

		for (i = 0; i < 80; ++i)
		{
			uint8_t a;

			a = G[(int)*T++ << 3];
			if  (v99x8.f_zoom)
			{
				pixel_put(pbuf, 0, (a & 0x80) ? fg : bg);
				pixel_put(pbuf, 1, (a & 0x40) ? fg : bg);
				pixel_put(pbuf, 2, (a & 0x20) ? fg : bg);
				pixel_put(pbuf, 3, (a & 0x10) ? fg : bg);
				pixel_put(pbuf, 4, (a & 0x08) ? fg : bg);
				pixel_put(pbuf, 5, (a & 0x04) ? fg : bg);
				pbuf += md_video_pixbytes(6);
			} else
			{
				pixel_put(pbuf, 0, m[a >> 6]);
				pixel_put(pbuf, 1, m[(a >> 4)& 0x03]);
				pixel_put(pbuf, 2, m[(a >> 2)& 0x03]);

				pbuf += md_video_pixbytes(3);
			}
		}
		pbuf += pp;
	}

	v99x8_refresh_stop();
}





/*
	Skelton for retropc emulator

	Origin : Zodiac
	Author : umaiboux
	Date   : 2014.12.xx -

	[ V99x8 ]
*/

void V99X8::initialize()
{
	// register event
	register_vline_event(this);

	v99x8.vram = this->vram;
	v99x8.f_zoom = TRUE;
	//v99x8_init();
}

void V99X8::reset()
{
	v99x8_init();
#ifdef USE_CMDTIME
	cmdtime_t = 0;
	cmdtime_m = 0;
#endif
	latch1 = -1;
	latch2 = -1;
}

void V99X8::write_io8(uint32_t addr, uint32_t data)
{
	data &= 0xFF;
	switch (addr & 3) {
	case 0:
		v99x8_out_0(data);
		break;
	case 1:
		v99x8_out_1(data);
		break;
	case 2:
		v99x8_out_2(data);
		break;
	default:
		v99x8_out_3(data);
		break;
	}
}

uint32_t V99X8::read_io8(uint32_t addr)
{
	uint8_t data;
	switch (addr & 1) {
	case 0:
		data = v99x8_in_0();
		break;
	default:
		data = v99x8_in_1();
		break;
	}
	return data;
}

void V99X8::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// store regs
		v99x8_t tmp_v99x8 = v99x8;
		bool tmp_intstat = intstat;
		
		// drive vlines
		for(int v = /*get_cur_vline() + 1*/0; v < get_lines_per_frame(); v++) {
			event_vline(v, 0);
		}
		
		// restore regs
		v99x8 = tmp_v99x8;
		intstat = tmp_intstat;
	}
	md_video_update(0, NULL);
}

void V99X8::event_vline(int v, int clock)
{
#ifdef USE_CMDTIME
	cmdtime_chk();
#endif
	hsync(v);
}

void V99X8::set_intstat(bool val)
{
	if(val != intstat) {
		if(!emu->now_waiting_in_debugger) {
			write_signals(&outputs_irq, val ? 0xffffffff : 0);
		}
		intstat = val;
	}
}

void V99X8::z80_intreq(int a)
{
	set_intstat(a != Z80_NOINT);
}

int V99X8::md_video_pitch(void)
{
	return SCREEN_WIDTH*4;
}

uint8_t *V99X8::md_video_lockline(int x, int y, int w, int h)
{
	return (uint8_t*)(screen+y*SCREEN_WIDTH+x);
#if 0
	if (SDL_MUSTLOCK(video.screen))
		SDL_LockSurface(video.screen); /* 戻り値チェック？ */

	return (Uint8_t *)video.screen->pixels
	       + video.screen->format->BytesPerPixel * (video.w + x)
	       + video.screen->pitch * (y + video.h);
#endif
}

void V99X8::md_video_update(int n, /*md_video_rect_t*/void *rp)
{
	if (rp == NULL) {
		scrntype_t *dst;
		int y = 0;
		int h = SCREEN_HEIGHT;
		for(;h>0; h-=2) {
			if((dst = emu->get_screen_buffer(y)) != NULL) {
				memcpy(dst, screen+y*SCREEN_WIDTH, SCREEN_WIDTH*4);
			}
			if((dst = emu->get_screen_buffer(y + 1)) != NULL) {
				memcpy(dst, screen+y*SCREEN_WIDTH, SCREEN_WIDTH*4);
			}
			y+=2;
		}
	}
	else {
		;
	}
}

void V99X8::md_video_fill(int x, int y, int w, int h, uint32_t c)
{
	int w2;
	for(;h>0; h--) {
		scrntype_t *p = screen+y*SCREEN_WIDTH+x;
		for(w2=w;w2>0; w2--) {
			*p++ = c;
		}
		y++;
	}
}

static void lmcm(void)
{
	;
}


static void stop(void)
{
#ifdef USE_CMDTIME
	cmdtime_t = 0;
	v99x8.status[2] &= ~0x01;
#endif
}


static void vcom_lfill_(int dx, int dy, int nx, int clr)
{
	if (v99x8.ctrl[45] & 0x04) { /* Direction to left */
		// Todo?: if (dx-nx<MIN_OF_X) xxx;
		for (;nx>0;nx--,dx--) vcom_lpset(dx, dy, clr);
	}
	else {
		// Todo?: if (dx+nx>MAX_OF_X) xxx;
		for (;nx>0;nx--,dx++) vcom_lpset(dx, dy, clr);
	}
}

static void lmmv_(void)
{
	int clr;

	vcom.sx = 0;
	vcom.sy = 0;
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();
	clr = v99x8.ctrl[44];

	if ((v99x8.ctrl[45] & 0x08) == 0) {  /* Direction to down */
		int ny = vcom.ny, dy = vcom.dy;
		// Todo?: if (dy+ny>MAX_OF_Y) xxx;
		for (;ny>0;ny--,dy++) vcom_lfill_(vcom.dx, dy, vcom.nx, clr);
		vcom_setdy(vcom.dy + vcom.ny);
	}
	else {
		int ny = vcom.ny, dy = vcom.dy;
		// Todo?: if (dy-ny<MIN_OF_Y) xxx;
		for (;ny>0;ny--,dy--) vcom_lfill_(vcom.dx, dy, vcom.nx, clr);
		vcom_setdy(vcom.dy - vcom.ny);
	}

	//vcom_setdy(vcom.dy + n);

	//if (vcom.ny != abs(n))
	//	vcom_setny(vcom.ny - abs(n));
	vcom_setny(0);
}


static void vcom_lcopy_(int sx, int sy, int dx, int dy, int nx)
{
	if (v99x8.ctrl[45] & 0x04) { /* Direction to left */
		// Todo?: if (dx-nx<MIN_OF_X) xxx;
		// Todo?: if (sx-nx<MIN_OF_X) xxx;
		for (;nx>0;nx--,dx--, sx--) vcom_lpset(dx, dy, vcom_point(sx, sy));
	}
	else {
		// Todo?: if (dx+nx>MAX_OF_X) xxx;
		// Todo?: if (sx+nx>MAX_OF_X) xxx;
		if (dx+nx>vcom.xmask+1) {
			nx=vcom.xmask-dx+1;
		}
		if (sx+nx>vcom.xmask+1) {
			nx=vcom.xmask-sx+1;
		}
		for (;nx>0;nx--,dx++, sx++) vcom_lpset(dx, dy, vcom_point(sx, sy));
	}
}


static void lmmm_(void)
{
	vcom_getsx();
	vcom_getsy();
	vcom_getdx();
	vcom_getdy();
	vcom_getnx();
	vcom_getny();

	if ((v99x8.ctrl[45] & 0x08) == 0) {  /* Direction to down */
		int ny = vcom.ny, dy = vcom.dy, sy = vcom.sy;
		// Todo?: if (dy+ny>MAX_OF_Y) xxx;
		// Todo?: if (sy+ny>MAX_OF_Y) xxx;
		for (;ny>0;ny--,dy++,sy++) vcom_lcopy_(vcom.sx, sy, vcom.dx, dy, vcom.nx);
		vcom_setsy(vcom.sy + vcom.ny);
		vcom_setdy(vcom.dy + vcom.ny);
	}
	else {
		int ny = vcom.ny, dy = vcom.dy, sy = vcom.sy;
		// Todo?: if (dy-ny<MIN_OF_Y) xxx;
		// Todo?: if (sy-ny<MIN_OF_Y) xxx;
		for (;ny>0;ny--,dy--,sy--) vcom_lcopy_(vcom.sx, sy, vcom.dx, dy, vcom.nx);
		vcom_setsy(vcom.sy - vcom.ny);
		vcom_setdy(vcom.dy - vcom.ny);
	}

	//vcom_setsy(vcom.sy + n);
	//vcom_setdy(vcom.dy + n);

	//if (vcom.ny != abs(n))
	//	vcom_setny(vcom.ny - abs(n));
	vcom_setny(0);
}

#ifdef USE_CMDTIME
void cmdtime_set(int m)
{
	cmdtime_m = m>>4;
	cmdtime_t = 0;

	/* provisional data */
	/*
	if ((v99x8.ctrl[8] & 0x02) == 0) && ((v99x8.ctrl[1] & 0x40) != 0)
	YMMM 4.83 microseconds/byte
	HMMM 5.85 microseconds/byte
	HMMV 3.05 microseconds/byte
	LMMM 6.87 microseconds/dot
	LMMV 5.91 microseconds/dot
	*/
	switch(m >> 4)
	{
	case 0xe:	/* ymmm */ vcom_getdx();vcom_getny();cmdtime_t=483*(v99x8.ctrl[45] & 0x04?vcom.dx:v99x8.mode.xsize-vcom.dx)*vcom.ny/pixmask.npix; break;
	case 0xd:	/* hmmm */ vcom_getnx();vcom_getny();cmdtime_t=585*vcom.nx*vcom.ny/pixmask.npix; break;
	case 0xc:	/* hmmv */ vcom_getnx();vcom_getny();cmdtime_t=305*vcom.nx*vcom.ny/pixmask.npix; break;
	case 0x9:	/* lmmm */ vcom_getnx();vcom_getny();cmdtime_t=687*vcom.nx*vcom.ny; break;
	case 0x8:	/* lmmv */ vcom_getnx();vcom_getny();cmdtime_t=591*vcom.nx*vcom.ny; break;
	}
	//cmdtime_t = cmdtime_t*60*262/1000000/100;
	cmdtime_t = cmdtime_t/6361;
	if (cmdtime_t<0) cmdtime_t=0;
}

void cmdtime_chk(void)
{
	if (cmdtime_t) {
		cmdtime_t--;
		if (0 == cmdtime_t) v99x8.status[2] &= ~0x01;
	}
}
#endif

#define STATE_VERSION	1

bool V99X8::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(&v99x8, sizeof(v99x8), 1);
#ifdef USE_CMDTIME
	state_fio->StateValue(cmdtime_t);
	state_fio->StateValue(cmdtime_m);
#endif
	state_fio->StateValue(latch1);
	state_fio->StateValue(latch2);
	state_fio->StateValue(vram_addr);
	state_fio->StateValue(vram_page);
	state_fio->StateValue(f_out3);
	state_fio->StateValue(f_mode);
	state_fio->StateValue(flag_frame);
	state_fio->StateArray(&vcom, sizeof(vcom), 1);
	if(loading) {
		vcom.src = vram + state_fio->FgetInt32_LE();
		vcom.dst = vram + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(vcom.src - vram));
		state_fio->FputInt32_LE((int)(vcom.dst - vram));
	}
	state_fio->StateArray(&r44, sizeof(r44), 1);
	state_fio->StateArray(&pixmask, sizeof(pixmask), 1);
	state_fio->StateArray(&v99x8_refresh, sizeof(v99x8_refresh), 1);
	state_fio->StateArray(pal, sizeof(pal), 1);
	state_fio->StateArray(pal_8, sizeof(pal_8), 1);
	state_fio->StateArray(pal_m, sizeof(pal_m), 1);
	state_fio->StateValue(col_bg);
	state_fio->StateArray(tbl_yjk_b, sizeof(tbl_yjk_b), 1);
	state_fio->StateArray(tbl_yjk_rg, sizeof(tbl_yjk_rg), 1);
	state_fio->StateArray(blackbuf, sizeof(blackbuf), 1);
	state_fio->StateArray(sbuf, sizeof(sbuf), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(intstat);
	
	// post process
	if(loading) {
		v99x8.vram = vram;
		f_scr = true;
	}
	return true;
}

