/*
 * Common source code project -> FM-7 -> Display -> Vram access
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Sep 27, 2015 : Split from display.cpp .
 */

#include "fm7_display.h"

uint8 DISPLAY::read_vram_8_200l(uint32 addr, uint32 offset)
{
	uint32 page_offset = 0;
	uint32 pagemod;
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) {
		page_offset = 0xc000;
	}
#endif
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) page_offset += 0x18000;
# endif
	pagemod = addr & 0xc000;
	return gvram[(((addr + offset) & 0x3fff) | pagemod) + page_offset];
}

uint8 DISPLAY::read_vram_l4_400l(uint32 addr, uint32 offset)
{
#if defined(_FM77L4)
	if(addr < 0x8000) {
		if(workram) {
			uint32 raddr = addr & 0x3fff;
			if((multimode_accessmask & 0x04) == 0) {
				return gvram[0x8000 + (raddr + offset) & 0x7fff];
			}
			return 0xff;
		}
		pagemod = addr & 0x4000;
		return gvram[((addr + offset) & mask) | pagemod];
	} else if(addr < 0x9800) {
		return textvram[addr & 0x0fff];
	} else { // $9800-$bfff
		return subrom_l4[addr - 0x9800];
	}
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_8_400l(uint32 addr, uint32 offset)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 color = vram_bank & 0x03;
	uint32 pagemod;
	uint32 page_offset = 0;
	uint32 raddr;
	if(addr >= 0x8000) return 0xff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) page_offset += 0x18000;
# endif
	if(color > 2) color = 0;
	pagemod = 0x8000 * color;
	return gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset];
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_8_400l_direct(uint32 addr, uint32 offset)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 pagemod;
	uint32 page_offset = 0;
	pagemod = addr & 0x18000;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	//addr = addr % 0x18000;
	if(vram_active_block != 0) page_offset += 0x18000;
# endif
	return gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset];
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_4096(uint32 addr, uint32 offset)
{
#if defined(_FM77AV_VARIANTS)
	uint32 page_offset = 0;
	uint32 pagemod;
	if(active_page != 0) {
		page_offset = 0xc000;
	}
	pagemod = addr & 0xe000;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) page_offset += 0x18000;
# endif
	return gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset];
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_256k(uint32 addr, uint32 offset)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 page_offset;
	uint32 pagemod;
	page_offset = 0xc000 * (vram_bank & 0x03);
	pagemod = addr & 0xe000;
	return gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset];
#endif
	return 0xff;
}


void DISPLAY::write_vram_8_200l(uint32 addr, uint32 offset, uint32 data)
{
	uint32 page_offset = 0;
	uint32 pagemod;
	uint8 val8 = data & 0xff;
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) {
		page_offset = 0xc000;
	}
#endif
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block) page_offset += 0x18000;
#endif
	pagemod = addr & 0xc000;
	gvram[(((addr + offset) & 0x3fff) | pagemod) + page_offset] = val8;
# if defined(_FM77AV_VARIANTS)	
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
# else
	vram_wrote = true;
# endif	
}

void DISPLAY::write_vram_l4_400l(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77L4)
	if(addr < 0x8000) {
		if(workram) {
			uint32 raddr = addr & 0x3fff;
			if((multimode_accessmask & 0x04) == 0) {
				gvram[0x8000 + (raddr + offset) & 0x7fff] = (uint8)data;
			}
			return;
		}
		pagemod = addr & 0x4000;
		gvram[((addr + offset) & mask) | pagemod] = (uint8)data;
	} else if(addr < 0x9800) {
	  textvram[addr & 0x0fff] = (uint8)data;
	} else { // $9800-$bfff
		//return subrom_l4[addr - 0x9800];
	}
	return;
#endif	
}

void DISPLAY::write_vram_8_400l(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 color = vram_bank & 0x03;
	uint32 pagemod;
	uint32 page_offset = 0;
	uint8 val8 = (uint8)(data & 0x00ff);
	if(addr >= 0x8000) return;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) page_offset += 0x18000;
# endif   
	if(color > 2) color = 0;
	pagemod = 0x8000 * color;
	//offset = (offset & 0x3fff) << 1;
	gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset] = val8;

	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
#endif
}

void DISPLAY::write_vram_8_400l_direct(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 pagemod;
	uint32 page_offset = 0;
	uint8 val8 = (uint8)(data & 0x00ff);
	//offset = offset & 0x7fff;
	pagemod = addr & 0x18000;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	//addr = addr % 0x18000;
	if(vram_active_block != 0) page_offset += 0x18000;
# endif 
	gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset] = val8;
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
#endif
}

