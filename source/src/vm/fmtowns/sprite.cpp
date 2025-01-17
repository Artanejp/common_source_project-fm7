/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.22 -

	[ Sprite ]
*/

#include "../../common.h"
#include "./vram.h"
#include "./sprite.h"
#include "./crtc.h"
#include "./fontroms.h"

#define EVENT_RENDER	1
#define EVENT_BUSY_OFF	2

namespace FMTOWNS {

void TOWNS_SPRITE::initialize(void)
{
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	frame_out = false;
	disp_page1 = false;
	draw_page1 = true;
	
	reg_spen = false;
	reg_addr = 0;
	memset(reg_data, 0x00, sizeof(reg_data));

	max_sprite_per_frame = 224;
	event_busy = -1;
	page_changed = true;

	is_older_sprite = true;

	register_frame_event(this);
}

void TOWNS_SPRITE::reset()
{
	// Clear RAMs?
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	disp_page1 = false;
	draw_page1 = true;
	reg_spen = false;
	reg_addr = 0;
	render_num = 0;
	frame_out = false;

	sprite_enabled = false;

	max_sprite_per_frame = 224;
	tvram_enabled = false;
	tvram_enabled_bak = false;

	sprite_busy = false;
	page_changed = true;

	memset(reg_data, 0x00, sizeof(reg_data)); // OK?
	is_older_sprite = true;
	sprite_usec = get_sprite_usec(224);
	
	clear_event(this, event_busy);

//	ankcg_enabled = false;
}

void TOWNS_SPRITE::render_text()
{
	int c = 0;
	uint32_t plane_offset = 0x40000;
	uint32_t linesize = 0x80 * 4;
	__LIKELY_IF(d_crtc !=NULL) {
		if(d_crtc->read_signal(SIG_TOWNS_CRTC_R50_PAGESEL) != 0) {
			plane_offset += 0x20000;
		}
		linesize = d_crtc->read_signal(SIG_TOWNS_CRTC_REG_LO1) * 4;
	}
	for(int y = 0; y < 25; y++) {
		uint32_t addr_of = y * (linesize * 16);
		if(c >= 0x1000) break;
		uint32_t romaddr = 0;
		for(int x = 0; x < 80; x++) {
			uint8_t attr;
			uint32_t t = get_font_address(c, attr);
			if(((attr & 0xc0) == 0) || ((attr & 0xc0) == 0x40)) {
				// ANK OR KANJI LEFT
				romaddr = t;
			} else if((attr & 0xc0) == 0x80) {
				// KANJI RIGHT
				romaddr = romaddr + 1;
			} else {
				// Illegal
				addr_of = (addr_of + 4) & 0x3ffff;
				c += 2;
				continue;
			}
			// Get data
			uint32_t color = attr & 0x07;
			if(attr & 0x20) color |= 0x08;
			// Do render
//			out_debug_log("ROMADDR=%08X", romaddr);
			uint32_t of = addr_of;
			for(int column = 0; column < 16; column++) {
				uint8_t tmpdata = 0;
				__LIKELY_IF(d_font != nullptr) {
					if((attr & 0xc0) == 0) {
						// ANK
						tmpdata = d_font->read_direct_data8(column + romaddr);
					} else {
						tmpdata = d_font->read_direct_data8(column * 2 + romaddr);
					}
				}
				if(attr & 0x08)
				{
					tmpdata = ~tmpdata;
				}
				__LIKELY_IF(d_vram != nullptr) {
					uint32_t pix = 0;
					uint8_t *p = d_vram->get_vram_address(of + plane_offset);
					__LIKELY_IF(p != nullptr) {
						d_vram->lock();
						__DECL_VECTORIZED_LOOP
						for(int nb = 0; nb < 8; nb += 2) {
							pix = ((tmpdata & 0x80) != 0) ? color : 0;
							pix = pix | (((tmpdata & 0x40) != 0) ? (color << 4) : 0);
							tmpdata <<= 2;
							*p++ = pix;
						}
						d_vram->unlock();
					}
				}
				of = (of + linesize) & 0x3ffff;
			}
//		_leave0:
			addr_of = (addr_of + 4) & 0x3ffff;
			c += 2;
		}
	}
}

// From MAME 0.216
// ToDo: Will refine.
uint32_t TOWNS_SPRITE::get_font_address(uint32_t c, uint8_t &attr)
{
	static const uint32_t addr_base_jis = 0x00000;
	static const uint32_t addr_base_ank = 0x3d800;
	uint32_t romaddr = 0;
	uint8_t *tvram_snapshot = &(pattern_ram[0x0000]);
	
	attr = tvram_snapshot[c + 1];
	switch(attr & 0xc0) {
	case 0x00:
		{
			uint8_t ank = tvram_snapshot[c];
			romaddr = addr_base_ank + (ank * 16);
		}
		break;
	case 0x40:
		{ // KANJI LEFT
			pair32_t jis;
			jis.b.h = tvram_snapshot[c + 0x2000]; // CA000-CAFFF
			jis.b.l = tvram_snapshot[c + 0x2001]; // CA000-CAFFF
			if(jis.b.h < 0x30) {
				romaddr =
					(((uint32_t)(jis.b.l & 0x1f)) << 4) |
					((uint32_t)((jis.b.l - 0x20) & 0x20) << 8) |
					((uint32_t)((jis.b.l - 0x20) & 0x40) << 6) |
					(((uint32_t)(jis.b.h & 0x07)) << 9);
				romaddr <<= 1;
			} else if(jis.b.h < 0x70) {
				romaddr =
					(((uint32_t)(jis.b.l & 0x1f)) << 5) +
					((uint32_t)((jis.b.l - 0x20) & 0x60) << 9) +
					((uint32_t)((jis.b.h & 0x0f)) << 10) +
					((uint32_t)((jis.b.h - 0x30) & 0x70) * 0xc00) +
					0x8000;
			} else {
				romaddr =
					(((uint32_t)(jis.b.l & 0x1f)) << 4) |
					((uint32_t)((jis.b.l - 0x20) & 0x20) << 8) |
					((uint32_t)((jis.b.l - 0x20) & 0x40) << 6) |
					(((uint32_t)(jis.b.h & 0x07)) << 9);
				romaddr <<= 1;
				romaddr |= 0x38000;
			}
			romaddr = addr_base_jis + romaddr;
		}
		break;
	default: // KANJI RIGHT or ILLEGAL
		return 0;
	}
	return romaddr;
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
	uint32_t color_offset = (((uint32_t)(color & 0xfff)) << 5) & 0x1ffff; // COL11 - COL0

	int xoffset = 0;
	int yoffset = 0;
	if((attr & 0x8000) != 0) { // OFFS
		xoffset = reg_hoffset & 0x1ff;
		yoffset = reg_voffset & 0x1ff;
	}
	uint8_t rot = attr >> 12;
	bool is_halfy = ((attr & 0x0800) != 0);
	bool is_halfx = ((attr & 0x0400) != 0); // SUX
	// From MAME 0.209, mame/drivers/video/fmtowns.cpp
	uint32_t ram_offset =  (((uint32_t)(attr & 0x3ff)) << 7) & 0x1ffff; // PAT9 - PAT0

	int rx = (x + xoffset) & 0x1ff;
	int ry = (y + yoffset) & 0x1ff;
	const int __max_width  = (is_halfx) ? 8 : 16;
	const int __max_height = (is_halfy) ? 8 : 16;
	const int __xstep = (is_halfx) ? 2 : 1;
	const int __ystep = (is_halfy) ? 2 : 1;
	if((rx >= 256) && ((rx + __max_width) < 512)) return;
	if((ry >= 256) && ((ry + __max_height) < 512)) return;
	int __xstart = rx;
	int __ystart = 0;
	int __xstart2 = 0;
	int __xend = __max_width;
	int __yend = __max_height;
	__UNLIKELY_IF(rx >= 256) { // Hidden
		if((rx + __max_width) >= 512) {
			__xstart = 0;
			__xstart2 = 512 - rx;
			//__xend = __max_width - __xstart2;
		} else {
			return;
		}
	} else { // rx < 256
		if((rx + __max_width) >= 256) {
			__xstart2 = 0;
			__xend = 256 - rx;
		}
	}
	__UNLIKELY_IF((__xstart2 >= __max_width) || (__xend > __max_width) ||
				  (__xstart2 < 0) || (__xend <= 0) || (__xend <= __xstart2)) {
		return;
	}

	__DECL_ALIGNED(32) uint16_t tbuf[16][16] = {0};
	__DECL_ALIGNED(32) uint16_t sbuf[16][16] = {0};
	__DECL_ALIGNED(16) uint16_t pixel_h[8];
	__DECL_ALIGNED(16) uint16_t pixel_l[8];
	__DECL_ALIGNED(16) uint16_t color_table[16] = {0};
	
	if(is_32768) {
		__DECL_ALIGNED(16) union {
			pair16_t pw[16];
			uint8_t b[32];
		} nnw;
		for(int yy = 0; yy < 16; yy++) {
			uint32_t addr = (yy << 5) + ram_offset;

			// P1 get data
//__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 32; xx++) {
				nnw.b[xx] = pattern_ram[(addr + xx) & 0x1ffff];
			}
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tbuf[yy][xx] = nnw.pw[xx].w;
			}
		}
	} else {
		__DECL_ALIGNED(16) union {
			pair16_t pw[16];
			uint8_t b[32];
		} nnw;
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 32; i++) {
			nnw.b[i] = pattern_ram[(color_offset + i) & 0x1ffff];
//			color_offset += 2;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			color_table[i] = nnw.pw[i].w;
		}
		color_table[0] = 0x8000; // Clear color
		for(int yy = 0; yy < 16; yy++) {
			uint32_t addr = (yy << 3) + ram_offset;
			uint8_t nnh, nnl;
			__DECL_ALIGNED(8) uint8_t nnb[8];
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx != 8; xx++) {
				nnb[xx] = pattern_ram[(addr + xx) & 0x1ffff];
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
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx += 2 ) {
				tbuf[yy][xx    ] = pixel_h[xx >> 1];
				tbuf[yy][xx + 1] = pixel_l[xx >> 1];
			}
		}
	}
	// Rotate
	switch(rot & 7) { // ROT1, ROT0
	case 0:
		// 0deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tbuf[yy][xx];
			}
		}
		break;
	case 1:
		// 180deg, mirror
		for(int yy = 0; yy < 16; yy++) {
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tbuf[15 - yy][xx];
			}
		}
		break;
	case 2:
		// 0deg, mirror
		for(int yy = 0; yy < 16; yy++) {
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tbuf[yy][15 - xx];
			}
		}
		break;
	case 3:
		// 180deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tbuf[15 - yy][15 - xx];
			}
		}
		break;
	case 4:
		// 270deg, mirror
