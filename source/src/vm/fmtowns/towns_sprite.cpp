/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.22 -

	[ Sprite ]
*/

#include "../../common.h"
#include "./towns_vram.h"
#include "./towns_sprite.h"
#include "./towns_crtc.h"

#define EVENT_FALL_DOWN	1
#define EVENT_BUSY_OFF	2
#define EVENT_RENDER	3

namespace FMTOWNS {

void TOWNS_SPRITE::initialize(void)
{
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	disp_page1 = false;
	draw_page1 = false;
	disp_page0 = false;
	reg_spen = false;
	reg_addr = 0;
	memset(reg_data, 0x00, sizeof(reg_data));

	max_sprite_per_frame = 224;
	event_busy = -1;
	page_changed = true;
	
	register_frame_event(this);
	register_vline_event(this);
}

void TOWNS_SPRITE::reset()
{
	// Clear RAMs?
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	disp_page1 = false;
	draw_page1 = false;
	disp_page0 = false;
	reg_spen = false;
	reg_addr = 0;
	render_num = 1024;

	sprite_enabled = false;

	max_sprite_per_frame = 224;
	tvram_enabled = false;
	tvram_enabled_bak = false;

	sprite_busy = false;
	page_changed = true;

	memset(reg_data, 0x00, sizeof(reg_data)); // OK?

	if(event_busy > -1) {
		cancel_event(this, event_busy);
		event_busy = -1;
	}
	
//	ankcg_enabled = false;
}
	// Still don't use cache.
void TOWNS_SPRITE::render_sprite(int num, int x, int y, uint16_t attr, uint16_t color)
{
//	uint16_t lot = reg_index & 0x3ff;
//	if(lot == 0) lot = 1024;
//	if(num < 0) return;
//	if(num >= lot) return;
//	if(/*!(reg_spen) || */!(sprite_enabled)) return; 

	bool is_32768 = ((color & 0x8000) == 0); // CTEN
	// ToDo: SPYS
	if((color & 0x2000) != 0) return; // DISP
//	out_debug_log(_T("RENDER #%d"), render_num);
	uint32_t color_offset = ((uint32_t)((color & 0xfff) << 5)) & 0x1ffff; // COL11 - COL0

	int xoffset = 0;
	int yoffset = 0;
	if((attr & 0x8000) != 0) { // OFFS
		xoffset = reg_hoffset & 0x1ff;
		yoffset = reg_voffset & 0x1ff;
	}
	bool swap_v_h = false;
	if((attr & 0x4000) != 0) { // ROT2
		swap_v_h = true;
	}
	uint8_t rot = attr >> 12;
	bool is_halfy = ((attr & 0x0800) != 0);
	bool is_halfx = ((attr & 0x0400) != 0); // SUX
	// From MAME 0.209, mame/drivers/video/fmtowns.cpp
	uint32_t ram_offset =  ((uint32_t)(attr & 0x3ff) << 7) & 0x1ffff; // PAT9 - PAT0

	int xbegin, xend;
	int ybegin, yend;
	int xinc, yinc;
	bool is_mirror;
	switch(rot & 3) { // ROT1, ROT0
	case 0:
		// 0deg, not mirror
		xbegin = 0;
		xend   = 15;
		ybegin = 0;
		yend = 15;
		xinc = 1;
		yinc = 1;
		is_mirror = false;
		break;
	case 1:
		// 180deg, mirror
		xbegin = 0;
		xend   = 15;
		ybegin = 15;
		yend = 0;
		xinc = 1;
		yinc = -1;
		is_mirror = true;
		break;
	case 2:
		// 0deg, mirror
		xbegin = 15;
		xend   = 0;
		ybegin = 0;
		yend = 15;
		xinc = -1;
		yinc = 1;
		is_mirror = true;
		break;
	case 3:
		// 180deg, not mirror
		xbegin = 15;
		xend   = 0;
		ybegin = 15;
		yend = 0;
		xinc = -1;
		yinc = -1;
		is_mirror = false;
		break;
		/*
	case 4:
		// 270deg, mirror
		break;
	case 5:
		// 90deg, not mirror
		break;
	case 6:
		// 270deg, not mirror
		break;
	case 7:
		// 90deg, mirror
		break;
		*/
	}
	if(swap_v_h) is_mirror = !(is_mirror);
	
	__DECL_ALIGNED(32) uint16_t sbuf[16][16] = {0}; 
	__DECL_ALIGNED(16) union {
		pair16_t pw[16];
		uint8_t b[32];
	} lbuf;
	__DECL_ALIGNED(16) union {
		pair16_t pw[16];
		uint8_t  b[32];
	} mbuf;
	__DECL_ALIGNED(16) uint16_t pixel_h[8];
	__DECL_ALIGNED(16) uint16_t pixel_l[8];
	__DECL_ALIGNED(16) uint16_t color_table[16] = {0};
	if(!(swap_v_h)) {
		if(is_32768) {
			// get from ram.
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 5) + (xbegin << 1) + ram_offset;
				__DECL_ALIGNED(16) union {
					pair16_t pw[16];
					uint8_t b[32];
				} nnw;
				if(xinc > 0) {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 32; xx++) {
						nnw.b[xx] = pattern_ram[(addr + xx) & 0x1ffff];
					}
				} else {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 32; xx++) {
						nnw.b[xx] = pattern_ram[(addr - xx) & 0x1ffff];
					}
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					sbuf[yy][xx] = nnw.pw[xx].w;
				}
			}
		} else { // 16 colors
			__DECL_ALIGNED(16) union {
				pair16_t pw[16];
				uint8_t  b[32];
			} nnw;
__DECL_VECTORIZED_LOOP						
			for(int i = 0; i < 32; i++) {
				nnw.b[i] = pattern_ram[(color_offset + i) & 0x1ffff];
//				color_offset += 2;
			}
__DECL_VECTORIZED_LOOP						
			for(int i = 0; i < 16; i++) {
				color_table[i] = nnw.pw[i].w;
			}
			color_table[0] = 0x8000; // Clear color
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 3) + (xbegin >> 1) + ram_offset;
				uint8_t nnh, nnl;
				__DECL_ALIGNED(8) uint8_t nnb[8];
				if(xinc > 0) {
__DECL_VECTORIZED_LOOP
					for(int xx = 0; xx < 8; xx++ ) {
						nnb[xx] = pattern_ram[(addr + xx) & 0x1ffff];
					}
				} else {
__DECL_VECTORIZED_LOOP
					for(int xx = 0; xx < 8; xx++ ) {
						nnb[xx] = pattern_ram[(addr - xx) & 0x1ffff];
					}
				}