void DISPLAY::write_vram_4096(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV_VARIANTS)
	uint32 page_offset = 0;
	uint32 pagemod;
	if(active_page != 0) {
		page_offset = 0xc000;
	}
	pagemod = addr & 0xe000;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) page_offset += 0x18000;
#endif
	gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset] = (uint8)data;
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
#endif
}

void DISPLAY::write_vram_256k(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 page_offset = 0;
	uint32 pagemod;
	page_offset = 0xc000 * (vram_bank & 0x03);
	pagemod = addr & 0xe000;
	gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset] = (uint8)(data & 0xff);
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
	return;
#endif
}

inline void DISPLAY::GETVRAM_8_200L(int yoff, scrntype *p, uint32 mask, bool window_inv = false)
{
	register uint8 b, r, g;
	register uint32 dot;
	uint32 yoff_d;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = vram_display_block;
#endif
#if defined(_FM77AV_VARIANTS)
	if(display_page == 1) { // Is this dirty?
		yoff_d = offset_point_bank1_bak;
	} else {
		yoff_d = offset_point_bak;
	}
#else
	yoff_d = offset_point;
#endif	
	yoff_d = (yoff + yoff_d) & 0x3fff;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(window_inv) {
		if(dpage == 0) {
			dpage = 1;
		} else {
			dpage = 0;
		}
	}
	if(dpage != 0) yoff_d += 0x18000;
#endif
	b = r = g = 0;
#if defined(_FM77AV_VARIANTS)
	if(display_page == 1) {
		if(mask & 0x01) b = gvram_shadow[yoff_d + 0x0c000];
		if(mask & 0x02) r = gvram_shadow[yoff_d + 0x10000];
		if(mask & 0x04) g = gvram_shadow[yoff_d + 0x14000];
	} else {
		if(mask & 0x01) b = gvram_shadow[yoff_d + 0x00000];
		if(mask & 0x02) r = gvram_shadow[yoff_d + 0x04000];
		if(mask & 0x04) g = gvram_shadow[yoff_d + 0x08000];
	}
#else
	if(mask & 0x01) b = gvram[yoff_d + 0x00000];
	if(mask & 0x02) r = gvram[yoff_d + 0x04000];
	if(mask & 0x04) g = gvram[yoff_d + 0x08000];
#endif	
	dot = ((g & 0x80) >> 5) | ((r & 0x80) >> 6) | ((b & 0x80) >> 7);
	p[0] = dpalette_pixel[dot];
	dot = ((g & 0x40) >> 4) | ((r & 0x40) >> 5) | ((b & 0x40) >> 6);
	p[1] = dpalette_pixel[dot];
	dot = ((g & 0x20) >> 3) | ((r & 0x20) >> 4) | ((b & 0x20) >> 5);
	p[2] = dpalette_pixel[dot];
	dot = ((g & 0x10) >> 2) | ((r & 0x10) >> 3) | ((b & 0x10) >> 4);
	p[3] = dpalette_pixel[dot];
					
	dot = ((g & 0x8) >> 1) | ((r & 0x8) >> 2) | ((b & 0x8) >> 3);
	p[4] = dpalette_pixel[dot];
	dot = (g & 0x4) | ((r & 0x4) >> 1) | ((b & 0x4) >> 2);
	p[5] = dpalette_pixel[dot];
	dot = ((g & 0x2) << 1) | (r & 0x2) | ((b & 0x2) >> 1);
	p[6] = dpalette_pixel[dot];
	dot = ((g & 0x1) << 2) | ((r & 0x1) << 1) | (b & 0x1);
	p[7] = dpalette_pixel[dot];
}

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
inline void DISPLAY::GETVRAM_8_400L(int yoff, scrntype *p, uint32 mask, bool window_inv = false)
{
	register uint8 b, r, g;
	register uint32 dot;
	uint32 yoff_d;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = vram_display_block;
# endif
	
	if(display_page == 1) { // Is this dirty?
		yoff_d = offset_point_bank1_bak;
	} else {
		yoff_d = offset_point_bak;
	}
	yoff_d = (yoff + (yoff_d << 1)) & 0x7fff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(window_inv) {
		if(dpage == 0) {
			dpage = 1;
		} else {
			dpage = 0;
		}
	}
	if(dpage != 0) yoff_d += 0x18000;
# endif
	b = r = g = 0;
	if(mask & 0x01) b = gvram_shadow[yoff_d + 0x00000];
	if(mask & 0x02) r = gvram_shadow[yoff_d + 0x08000];
	if(mask & 0x04) g = gvram_shadow[yoff_d + 0x10000];

	dot = ((g & 0x80) >> 5) | ((r & 0x80) >> 6) | ((b & 0x80) >> 7);
	p[0] = dpalette_pixel[dot];
	dot = ((g & 0x40) >> 4) | ((r & 0x40) >> 5) | ((b & 0x40) >> 6);
	p[1] = dpalette_pixel[dot];
	dot = ((g & 0x20) >> 3) | ((r & 0x20) >> 4) | ((b & 0x20) >> 5);
	p[2] = dpalette_pixel[dot];
	dot = ((g & 0x10) >> 2) | ((r & 0x10) >> 3) | ((b & 0x10) >> 4);
	p[3] = dpalette_pixel[dot];
					
	dot = ((g & 0x8) >> 1) | ((r & 0x8) >> 2) | ((b & 0x8) >> 3);
	p[4] = dpalette_pixel[dot];
	dot = (g & 0x4) | ((r & 0x4) >> 1) | ((b & 0x4) >> 2);
	p[5] = dpalette_pixel[dot];
	dot = ((g & 0x2) << 1) | (r & 0x2) | ((b & 0x2) >> 1);
	p[6] = dpalette_pixel[dot];
	dot = ((g & 0x1) << 2) | ((r & 0x1) << 1) | (b & 0x1);
	p[7] = dpalette_pixel[dot];
}