//__DECL_VECTORIZED_LOOP
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(16) uint16_t tmpvec[16];
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec[xx] = tbuf[xx][yy];
			}
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tmpvec[xx];
//				sbuf[yy][xx] = tbuf[xx][yy];
			}
		}
		break;
	case 5:
		// 90deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(16) uint16_t tmpvec[16];
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec[xx] = tbuf[xx][15 - yy];
			}
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tmpvec[xx];
//				sbuf[yy][xx] = tbuf[xx][15 - yy];
			}
		}
		break;
	case 6:
		// 270deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(16) uint16_t tmpvec[16];
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec[xx] = tbuf[15 - xx][yy];
			}
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy][xx] = tmpvec[xx];
//				sbuf[yy][xx] = tbuf[15 - xx][yy];
			}
		}
		break;
	case 7:
		// 90deg, mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(16) uint16_t tmpvec[16];
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec[xx] = tbuf[15 - xx][15 - yy];
			}
__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
//				sbuf[xx][yy] = tbuf[15 - yy][15 - xx];
				sbuf[yy][xx] = tmpvec[xx];
			}
		}
		break;
	}

	// Zoom and rendering.
	__UNLIKELY_IF(d_vram == NULL) return; // Skip if VRAM not exists.
	uint32_t noffset = (draw_page1) ? 0x40000 : 0x60000;
	uint32_t vpaddr = ((rx + (ry * 256)) << 1) & 0x1ffff;
	
	csp_vector8<uint16_t> source[2];
	csp_vector8<uint16_t> lbuf[2];						// Holizonal line buffer
	csp_vector8<uint16_t> color_values[2];				// Pixel color values
	csp_vector8<uint16_t> maskbuf_posi[2];				// Mask values; 0xffff when transparent.
	csp_vector8<uint16_t> maskbuf_nega[2];				// Mask negative values; 0x00 when transparent.
	csp_vector8<uint16_t> mask_transparent(0x8000);
	csp_vector8<uint16_t> mask_value(0x7fff);
	csp_vector8<bool>     is_transparent[2];
	
	/* Re-Implement new Logic */
	// Get first line

	// For second line.
	csp_vector8<uint16_t> lbuf2[2];				// Holizonal line buffer
	csp_vector8<uint16_t> color_values2[2];		// Pixel color values
	csp_vector8<uint16_t> maskbuf_posi2[2];		// Mask values; 0xffff when transparent.
	//csp_vector8<uint16_t> maskbuf_nega2[2];		// Mask negative values; 0x00 when transparent.
	csp_vector8<bool>	  is_transparent2[2];
		
	csp_vector8<uint16_t> zoomed_mask_posi;
	csp_vector8<uint16_t> zoomed_mask_nega;
	csp_vector8<uint16_t> zoomed_value;

	for(int yy = 0, yy2 = 0; yy < 16;  yy += __ystep, yy2++) {
		int yoff = (yy2 + ry) & 0x1ff;
		if(yoff < 256) {
			// Get From source VRAM
			vpaddr = ((__xstart + (yoff << 8)) << 1) & 0x1ffff;
			if(is_halfx) {
				source[1].clear();
			}
			d_vram->get_vram_to_buffer(vpaddr + noffset, source, (is_halfx) ? 8 : 16);
			// Get first line of SPRITE.
			for(int rx1 = 0; rx1 < 2; rx1++) {
				lbuf[rx1] = mask_transparent;
				is_transparent[rx1].clear();
			}

			lbuf[0].load(&(sbuf[yy][0]));
			lbuf[1].load(&(sbuf[yy][8]));
			__UNLIKELY_IF((__xend != __max_width) || (__xstart2 != 0)) {
				shift_vector_data(__xstart2, __xend, __xstep - 1, lbuf);
			}
			__UNLIKELY_IF(is_halfy) {
				for(int rx1 = 0; rx1 < 2; rx1++) {
					lbuf2[rx1] = mask_transparent;
					is_transparent2[rx1].clear();
				}
				lbuf2[0].load(&(sbuf[yy + 1][0]));
				lbuf2[1].load(&(sbuf[yy + 1][8]));
				__UNLIKELY_IF((__xend != __max_width) || (__xstart2 != 0)) {
					shift_vector_data(__xstart2, __xend, __xstep - 1, lbuf2);
				}
			}
			// Make MASK
			for(int rx1 = 0; rx1 < 2; rx1++) {
				maskbuf_posi[rx1] = lbuf[rx1];
			}
			for(int rx1 = 0; rx1 < 2; rx1++) {
				maskbuf_posi[rx1] &= mask_transparent;
			}
			for(int rx1 = 0; rx1 < 2; rx1++) {
				maskbuf_posi[rx1].not_equals(is_transparent[rx1], 0x0000);
			}
			for(int rx1 = 0; rx1 < 2; rx1++) {
				maskbuf_posi[rx1] = maskbuf_posi[rx1].set_cond(is_transparent[rx1], 0x0000, 0xffff);
			}
			for(int rx1 = 0; rx1 < 2; rx1++) {
				color_values[rx1] = lbuf[rx1];
			}
			for(int rx1 = 0; rx1 < 2; rx1++) {
				color_values[rx1] &= mask_value;
			}
			for(int rx1 = 0; rx1 < 2; rx1++) {
				color_values[rx1] &= maskbuf_posi[rx1];
			}
			if(is_halfy) {
				// Make Mask 2nd line and zoom a data 
				for(int rx1 = 0; rx1 < 2; rx1++) {
					maskbuf_posi2[rx1] = lbuf2[rx1];
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					maskbuf_posi2[rx1] &= mask_transparent;
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					maskbuf_posi2[rx1].not_equals(is_transparent2[rx1], 0x0000);
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					maskbuf_posi2[rx1] = maskbuf_posi2[rx1].set_cond(is_transparent2[rx1], 0x0000, 0xffff);
				}
				//for(int rx1 = 0; rx1 < 2; rx1++) {
				//	maskbuf_nega2[rx1] = ~maskbuf_posi2[rx1];
				//}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					color_values2[rx1] = lbuf2[rx1];
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					color_values2[rx1] &= mask_value;
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					color_values2[rx1] &= maskbuf_posi2[rx1];
				}
				// Zoom to buffer1
				for(int rx1 = 0; rx1 < 2; rx1++) {
					maskbuf_posi[rx1] |= maskbuf_posi2[rx1];
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					__DECL_VECTORIZED_LOOP
					for(size_t rx2 = 0; rx2 < 8; rx2++) {
						color_values[rx1].set(rx2, (color_values[rx1].at(rx2) != 0) ? color_values[rx1].at(rx2) : color_values2[rx1].at(rx2)); 
					}
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					color_values[rx1] &= maskbuf_posi[rx1];
				}
			}
			__LIKELY_IF(!(is_halfx)) {
				// Store without Zoom
				for(int rx1 = 0; rx1 < 2; rx1++) {
					maskbuf_nega[rx1] = ~maskbuf_posi[rx1];
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					source[rx1] &= maskbuf_nega[rx1];
				}
				for(int rx1 = 0; rx1 < 2; rx1++) {
					source[rx1] |= color_values[rx1];
				}
				d_vram->set_buffer_to_vram(vpaddr + noffset, source, 16);
			} else { // Halfx
				// Make Mask
				zoomed_mask_posi.clear();
				zoomed_value.clear();
				__DECL_VECTORIZED_LOOP
				for(size_t rx3 = 0, rx2 = 0; rx3 < 4; rx2 += 2, rx3++) {
					uint16_t _lval = maskbuf_posi[0].at(rx2);
					uint16_t _rval = maskbuf_posi[0].at(rx2 + 1);
					zoomed_mask_posi.set(rx3, _lval | _rval);
				}
				__DECL_VECTORIZED_LOOP
				for(size_t rx3 = 4, rx2 = 0; rx3 < 8; rx2 += 2, rx3++) {
					uint16_t _lval = maskbuf_posi[1].at(rx2);
					uint16_t _rval = maskbuf_posi[1].at(rx2 + 1);
					zoomed_mask_posi.set(rx3, _lval | _rval);
				}
				__DECL_VECTORIZED_LOOP
				for(size_t rx3 = 0, rx2 = 0; rx3 < 4; rx2 += 2, rx3++) {
					uint16_t _lval = color_values[0].at(rx2);
					uint16_t _rval = color_values[0].at(rx2 + 1);
					zoomed_value.set(rx3, (_lval != 0) ? _lval : _rval);
				}
				__DECL_VECTORIZED_LOOP
				for(size_t rx3 = 4, rx2 = 0; rx3 < 8; rx2 += 2, rx3++) {
					uint16_t _lval = color_values[1].at(rx2);
					uint16_t _rval = color_values[1].at(rx2 + 1);
					zoomed_value.set(rx3, (_lval != 0) ? _lval : _rval);
				}
				// Store with Zooming
				zoomed_value &= zoomed_mask_posi;
				zoomed_mask_nega = ~zoomed_mask_posi;
				source[0] &= zoomed_mask_nega;
				source[0] |= zoomed_value;
				
				d_vram->set_buffer_to_vram(vpaddr + noffset, source, 8);
			}
		}
	}

}