__DECL_VECTORIZED_LOOP
				for(int xx = 0; xx < 8; xx++ ) {
					nnh = nnb[xx] & 0x0f;
					pixel_h[xx] = color_table[nnh];
				}	
__DECL_VECTORIZED_LOOP
				for(int xx = 0; xx < 8; xx++ ) {
					nnh = nnb[xx] & 0x0f;
					pixel_h[xx] = color_table[nnh];
				}	
__DECL_VECTORIZED_LOOP
				for(int xx = 0; xx < 8; xx++ ) {
					nnh = nnb[xx] & 0x0f;
					pixel_h[xx] = color_table[nnh];
				}	
__DECL_VECTORIZED_LOOP
				for(int xx = 0; xx < 8; xx++ ) {
					nnl = nnb[xx] >> 4;
					pixel_l[xx] = color_table[nnl];
				}
				if(xinc < 0) {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx += 2 ) {
						sbuf[yy][xx    ] = pixel_l[xx >> 1];
						sbuf[yy][xx + 1] = pixel_h[xx >> 1];
					}
				} else {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx += 2 ) {
						sbuf[yy][xx    ] = pixel_h[xx >> 1];
						sbuf[yy][xx + 1] = pixel_l[xx >> 1];
					}
				}
			}
		}
	} else { // swap v and h
		if(is_32768) {
			// get from ram.
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 5) + (xbegin << 1) + ram_offset;
				__DECL_ALIGNED(16) union {
					pair16_t pw[16];
					uint8_t  b[32];
				} nnw;
				if(xinc > 0) {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 32; xx++) {
						nnw.b[xx] = pattern_ram[(addr + xx) & 0x1ffff];
					}
				} else {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 32; xx++) {
						nnw.b[xx] = pattern_ram[(addr - xx) & 0x1ffff];
					}
				}
				if(yinc > 0) {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx++) {
						sbuf[xx][yy] = nnw.pw[xx].w;
					}
				} else {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx++) {
						sbuf[15 - xx][yy] = nnw.pw[xx].w;
					}
				}
			}
		} else { // 16 colors
			pair16_t nnp;
__DECL_VECTORIZED_LOOP						
			for(int i = 0; i < 16; i++) {
				nnp.b.l = pattern_ram[(color_offset + 0) & 0x1ffff];
				nnp.b.h = pattern_ram[(color_offset + 1) & 0x1ffff];
				color_offset += 2;
				color_table[i] = nnp.w;
			}
			color_table[0] = 0x8000; // Clear color
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 3) + (xbegin >> 1) + ram_offset;
				uint8_t nnh, nnl;
				uint8_t nnb;
				if(xinc > 0) {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 8; xx++ ) {
						nnb = pattern_ram[(addr + xx) & 0x1ffff];
						nnh = nnb & 0x0f;
						nnl = nnb >> 4;
						pixel_h[xx] = color_table[nnh];
						pixel_l[xx] = color_table[nnl];
					}
				} else {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 8; xx++ ) {
						nnb = pattern_ram[(addr - xx) & 0x1ffff];
						nnh = nnb & 0x0f;
						nnl = nnb >> 4;
						pixel_h[xx] = color_table[nnh];
						pixel_l[xx] = color_table[nnl];
					}
				}
				if(yinc < 0) {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx += 2 ) {
						sbuf[xx    ][yy] = pixel_l[xx >> 1];
						sbuf[xx + 1][yy] = pixel_h[xx >> 1];
					}
				} else {
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx += 2 ) {
						sbuf[xx    ][yy] = pixel_h[xx >> 1];
						sbuf[xx + 1][yy] = pixel_l[xx >> 1];
					}
				}
			}
		}
	}
	uint32_t noffset = (draw_page1) ? 0x40000 : 0x60000;
	uint32_t vpaddr = (((x - xoffset) % 256 + ((y - yoffset) * 256)) << 1) & 0x1ffff;
	if(!(is_halfx) && !(is_halfy)) { // not halfed
		int __xstart = 0;		
		int __xend = 16;
		int __ystart = 0;
		int __yend = 16;
		for(int yy = 0; yy < 16;  yy++) {
			if(d_vram != NULL) {
				__DECL_ALIGNED(32) uint8_t source[32];
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 16);
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf.pw[xx].w = 0;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf.pw[xx].w = sbuf[yy][xx];
				}
				__DECL_ALIGNED(16) uint16_t mbuf2[16];
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf2[xx] = mbuf.pw[xx].w;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf2[xx] = (lbuf.pw[xx].w  >> 15);  // All values are either 1 or 0.
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf2[xx] = mbuf2[xx] * 0xffff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf.pw[xx].w = mbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
//					lbuf.pw[xx].w &= 0x7fff;
					lbuf.pw[xx].w &= (~(mbuf2[xx]) & 0x7fff); // OK?
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					source[xx] &= mbuf.b[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					source[xx] |= lbuf.b[xx];
				}
				d_vram->set_buffer_to_vram(vpaddr + noffset, source, 16);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	} else if((is_halfx) && !(is_halfy)) { // halfx only
		int __xstart = 0;		
		int __xend = 8;
		int __ystart = 0;
		int __yend = 16;
		for(int yy = 0; yy < 16;  yy++) {
			if(d_vram != NULL) {
				__DECL_ALIGNED(16) uint8_t source[16];
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 8);
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf.pw[xx].w = 0x0;
					mbuf.pw[xx].w = 0;
				}
				__DECL_ALIGNED(16) uint16_t sbuf2[16];
				__DECL_ALIGNED(16) uint16_t sbuf3[16];
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					sbuf2[xx] = sbuf[yy][xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					sbuf3[xx] = sbuf2[xx] & 0x8000;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					sbuf2[xx] = sbuf2[xx] & 0x7fff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf.pw[xx >> 1].w += sbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf.pw[xx >> 1].w |= sbuf3[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w = (lbuf.pw[xx].w >> 1);
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w &= 0x7fff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w |= mbuf.pw[xx].w;
				}
				__DECL_ALIGNED(16) uint16_t mbuf2[8];
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf2[xx] = mbuf.pw[xx].w;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf2[xx] = (lbuf.pw[xx].w  >> 15); // All values are either 1 or 0.
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf2[xx] = mbuf2[xx] * 0xffff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf.pw[xx].w = mbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w &= (~(mbuf2[xx]));
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w &= 0x7fff;
//					lbuf.pw[xx].w &= ~(mbuf.pw[xx].w);
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					source[xx] &= mbuf.b[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					source[xx] |= lbuf.b[xx];
				}
				d_vram->set_buffer_to_vram(vpaddr + noffset, source, 8);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	} else if(is_halfy) { // halfy only
		int __xstart = 0;		
		int __xend = 16;
		int __ystart = 0;
		int __yend = 8;
		for(int yy = (__ystart << 1); yy < (__yend << 1);  yy += 2) {
			if(d_vram != NULL) {
				__DECL_ALIGNED(32) uint8_t source[32];
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 16);
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf.pw[xx].w = 0x0;
					mbuf.pw[xx].w = 0;
				}
				__DECL_ALIGNED(32) uint16_t sbuf2[32];
				__DECL_ALIGNED(32) uint16_t sbuf3[32];
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					sbuf2[xx] = sbuf[yy][xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 16, xx2 = 0; xx < 32; xx++, xx2++) {
					sbuf2[xx] = sbuf[yy + 1][xx2];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					sbuf3[xx] = sbuf2[xx] & 0x8000;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					sbuf2[xx] = sbuf2[xx] & 0x7fff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					lbuf.pw[xx >> 1].w += sbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					mbuf.pw[xx >> 1].w |= sbuf3[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf.pw[xx].w = ((lbuf.pw[xx].w >> 1) & 0x7fff) | mbuf.pw[xx].w;
				}
				__DECL_ALIGNED(16) uint16_t mbuf2[16];
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf2[xx] = mbuf.pw[xx].w;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf2[xx] = (lbuf.pw[xx].w  >> 15);  // All values are either 1 or 0.
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf2[xx] = mbuf2[xx] * 0xffff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					mbuf.pw[xx].w = mbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
//					lbuf.pw[xx].w &= (~(mbuf.pw[xx].w) & 0x7fff);
					lbuf.pw[xx].w &= (~mbuf2[xx] & 0x7fff);
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					source[xx] &= mbuf.b[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					source[xx] |= lbuf.b[xx];
				}
				d_vram->set_buffer_to_vram(vpaddr + noffset, source, 16);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	} else { //halfx &&halfy
		int __xstart = 0;		
		int __xend = 8;
		int __ystart = 0;
		int __yend = 8;
		for(int yy = (__ystart << 1); yy < (__yend << 1);  yy += 2) {
			if(d_vram != NULL) {
				//d_vram->write_sprite_data(x, y + (yy >>1), xoffset, yoffset, lbuf, 8);
				__DECL_ALIGNED(16) uint8_t source[16];
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 8);
				__DECL_ALIGNED(32) uint16_t sbuf2[32];
				__DECL_ALIGNED(32) uint16_t sbuf3[32];
				__DECL_ALIGNED(16) uint16_t lbuf4[16];
				__DECL_ALIGNED(16) uint16_t mbuf5[16];

__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf4[xx] = 0x0000;
					mbuf5[xx] = 0;
					
				}
				// Phase.1 Get RAW DATA
				// Get Column 0
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					sbuf2[xx] = sbuf[yy][xx];
				}
				// Get Column 1
__DECL_VECTORIZED_LOOP						
				for(int xx = 16, xx2 = 0; xx < 32; xx++, xx2++) {
					sbuf2[xx] = sbuf[yy + 1][xx2];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					sbuf3[xx] = sbuf2[xx] & 0x8000;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					sbuf2[xx] = sbuf2[xx] & 0x7fff;
				}
				// Phase.2 Shrink X
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					lbuf4[xx >> 1] += sbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf4[xx] >>= 1;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf4[xx] &= 0x7fff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 32; xx++) {
					mbuf5[xx >> 1] |= sbuf3[xx];
				}

				// Phase.3 Shrink Y
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w = lbuf4[xx] + lbuf4[xx + 8];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w >>= 1;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w &= 0x7fff;
				}

