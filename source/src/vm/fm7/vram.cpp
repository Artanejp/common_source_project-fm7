/*
 * Common source code project -> FM-7 -> Display -> Vram access
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Sep 27, 2015 : Split from display.cpp .
 */

#include "vm.h"
#include "emu.h"
#include "fm7_display.h"
#if defined(_OPENMP)
#include <omp.h>
#endif

uint8_t DISPLAY::read_vram_l4_400l(uint32_t addr, uint32_t offset)
{
#if defined(_FM77L4)
	if(addr < 0x8000) {
		if(workram) {
			uint32_t raddr = addr & 0x3fff;
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

void DISPLAY::write_vram_l4_400l(uint32_t addr, uint32_t offset, uint32_t data)
{
#if defined(_FM77L4)
	if(addr < 0x8000) {
		if(workram) {
			uint32_t raddr = addr & 0x3fff;
			if((multimode_accessmask & 0x04) == 0) {
				gvram[0x8000 + (raddr + offset) & 0x7fff] = (uint8_t)data;
			}
			return;
		}
		pagemod = addr & 0x4000;
		gvram[((addr + offset) & mask) | pagemod] = (uint8_t)data;
	} else if(addr < 0x9800) {
		textvram[addr & 0x0fff] = (uint8_t)data;
	} else { // $9800-$bfff
		//return subrom_l4[addr - 0x9800];
	}
	return;
#endif	
}

void DISPLAY::GETVRAM_8_200L(int yoff, scrntype_t *p,
							 scrntype_t *px,
							 bool window_inv = false,
							 bool scan_line = false)
{
	uint8_t b, r, g;
	uint32_t yoff_d;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = vram_display_block;
#endif
	if(p == NULL) return;
	yoff_d = 0;
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
	if(display_page_bak == 1) yoff_d += 0xc000;
#endif
	if(!multimode_dispflags[0]) b = gvram_shadow[yoff_d + 0x00000];
	if(!multimode_dispflags[1]) r = gvram_shadow[yoff_d + 0x04000];
	if(!multimode_dispflags[2]) g = gvram_shadow[yoff_d + 0x08000];

	uint16_t *pg = &(bit_trans_table_0[g][0]);
	uint16_t *pr = &(bit_trans_table_1[r][0]);
	uint16_t *pb = &(bit_trans_table_2[b][0]);
	uint16_t tmp_d[8];

	scrntype_t tmp_dd[8];
	for(int i = 0; i < 8; i++) {
		tmp_d[i] = pg[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_d[i] = tmp_d[i] | pr[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_d[i] = tmp_d[i] | pb[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_d[i] = tmp_d[i] >> 5;
	}
	for(int i = 0; i < 8; i++) {
		tmp_dd[i] = dpalette_pixel[tmp_d[i]];
	}
	for(int i = 0; i < 8; i++) {
		p[i] = tmp_dd[i];
	}
#if defined(FIXED_FRAMEBUFFER_SIZE)
	if(scan_line) {
#if 0
		static const scrntype_t dblack[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		for(int i = 0; i < 8; i++) {
			px[i] = dblack[i];
		}
#else /* Fancy scanline */
		static const scrntype_t dblack[8] = { RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32),
											  RGB_COLOR(32, 32, 32)};

		for(int i = 0; i < 8; i++) {
#if defined(_RGB888) || defined(_RGBA888)
			tmp_dd[i] = tmp_dd[i] >> 3;
#elif defined(_RGB555)
			tmp_dd[i] = tmp_dd[i] >> 2;
#elif defined(_RGB565)
			tmp_dd[i] = tmp_dd[i] >> 2;
#endif
		}
		for(int i = 0; i < 8; i++) {
			tmp_dd[i] = tmp_dd[i] & RGBA_COLOR(31, 31, 31, 255);
		}
		for(int i = 0; i < 8; i++) {
			px[i] = tmp_dd[i];
		}
#endif
	} else {
		for(int i = 0; i < 8; i++) {
			px[i] = tmp_dd[i];
		}
	}
#endif	
}

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
void DISPLAY::GETVRAM_8_400L(int yoff, scrntype_t *p,
							 bool window_inv = false)
{
	uint8_t b, r, g;
	uint32_t dot;
	uint32_t yoff_d;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = vram_display_block;
# endif
	if(p == NULL) return;
	yoff_d = yoff;
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
	if(!multimode_dispflags[0]) b = gvram_shadow[yoff_d + 0x00000];
	if(!multimode_dispflags[1]) r = gvram_shadow[yoff_d + 0x08000];
	if(!multimode_dispflags[2]) g = gvram_shadow[yoff_d + 0x10000];

	uint16_t *pg = &(bit_trans_table_0[g][0]);
	uint16_t *pr = &(bit_trans_table_1[r][0]);
	uint16_t *pb = &(bit_trans_table_2[b][0]);
	uint16_t tmp_d[8];
	scrntype_t tmp_dd[8];
	
	for(int i = 0; i < 8; i++) {
		tmp_d[i] = pg[i] | pr[i] | pb[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_d[i] = tmp_d[i] >> 5;
	}
	for(int i = 0; i < 8; i++) {
		tmp_dd[i] = dpalette_pixel[tmp_d[i]];
	}
	for(int i = 0; i < 8; i++) {
		p[i] = tmp_dd[i];
	}
}

void DISPLAY::GETVRAM_256k(int yoff, scrntype_t *p, scrntype_t *px, bool scan_line = false)
{
	uint32_t b3, r3, g3;
	uint32_t b4, r4, g4;
	uint32_t btmp, rtmp, gtmp;
	
	scrntype_t b, r, g;
	scrntype_t pixel;
	uint32_t _bit;
	int _shift;
	int cp;
	uint32_t yoff_d1;
	uint32_t yoff_d2;
	if(p == NULL) return;
	
	r3 = g3 = b3 = 0;
	r4 = g4 = b4 = 0;
	r = g = b = 0;
	
	yoff_d1 = yoff;
	yoff_d2 = yoff;

	uint8_t  bb[8], rr[8], gg[8];
	uint16_t *p0, *p1, *p2, *p3, *p4, *p5;
	uint32_t _btmp[8], _rtmp[8], _gtmp[8];
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	scrntype_t tmp_dd[8];
#else
	scrntype_t tmp_dd[16];
#endif
//	if(mask & 0x01) {
	if(!multimode_dispflags[0]) {
		// B
		bb[0] = gvram_shadow[yoff_d1];
		bb[1] = gvram_shadow[yoff_d1 + 0x02000];
		
		bb[2] = gvram_shadow[yoff_d2 + 0x0c000];
		bb[3] = gvram_shadow[yoff_d2 + 0x0e000];
	
		bb[4] = gvram_shadow[yoff_d1 + 0x18000];
		bb[5] = gvram_shadow[yoff_d1 + 0x1a000];
		
		p0 = &(bit_trans_table_0[bb[0]][0]);
		p1 = &(bit_trans_table_1[bb[1]][0]);
		p2 = &(bit_trans_table_2[bb[2]][0]);
		p3 = &(bit_trans_table_3[bb[3]][0]);
		p4 = &(bit_trans_table_4[bb[4]][0]);
		p5 = &(bit_trans_table_5[bb[5]][0]);
		for(int i = 0; i < 8; i++) {
			_btmp[i] = p0[i];
		}
		for(int i = 0; i < 8; i++) {
			_btmp[i] = _btmp[i] | p1[i];
		}
		for(int i = 0; i < 8; i++) {
			_btmp[i] = _btmp[i] | p2[i];
		}
		for(int i = 0; i < 8; i++) {
			_btmp[i] = _btmp[i] | p3[i];
		}
		for(int i = 0; i < 8; i++) {
			_btmp[i] = _btmp[i] | p4[i];
		}
		for(int i = 0; i < 8; i++) {
			_btmp[i] = _btmp[i] | p5[i];
		}
	} else {
		for(int i = 0; i < 8; i++) {
			_btmp[i] = 0;
		}
	}
	if(!multimode_dispflags[1]) {
		//if(mask & 0x02) {
		// R
		rr[0] = gvram_shadow[yoff_d1 + 0x04000];
		rr[1] = gvram_shadow[yoff_d1 + 0x06000];
		
		rr[2] = gvram_shadow[yoff_d2 + 0x10000];
		rr[3] = gvram_shadow[yoff_d2 + 0x12000];
	
		rr[4] = gvram_shadow[yoff_d1 + 0x1c000];
		rr[5] = gvram_shadow[yoff_d1 + 0x1e000];
		
		p0 = &(bit_trans_table_0[rr[0]][0]);
		p1 = &(bit_trans_table_1[rr[1]][0]);
		p2 = &(bit_trans_table_2[rr[2]][0]);
		p3 = &(bit_trans_table_3[rr[3]][0]);
		p4 = &(bit_trans_table_4[rr[4]][0]);
		p5 = &(bit_trans_table_5[rr[5]][0]);
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = p0[i];
		}
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = _rtmp[i] | p1[i];
		}
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = _rtmp[i] | p2[i];
		}
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = _rtmp[i] | p3[i];
		}
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = _rtmp[i] | p4[i];
		}
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = _rtmp[i] | p5[i];
		}
	} else {
		for(int i = 0; i < 8; i++) {
			_rtmp[i] = 0;
		}
	}
	if(!multimode_dispflags[2]) {
		//if(mask & 0x04) {
		// G
		gg[0] = gvram_shadow[yoff_d1 + 0x08000];
		gg[1] = gvram_shadow[yoff_d1 + 0x0a000];
		
		gg[2] = gvram_shadow[yoff_d2 + 0x14000];
		gg[3] = gvram_shadow[yoff_d2 + 0x16000];
	
		gg[4] = gvram_shadow[yoff_d1 + 0x20000];
		gg[5] = gvram_shadow[yoff_d1 + 0x22000];
		
		p0 = &(bit_trans_table_0[gg[0]][0]);
		p1 = &(bit_trans_table_1[gg[1]][0]);
		p2 = &(bit_trans_table_2[gg[2]][0]);
		p3 = &(bit_trans_table_3[gg[3]][0]);
		p4 = &(bit_trans_table_4[gg[4]][0]);
		p5 = &(bit_trans_table_5[gg[5]][0]);
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = p0[i];
		}
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = _gtmp[i] | p1[i];
		}
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = _gtmp[i] | p2[i];
		}
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = _gtmp[i] | p3[i];
		}
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = _gtmp[i] | p4[i];
		}
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = _gtmp[i] | p5[i];
		}
	} else {
		for(int i = 0; i < 8; i++) {
			_gtmp[i] = 0;
		}
	}
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	for(int i = 0; i < 8; i++) {
		tmp_dd[i] = RGB_COLOR(_rtmp[i], _gtmp[i], _btmp[i]);
	}
	for(int i = 0; i < 8; i++) {
		p[i] = tmp_dd[i];
	}