void TOWNS_SPRITE::render_part()
{
	// ToDo: Implement Register #2-5
	if((render_num <= 0) || (render_num > 1024)) {
		render_num = 0;
		sprite_busy = false;
		clear_event(this, event_busy);
		return;
	}
	uint32_t addr;
	addr = (1024 - render_num) << 3;

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
	//out_debug_log(_T("RENDER %d X=%d Y=%d ATTR=%04X COLOR=%04X"), render_num, xaddr, yaddr, _nattr.w, _ncol.w);

	render_sprite(render_num, xaddr, yaddr, _nattr.w, _ncol.w);
}

// ToDo: Discard cache(s) if dirty color index and if used this cache at 16 colors.
void TOWNS_SPRITE::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0: // ALIAS of 0450h
		reg_addr = data & 7;
		break;
	case 2: // ALIAS of 0452h
		write_reg(reg_addr, data);
		break;
	default:
		break;
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
		reg_data[6] = reg_data[6] & 0x88; // From Tsugaru
		break;
	default:
		break;
	}
}

uint32_t TOWNS_SPRITE::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	switch(addr) {
	case 0: // ALIAS of 0450h
		val = (reg_addr & 0x07);
		break;
	case 2: // ALIAS of 0452h
		val = read_reg(reg_addr);
		break;
	case 0x05c8:
	case 8: // ALIAS of 05C8h
		val = (tvram_enabled) ? 0x80 : 0;
		tvram_enabled = false;
		break;
	default:
		break;
	}
	return val;
}
uint8_t TOWNS_SPRITE::read_reg(uint32_t addr)
{
	uint8_t val = 0xff;
	addr = addr & 7;
	switch(addr) {
	case 1:
		val = reg_data[addr] & 0x7f; // From Tsugaru
		val = val | ((sprite_enabled) ? 0x80 : 0x00);
		break;
	case 6:
		// From Tsugaru
		val = ((reg_data[addr] & 0x08) != 0) ? 0x01 : 0x00;
		val = val | (((reg_data[addr] & 0x80) != 0) ? 0x10 : 0x00);
		break;
	default:
//		val = 0x00;
		val = reg_data[addr]; // From Tsugaru
		break;
	}
	return val;
}