__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf.pw[xx].w = mbuf5[xx] | mbuf5[xx + 8];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w |= mbuf.pw[xx].w;
				}
				__DECL_ALIGNED(16) uint16_t mbuf2[8];
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf2[xx] = mbuf.pw[xx].w;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf2[xx] = (lbuf.pw[xx].w  >> 15);  // All values are either 1 or 0.
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf2[xx] = mbuf2[xx] * 0xffff;
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					mbuf.pw[xx].w = mbuf2[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf.pw[xx].w &= (~mbuf2[xx] & 0x7fff);
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					source[xx] &= mbuf.b[xx];
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					source[xx] |= lbuf.b[xx];
				}
				d_vram->set_buffer_to_vram(vpaddr + noffset, source, 8);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	}
}
	
void TOWNS_SPRITE::render_part()
{
	// ToDo: Implement Register #2-5
//	if((start < 0) || (end < 0)) return;
//	if(start > end) return;
//	out_debug_log(_T("VLINE NUM=%d"),render_num);
	// ToDo: Implement registers.
//	if(reg_spen) {
//		if((frame_sprite_count >= max_sprite_per_frame) && (max_sprite_per_frame > 0)) return;
	/*for(render_num = start; render_num < end; render_num++) */
	{
			uint32_t addr = render_num << 3;
			pair16_t _nx, _ny, _nattr, _ncol;
			_nx.b.l = pattern_ram[addr + 0];
			_nx.b.h = pattern_ram[addr + 1];
			_ny.b.l = pattern_ram[addr + 2];
			_ny.b.h = pattern_ram[addr + 3];
			_nattr.b.l = pattern_ram[addr + 4];
			_nattr.b.h = pattern_ram[addr + 5];
			_ncol.b.l  = pattern_ram[addr + 6];
			_ncol.b.h  = pattern_ram[addr + 7];
			
			int xaddr = _nx.w & 0x1ff;
			int yaddr = _ny.w & 0x1ff;
			// ToDo: wrap round.This is still bogus implement.
			// ToDo: wrap round.This is still bogus implement.
//			out_debug_log(_T("RENDER %d X=%d Y=%d ATTR=%04X COLOR=%04X"), render_num, xaddr, yaddr, _nattr.w, _ncol.w);
			
			render_sprite(render_num, xaddr, yaddr, _nattr.w, _ncol.w);
		}
//	}
}

