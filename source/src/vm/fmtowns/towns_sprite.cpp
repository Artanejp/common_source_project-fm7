/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.22 -

	[ Sprite ]
*/

#include "../../common.h"
#include "./towns_vram.h"
#include "./towns_sprite.h"

namespace FMTOWNS {
	
void TOWNS_SPRITE::initialize(void)
{
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	disp_page1 = false;
	disp_page0 = false;
	reg_spen = false;
	reg_addr = 0;
	memset(reg_data, 0x00, sizeof(reg_data));

	max_sprite_per_frame = 224;
	frame_sprite_count = 0;	
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
	disp_page0 = false;
	reg_spen = false;
	reg_addr = 0;
	render_num = 0;
	render_mod = 0;
	render_lines = 224;
	split_rendering = true;

	sprite_enabled = false;
	now_transferring = false;
	max_sprite_per_frame = 224;
	frame_sprite_count = 0;
	tvram_enabled = false;
	tvram_enabled_bak = false;
	
	memset(reg_data, 0x00, sizeof(reg_data)); // OK?
//	ankcg_enabled = false;
}
	// Still don't use cache.
void TOWNS_SPRITE::render_sprite(int num, int x, int y, uint16_t attr, uint16_t color)
{
	uint16_t lot = reg_index & 0x3ff;
	if(lot == 0) lot = 1024;
	if(num < 0) return;
	if(num >= lot) return;
	if(!(reg_spen) || !(sprite_enabled)) return; 

	bool is_32768 = ((color & 0x8000) == 0); // CTEN
	// ToDo: SPYS
	if((color & 0x2000) != 0) return; // DISP
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
	switch(rot & 3) { // ROT1, ROT0
	case 0:
		// 0deg, not mirror
		xbegin = 0;
		xend   = 15;
		ybegin = 0;
		yend = 15;
		xinc = 1;
		yinc = 1;
		break;
	case 1:
		// 180deg, mirror
		xbegin = 0;
		xend   = 15;
		ybegin = 15;
		yend = 0;
		xinc = 1;
		yinc = -1;
		break;
	case 2:
		// 0deg, mirror
		xbegin = 15;
		xend   = 0;
		ybegin = 0;
		yend = 15;
		xinc = -1;
		yinc = 1;
		break;
	case 3:
		// 180deg, not mirror
		xbegin = 15;
		xend   = 0;
		ybegin = 15;
		yend = 0;
		xinc = -1;
		yinc = -1;
		break;
		/*
	case 4:
		// 270deg, mirror
		xbegin = 0;
		xend   = 15;
		ybegin = 0;
		yend = 15;
		xinc = 1;
		yinc = 1;
		swap_v_h = true;
		break;
	case 5:
		// 90deg, not mirror
		xbegin = 0;
		xend   = 15;
		ybegin = 15;
		yend = 0;
		xinc = 1;
		yinc = -1;
		swap_v_h = true;
		break;
	case 6:
		// 270deg, not mirror
		xbegin = 15;
		xend   = 0;
		ybegin = 0;
		yend = 15;
		xinc = -1;
		yinc = 1;
		swap_v_h = true;
		break;
	case 7:
		// 90deg, mirror
		xbegin = 15;
		xend   = 0;
		ybegin = 15;
		yend = 0;
		xinc = -1;
		yinc = -1;
		swap_v_h = true;
		break;
		*/
	}
	now_transferring = true;
	__DECL_ALIGNED(32) uint16_t sbuf[16][16];
	__DECL_ALIGNED(32) pair16_t lbuf[16];
	__DECL_ALIGNED(32) pair16_t mbuf[16];
	__DECL_ALIGNED(16) uint16_t pixel_h[8];
	__DECL_ALIGNED(16) uint16_t pixel_l[8];
	__DECL_ALIGNED(16) uint16_t color_table[16] = {0};
	if(!(swap_v_h)) {
		if(is_32768) {
			// get from ram.
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 5) + (xbegin << 1) + ram_offset;
				pair16_t nn;
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					nn.b.l = pattern_ram[(addr + 0) & 0x1ffff];
					nn.b.h = pattern_ram[(addr + 1) & 0x1ffff];
					sbuf[yy][xx] = nn.w;
					addr = (addr + (xinc << 1)) & 0x1ffff;
				}
			}
		} else { // 16 colors
			pair16_t nn;
__DECL_VECTORIZED_LOOP						
			for(int i = 0; i < 16; i++) {
				nn.b.l = pattern_ram[(color_offset + 0) & 0x1ffff];
				nn.b.h = pattern_ram[(color_offset + 1) & 0x1ffff];
				color_offset += 2;
				color_table[i] = nn.w;
			}
			color_table[0] = 0x8000; // Clear color
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 3) + (xbegin >> 1) + ram_offset;
				uint8_t nnh, nnl;
				uint8_t nn;
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++ ) {
					nn = pattern_ram[(addr + xx * xinc) & 0x1ffff];
					nnh = nn >> 4;
					nnl = nn & 0x0f;
					pixel_h[xx] = color_table[nnh];
					pixel_l[xx] = color_table[nnl];
				}
				if(yinc < 0) {
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
				pair16_t nn;
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					nn.b.l = pattern_ram[(addr + 0) & 0x1ffff];
					nn.b.h = pattern_ram[(addr + 1) & 0x1ffff];
					sbuf[xx][yy] = nn.w;
					addr = (addr + (xinc << 1)) & 0x1ffff;
				}
			}
		} else { // 16 colors
			pair16_t nn;
__DECL_VECTORIZED_LOOP						
			for(int i = 0; i < 16; i++) {
				nn.b.l = pattern_ram[(color_offset + 0) & 0x1ffff];
				nn.b.h = pattern_ram[(color_offset + 1) & 0x1ffff];
				color_offset += 2;
				color_table[i] = nn.w;
			}
			color_table[0] = 0x8000; // Clear color
			for(int yy = 0; yy < 16; yy++) {
				uint32_t addr = ((ybegin + yy * yinc) << 3) + (xbegin >> 1) + ram_offset;
				uint8_t nnh, nnl;
				uint8_t nn;
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++ ) {
					nn = pattern_ram[(addr + xx * xinc) & 0x1ffff];
					nnh = nn >> 4;
					nnl = nn & 0x0f;
					pixel_h[xx] = color_table[nnh];
					pixel_l[xx] = color_table[nnl];
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
	uint32_t noffset = (disp_page1) ? 0x60000 : 0x40000;
	uint32_t vpaddr = (((x - xoffset) % 256 + ((y - yoffset) * 256)) << 1) & 0x1ffff;
	if(!(is_halfx) && !(is_halfy)) { // not halfed
		int __xstart = 0;		
		int __xend = 16;
		int __ystart = 0;
		int __yend = 16;
		/*
		if(x < xoffset) {
			__xstart = ((xoffset - x + 16) >= 0) ? (xoffset - x + 16) : 0;
			__xend = ((xoffset - x + 16) >= 0)  ? 16 : 0;
		} else if(x > (xoffset + 256)) {
			__xend = 16 - (x - (xoffset + 256));
			__xstart = 0;
			if(__xend < 0) goto __noop;
		} else { // INSIDE OF WINDOW
			__xstart = 0;
			__xend = 16;
		}
		if(((__xstart <= 0) && (__xend <= 0))) goto __noop;
		int __ystart;
		int __yend;
		if(y < yoffset) {
			__ystart = ((yoffset - y + 16) >= 0) ? (yoffset - y + 16) : 0;  
			__yend = ((yoffset - y + 16) >= 0) ? 16 : 0;
		} else if(y > (yoffset + 256)) {
			__ystart = y - (yoffset + 256);
			if(__ystart >= 16) {
				__ystart = 0;
				__yend = 0;
			} else {
				__ystart = 0;
				__yend = 16 - __ystart;
			}
		} else { // INSIDE OF WINDOW
			__ystart = 0;
			__yend = 16;
		}
		if(((__ystart <= 0) && (__yend <= 0))) goto __noop;
		*/
		for(int yy = __ystart; yy < __yend;  yy++) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx].w = 0;
				mbuf[xx].w = 0;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx].w = sbuf[yy][xx];
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				mbuf[xx].w = (lbuf[xx].w == 0) ? 0xffff :  0x0000;
			}
			if(d_vram != NULL) {
				__DECL_ALIGNED(16) uint8_t source[32] = {0};
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 16);
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx++) {
						source[(xx << 1) + 0] &= mbuf[xx].b.l;
						source[(xx << 1) + 1] &= mbuf[xx].b.h;
					}
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx++) {
						source[(xx << 1) + 0] |= lbuf[xx].b.l;
						source[(xx << 1) + 1] |= lbuf[xx].b.h;
					}
					d_vram->set_buffer_to_vram(vpaddr + noffset, source, 16);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	} else if((is_halfx) && !(is_halfy)) { // halfx only
		/*
		int __xstart;
		int __xend;
		if(x < xoffset) {
			__xstart = ((xoffset - x + 8) >= 0) ? (xoffset - x + 8) : 0;
			__xend = ((xoffset - x + 8) >= 0)  ? 8 : 0;
		} else if(x > (xoffset + 256)) {
			__xend = 8 - (x - (xoffset + 256));
			__xstart = 0;
			if(__xend < 0) goto __noop;
		} else { // INSIDE OF WINDOW
			__xstart = 0;
			__xend = 8;
		}
		if(((__xstart <= 0) && (__xend <= 0))) goto __noop;
		int __ystart;
		int __yend;
		if(y < yoffset) {
			__ystart = ((yoffset - y + 16) >= 0) ? (yoffset - y + 16) : 0;  
			__yend = ((yoffset - y + 16) >= 0) ? 16 : 0;
		} else if(y > (yoffset + 256)) {
			__ystart = y - (yoffset + 256);
			if(__ystart >= 16) {
				__ystart = 0;
				__yend = 0;
			} else {
				__ystart = 0;
				__yend = 16 - __ystart;
			}
		} else { // INSIDE OF WINDOW
			__ystart = 0;
			__yend = 16;
		}
		if(((__ystart <= 0) && (__yend <= 0))) goto __noop;
		*/
		int __xstart = 0;		
		int __xend = 8;
		int __ystart = 0;
		int __yend = 16;
		for(int yy = __ystart; yy < __yend;  yy++) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx].w = 0;
				mbuf[xx].w = 0;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx >> 1].w += (sbuf[yy][xx] & 0x7fff);
				mbuf[xx >> 1].w |= (sbuf[yy][xx] & 0x8000);
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 8; xx++) {
				lbuf[xx].w = ((lbuf[xx].w >> 1) & 0x7fff) | mbuf[xx].w;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 8; xx += 1) {
				mbuf[xx].w = (lbuf[xx].w == 0) ? 0xffff : 0x0000;
			}
			if(d_vram != NULL) {
				__DECL_ALIGNED(16) uint8_t source[32] = {0};
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 8);
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 8; xx++) {
						source[(xx << 1) + 0] &= mbuf[xx].b.l;
						source[(xx << 1) + 1] &= mbuf[xx].b.h;
					}
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 8; xx++) {
						source[(xx << 1) + 0] |= lbuf[xx].b.l;
						source[(xx << 1) + 1] |= lbuf[xx].b.h;
					}
					d_vram->set_buffer_to_vram(vpaddr + noffset, source, 8);
				}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	} else if(is_halfy) { // halfy only
/*		int __xstart;
		int __xend;
		if(x < xoffset) {
			__xstart = ((xoffset - x + 16) >= 0) ? (xoffset - x + 16) : 0;
			__xend = ((xoffset - x + 16) >= 0)  ? 16 : 0;
		} else if(x > (xoffset + 256)) {
			__xend = 16 - (x - (xoffset + 256));
			__xstart = 0;
			if(__xend < 0) goto __noop;
		} else { // INSIDE OF WINDOW
			__xstart = 0;
			__xend = 16;
		}
		if(((__xstart <= 0) && (__xend <= 0))) goto __noop;
		int __ystart;
		int __yend;
		if(y < yoffset) {
			__ystart = ((yoffset - y + 8) >= 0) ? (yoffset - y + 8) : 0;  
			__yend = ((yoffset - y + 8) >= 0) ? 8 : 0;
		} else if(y > (yoffset + 256)) {
			__ystart = y - (yoffset + 256);
			if(__ystart >= 8) {
				__ystart = 0;
				__yend = 0;
			} else {
				__ystart = 0;
				__yend = 8 - __ystart;
			}
		} else { // INSIDE OF WINDOW
			__ystart = 0;
			__yend = 8;
		}
		if(((__ystart <= 0) && (__yend <= 0))) goto __noop;
*/
		int __xstart = 0;		
		int __xend = 16;
		int __ystart = 0;
		int __yend = 8;
		for(int yy = (__ystart << 1); yy < (__yend << 1);  yy += 2) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx].w = 0;
				mbuf[xx].w = 0;
			}
			for(int yy2 = 0; yy2 < 2; yy2++) {
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf[xx].w += (sbuf[yy + yy2][xx] & 0x7fff);
					mbuf[xx].w |= (sbuf[yy + yy2][xx] & 0x8000);
				}
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx].w = ((lbuf[xx].w >> 1) & 0x7fff) | mbuf[xx].w;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				mbuf[xx].w = (lbuf[xx].w == 0) ? 0xffff : 0x0000;
			}
			if(d_vram != NULL) {
				__DECL_ALIGNED(16) uint8_t source[32] = {0};
				d_vram->get_vram_to_buffer(vpaddr + noffset, source, 16);
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx++) {
						source[(xx << 1) + 0] &= mbuf[xx].b.l;
						source[(xx << 1) + 1] &= mbuf[xx].b.h;
					}
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 16; xx++) {
						source[(xx << 1) + 0] |= lbuf[xx].b.l;
						source[(xx << 1) + 1] |= lbuf[xx].b.h;
					}
					d_vram->set_buffer_to_vram(vpaddr + noffset, source, 16);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	} else { //halfx &&halfy
/*		int __xstart;
		int __xend;
		if(x < xoffset) {
			__xstart = ((xoffset - x + 8) >= 0) ? (xoffset - x + 8) : 0;
			__xend = ((xoffset - x + 8) >= 0)  ? 8 : 0;
		} else if(x > (xoffset + 256)) {
			__xend = 8 - (x - (xoffset + 256));
			__xstart = 0;
			if(__xend < 0) goto __noop;
		} else { // INSIDE OF WINDOW
			__xstart = 0;
			__xend = 8;
		}
		if(((__xstart <= 0) && (__xend <= 0))) goto __noop;
		int __ystart;
		int __yend;
		if(y < yoffset) {
			__ystart = ((yoffset - y + 8) >= 0) ? (yoffset - y + 8) : 0;  
			__yend = ((yoffset - y + 8) >= 0) ? 8 : 0;
		} else if(y > (yoffset + 256)) {
			__ystart = y - (yoffset + 256);
			if(__ystart >= 8) {
				__ystart = 0;
				__yend = 0;
			} else {
				__ystart = 0;
				__yend = 8 - __ystart;
			}
		} else { // INSIDE OF WINDOW
			__ystart = 0;
			__yend = 8;
		}
		if(((__ystart <= 0) && (__yend <= 0))) goto __noop;
*/
		int __xstart = 0;		
		int __xend = 8;
		int __ystart = 0;
		int __yend = 8;
		for(int yy = (__ystart << 1); yy < (__yend << 1);  yy += 2) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx].w = 0;
				mbuf[xx].w = 0;
			}
			for(int yy2 = 0; yy2 < 2; yy2++) {
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx += 2) {
					lbuf[xx >> 1].w += (sbuf[yy + yy2][xx] & 0x7fff);
					lbuf[xx >> 1].w += (sbuf[yy + yy2][xx + 1] & 0x7fff);
					mbuf[xx >> 1].w |= (sbuf[yy + yy2][xx] & 0x8000);
					mbuf[xx >> 1].w |= (sbuf[yy + yy2][xx + 1] & 0x8000);
				}
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 8; xx++) {
					lbuf[xx].w >>= 1;
				}
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 8; xx++) {
				lbuf[xx].w = ((lbuf[xx].w >> 2) & 0x7fff) | mbuf[xx].w;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 8; xx++) {
				mbuf[xx].w = (lbuf[xx].w == 0x0000) ? 0xffff : 0x0000;
			}
			if(d_vram != NULL) {
				//d_vram->write_sprite_data(x, y + (yy >>1), xoffset, yoffset, lbuf, 8);
				__DECL_ALIGNED(16) uint8_t source[32] = {0};
					d_vram->get_vram_to_buffer(vpaddr + noffset, source, 8);
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 8; xx++) {
						source[(xx << 1) + 0] &= mbuf[xx].b.l;
						source[(xx << 1) + 1] &= mbuf[xx].b.h;
					}