uint32_t TOWNS_SPRITE::read_dma_data8w(uint32_t addr, int* wait)
{
	return read_memory_mapped_io8w(addr, wait);
}

uint32_t TOWNS_SPRITE::read_dma_data16w(uint32_t addr, int* wait)
{
	return read_memory_mapped_io16w(addr, wait);
}

void TOWNS_SPRITE::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	write_memory_mapped_io8w(addr, data, wait);
}

void TOWNS_SPRITE::write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
{
	write_memory_mapped_io16w(addr, data, wait);
}

uint32_t TOWNS_SPRITE::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	return pattern_ram[addr & 0x1ffff];
}

uint32_t TOWNS_SPRITE::read_memory_mapped_io16w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	pair16_t n;
	addr = addr & 0x1ffff;
	__UNLIKELY_IF(addr == 0x1ffff) {
		n.b.h = 0xff;
		n.b.l = pattern_ram[0x1ffff];
	} else {
		n.read_2bytes_le_from(&(pattern_ram[addr]));
	}
	return n.w;
}

uint32_t TOWNS_SPRITE::read_memory_mapped_io32w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	pair32_t d;
	addr = addr & 0x1ffff;
	__UNLIKELY_IF(addr > 0x1fffc) {
		d.d = 0xffffffff;
		switch(addr) {
		case 0x1ffff:
			d.b.l = pattern_ram[0x1ffff];
			break;
		case 0x1fffe:
			d.b.h = pattern_ram[0x1ffff];
			d.b.l = pattern_ram[0x1fffe];
			break;
		case 0x1fffd:
			d.b.h2 = pattern_ram[0x1ffff];
			d.b.h  = pattern_ram[0x1fffe];
			d.b.l  = pattern_ram[0x1fffd];
			break;
		default:
			break;
		}
	} else {
		d.read_4bytes_le_from(&(pattern_ram[addr]));
	}
	return d.d;
}