inline void DISPLAY::GETVRAM_256k(int yoff, scrntype *p, uint32 mask)
{
	register uint32 b3, r3, g3;
	register uint32 b4, r4, g4;
	register uint32 btmp, rtmp, gtmp;
	
	register scrntype b, r, g;
	scrntype pixel;
	uint32 yoff_d1, yoff_d2;
	uint32 _bit;
	int _shift;
	int cp;
	
	r3 = g3 = b3 = 0;
	r4 = g4 = b4 = 0;
	r = g = b = 0;
	
	yoff_d1 = offset_point_bak;
	yoff_d2 = offset_point_bank1_bak;
	yoff_d1 = (yoff + yoff_d1) & 0x1fff;
	yoff_d2 = (yoff + yoff_d2) & 0x1fff;
	if(mask & 0x01) {
		b3  = gvram_shadow[yoff_d1] << 24;
		b3 |= gvram_shadow[yoff_d1 + 0x02000] << 16;
		
		b3 |= gvram_shadow[yoff_d2 + 0x0c000] << 8;
		b3 |= gvram_shadow[yoff_d2 + 0x0e000] << 0;
	
		b4  = gvram_shadow[yoff_d1 + 0x18000] << 8;
		b4 |= gvram_shadow[yoff_d1 + 0x1a000] << 0;
	}
	if(mask & 0x02) {
		r3  = gvram_shadow[yoff_d1 + 0x04000] << 24;
		r3 |= gvram_shadow[yoff_d1 + 0x06000] << 16;
		r3 |= gvram_shadow[yoff_d2 + 0x10000] << 8;
		r3 |= gvram_shadow[yoff_d2 + 0x12000] << 0;
		r4  = gvram_shadow[yoff_d1 + 0x1c000] << 8;
		r4 |= gvram_shadow[yoff_d1 + 0x1e000] << 0;
	}

	if(mask & 0x04) {
		g3  = gvram_shadow[yoff_d1 + 0x08000] << 24;
		g3 |= gvram_shadow[yoff_d1 + 0x0a000] << 16;
		g3 |= gvram_shadow[yoff_d2 + 0x14000] << 8;
		g3 |= gvram_shadow[yoff_d2 + 0x16000] << 0;
		
		g4  = gvram_shadow[yoff_d1 + 0x20000] << 8;
		g4 |= gvram_shadow[yoff_d1 + 0x22000] << 0;
	}
	
	cp = 0;
	for(_shift = 7; _shift >= 0; _shift--) {
		_bit = 0x01010101 << _shift;
		r = g = b = 0;
		if(mask & 0x01) {
			btmp = (b3 & _bit) >> _shift;
			b = (((btmp & (0x01 << 24)) != 0) ? 0x80 : 0) | (((btmp & (0x01 << 16)) != 0)? 0x40 : 0)
				| (((btmp & (0x01 << 8)) != 0) ? 0x20 : 0) | (((btmp & 0x01) != 0) ? 0x10   : 0);
			btmp = (b4 & _bit) >> _shift;
			b = b | (((btmp & (0x01 << 8)) != 0) ? 0x08 : 0) | (((btmp & 0x01) != 0) ? 0x04 : 0);
		}
		if(mask & 0x02) {
			rtmp = (r3 & _bit) >> _shift;
			r = ((rtmp & (0x01 << 24)) ? 0x80 : 0) | ((rtmp & (0x01 << 16)) ? 0x40 : 0)
				| ((rtmp & (0x01 << 8)) ? 0x20 : 0) | ((rtmp & 0x01) ? 0x10   : 0);
			rtmp = (r4 & _bit) >> _shift;
			r = r | ((rtmp & (0x01 << 8)) ? 0x08 : 0) | ((rtmp & 0x01) ? 0x04 : 0);
		}
		if(mask & 0x04) {
			gtmp = (g3 & _bit) >> _shift;
			g = ((gtmp & (0x01 << 24)) ? 0x80 : 0) | ((gtmp & (0x01 << 16)) ? 0x40 : 0)
				| ((gtmp & (0x01 << 8)) ? 0x20 : 0) | ((gtmp & 0x01) ? 0x10   : 0);
			gtmp = (g4 & _bit) >> _shift;
			g = g | ((gtmp & (0x01 << 8)) ? 0x08 : 0) | ((gtmp & 0x01) ? 0x04 : 0);
		}
	
		pixel = RGB_COLOR(r, g, b);
		p[cp] = pixel;
		p[cp + 1] = pixel;
		cp += 2;
	}
	
}
#endif