__DECL_VECTORIZED_LOOP						
					for(int xx = 0; xx < 8; xx++) {
						source[(xx << 1) + 0] |= lbuf[xx].b.l;
						source[(xx << 1) + 1] |= lbuf[xx].b.h;
					}
					d_vram->set_buffer_to_vram(vpaddr + noffset, source, 8);
			}
			vpaddr = (vpaddr + (256 << 1)) & 0x1ffff;
		}
	}
__noop:
	now_transferring = false;

}
	
// Q: Does split rendering per vline?
void TOWNS_SPRITE::render_full()
{
	// ToDo: Implement Register #2-5
	uint16_t lot = reg_index & 0x3ff;
	if(lot == 0) lot = 1024;
	
	// Clear buffer?
	//memset(buffer, 0x00, 256 * 256 * sizeof(uint16_t));
	//memset(mask, 0x00, 256 * 256 * sizeof(uint16_t));
	// ToDo: Implement registers.
	if(reg_spen) {
		if((frame_sprite_count >= max_sprite_per_frame) && (max_sprite_per_frame > 0)) return;
		for(; render_num < (int)lot; render_num++) {
			
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
			out_debug_log(_T("RENDER %d X=%d Y=%d ATTR=%04X COLOR=%04X"), render_num, xaddr, yaddr, _nattr.w, _ncol.w);
			render_sprite(render_num, xaddr, yaddr, _nattr.w, _ncol.w);
			frame_sprite_count++;
			if((frame_sprite_count >= max_sprite_per_frame) && (max_sprite_per_frame > 0)) break;
		}
	}
}