void TOWNS_SPRITE::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	addr = addr & 0x1ffff;
	pattern_ram[addr] = data;
	return;
}

void TOWNS_SPRITE::write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	addr = addr & 0x1ffff;
	pair16_t n;
	n.w = data;
	__UNLIKELY_IF(addr == 0x1ffff) {
		pattern_ram[0x1ffff] = n.b.l;
	} else {
		n.write_2bytes_le_to(&(pattern_ram[addr]));
	}
	return;
}

void TOWNS_SPRITE::write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	addr = addr & 0x1ffff;
	pair32_t d;
	d.d = data;
	__UNLIKELY_IF(addr > 0x1fffc) {
		d.d = 0xffffffff;
		switch(addr) {
		case 0x1ffff:
			pattern_ram[0x1ffff] = d.b.l;
			break;
		case 0x1fffe:
			pattern_ram[0x1ffff] = d.b.h;
			pattern_ram[0x1fffe] = d.b.l;
			break;
		case 0x1fffd:
			pattern_ram[0x1ffff] = d.b.h2;
			pattern_ram[0x1fffe] = d.b.h;
			pattern_ram[0x1fffd] = d.b.l;
			break;
		default:
			break;
		}
	} else {
		d.write_4bytes_le_to(&(pattern_ram[addr]));
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
	my_stprintf_s(sstr, 127, _T("SPRITE:%s LOT=%d NUM=%d\nHOFFSET=%d VOFFSET=%d DISP_PAGE=%s\n")
				  , (reg_spen) ? _T("ENABLED ") : _T("DISABLED")
				  , ((reg_index & 0x3ff) == 0) ? 1024 : (reg_index & 0x3ff)
				  , render_num
				  , reg_hoffset
				  , reg_voffset
				  , (read_signal(SIG_TOWNS_SPRITE_BANK) != 0) ? 1 : 0
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
	case EVENT_RENDER:
		if((sprite_enabled) && (sprite_busy)) {
			render_part();
			render_num--;
		}
		break;
	case EVENT_BUSY_OFF:
		sprite_busy = false;
		render_num = 0;
		event_busy = -1;
		break;
	default:
		break;
	}
}