#else
	for(int i = 0; i < 8; i++) {
		tmp_dd[i * 2] = tmp_dd[i * 2 + 1] = RGB_COLOR(_rtmp[i], _gtmp[i], _btmp[i]);
	}
	for(int i = 0; i < 16; i++) {
		p[i] = tmp_dd[i];
	}
	if(scan_line) {
#if 0
		static const scrntype_t dblack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		for(int i = 0; i < 16; i++) {
			px[i] = dblack[i];
		}
#else /* Fancy scanline */
		static const scrntype_t dblack[16] = { RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32)};

		for(int i = 0; i < 16; i++) {
#if defined(_RGB888) || defined(_RGBA888)
			tmp_dd[i] = tmp_dd[i] >> 3;
#elif defined(_RGB555)
			tmp_dd[i] = tmp_dd[i] >> 2;
#elif defined(_RGB565)
			tmp_dd[i] = tmp_dd[i] >> 2;
#endif
		}
		for(int i = 0; i < 16; i++) {
			tmp_dd[i] = tmp_dd[i] & RGBA_COLOR(31, 31, 31, 256);
		}
		for(int i = 0; i < 16; i++) {
			px[i] = tmp_dd[i];
		}
#endif		
	} else {
		for(int i = 0; i < 16; i++) {
			px[i] = tmp_dd[i];
		}
	}