// ToDo: Discard cache(s) if dirty color index and if used this cache at 16 colors.
// ToDo: Discard cache(s) if dirty 
void TOWNS_SPRITE::write_io8(uint32_t addr, uint32_t data)
{
	if(addr == 0x0450) {
		reg_addr = data & 7;
	} else if(addr != 0x0452) {
		return;
	} else {
//		if(!sprite_enabled) {
			write_reg(reg_addr, data);
//		}
	}
}
	
void TOWNS_SPRITE::write_reg(uint32_t addr, uint32_t data)
{
	reg_data[addr] = (uint8_t)data;
	
	switch(addr) {
	case 0:
		reg_index = ((uint16_t)(reg_data[0]) + (((uint16_t)(reg_data[1] & 0x03)) << 8));
		break;
	case 1:
		reg_index = ((uint16_t)(reg_data[0]) + (((uint16_t)(reg_data[1] & 0x03)) << 8));
		reg_spen = ((reg_data[1] & 0x80) != 0) ? true : false;
		reg_data[1] = reg_data[1] & 0x7f; // From Tsugaru
		break;
	case 2:
	case 3:
		reg_hoffset = ((uint16_t)(reg_data[2]) + (((uint16_t)(reg_data[3] & 0x01)) << 8));
		break;
	case 4:
	case 5:
		reg_voffset = ((uint16_t)(reg_data[4]) + (((uint16_t)(reg_data[5] & 0x01)) << 8));
		break;
	case 6:
		{
			// From Tsugaru
			bool _bak = disp_page1;
			disp_page0 = ((data & 0x08) != 0) ? true : false;
			disp_page1 = ((data & 0x80) != 0) ? true : false;
			/*if(_bak != disp_page1) */page_changed = true;
		}
		break;
	default:
		break;
	}
}