void TOWNS_SPRITE::render_part(int start, int end)
{
	// ToDo: Implement Register #2-5
	if((start < 0) || (end < 0)) return;
	if(start > end) return;
//	out_debug_log(_T("VLINE NUM=%d"),render_num);
	// ToDo: Implement registers.
	if(reg_spen) {
		if((frame_sprite_count >= max_sprite_per_frame) && (max_sprite_per_frame > 0)) return;
		for(render_num = start; render_num < end; render_num++) {
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
			//out_debug_log(_T("RENDER %d X=%d Y=%d ATTR=%04X COLOR=%04X"), render_num, xaddr, yaddr, _nattr.w, _ncol.w);
			
			render_sprite(render_num, xaddr, yaddr, _nattr.w, _ncol.w);
			frame_sprite_count++;
			if((frame_sprite_count >= max_sprite_per_frame) && (max_sprite_per_frame > 0)) break;
		}
	}
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
		if(!(now_transferring)) {
			disp_page0 = ((data & 0x01) != 0) ? true : false;
			disp_page1 = ((data & 0x10) != 0) ? true : false;
			if(d_vram != NULL) {
				d_vram->write_signal(SIG_TOWNS_VRAM_DP0, (disp_page0) ? 0xffffffff : 0 , 0xffffffff);
				d_vram->write_signal(SIG_TOWNS_VRAM_DP1, (disp_page1) ? 0xffffffff : 0 , 0xffffffff);
			}
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
	case 0:
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
		break;
	case 6:
		val = (disp_page0) ? 0x08 : 0x00;
		val = val | ((disp_page1) ? 0x80 : 0x00);
		break;
	default:
		val = 0x00;
		break;
	}
	return val;
}