void TOWNS_SPRITE::check_and_clear_vram()
{
	if((sprite_enabled) && (render_num <= 0)) {
		uint16_t lot = reg_index & 0x3ff;
		render_num = 1024 - lot;
		if(render_num > max_sprite_per_frame) {
			render_num = max_sprite_per_frame;
		}
		sprite_usec = get_sprite_usec(render_num);
		__LIKELY_IF(render_num <= 1024) {
			draw_page1 = disp_page1;
			disp_page1 = !(disp_page1);
		}		
		uint32_t noffset = (draw_page1) ? 0x40000 : 0x60000;
		__LIKELY_IF(d_vram != NULL){
			__LIKELY_IF(render_num <= 1024) {
				d_vram->lock();
				pair16_t *p = (pair16_t*)(d_vram->get_vram_address(noffset));
				__LIKELY_IF(p != NULL) {
					for(int x = 0; x < 0x10000; x++) {
						p[x].w = 0x8000; //
					}
				}
				d_vram->unlock();
			}
			page_changed = false;
		}
	}
}
void TOWNS_SPRITE::event_frame()
{
	clear_event(this, event_busy);
	sprite_busy = false;
	frame_out = true;

	if(sprite_enabled != reg_spen) {
		if(reg_spen) {
			render_num = 0;
		}
	}
	sprite_enabled = reg_spen;
	check_and_clear_vram();
}