uint32_t TOWNS_SPRITE::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	
	if(addr == 0x05c8) {
		val = (tvram_enabled) ? 0x80 : 0;
		tvram_enabled = false;
		return val;
	} else if(addr == 0x0450) {
		return (reg_addr & 0x07);
	} else if(addr != 0x0452) {
		return 0xff;
	}
	switch(reg_addr) {
/*	case 0:
		val = reg_index & 0xff;
		break;
	case 1:
		val = (reg_index & 0x0300) >> 8;
		val = val | ((reg_spen) ? 0x80 : 0x00);
		break;
	case 2:
		val = reg_hoffset & 0xff;
		break;
	case 3:
		val = (reg_hoffset & 0x0100) >> 8;
		break;
	case 4:
		val = reg_voffset & 0xff;
		break;
	case 5:
		val = (reg_voffset & 0x0100) >> 8;
		break;
*/
	case 1:
		val = reg_data[reg_addr] & 0x7f; // From Tsugaru
		val = val | ((sprite_enabled) ? 0x80 : 0x00);
		break;
	case 6:
		// From Tsugaru
		val = (disp_page0) ? 0x01 : 0x00;
		val = val | ((disp_page1) ? 0x10 : 0x00);
		break;
	default:
//		val = 0x00;
		val = reg_data[reg_addr]; // From Tsugaru
		break;
	}
	return val;
}