#if defined(_FM77AV_VARIANTS)
inline void DISPLAY::GETVRAM_4096(int yoff, scrntype *p, uint32 mask, bool window_inv = false)
{
	uint32 b3, r3, g3;
	scrntype b, r, g;
	uint32 idx;;
	scrntype pixel;
	uint32 yoff_d1, yoff_d2;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = vram_display_block;
# endif
	
	yoff_d1 = offset_point_bak;
	yoff_d2 = offset_point_bank1_bak;
	yoff_d1 = (yoff + yoff_d1) & 0x1fff;
	yoff_d2 = (yoff + yoff_d2) & 0x1fff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(window_inv) {
		if(dpage == 0) {
			dpage = 1;
		} else {
			dpage = 0;
		}
	}
	if(dpage != 0) {
		yoff_d1 += 0x18000;
		yoff_d2 += 0x18000;
	}
# endif

	b3  = gvram_shadow[yoff_d1] << 24;
	b3 |= gvram_shadow[yoff_d1 + 0x02000] << 16;
	r3  = gvram_shadow[yoff_d1 + 0x04000] << 24;
	r3 |= gvram_shadow[yoff_d1 + 0x06000] << 16;
		
	g3  = gvram_shadow[yoff_d1 + 0x08000] << 24;
	g3 |= gvram_shadow[yoff_d1 + 0x0a000] << 16;
		
	b3 |= gvram_shadow[yoff_d2 + 0x0c000] << 8;
	b3 |= gvram_shadow[yoff_d2 + 0x0e000] << 0;
		
	r3 |= gvram_shadow[yoff_d2 + 0x10000] << 8;
	r3 |= gvram_shadow[yoff_d2 + 0x12000] << 0;
	g3 |= gvram_shadow[yoff_d2 + 0x14000] << 8;
	g3 |= gvram_shadow[yoff_d2 + 0x16000] << 0;
   
	g = ((g3 & (0x80 << 24)) >> 20) | ((g3 & (0x80 << 16)) >> 13) | ((g3 & (0x80 << 8)) >> 6)  | ((g3 & 0x80) << 1);
	r = ((r3 & (0x80 << 24)) >> 24) | ((r3 & (0x80 << 16)) >> 17) | ((r3 & (0x80 << 8)) >> 10) | ((r3 & 0x80) >> 3);
	b = ((b3 & (0x80 << 24)) >> 28) | ((b3 & (0x80 << 16)) >> 21) | ((b3 & (0x80 << 8)) >> 14) | ((b3 & 0x80) >> 7);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[0] = pixel;
	p[1] = pixel;

	g = ((g3 & (0x40 << 24)) >> 19) | ((g3 & (0x40 << 16)) >> 12) | ((g3 & (0x40 << 8)) >> 5)  | ((g3 & 0x40) << 2);
	r = ((r3 & (0x40 << 24)) >> 23) | ((r3 & (0x40 << 16)) >> 16) | ((r3 & (0x40 << 8)) >> 9)  | ((r3 & 0x40) >> 2);
	b = ((b3 & (0x40 << 24)) >> 27) | ((b3 & (0x40 << 16)) >> 20) | ((b3 & (0x40 << 8)) >> 13) | ((b3 & 0x40) >> 6);
	
	//g = ((g3 & (0x40 << 24)) ? 0x800 : 0) | ((g3 & (0x40 << 16)) ? 0x400 : 0) | ((g3 & (0x40 << 8)) ? 0x200 : 0) | ((g3 & 0x40) ? 0x100 : 0);
	//r = ((r3 & (0x40 << 24)) ? 0x80  : 0) | ((r3 & (0x40 << 16)) ? 0x40  : 0) | ((r3 & (0x40 << 8)) ? 0x20  : 0) | ((r3 & 0x40) ? 0x10  : 0);
	//b = ((b3 & (0x40 << 24)) ? 0x8   : 0) | ((b3 & (0x40 << 16)) ? 0x4   : 0) | ((b3 & (0x40 << 8)) ? 0x2   : 0) | ((b3 & 0x40) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[2] = pixel;
	p[3] = pixel;

//	g = ((g3 & (0x20 << 24)) ? 0x800 : 0) | ((g3 & (0x20 << 16)) ? 0x400 : 0) | ((g3 & (0x20 << 8)) ? 0x200 : 0) | ((g3 & 0x20) ? 0x100 : 0);
//	r = ((r3 & (0x20 << 24)) ? 0x80  : 0) | ((r3 & (0x20 << 16)) ? 0x40  : 0) | ((r3 & (0x20 << 8)) ? 0x20  : 0) | ((r3 & 0x20) ? 0x10  : 0);
//	b = ((b3 & (0x20 << 24)) ? 0x8   : 0) | ((b3 & (0x20 << 16)) ? 0x4   : 0) | ((b3 & (0x20 << 8)) ? 0x2   : 0) | ((b3 & 0x20) ? 0x1   : 0);
	g = ((g3 & (0x20 << 24)) >> 18) | ((g3 & (0x20 << 16)) >> 11) | ((g3 & (0x20 << 8)) >> 4)  | ((g3 & 0x20) << 3);
	r = ((r3 & (0x20 << 24)) >> 22) | ((r3 & (0x20 << 16)) >> 15) | ((r3 & (0x20 << 8)) >> 8)  | ((r3 & 0x20) >> 1);
	b = ((b3 & (0x20 << 24)) >> 26) | ((b3 & (0x20 << 16)) >> 19) | ((b3 & (0x20 << 8)) >> 12) | ((b3 & 0x20) >> 5);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[4] = pixel;
	p[5] = pixel;

	//g = ((g3 & (0x10 << 24)) ? 0x800 : 0) | ((g3 & (0x10 << 16)) ? 0x400 : 0) | ((g3 & (0x10 << 8)) ? 0x200 : 0) | ((g3 & 0x10) ? 0x100 : 0);
	//r = ((r3 & (0x10 << 24)) ? 0x80  : 0) | ((r3 & (0x10 << 16)) ? 0x40  : 0) | ((r3 & (0x10 << 8)) ? 0x20  : 0) | ((r3 & 0x10) ? 0x10  : 0);
	//b = ((b3 & (0x10 << 24)) ? 0x8   : 0) | ((b3 & (0x10 << 16)) ? 0x4   : 0) | ((b3 & (0x10 << 8)) ? 0x2   : 0) | ((b3 & 0x10) ? 0x1   : 0);
	g = ((g3 & (0x10 << 24)) >> 17) | ((g3 & (0x10 << 16)) >> 10) | ((g3 & (0x10 << 8)) >> 3)  | ((g3 & 0x10) << 4);
	r = ((r3 & (0x10 << 24)) >> 21) | ((r3 & (0x10 << 16)) >> 14) | ((r3 & (0x10 << 8)) >> 7)  | ((r3 & 0x10) >> 0);
	b = ((b3 & (0x10 << 24)) >> 25) | ((b3 & (0x10 << 16)) >> 18) | ((b3 & (0x10 << 8)) >> 11) | ((b3 & 0x10) >> 4);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[6] = pixel;
	p[7] = pixel;


	//g = ((g3 & (0x8 << 24)) ? 0x800 : 0) | ((g3 & (0x8 << 16)) ? 0x400 : 0) | ((g3 & (0x8 << 8)) ? 0x200 : 0) | ((g3 & 0x8) ? 0x100 : 0);
	//r = ((r3 & (0x8 << 24)) ? 0x80  : 0) | ((r3 & (0x8 << 16)) ? 0x40  : 0) | ((r3 & (0x8 << 8)) ? 0x20  : 0) | ((r3 & 0x8) ? 0x10  : 0);
	//b = ((b3 & (0x8 << 24)) ? 0x8   : 0) | ((b3 & (0x8 << 16)) ? 0x4   : 0) | ((b3 & (0x8 << 8)) ? 0x2   : 0) | ((b3 & 0x8) ? 0x1   : 0);
	g = ((g3 & (0x8 << 24)) >> 16) | ((g3 & (0x8 << 16)) >> 9)  | ((g3 & (0x8 << 8)) >> 2)  | ((g3 & 0x8) << 5);
	r = ((r3 & (0x8 << 24)) >> 20) | ((r3 & (0x8 << 16)) >> 13) | ((r3 & (0x8 << 8)) >> 6)  | ((r3 & 0x8) << 1);
	b = ((b3 & (0x8 << 24)) >> 24) | ((b3 & (0x8 << 16)) >> 17) | ((b3 & (0x8 << 8)) >> 10) | ((b3 & 0x8) >> 3);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[8] = pixel;
	p[9] = pixel;

	
	//g = ((g3 & (0x4 << 24)) ? 0x800 : 0) | ((g3 & (0x4 << 16)) ? 0x400 : 0) | ((g3 & (0x4 << 8)) ? 0x200 : 0) | ((g3 & 0x4) ? 0x100 : 0);
	//r = ((r3 & (0x4 << 24)) ? 0x80  : 0) | ((r3 & (0x4 << 16)) ? 0x40  : 0) | ((r3 & (0x4 << 8)) ? 0x20  : 0) | ((r3 & 0x4) ? 0x10  : 0);
	//b = ((b3 & (0x4 << 24)) ? 0x8   : 0) | ((b3 & (0x4 << 16)) ? 0x4   : 0) | ((b3 & (0x4 << 8)) ? 0x2   : 0) | ((b3 & 0x4) ? 0x1   : 0);
	g = ((g3 & (0x4 << 24)) >> 15) | ((g3 & (0x4 << 16)) >> 8)  | ((g3 & (0x4 << 8)) >> 1) | ((g3 & 0x4) << 6);
	r = ((r3 & (0x4 << 24)) >> 19) | ((r3 & (0x4 << 16)) >> 12) | ((r3 & (0x4 << 8)) >> 5) | ((r3 & 0x4) << 2);
	b = ((b3 & (0x4 << 24)) >> 23) | ((b3 & (0x4 << 16)) >> 16) | ((b3 & (0x4 << 8)) >> 9) | ((b3 & 0x4) >> 2);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[10] = pixel;
	p[11] = pixel;

	//g = ((g3 & (0x2 << 24)) ? 0x800 : 0) | ((g3 & (0x2 << 16)) ? 0x400 : 0) | ((g3 & (0x2 << 8)) ? 0x200 : 0) | ((g3 & 0x2) ? 0x100 : 0);
	//r = ((r3 & (0x2 << 24)) ? 0x80  : 0) | ((r3 & (0x2 << 16)) ? 0x40  : 0) | ((r3 & (0x2 << 8)) ? 0x20  : 0) | ((r3 & 0x2) ? 0x10  : 0);
	//b = ((b3 & (0x2 << 24)) ? 0x8   : 0) | ((b3 & (0x2 << 16)) ? 0x4   : 0) | ((b3 & (0x2 << 8)) ? 0x2   : 0) | ((b3 & 0x2) ? 0x1   : 0);
	g = ((g3 & (0x2 << 24)) >> 14) | ((g3 & (0x2 << 16)) >> 7)  | ((g3 & (0x2 << 8)) >> 0) | ((g3 & 0x2) << 7);
	r = ((r3 & (0x2 << 24)) >> 18) | ((r3 & (0x2 << 16)) >> 11) | ((r3 & (0x2 << 8)) >> 4) | ((r3 & 0x2) << 3);
	b = ((b3 & (0x2 << 24)) >> 22) | ((b3 & (0x2 << 16)) >> 15) | ((b3 & (0x2 << 8)) >> 8) | ((b3 & 0x2) >> 1);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[12] = pixel;
	p[13] = pixel;

	//g = ((g3 & (0x1 << 24)) ? 0x800 : 0) | ((g3 & (0x1 << 16)) ? 0x400 : 0) | ((g3 & (0x1 << 8)) ? 0x200 : 0) | ((g3 & 0x1) ? 0x100 : 0);
	//r = ((r3 & (0x1 << 24)) ? 0x80  : 0) | ((r3 & (0x1 << 16)) ? 0x40  : 0) | ((r3 & (0x1 << 8)) ? 0x20  : 0) | ((r3 & 0x1) ? 0x10  : 0);
	//b = ((b3 & (0x1 << 24)) ? 0x8   : 0) | ((b3 & (0x1 << 16)) ? 0x4   : 0) | ((b3 & (0x1 << 8)) ? 0x2   : 0) | ((b3 & 0x1) ? 0x1   : 0);
	g = ((g3 & (0x1 << 24)) >> 13) | ((g3 & (0x1 << 16)) >> 6)  | ((g3 & (0x1 << 8)) << 1) | ((g3 & 0x1) << 8);
	r = ((r3 & (0x1 << 24)) >> 17) | ((r3 & (0x1 << 16)) >> 10) | ((r3 & (0x1 << 8)) >> 3) | ((r3 & 0x1) << 4);
	b = ((b3 & (0x1 << 24)) >> 21) | ((b3 & (0x1 << 16)) >> 14) | ((b3 & (0x1 << 8)) >> 7) | ((b3 & 0x1) >> 0);
	   
	idx = (g  | b | r ) & mask;
	pixel = analog_palette_pixel[idx];
	p[14] = pixel;
	p[15] = pixel;
}
#endif