void TOWNS_SPRITE::event_pre_frame()
{
	clear_event(this, event_busy);

	sprite_busy = false;
}


// Q: Is changing pages syncing to Frame?
// ToDo: Implement VRAM.
void TOWNS_SPRITE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_TOWNS_SPRITE_TEXT_RENDER:
		if(tvram_enabled_bak) {
			render_text();
			tvram_enabled_bak = false;
		}
		break;
	case SIG_TOWNS_SPRITE_TVRAM_ENABLED: // Wrote to C8000h - CAFFFh.
		tvram_enabled = ((data & mask) != 0);
		tvram_enabled_bak = tvram_enabled;
		break;
	case SIG_TOWNS_SPRITE_HOOK_VLINE:
		if(sprite_enabled) {
			if(data < 1024) {
				frame_out = false;
				if(render_num > 0) {
					sprite_busy = true;
					event_callback(EVENT_RENDER, (int)data);
				} else {
					event_callback(EVENT_BUSY_OFF, (int)data);
				}
			}
		}
		break;
	case SIG_TOWNS_SPRITE_VSYNC: //
		if((sprite_enabled) && (frame_out)) { // AT FIRST VSYNC.
			frame_out = false;
			if(render_num > 0) {
				sprite_busy = true;
				event_callback(EVENT_RENDER, 0);
				clear_event(this, event_busy);
				if(render_num > 0) {
					register_event(this, EVENT_RENDER, sprite_usec, true, &event_busy);
				} else {
					register_event(this, EVENT_BUSY_OFF, sprite_usec, false, &event_busy);
				}
			}
		}
		break;
	case SIG_TOWNS_SPRITE_ANKCG: //
		break;
	case SIG_TOWNS_SPRITE_MAX_NUMBERS:
		max_sprite_per_frame = (data & 0x3ff) + 1;
		break;
	default:
		break;
	}
}