uint32_t TOWNS_SPRITE::read_memory_mapped_io8(uint32_t addr)
{
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		return pattern_ram[addr & 0x1ffff];
	}/* else if((addr >= 0xc8000) && (addr < 0xc9000)) {
		return pattern_ram[addr - 0xc8000];
	} else if((addr >= 0xca000) && (addr < 0xcb000)) {
		return pattern_ram[addr - 0xc8000];
	}*/
	else if((addr >= 0xc8000) && (addr < 0xcb000)) { // OK? From TSUGARU
		return pattern_ram[addr - 0xc8000];
	}
	return 0x00;
}


void TOWNS_SPRITE::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		pattern_ram[addr & 0x1ffff] = data;
	} /*else if((addr >= 0xc8000) && (addr < 0xc9000)) {
		tvram_enabled = true;
		tvram_enabled_bak = true;
		pattern_ram[addr - 0xc8000] = data;
	} else if((addr >= 0xca000) && (addr < 0xcb000)) {
		tvram_enabled = true;
		tvram_enabled_bak = true;
		pattern_ram[addr - 0xc8000] = data;
	}*/
	 else if((addr >= 0xc8000) && (addr < 0xcb000)) { // OK? From TSUGARU
		tvram_enabled = true;
		tvram_enabled_bak = true;
		pattern_ram[addr - 0xc8000] = data;
	 }
	return;
}