uint32_t TOWNS_SPRITE::read_memory_mapped_io8(uint32_t addr)
{
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		return pattern_ram[addr & 0x1ffff];
	} else if((addr >= 0xc8000) && (addr < 0xc9000)) {
		return pattern_ram[addr - 0xc8000];
	} else if((addr >= 0xca000) && (addr < 0xcb000)) {
		return pattern_ram[addr - 0xc8000];
	}
	return 0x00;
}


void TOWNS_SPRITE::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		pattern_ram[addr & 0x1ffff] = data;
	} else if((addr >= 0xc8000) && (addr < 0xc9000)) {
		tvram_enabled = true;
		tvram_enabled_bak = true;
		pattern_ram[addr - 0xc8000] = data;
	} else if((addr >= 0xca000) && (addr < 0xcb000)) {
		tvram_enabled = true;
		tvram_enabled_bak = true;
		pattern_ram[addr - 0xc8000] = data;
	}
	return;
}

bool TOWNS_SPRITE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR regstr[1024] = {0};
	_TCHAR sstr[64] = {0};
	my_stprintf_s(sstr, 63, _T("TEXT VRAM:%s \n"), ((tvram_enabled) || (tvram_enabled_bak)) ? _T("WROTE") : _T("NOT WROTE"));
	my_tcscat_s(regstr, 1024, sstr);
	
	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 63, _T("SPRITE:%s \n"), (sprite_enabled) ? _T("ENABLED ") : _T("DISABLED"));
	my_tcscat_s(regstr, 1024, sstr);
	
	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 64, _T("A:%02X \n"), reg_addr & 0x07);
	my_tcscat_s(regstr, 1024, sstr);
	
	for(int r = 0; r < 8; r++) {
		memset(sstr, 0x00, sizeof(sstr));
		my_stprintf_s(sstr, 32, _T("R%d:%02X "), r, reg_data[r]);
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

void TOWNS_SPRITE::event_frame()
{
	uint16_t lot = reg_index & 0x3ff;
//	if(reg_spen && !(sprite_enabled)) {
	sprite_enabled = true;
	render_num = 0;
	render_mod = 0;
//	}
	if(lot == 0) lot = 1024;
	frame_sprite_count = 0;
	if(sprite_enabled){
		if(d_vram != NULL) {
			// Set split_rendering from DIPSW.
			// Set cache_enabled from DIPSW.
			if(!split_rendering) {
				render_full();
			}
			//} else {
			//render_num = 0;
			//render_mod = 0;
			//sprite_enabled = false;
			//}
		} else {
			render_num = 0;
			render_mod = 0;
			sprite_enabled = false;
		}
	}
}

void TOWNS_SPRITE::event_vline(int v, int clock)
{
	do_vline_hook(v);
}

void TOWNS_SPRITE::do_vline_hook(int line)
{
	int lot = reg_index & 0x3ff;
	if(!split_rendering) return;
	if(lot == 0) lot = 1024;
	if((max_sprite_per_frame > 0) && (max_sprite_per_frame < lot)) lot = max_sprite_per_frame;
//	if(line > 128) sprite_enabled = false; // DEBUG

	if((sprite_enabled) && (render_lines > 0)) {
		int nf = lot / render_lines;
		int nm = lot % render_lines;
		render_mod += nm;
		if(render_mod >= render_lines) {
			nf++;
			render_mod -= render_lines;
		}
		if((nf >= 1) && (render_num < lot) && (sprite_enabled)) {
			render_part(render_num, render_num + nf);
		} else if(render_num >= lot) {
			sprite_enabled = false;
		}
	}
}
// Q: Is changing pages syncing to Frame?
// ToDo: Implement VRAM.
void TOWNS_SPRITE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_TOWNS_SPRITE_HOOK_VLINE) {
		int line = data & 0x1ff;
		do_vline_hook(line);
	} else if(id == SIG_TOWNS_SPRITE_SET_LINES) {
		int line = data & 0x7ff; // 2048 - 1
		render_lines = line;
	} /*else if(id == SIG_TOWNS_SPRITE_ANKCG) {  // write CFF19
		ankcg_enabled = ((data & mask) != 0);
	} */else if(id == SIG_TOWNS_SPRITE_TVRAM_ENABLED) {  // write CFF19
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
		int lot = reg_index & 0x3ff;
		if(lot == 0) lot = 1024;
		return (/*(render_num < lot) && */(sprite_enabled)) ? 0xffffffff : 0;
	} else if(id == SIG_TOWNS_SPRITE_TVRAM_ENABLED) {
		 uint32_t v = ((tvram_enabled_bak) ? 0xffffffff : 0);
		 tvram_enabled_bak = false;
		 return v;
	} else {
		id = id - SIG_TOWNS_SPRITE_PEEK_TVRAM;
		if((id < 0x1ffff) && (id>= 0)) {
			return pattern_ram[id];
		}
	}
	return 0;
}

#define STATE_VERSION	1
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

	state_fio->StateValue(sprite_enabled);
	state_fio->StateValue(frame_sprite_count);
	
	state_fio->StateValue(render_num);
	state_fio->StateValue(render_mod);
	state_fio->StateValue(render_lines);
	state_fio->StateValue(now_transferring);
	
	state_fio->StateValue(frame_sprite_count);
	state_fio->StateValue(max_sprite_per_frame);
	state_fio->StateValue(tvram_enabled);
	state_fio->StateValue(tvram_enabled_bak);

//	state_fio->StateValue(ankcg_enabled);
	//state_fio->StateValue(split_rendering);

	return true;
}

}