void DISPLAY::draw_screen()
{
	int y;
	int x;
	scrntype *p, *pp;
	register int yoff;
	register uint32 rgbmask;
	uint16 wx_begin, wx_end, wy_low, wy_high;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	{
		wx_begin = window_xbegin;
		wx_end   = window_xend;
		wy_low   = window_low;
		wy_high  = window_high;
		bool _flag = window_opened; 
		if((wx_begin < wx_end) && (wy_low < wy_high)) {
			window_opened = true;
		} else {
			window_opened = false;
		}
		if(_flag != window_opened) vram_wrote_shadow = true;
	}
#endif
	frame_skip_count++;
#if defined(_FM77AV_VARIANTS)
	{
		int factor = (config.dipswitch & FM7_DIPSW_FRAMESKIP) >> 28;
		if(frame_skip_count <= factor) return;
		//vram_wrote_shadow = false;
		frame_skip_count = 0;
	}
#else
	{
		int factor = (config.dipswitch & FM7_DIPSW_FRAMESKIP) >> 28;
		if((frame_skip_count <= factor) || !(vram_wrote)) return;
		vram_wrote = false;
		frame_skip_count = 0;
	}
#endif
	  // Set blank
	if(!crt_flag) {
		for(y = 0; y < 400; y++) {
			memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
		return;
	}

	if(display_mode == DISPLAY_MODE_8_200L) {
		yoff = 0;
		rgbmask = ~multimode_dispmask;
		for(y = 0; y < 400; y += 2) {
# if defined(_FM77AV_VARIANTS)
			if(!vram_wrote_shadow && !vram_draw_table[y >> 1]) continue;
			vram_draw_table[y >> 1] = false;
# endif			
			p = emu->screen_buffer(y);
			pp = p;
			yoff = (y / 2) * 80;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(window_opened && (wy_low <= y) && (wy_high > y)) {
					for(x = 0; x < 80; x++) {
						if((x >= wx_begin) && (x < wx_end)) {
							GETVRAM_8_200L(yoff, p, rgbmask, true);
						} else {
							GETVRAM_8_200L(yoff, p, rgbmask, false);
						}
						p += 8;
						yoff++;
					}
			} else
# endif
			{
				for(x = 0; x < 10; x++) {
					GETVRAM_8_200L(yoff + 0, p, rgbmask, false);
					p += 8;
					
					GETVRAM_8_200L(yoff + 1, p, rgbmask, false);
					p += 8;
					
					GETVRAM_8_200L(yoff + 2, p, rgbmask, false);
					p += 8;
					
					GETVRAM_8_200L(yoff + 3, p, rgbmask, false);
					p += 8;
					
					GETVRAM_8_200L(yoff + 4, p, rgbmask, false);
					p += 8;
					
					GETVRAM_8_200L(yoff + 5, p, rgbmask, false);
					p += 8;
						
					GETVRAM_8_200L(yoff + 6, p, rgbmask, false);
					p += 8;
			  
					GETVRAM_8_200L(yoff + 7, p, rgbmask, false);
					p += 8;
					yoff += 8;
				}
			}
			if(config.scan_line == 0) {
				memcpy((void *)emu->screen_buffer(y + 1), pp, 640 * sizeof(scrntype));
			} else {
				memset((void *)emu->screen_buffer(y + 1), 0x00, 640 * sizeof(scrntype));
			}
		}
# if defined(_FM77AV_VARIANTS)
		vram_wrote_shadow = false;
# endif		
		return;
	}
# if defined(_FM77AV_VARIANTS)
	if(display_mode == DISPLAY_MODE_4096) {
		uint32 mask = 0;
		yoff = 0;
		rgbmask = multimode_dispmask;
		if((rgbmask & 0x01) == 0) mask = 0x00f;
		if((rgbmask & 0x02) == 0) mask = mask | 0x0f0;
		if((rgbmask & 0x04) == 0) mask = mask | 0xf00;
		for(y = 0; y < 400; y += 2) {
# if defined(_FM77AV_VARIANTS)
			if(!vram_wrote_shadow && !vram_draw_table[y >> 1]) continue;
			vram_draw_table[y >> 1] = false;
# endif			
			p = emu->screen_buffer(y);
			pp = p;
			yoff = y * (40 / 2);
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(window_opened && (wy_low <= y) && (wy_high > y)) {
					for(x = 0; x < 40; x++) {
						if((x >= wx_begin) && (x < wx_end)) {
							GETVRAM_4096(yoff, p, mask, true);
						} else {
							GETVRAM_4096(yoff, p, mask, false);
						}
						p += 16;
						yoff++;
					}
			} else
#  endif
			{
				for(x = 0; x < 5; x++) {
					GETVRAM_4096(yoff + 0, p, mask);
					p += 16;
					
					GETVRAM_4096(yoff + 1, p, mask);
					p += 16;
					
					GETVRAM_4096(yoff + 2, p, mask);
					p += 16;
					
					GETVRAM_4096(yoff + 3, p, mask);
					p += 16;
					
					GETVRAM_4096(yoff + 4, p, mask);
					p += 16;
			  
					GETVRAM_4096(yoff + 5, p, mask);
					p += 16;
					
					GETVRAM_4096(yoff + 6, p, mask);
					p += 16;
					
					GETVRAM_4096(yoff + 7, p, mask);
					p += 16;
					yoff += 8;
				}
			}
			if(config.scan_line == 0) {
				memcpy((void *)emu->screen_buffer(y + 1), pp, 640 * sizeof(scrntype));
			} else {
				memset((void *)emu->screen_buffer(y + 1), 0x00, 640 * sizeof(scrntype));
			}
		}
# if defined(_FM77AV_VARIANTS)
		vram_wrote_shadow = false;
# endif		
		return;
	}
#  if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	else if(display_mode == DISPLAY_MODE_8_400L) {
		yoff = 0;
		rgbmask = ~multimode_dispmask;
		for(y = 0; y < 400; y++) {
# if defined(_FM77AV_VARIANTS)
			if(!vram_wrote_shadow && !vram_draw_table[y]) continue;
			vram_draw_table[y] = false;	
# endif			
			p = emu->screen_buffer(y);
			pp = p;
			yoff = y  * 80;
#    if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(window_opened && (wy_low <= y) && (wy_high  > y)) {
				for(x = 0; x < 80; x++) {
					if((x >= wx_begin) && (x < wx_end)) {
						GETVRAM_8_400L(yoff, p, rgbmask, true);
					} else {
						GETVRAM_8_400L(yoff, p, rgbmask, false);
					}
					p += 8;
					yoff++;
				}
			} else
#    endif
			for(x = 0; x < 10; x++) {
			  GETVRAM_8_400L(yoff + 0, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_400L(yoff + 1, p, rgbmask);
			  p += 8;

  			  GETVRAM_8_400L(yoff + 2, p, rgbmask);
			  p += 8;

			  GETVRAM_8_400L(yoff + 3, p, rgbmask);
			  p += 8;

			  GETVRAM_8_400L(yoff + 4, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_400L(yoff + 5, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_400L(yoff + 6, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_400L(yoff + 7, p, rgbmask);
			  p += 8;
			  yoff += 8;
			}
		}
# if defined(_FM77AV_VARIANTS)
		vram_wrote_shadow = false;
# endif		
		return;
	} else if(display_mode == DISPLAY_MODE_256k) {
		rgbmask = ~multimode_dispmask;
		for(y = 0; y < 400; y += 2) {
# if defined(_FM77AV_VARIANTS)
			if(!vram_wrote_shadow && !vram_draw_table[y >> 1]) continue;
			vram_draw_table[y >> 1] = false;	
# endif			
			p = emu->screen_buffer(y);
			pp = p;
			yoff = y * (40 / 2);
			{
				for(x = 0; x < 5; x++) {
					GETVRAM_256k(yoff + 0, p, rgbmask);
					p += 16;
					
					GETVRAM_256k(yoff + 1, p, rgbmask);
					p += 16;
					
					GETVRAM_256k(yoff + 2, p, rgbmask);
					p += 16;
					
					GETVRAM_256k(yoff + 3, p, rgbmask);
					p += 16;
					
					GETVRAM_256k(yoff + 4, p, rgbmask);
					p += 16;
			  
					GETVRAM_256k(yoff + 5, p, rgbmask);
					p += 16;
					
					GETVRAM_256k(yoff + 6, p, rgbmask);
					p += 16;
					
					GETVRAM_256k(yoff + 7, p, rgbmask);
					p += 16;
					yoff += 8;
				}
			}
			if(config.scan_line == 0) {
				memcpy((void *)emu->screen_buffer(y + 1), pp, 640 * sizeof(scrntype));
			} else {
				memset((void *)emu->screen_buffer(y + 1), 0x00, 640 * sizeof(scrntype));
			}
		}
# if defined(_FM77AV_VARIANTS)
		vram_wrote_shadow = false;
# endif		
		return;
	}
#  endif // _FM77AV40
# endif //_FM77AV_VARIANTS

}