bool TOWNS_SPRITE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR regstr[1024] = {0};
	_TCHAR sstr[128] = {0};
	my_stprintf_s(sstr, 127, _T("TEXT VRAM:%s \n\n"), ((tvram_enabled) || (tvram_enabled_bak)) ? _T("WROTE") : _T("NOT WROTE"));
	my_tcscat_s(regstr, 1024, sstr);
	
	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 127, _T("SPRITE:%s LOT=%d NUM=%d\nHOFFSET=%d VOFFSET=%d DISP0=%d DISP1=%d\n")
				  , (reg_spen) ? _T("ENABLED ") : _T("DISABLED")
				  , ((reg_index & 0x3ff) == 0) ? 1024 : (reg_index & 0x3ff)
				  , render_num
				  , reg_hoffset
				  , reg_voffset
				  , (disp_page0) ? 1 : 0
				  , (disp_page1) ? 1 : 0
		);
	my_tcscat_s(regstr, 1024, sstr);
	
	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 127, _T("TRANSFER:%s (%s)\n")
				  , (sprite_enabled) ? _T("ON ") : _T("OFF")
				  , (sprite_busy) ? _T("BUSY") : _T("IDLE")
		);
	my_tcscat_s(regstr, 1024, sstr);

	
	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 127, _T("REGISTER ADDRESS:%02X \n"), reg_addr & 0x07);
	my_tcscat_s(regstr, 1024, sstr);
	
	for(int r = 0; r < 8; r++) {
		memset(sstr, 0x00, sizeof(sstr));
		my_stprintf_s(sstr, 127, _T("R%d:%02X "), r, reg_data[r]);
		my_tcscat_s(regstr, 1024, sstr);
		if((r & 3) == 3) {
			my_tcscat_s(regstr, 1024, _T("\n"));
		}
	}
	my_tcscpy_s(buffer, (buffer_len >= 1024) ? 1023 : buffer_len, regstr);
	return true;
}

bool TOWNS_SPRITE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(reg == NULL) return false;
	if((reg[0] == 'R') || (reg[0] == 'r')) {
		if((reg[1] >= '0') && (reg[1] <= '7')) {
			if(reg[2] != '\0') return false;
			int rnum = reg[1] - '0';
			write_reg(rnum, data);
			return true;
		}
	} else 	if((reg[0] == 'A') || (reg[0] == 'a')) {
		if(reg[1] != '\0') return false;
		reg_addr = data & 7;
		return true;
	}
	return false;
}

void TOWNS_SPRITE::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_FALL_DOWN:
//		sprite_enabled = false;
		disp_page1 = !(disp_page1); // Change page
		break;
	case EVENT_BUSY_OFF:
		event_busy = -1;
		sprite_busy = false;
		/*if(render_num >= 1024) */{
			int lot = reg_index & 0x3ff;
			if(lot == 0) lot = 1024;
			render_num = lot;
			if(lot < 1024) {
				disp_page1 = !(disp_page1);
			}
		}
		break;
	case EVENT_RENDER:
		event_busy = -1;
		if((sprite_enabled) /*&& (render_num < 1024) */&& (sprite_busy)) {
//			sprite_busy = true;
			int _bak = render_num;
			for(; render_num < 1024; ){
				render_part();
				render_num++;
			}
			if(_bak >= 1024) {
				_bak = 1023;
			}
			register_event(this, EVENT_BUSY_OFF, 75.0 * (1024 - _bak), false, &event_busy);
//			register_event(this, EVENT_BUSY_OFF, 75.0 / 2, false, &event_busy);
		} else {
			sprite_busy = false;
			int lot = reg_index & 0x3ff;
			if(lot == 0) lot = 1024;
			render_num = lot;
			if(lot < 1024) {
				disp_page1 = !(disp_page1);
			}
		}
	}
}

