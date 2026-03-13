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
#include "../../types/simd/primitives_128.hpp"

#define EVENT_RENDER				1
#define EVENT_BUSY_OFF				2
#define EVENT_CLEAR_VRAM_COMPLETED	3

namespace FMTOWNS {

void TOWNS_SPRITE::initialize(void)
{
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	frame_out = true;
	disp_page1 = false;
	draw_page1 = true;
	
	reg_spen = false;
	reg_addr = 0;
	memset(reg_data, 0x00, sizeof(reg_data));

	max_sprite_per_frame = 224;
	event_busy = -1;
	reg06_wrote = false;

	is_older_sprite = true;

	register_frame_event(this);
}

void TOWNS_SPRITE::reset()
{
	// Clear RAMs? -> Yes
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	reg_index = 0x0000;
	disp_page1 = false;
	draw_page1 = true;
	reg06_wrote = false;
	reg_spen = false;
	reg_addr = 0;
	render_num = 0;
	frame_out = true;

	sprite_enabled = false;

	max_sprite_per_frame = 224;
	tvram_enabled = false;
	need_render_text = false;

	sprite_busy = false;
	reg06_wrote = false;

	memset(reg_data, 0x00, sizeof(reg_data)); // OK?
	is_older_sprite = true;
	sprite_usec = get_sprite_usec(224);
	
	clear_event(this, event_busy);

//	ankcg_enabled = false;
}

void TOWNS_SPRITE::render_text()
{
	uint16_t c = 0;
	uint32_t plane_offset = 0x40000;
	uint32_t linesize = 0x80 * 4;
	//__LIKELY_IF(d_crtc !=NULL) {
	//	if(d_crtc->read_signal(SIG_TOWNS_CRTC_R50_PAGESEL) != 0) {
	//		plane_offset += 0x20000;
	//	}
	//	linesize = d_crtc->read_signal(SIG_TOWNS_CRTC_REG_LO1) * 4;
	//}
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
uint32_t TOWNS_SPRITE::get_font_address(const uint16_t c, uint8_t &attr)
{
	static const uint32_t addr_base_jis = 0x00000;
	static const uint32_t addr_base_ank = 0x3d800;
	uint32_t romaddr = 0;
	uint8_t *tvram_snapshot = &(pattern_ram[c & 0xfff]);
	attr = tvram_snapshot[1];
	switch(attr & 0xc0) {
	case 0x00:
		{
			uint8_t ank = tvram_snapshot[0];
			romaddr = addr_base_ank + (ank * 16);
		}
		break;
	case 0x40:
		{ // KANJI LEFT
			pair32_t jis;
			jis.b.h = tvram_snapshot[0x2000]; // CA000-CAFFF
			jis.b.l = tvram_snapshot[0x2001]; // CA000-CAFFF
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


uint16_16_t TOWNS_SPRITE::shift_vector_data(int _xstart, int _xend, int _xshift, uint16_16_t _lbuf)
{
	__UNLIKELY_IF((_xshift < 0) || (_xshift > 1)) {
		return _lbuf; // NOP : OK?
	}
	__UNLIKELY_IF((_xstart < 0) || (_xend < 0)) {
		return _lbuf; // NOP : OK?
	}
	size_t __lstart = (size_t)(_xstart) << _xshift;
	size_t __lend = (size_t)(_xend) << _xshift;
	size_t __lwidth;
	__DECL_ALIGNED(32) uint16_16_t tmpbuf1;
	//tmpbuf1.v = simd_256bit::op_set16(0x8000);
	__UNLIKELY_IF(__lstart >= 16) {
		return _lbuf; // NOP
	}
	__UNLIKELY_IF(__lend >= 16) {
		__lend = 16;
	}
	__UNLIKELY_IF(__lend <= __lstart) {
		return _lbuf; // NOP
	}
	__lwidth = __lend - __lstart;
	__UNLIKELY_IF(__lwidth > 16) {
		__lwidth = 16;
	}
//	__UNLIKELY_IF((__lstart == 0) && (__lwidth == 16)) {
//		// No Shift
//		return _lbuf;
//	}
	tmpbuf1.v = _lbuf.v;
	for(size_t xs = __lstart, xt = 0; xs < __lend; xs++, xt++) {
		tmpbuf1.u16[xt] = tmpbuf1.u16[xs];
	}
	
	if(__lwidth < 16) {
		for(size_t xt = __lwidth; xt < 16; xt++) {
			tmpbuf1.u16[xt] = 0x8000; // Clear
		}
	}
	return tmpbuf1;
}

#undef __M__MINIMUM_ALIGN_LENGTH

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

	int xoffset = 0;
	int yoffset = 0;
	if((attr & 0x8000) != 0) { // OFFS
		xoffset = reg_hoffset & 0x1ff;
		yoffset = reg_voffset & 0x1ff;
	}
	uint8_t rot = attr >> 12;
	bool is_halfy = ((attr & 0x0800) != 0);
	bool is_halfx = ((attr & 0x0400) != 0); // SUX

	uint32_t color_offset = (uint32_t)(color & 0xfff); // COL11 - COL0
	// From MAME 0.209, mame/drivers/video/fmtowns.cpp
	uint32_t ram_offset =  ((uint32_t)(attr & 0x3ff)) << 2; // PAT9 - PAT0

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

	__DECL_ALIGNED(32) uint16_16_t tbuf[16];
	__DECL_ALIGNED(32) uint16_16_t sbuf[16];
	__DECL_ALIGNED(32) uint16_16_t color_table;

	for(size_t i = 0; i < 16; i++) {
		tbuf[i].v = simd_256bit::op_clear();
		sbuf[i].v = simd_256bit::op_clear();
	}
	color_table.v = simd_256bit::op_clear();
	
	__DECL_ALIGNED(32) uint16_16_t pixel_hl;
	__DECL_ALIGNED(32) uint16_16_t nnw;
	if(is_32768) {
		for(int yy = 0; yy < 16; yy++) {
			//uint32_t addr = (yy << 5) + ram_offset;
			load_16words_from_pattern_ram(ram_offset, (uint32_t)yy, &(nnw.u16[0]));
			// P1 get data
			tbuf[yy].v = nnw.v;
		}
	} else {
		load_16words_from_pattern_ram((uint32_t)color_offset, 0, &(nnw.u16[0]));
		color_table.v = simd_256bit::op_and(nnw.v, simd_256bit::op_set16(0x7fff));
		// Color[0] must be transparent.
		// (Related by page 127 of technical manual.)
		// 20250123 K.O
		color_table.u16[0] = 0x8000; 
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(16) uint16_8_t nnhl;
			__DECL_ALIGNED(8) uint8_8_t nnh;
			
			load_8bytes_from_pattern_ram(ram_offset, (uint32_t)yy, &(nnh.u8[0]));
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
				nnhl.u8[j]     = nnh.u8[i];
				nnhl.u8[j + 1] = nnh.u8[i];
			}
			__DECL_VECTORIZED_LOOP
			for(size_t j = 1; j < 16; j += 2) {
				nnhl.u8[j] >>= 4;
			}
			nnhl.v = simd_128bit::op_and(nnhl.v, simd_128bit::op_set8(0x0f));
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++ ) {
				pixel_hl.u16[xx] = color_table.u16[nnhl.u8[xx]];
			}
			tbuf[yy].v = pixel_hl.v;
		}
	}
	// Rotate
	switch(rot & 7) { // ROT1, ROT0
	case 0:
		// 0deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
			sbuf[yy].v = tbuf[yy].v;
		}
		break;
	case 1:
		// 180deg, mirror
		for(int yy = 0; yy < 16; yy++) {
			sbuf[yy].v = tbuf[15 - yy].v;
		}
		break;
	case 2:
		// 0deg, mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(32) uint16_16_t tmpvec;
			tmpvec.v = tbuf[yy].v;
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy].u16[xx] = tmpvec.u16[15 - xx];
			}
		}
		break;
	case 3:
		// 180deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(32) uint16_16_t tmpvec;
			tmpvec.v = tbuf[15 - yy].v;
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				sbuf[yy].u16[xx] = tmpvec.u16[15 - xx];
			}
		}
		break;
	case 4:
		// 270deg, mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(32) uint16_16_t tmpvec;
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec.u16[xx] = tbuf[xx].u16[yy];
			}
			sbuf[yy].v = tmpvec.v;
		}
		break;
	case 5:
		// 90deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(32) uint16_16_t tmpvec;
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec.u16[xx] = tbuf[xx].u16[15 - yy];
			}
			sbuf[yy].v = tmpvec.v;
		}
		break;
	case 6:
		// 270deg, not mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(32) uint16_16_t tmpvec;
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec.u16[xx] = tbuf[15 - xx].u16[yy];
			}
			sbuf[yy].v = tmpvec.v;
		}
		break;
	case 7:
		// 90deg, mirror
		for(int yy = 0; yy < 16; yy++) {
			__DECL_ALIGNED(32) uint16_16_t tmpvec;
			__DECL_VECTORIZED_LOOP
			for(int xx = 0; xx < 16; xx++) {
				tmpvec.u16[xx] = tbuf[15 - xx].u16[15 - yy];
			}
			sbuf[yy].v = tmpvec.v;
		}
		break;
	}

	// Zoom and rendering.
	__UNLIKELY_IF(d_vram == NULL) return; // Skip if VRAM not exists.
	uint32_t noffset = (draw_page1) ? 0x40000 : 0x60000;
	uint32_t vpaddr = ((rx + (ry * 256)) << 1) & 0x1ffff;

	__DECL_ALIGNED(32) uint16_16_t source;
	__DECL_ALIGNED(32) uint16_16_t lbuf;			// Holizonal line buffer
	__DECL_ALIGNED(32) uint16_16_t color_values;	// Pixel color values
	__DECL_ALIGNED(32) uint16_16_t maskbuf_posi;	// Mask values; 0xffff when transparent.
	__DECL_ALIGNED(32) uint16_16_t maskbuf_nega;	// Mask negative values; 0x00 when transparent.
	__DECL_ALIGNED(32) const simde__m256i mask_transparent = simd_256bit::op_set16(0x8000);
	__DECL_ALIGNED(32) const simde__m256i mask_value = simd_256bit::op_set16(0x7fff);
	__DECL_ALIGNED(32) const simde__m256i zero_value = simd_256bit::op_clear();
	
	/* Re-Implement new Logic */
	// Get first line

	// For second line.
	__DECL_ALIGNED(32) uint16_16_t lbuf2;				// Holizonal line buffer
	__DECL_ALIGNED(32) uint16_16_t color_values2;		// Pixel color values
	__DECL_ALIGNED(32) uint16_16_t maskbuf_posi2;		// Mask values; 0xffff when transparent.
		
	__DECL_ALIGNED(16) uint16_8_t  zoomed_mask_posi;
	__DECL_ALIGNED(16) uint16_8_t  zoomed_mask_nega;
	__DECL_ALIGNED(16) uint16_8_t zoomed_value;

	for(int yy = 0, yy2 = 0; yy < 16;  yy += __ystep, yy2++) {
		int yoff = (yy2 + ry) & 0x1ff;

		// From TownsSprite::Render(), sprite.cpp, Tsugaru (from FM-Towns Technical manual):
		// [2] pp.368 (Sprite BIOS AH=00H) tells, the top 2-lines of the VRAM page are VRAM-clear data.
		//     So, apparently it is possible to clear the sprite page with non-0x8000 values.
		if((yoff < 256) && (yoff >= 2)) { // From Tsugaru.
			// Get From source VRAM
			vpaddr = ((__xstart + (yoff << 8)) << 1) & 0x1ffff;
			if(is_halfx) {
				source.u16_8.array[1].v = simd_128bit::op_clear();
			}
			int _gwords = (is_halfx) ? 8 : 16;
			if((vpaddr + (_gwords << 1)) > 0x20000) {
				__UNLIKELY_IF(vpaddr >= 0x1ffff) {
					break;
				}
				_gwords = 0x20000 - ((int)vpaddr);
				_gwords >>= 1;
				if(is_halfx) {
					__UNLIKELY_IF(_gwords > 8) {
						_gwords = 8;
					}
				} else {
					__UNLIKELY_IF(_gwords > 16) {
						_gwords = 16;
					}
				}
				__UNLIKELY_IF(_gwords <= 0) {
					break;
				}
			}
			d_vram->get_vram_to_buffer(vpaddr + noffset, &(source.u16[0]), _gwords);
			// Get first line of SPRITE.
			lbuf.v = mask_transparent;
			lbuf.v = simd_256bit::load_aligned(&(sbuf[yy].v));
			__UNLIKELY_IF((__xend != __max_width) || (__xstart2 != 0)) {
				lbuf = shift_vector_data(__xstart2, __xend, __xstep - 1, lbuf);
			}
			// Make MASK
			// if lbuf.u16[foo] < 0x8000 (== plus), set 0xffff .
//			maskbuf_posi.v = simd_256bit::op_greater16(lbuf.v, zero_value);
			maskbuf_posi.v = lbuf.v;
			maskbuf_posi.v = simd_256bit::op_and(maskbuf_posi.v, simd_256bit::op_set16(0x8000));
			maskbuf_posi.v = simd_256bit::op_equals16(maskbuf_posi.v, simd_256bit::op_set16(0x0000));
			
			color_values.v = simd_256bit::op_and(lbuf.v, mask_value);
			color_values.v = simd_256bit::op_and(color_values.v, maskbuf_posi.v);
			if(is_halfy) {
				// Make Mask 2nd line and zoom a data
				lbuf2.v = mask_transparent;
				lbuf2.v = simd_256bit::load_aligned(&(sbuf[yy + 1].v));
				__UNLIKELY_IF((__xend != __max_width) || (__xstart2 != 0)) {
					lbuf2 = shift_vector_data(__xstart2, __xend, __xstep - 1, lbuf2);
				}
				//maskbuf_posi2.v = simd_256bit::op_greater16(lbuf2.v, zero_value);
				maskbuf_posi2.v = lbuf2.v;
				maskbuf_posi2.v = simd_256bit::op_and(maskbuf_posi2.v, simd_256bit::op_set16(0x8000));
				maskbuf_posi2.v = simd_256bit::op_equals16(maskbuf_posi2.v, simd_256bit::op_set16(0x0000));
				color_values2.v = simd_256bit::op_and(lbuf2.v, mask_value);
				color_values2.v = simd_256bit::op_and(color_values2.v, maskbuf_posi2.v);
				// Zoom to buffer1
				maskbuf_posi.v = simd_256bit::op_or(maskbuf_posi.v, maskbuf_posi2.v);
				__DECL_ALIGNED(32) uint16_16_t tmp_color; 
				__DECL_VECTORIZED_LOOP
				for(size_t rx1 = 0; rx1 < 16; rx1++) {
					tmp_color.u16[rx1] = (color_values.u16[rx1] != 0) ? color_values.u16[rx1] : color_values2.u16[rx1]; 
				}
				color_values.v = simd_256bit::op_and(tmp_color.v, maskbuf_posi.v);
			}
			__LIKELY_IF(!(is_halfx)) {
				// Store without Zoom
				maskbuf_nega.v = simd_256bit::op_not(maskbuf_posi.v);
				source.v = simd_256bit::op_and(source.v, maskbuf_nega.v);
				source.v = simd_256bit::op_or(source.v, color_values.v);
				__UNLIKELY_IF(_gwords > 16) {
					_gwords = 16;
				}
				__LIKELY_IF(_gwords > 0) {
					d_vram->set_buffer_to_vram(vpaddr + noffset, &(source.u16[0]), _gwords);
				}
			} else { // Halfx
				// Make Mask
				zoomed_mask_posi.v = simd_128bit::op_clear();
				zoomed_value.v = simd_128bit::op_clear();
				__DECL_VECTORIZED_LOOP
				for(size_t rx3 = 0, rx2 = 0; rx3 < 8; rx2 += 2, rx3++) {
					uint16_t _lval = maskbuf_posi.u16[rx2];
					uint16_t _rval = maskbuf_posi.u16[rx2 + 1];
					zoomed_mask_posi.u16[rx3] = _lval | _rval;
				}
				__DECL_VECTORIZED_LOOP
				for(size_t rx3 = 0, rx2 = 0; rx3 < 8; rx2 += 2, rx3++) {
					uint16_t _lval = color_values.u16[rx2];
					uint16_t _rval = color_values.u16[rx2 + 1];
					zoomed_value.u16[rx3] =  (_lval != 0) ? _lval : _rval;
				}
				// Store with Zooming
				zoomed_value.v = simd_128bit::op_and(zoomed_value.v, zoomed_mask_posi.v);
				zoomed_mask_nega.v = simd_128bit::op_not(zoomed_mask_posi.v);
				source.u16_8.array[0].v = simd_128bit::op_and(source.u16_8.array[0].v, zoomed_mask_nega.v);
				source.u16_8.array[0].v = simd_128bit::op_or(source.u16_8.array[0].v, zoomed_value.v);
				
				__UNLIKELY_IF(_gwords > 8) {
					_gwords = 8;
				}
				__LIKELY_IF(_gwords > 0) {
					d_vram->set_buffer_to_vram(vpaddr + noffset, &(source.u16[0]), _gwords);
				}
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
		reg06_wrote = true;
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
		val = get_tvram_enabled(0x80);
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
	morph_address(addr, false);
	return pattern_ram[addr];
}

uint32_t TOWNS_SPRITE::read_memory_mapped_io16w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	pair16_t n;
	morph_address(addr, false);

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
	morph_address(addr, false);

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
	morph_address(addr, true);
	pattern_ram[addr] = data;
	return;
}

void TOWNS_SPRITE::write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	morph_address(addr, true);

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
	morph_address(addr, true);

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
	my_stprintf_s(sstr, 127, _T("TEXT VRAM:%s \n\n"), (tvram_enabled) ? _T("WROTE") : _T("NOT WROTE"));
	my_tcscat_s(regstr, 1024, sstr);

	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 127, _T("SPRITE:%s LOT=%d NUM=%d\nHOFFSET=%d VOFFSET=%d DISP_PAGE=%d\n")
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
	case EVENT_CLEAR_VRAM_COMPLETED:
		#if 0
		if(sprite_enabled) {
			uint16_t lot = reg_index & 0x3ff;
			render_num = 1024 - lot;
			sprite_usec = get_sprite_usec();
			//if(render_num > max_sprite_per_frame) {
			//	render_num = max_sprite_per_frame;
			//}
			sprite_busy = true;
			__LIKELY_IF(render_num > 0) {
				event_callback(EVENT_RENDER, 0);
			}
			if(render_num > 0) {
				register_event(this, EVENT_RENDER, sprite_usec, true, &event_busy);
			} else {
				sprite_busy = false;
			}
		}
		#endif
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
		//if(render_num > max_sprite_per_frame) {
		//	render_num = max_sprite_per_frame;
		//}
		if(reg06_wrote) {
			// Manually set reg06.
			disp_page1 = ((reg_data[6] & 0x80) != 0) ? true : false;
			reg06_wrote = false;
		} else {
			disp_page1 = !(disp_page1);
		}
		draw_page1 = !(disp_page1);
		uint32_t noffset = (draw_page1) ? 0x40000 : 0x60000;
		__LIKELY_IF(d_vram != NULL){
			__DECL_ALIGNED(16) uint8_t headbuf[256 * 2 * 2];			   
			d_vram->lock();
			uint8_t *p = d_vram->get_vram_address(noffset);
			__LIKELY_IF(p != NULL) {

				// From TownsSprite::Render(), sprite.cpp, Tsugaru (from FM-Towns Technical manual):
				// [2] pp.368 (Sprite BIOS AH=00H) tells, the top 2-lines of the VRAM page are VRAM-clear data.
				//     So, apparently it is possible to clear the sprite page with non-0x8000 values.
				// 1st. get 2lines of head.
				for(int xx = 0; xx < (256 * 2 * 2); xx++) {
					headbuf[xx] = p[xx];
				}
				// 2nd. Fill buffer
				for(int yy = 2; yy < (256 - 2); yy += 2) {
					uint8_t *pp = d_vram->get_vram_address(noffset + (yy * 256 * 2));
					__LIKELY_IF(pp != NULL) {
						for(int xx = 0; xx < (256 * 2 * 2); xx++) {
							pp[xx] = headbuf[xx];
						}
					}
				}
			}
			d_vram->unlock();
		}
	}
}
void TOWNS_SPRITE::event_frame()
{
//	frame_out = true;

	if(!(reg_spen)) {
		sprite_enabled = false;
		return;
	}
	sprite_enabled = true;
	check_and_clear_vram();
}