uint32_t TOWNS_SPRITE::read_signal(int id)
{
	switch(id) {
	case SIG_TOWNS_SPRITE_ANKCG:
		//return ((ankcg_enabled) ? 0xffffffff : 0);
		 break;
	case SIG_TOWNS_SPRITE_ENABLED:
		return (sprite_enabled) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_SPRITE_BUSY:
		return (sprite_busy) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_SPRITE_DISP_PAGE1:
		if(tvram_enabled_bak) {
			return 0;
		}
		return (disp_page1) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_SPRITE_BANK:
		__LIKELY_IF(sprite_enabled) {
			return (draw_page1) ? 0xffffffff : 0;
		} else {
			return ((reg_data[6] & 0x80) != 0) ? 0xffffffff : 0;
		}
		break;
	case SIG_TOWNS_SPRITE_FRAME_IN:
		return (frame_out) ? 0x00000000 : 0xffffffff;
		break;
	case SIG_TOWNS_SPRITE_TVRAM_ENABLED:
		{
			uint32_t v = ((tvram_enabled_bak) ? 0xffffffff : 0);
			tvram_enabled_bak = false;
			return v;
		}
		break;
	case SIG_TOWNS_SPRITE_MAX_NUMBERS:
		__LIKELY_IF(max_sprite_per_frame > 0) {
			return max_sprite_per_frame;
		}
		return 0;
		break;
	default:
		if(id >= SIG_TOWNS_SPRITE_PEEK_TVRAM) {
			id = id - SIG_TOWNS_SPRITE_PEEK_TVRAM;
			if(id < 0x20000) {
				return pattern_ram[id];
			}
		}
		break;
	}
	return 0;
}

#define STATE_VERSION	6

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
	state_fio->StateValue(disp_page1);
	state_fio->StateValue(draw_page1);

	state_fio->StateValue(frame_out);
	state_fio->StateValue(sprite_busy);
	state_fio->StateValue(sprite_enabled);
	state_fio->StateValue(page_changed);

	state_fio->StateValue(render_num);

	state_fio->StateValue(max_sprite_per_frame);
	state_fio->StateValue(tvram_enabled);
	state_fio->StateValue(tvram_enabled_bak);
	state_fio->StateValue(is_older_sprite);
	state_fio->StateValue(sprite_usec);

	state_fio->StateValue(event_busy);

	return true;
}

}