#endif	
}
#endif

#if defined(_FM77AV_VARIANTS)
void DISPLAY::GETVRAM_4096(int yoff, scrntype_t *p, scrntype_t *px,
						   uint32_t mask,
						   bool window_inv = false,
						   bool scan_line = false)
{
	uint32_t b3, r3, g3;
	uint8_t  bb[4], rr[4], gg[4];
	uint16_t pixels[8];
	const uint16_t __masks[8] = {(uint16_t)mask, (uint16_t)mask, (uint16_t)mask, (uint16_t)mask, (uint16_t)mask, (uint16_t)mask, (uint16_t)mask, (uint16_t)mask};
	scrntype_t b, r, g;
	uint32_t idx;;
	scrntype_t pixel;
	uint32_t yoff_d1;
	uint32_t yoff_d2;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = vram_display_block;
# endif
	if(p == NULL) return;
	
	yoff_d1 = yoff;
	yoff_d2 = yoff;
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
	bb[0] = gvram_shadow[yoff_d1];
	bb[1] = gvram_shadow[yoff_d1 + 0x02000];
	rr[0] = gvram_shadow[yoff_d1 + 0x04000];
	rr[1] = gvram_shadow[yoff_d1 + 0x06000];
		
	gg[0] = gvram_shadow[yoff_d1 + 0x08000];
	gg[1] = gvram_shadow[yoff_d1 + 0x0a000];
		
	bb[2] = gvram_shadow[yoff_d2 + 0x0c000];
	bb[3] = gvram_shadow[yoff_d2 + 0x0e000];
		
	rr[2] = gvram_shadow[yoff_d2 + 0x10000];
	rr[3] = gvram_shadow[yoff_d2 + 0x12000];
	gg[2] = gvram_shadow[yoff_d2 + 0x14000];
	gg[3] = gvram_shadow[yoff_d2 + 0x16000];

	uint16_t tmp_g[8], tmp_r[8], tmp_b[8];
	uint16_t *p0, *p1, *p2, *p3;
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	scrntype_t tmp_dd[8];
#else
	scrntype_t tmp_dd[16];
#endif
	// G
	p0 = &(bit_trans_table_0[gg[0]][0]);
	p1 = &(bit_trans_table_1[gg[1]][0]);
	p2 = &(bit_trans_table_2[gg[2]][0]);
	p3 = &(bit_trans_table_3[gg[3]][0]);
	for(int i = 0; i < 8; i++) {
		tmp_g[i]  = p0[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_g[i]  = tmp_g[i] | p1[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_g[i]  = tmp_g[i] | p2[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_g[i]  = tmp_g[i] | p3[i];
	}
	// R
	p0 = &(bit_trans_table_0[rr[0]][0]);
	p1 = &(bit_trans_table_1[rr[1]][0]);
	p2 = &(bit_trans_table_2[rr[2]][0]);
	p3 = &(bit_trans_table_3[rr[3]][0]);
	for(int i = 0; i < 8; i++) {
		tmp_r[i]  = p0[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_r[i]  = tmp_r[i] | p1[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_r[i]  = tmp_r[i] | p2[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_r[i]  = tmp_r[i] | p3[i];
	}
	// B
	p0 = &(bit_trans_table_0[bb[0]][0]);
	p1 = &(bit_trans_table_1[bb[1]][0]);
	p2 = &(bit_trans_table_2[bb[2]][0]);
	p3 = &(bit_trans_table_3[bb[3]][0]);
	for(int i = 0; i < 8; i++) {
		tmp_b[i]  = p0[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_b[i]  = tmp_b[i] | p1[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_b[i]  = tmp_b[i] | p2[i];
	}
	for(int i = 0; i < 8; i++) {
		tmp_b[i]  = tmp_b[i] | p3[i];
	}
	for(int i = 0; i < 8; i++) {
		pixels[i] = tmp_b[i] >> 4;
	}
	for(int i = 0; i < 8; i++) {
		pixels[i] = pixels[i] | tmp_r[i];
	}
	for(int i = 0; i < 8; i++) {
		pixels[i] = pixels[i] | tmp_g[i] << 4;
	}

	for(int i = 0; i < 8; i++) {
		pixels[i] = pixels[i] & __masks[i];
	}
	//for(int i = 0; i < 8; i++) {
	//	pixels[i] = pixels[i] & mask;
	//}
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	for(int i = 0; i < 8; i++) {
		tmp_dd[i] = analog_palette_pixel[pixels[i]];
	}

	for(int i = 0; i < 8; i++) {
		p[i] = tmp_dd[i];
	}
#else
	for(int i = 0; i < 8; i++) {
		tmp_dd[i * 2] = tmp_dd[i * 2 + 1] = analog_palette_pixel[pixels[i]];;
	}
	for(int i = 0; i < 16; i++) {
		p[i] = tmp_dd[i];
	}
	if(scan_line) {
#if 0
		static const scrntype_t dblack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		for(int i = 0; i < 16; i++) {
			px[i] = dblack[i];
		}
#else /* Fancy scanline */
		static const scrntype_t dblack[16] = { RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32),
											   RGB_COLOR(32, 32, 32)};

		for(int i = 0; i < 16; i++) {
#if defined(_RGB888) || defined(_RGBA888)
			tmp_dd[i] = tmp_dd[i] >> 3;
#elif defined(_RGB555)
			tmp_dd[i] = tmp_dd[i] >> 2;
#elif defined(_RGB565)
			tmp_dd[i] = tmp_dd[i] >> 2;
#endif
		}
		for(int i = 0; i < 16; i++) {
			tmp_dd[i] = tmp_dd[i] & RGBA_COLOR(31, 31, 31, 255);
		}
		for(int i = 0; i < 16; i++) {
			px[i] = tmp_dd[i];
		}
#endif		
	} else {
		for(int i = 0; i < 16; i++) {
			px[i] = tmp_dd[i];
		}
	}
#endif

}
#endif


void DISPLAY::draw_screen()
{
//#if !defined(_FM77AV_VARIANTS)
	this->draw_screen2();
//#endif	
}

extern config_t config;
void DISPLAY::draw_screen2()
{
	int y;
	int x;
	scrntype_t *p, *pp, *p2;
	int yoff;
	int yy;
	int k;
	//uint32_t rgbmask;
	uint32_t yoff_d1, yoff_d2;
	uint16_t wx_begin, wx_end, wy_low, wy_high;
	bool scan_line = config.scan_line;

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
//	frame_skip_count_draw++;
#if defined(_FM77AV_VARIANTS)
	yoff_d2 = 0;
	yoff_d1 = 0;
#else
	//if(!(vram_wrote_shadow)) return;
	yoff_d1 = yoff_d2 = offset_point;
#endif
	// Set blank
	if(!crt_flag) {
		if(crt_flag_bak) {
			scrntype_t *ppp;
			if(display_mode == DISPLAY_MODE_8_200L) {
#if 1
# if !defined(FIXED_FRAMEBUFFER_SIZE)
				emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
# endif
				emu->set_vm_screen_lines(200);
#endif
#if !defined(FIXED_FRAMEBUFFER_SIZE)
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(ppp, yy)
#endif
				for(y = 0; y < 200; y += 8) {
					for(yy = 0; yy < 8; yy++) {
						vram_draw_table[y + yy] = false;
						ppp = emu->get_screen_buffer(y + yy);
						if(ppp != NULL) memset(ppp, 0x00, 640 * sizeof(scrntype_t));
					}
				}
#else
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(ppp, yy)
#endif
				for(y = 0; y < 400; y += 8) {
					for(yy = 0; yy < 8; yy++) {
						vram_draw_table[y + yy] = false;
						ppp = emu->get_screen_buffer(y + yy);
						if(ppp != NULL) memset(ppp, 0x00, 640 * sizeof(scrntype_t));
					}
				}
#endif
			} else if(display_mode == DISPLAY_MODE_8_400L) {
#if 1
				emu->set_vm_screen_lines(400);
				emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
#endif
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(ppp, yy)
#endif
				for(y = 0; y < 400; y += 8) {
					for(yy = 0; yy < 8; yy++) {
						vram_draw_table[y + yy] = false;
						ppp = emu->get_screen_buffer(y + yy);
						if(ppp != NULL) memset(ppp, 0x00, 640 * sizeof(scrntype_t));
					}
				}
			} else { // 320x200
#if 1
# if !defined(FIXED_FRAMEBUFFER_SIZE)
				emu->set_vm_screen_size(320, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
# endif
				emu->set_vm_screen_lines(200);
#endif
#if !defined(FIXED_FRAMEBUFFER_SIZE)
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(ppp, yy)
#endif
				for(y = 0; y < 200; y += 8) {
					for(yy = 0; yy < 8; yy++) {
						vram_draw_table[y + yy] = false;
						ppp = emu->get_screen_buffer(y + yy);
						if(ppp != NULL) memset(ppp, 0x00, 320 * sizeof(scrntype_t));
					}
				}
#else
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(ppp, yy)
#endif
				for(y = 0; y < 400; y++) {
					for(yy = 0; yy < 8; yy++) {   
						vram_draw_table[y + yy] = false;
						ppp = emu->get_screen_buffer(y + yy);
						if(ppp != NULL) memset(ppp, 0x00, 640 * sizeof(scrntype_t));
					}
				}
#endif				
			}
		}
		crt_flag_bak = crt_flag;
		return;
	}
	crt_flag_bak = crt_flag;
	if(!vram_wrote_shadow && !palette_changed) return;
	vram_wrote_shadow = false;
	if(palette_changed) {
		for(y = 0; y < 400; y++) {
			vram_draw_table[y] = true;
		}
		palette_changed = false;
	}
	if(display_mode == DISPLAY_MODE_8_200L) {
		int ii;
#if 1
# if !defined(FIXED_FRAMEBUFFER_SIZE)
		emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
# endif
		emu->set_vm_screen_lines(200);
#endif
		yoff = 0;
		//rgbmask = ~multimode_dispmask;
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(p, p2, yoff, ii, x, yy)
#endif
		for(y = 0; y < 200; y += 8) {
			for(yy = 0; yy < 8; yy++) {
			
				if(!vram_draw_table[y + yy]) continue;
				vram_draw_table[y + yy] = false;
#if !defined(FIXED_FRAMEBUFFER_SIZE)
				p = emu->get_screen_buffer(y + yy);
				p2 = NULL;
#else
				p = emu->get_screen_buffer((y + yy) * 2);
				p2 = emu->get_screen_buffer((y + yy) * 2 + 1);
#endif
				if(p == NULL) continue;
				yoff = (y + yy) * 80;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				if(window_opened && (wy_low <= (y + yy)) && (wy_high > (y + yy))) {
					for(x = 0; x < 80; x++) {
						if((x >= wx_begin) && (x < wx_end)) {
							GETVRAM_8_200L(yoff, p, p2, true, scan_line);
						} else {
							GETVRAM_8_200L(yoff, p, p2, false, scan_line);
						}
#if defined(FIXED_FRAMEBUFFER_SIZE)
						p2 += 8;
#endif
						p += 8;
						yoff++;
					}
				} else
# endif
				{
					for(x = 0; x < 10; x++) {
						for(ii = 0; ii < 8; ii++) {
							GETVRAM_8_200L(yoff + ii, p, p2, false, scan_line);
#if defined(FIXED_FRAMEBUFFER_SIZE)
							p2 += 8;
#endif
							p += 8;
						}
						yoff += 8;
					}
				}
			}
		   
		}
		return;
	}
# if defined(_FM77AV_VARIANTS)
	if(display_mode == DISPLAY_MODE_4096) {
#if 1
# if !defined(FIXED_FRAMEBUFFER_SIZE)
		emu->set_vm_screen_size(320, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
# endif
		emu->set_vm_screen_lines(200);
#endif
		uint32_t mask = 0;
		int ii;
		yoff = 0;
		if(!multimode_dispflags[0]) mask = 0x00f;
		if(!multimode_dispflags[1]) mask = mask | 0x0f0;
		if(!multimode_dispflags[2]) mask = mask | 0xf00;
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(p, p2, yoff, ii, x, yy)
#endif
		for(y = 0; y < 200; y += 4) {
			for(yy = 0; yy < 4; yy++) {
				if(!vram_draw_table[y + yy]) continue;
				vram_draw_table[y + yy] = false;

#if !defined(FIXED_FRAMEBUFFER_SIZE)
				p = emu->get_screen_buffer(y + yy);
				p2 = NULL;
#else
				p = emu->get_screen_buffer((y + yy) * 2 );
				p2 = emu->get_screen_buffer((y + yy) * 2 + 1);
#endif
				if(p == NULL) continue;
				yoff = (y + yy) * 40;
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				if(window_opened && (wy_low <= (y + yy)) && (wy_high > (y + yy))) {
					for(x = 0; x < 40; x++) {
						if((x >= wx_begin) && (x < wx_end)) {
							GETVRAM_4096(yoff, p, p2, mask, true, scan_line);
						} else {
							GETVRAM_4096(yoff, p, p2, mask, false, scan_line);
						}
#if defined(FIXED_FRAMEBUFFER_SIZE)
						p2 += 16;
						p += 16;
#else
						p += 8;
#endif
						yoff++;
					}
				} else
#  endif
				{
					for(x = 0; x < 5; x++) {
						for(ii = 0; ii < 8; ii++) {
							GETVRAM_4096(yoff + ii, p, p2, mask, false, scan_line);
#if defined(FIXED_FRAMEBUFFER_SIZE)
							p2 += 16;
							p += 16;
#else
							p += 8;
#endif
						}
						yoff += 8;
					}
				}
			}
		   
		}
		return;
	}
#  if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	else if(display_mode == DISPLAY_MODE_8_400L) {
		int ii;
#if 1
		emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
		emu->set_vm_screen_lines(400);
#endif
		yoff = 0;
		//rgbmask = ~multimode_dispmask;
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(pp, p, yoff, x, ii, yy)
#endif
		for(y = 0; y < 400; y += 8) {
			for(yy = 0; yy < 8; yy++) {
				if(!vram_draw_table[y + yy]) continue;
				vram_draw_table[y + yy] = false;

				p = emu->get_screen_buffer(y + yy);
				if(p == NULL) continue;
				pp = p;
				yoff = (y + yy) * 80;
#    if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				if(window_opened && (wy_low <= (y + yy)) && (wy_high  > (y + yy))) {
					for(x = 0; x < 80; x++) {
						if((x >= wx_begin) && (x < wx_end)) {
							GETVRAM_8_400L(yoff, p, true);
						} else {
							GETVRAM_8_400L(yoff, p, false);
						}
						p += 8;
						yoff++;
					}
				} else
#    endif
					for(x = 0; x < 10; x++) {

						for(ii = 0; ii < 8; ii++) {
							GETVRAM_8_400L(yoff + ii, p);
							p += 8;
						}
						yoff += 8;
					}
			}
		}
		return;
	} else if(display_mode == DISPLAY_MODE_256k) {
		int ii;
#if 1
# if !defined(FIXED_FRAMEBUFFER_SIZE)
		emu->set_vm_screen_size(320, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
# endif
		emu->set_vm_screen_lines(200);
#endif
		//rgbmask = ~multimode_dispmask;
#if defined(_OPENMP)
#pragma omp parallel for
#endif
#if defined(_OPENMP)
#pragma omp parallel for shared(vram_draw_table), private(pp, p, yoff, x, ii, yy)
#endif
		for(y = 0; y < 200; y += 4) {
			for(yy = 0; yy < 4; yy++) {
				if(!vram_draw_table[y + yy]) continue;
				vram_draw_table[y + yy] = false;
#if !defined(FIXED_FRAMEBUFFER_SIZE)
				p = emu->get_screen_buffer(y + yy);
				p2 = NULL;
#else
				p = emu->get_screen_buffer((y + yy) * 2 );
				p2 = emu->get_screen_buffer((y + yy) * 2 + 1);
#endif
				if(p == NULL) continue;
				pp = p;
				yoff = (y + yy) * 40;
				{
					for(x = 0; x < 5; x++) {
						for(ii = 0; ii < 8; ii++) {
							GETVRAM_256k(yoff + ii, p, p2, scan_line);
#if !defined(FIXED_FRAMEBUFFER_SIZE)
							p += 8;
#else
							p += 16;
							p2 += 16;
#endif
						}
						yoff += 8;
					}
				}
			}
		}
		return;
	}
#  endif // _FM77AV40
# endif //_FM77AV_VARIANTS
}

bool DISPLAY::screen_update(void)
{
	if(crt_flag) {
		bool f = screen_update_flag;
		screen_update_flag = false;
		return f;
	} else {
		if(crt_flag_bak) return true;
	}
	return false;
}

void DISPLAY::reset_screen_update(void)
{
	screen_update_flag = false;
}