void TOWNS_SPRITE::event_pre_frame()
{
	// Note:
	// By After Burner II (and others),
	// Assume sprite is stopping at VBLANK (??).
	// So, below tricky logic seems to be implemented.
	// - 20250128 K.O
	clear_event(this, event_busy);
	sprite_busy = false;
	frame_out = true;
}


// Q: Is changing pages syncing to Frame?
// ToDo: Implement VRAM.
void TOWNS_SPRITE::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_TOWNS_SPRITE_TEXT_RENDER:
		if(need_render_text) {
			need_render_text = false;
			render_text();
		}
		break;
	case SIG_TOWNS_SPRITE_VSYNC: //
		// Same value.
		if((sprite_enabled) && (frame_out)) {
			frame_out = false;
			__LIKELY_IF(render_num > 0) {
				// Note: Sprite don't limit by VSYNC timing,
				// If sprites are too many to render within a frame,
				// (maybe stopping at VBLANK) and continueing at next frame.
				// - 20250128 K.O
				sprite_busy = true;
				sprite_usec = get_sprite_usec(render_num);
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
		return (disp_page1) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_SPRITE_BANK:
		__LIKELY_IF(sprite_enabled) {
			return (draw_page1) ? 0xffffffff : 0; // Not drawn page == Displaying page
		} else {
			return ((reg_data[6] & 0x80) != 0) ? 0xffffffff : 0; // Value of DP1
		}
		break;
	case SIG_TOWNS_SPRITE_FRAME_IN:
		return (frame_out) ? 0x00000000 : 0xffffffff;
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

#define STATE_VERSION	8

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
	state_fio->StateValue(reg06_wrote);
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

	state_fio->StateValue(render_num);

	state_fio->StateValue(max_sprite_per_frame);
	state_fio->StateValue(tvram_enabled);
	state_fio->StateValue(need_render_text);
	state_fio->StateValue(is_older_sprite);
	state_fio->StateValue(sprite_usec);

	state_fio->StateValue(event_busy);

	return true;
}

}