void TOWNS_SPRITE::check_and_clear_vram()
{
	if(sprite_enabled != reg_spen) {
		if(event_busy > -1) {
			cancel_event(this, event_busy);
			event_busy = -1;
		}
		sprite_busy = false;
	}
	sprite_enabled = reg_spen;
	if(!(sprite_busy) && (sprite_enabled)) {
		uint16_t lot = reg_index & 0x3ff;
		if(lot == 0) lot = 1024;
		render_num = lot;
		sprite_busy = true;
		if(event_busy > -1) {
			cancel_event(this, event_busy);
			event_busy = -1;
		}
		
		{
			uint32_t noffset = (disp_page1) ? 0x40000 : 0x60000;
			draw_page1 = disp_page1;
			if((d_vram != NULL) && (render_num < 1024)) {
				pair16_t *p = (pair16_t*)(d_vram->get_vram_address(noffset));
				if(p != NULL) {
					for(int x = 0; x < 0x10000; x++) {
						p[x].w = 0x8000; //
					}
				}
			}
		
			page_changed = false;
			register_event(this, EVENT_RENDER, 32.0, false, &event_busy);

//			register_event(this, EVENT_RENDER, 1.0, false, &event_busy);
		}
	}
}
void TOWNS_SPRITE::event_pre_frame()
{
	check_and_clear_vram();
}

void TOWNS_SPRITE::event_vline(int v, int clock)
{
//	check_and_clear_vram();
}

// Q: Is changing pages syncing to Frame?
// ToDo: Implement VRAM.
void TOWNS_SPRITE::write_signal(int id, uint32_t data, uint32_t mask)
{
	 /*else if(id == SIG_TOWNS_SPRITE_ANKCG) {  // write CFF19
		ankcg_enabled = ((data & mask) != 0);
	} else */if(id == SIG_TOWNS_SPRITE_TVRAM_ENABLED) {  // write CFF19
		tvram_enabled = ((data & mask) != 0);
		tvram_enabled_bak = tvram_enabled;
	}
}

uint32_t TOWNS_SPRITE::read_signal(int id)
{
	/*if(id == SIG_TOWNS_SPRITE_ANKCG) {  // write CFF19
		 return ((ankcg_enabled) ? 0xffffffff : 0);
	 } else */
	if(id == SIG_TOWNS_SPRITE_BUSY) {
		return (sprite_busy) ? 0xffffffff : 0;
	} else if(id == SIG_TOWNS_SPRITE_TVRAM_ENABLED) {
		 uint32_t v = ((tvram_enabled_bak) ? 0xffffffff : 0);
		 tvram_enabled_bak = false;
		 return v;
	} else if(id == SIG_TOWNS_SPRITE_DISP_PAGE0) {
		return (disp_page0) ? 0xffffffff : 0;
	} else if(id == SIG_TOWNS_SPRITE_DISP_PAGE1) {
		return (disp_page1) ? 0xffffffff : 0;
	} else {
		id = id - SIG_TOWNS_SPRITE_PEEK_TVRAM;
		if((id < 0x1ffff) && (id>= 0)) {
			return pattern_ram[id];
		}
	}
	return 0;
}

#define STATE_VERSION	4

bool TOWNS_SPRITE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	
	state_fio->StateValue(reg_addr);
	state_fio->StateValue(reg_ctrl);
	state_fio->StateArray(reg_data, sizeof(reg_data), 1);
	// RAMs
	state_fio->StateArray(pattern_ram, sizeof(pattern_ram), 1);
	
	state_fio->StateValue(reg_spen);
	state_fio->StateValue(reg_index);
	state_fio->StateValue(reg_voffset);
	state_fio->StateValue(reg_hoffset);
	state_fio->StateValue(disp_page0);
	state_fio->StateValue(disp_page1);
	state_fio->StateValue(draw_page1);

	state_fio->StateValue(sprite_busy);
	state_fio->StateValue(sprite_enabled);
	state_fio->StateValue(page_changed);
	
	state_fio->StateValue(render_num);
	
	state_fio->StateValue(max_sprite_per_frame);
	state_fio->StateValue(tvram_enabled);
	state_fio->StateValue(tvram_enabled_bak);

	state_fio->StateValue(event_busy);

	return true;
}

}
