/*
 * Common source code project -> FM-7 -> Display -> Vram access
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Sep 27, 2015 : Split from display.cpp .
 */

#include "vm.h"
#include "fm7_display.h"
#if defined(_FM77L4)
#include "../hd46505.h"
#endif
#include "../../config.h"
#include "../../types/util_video.hpp"

//extern config_t config;

namespace FM7 {

void DISPLAY::clear_display(int dmode, int w, int h)
{
#if defined(FIXED_FRAMEBUFFER_SIZE)
	if((dmode != DISPLAY_MODE_8_400L) && (dmode != DISPLAY_MODE_1_400L)) {
		h = h * 2;
	}
#endif
	for(int yy = 0; yy < h; yy++) {
		scrntype_t *p;
		p = emu->get_screen_buffer(yy);
		if(p != NULL) {
			memset(p, 0x00, sizeof(scrntype_t) * w);
		}
	}
}

void DISPLAY::draw_window(int dmode, int y, int begin, int bytes, bool window_inv, bool scan_line)
{
//	_render_command_data_t cmd;
//	bool use_cmd = false;
	int xzoom = 1;
	uint32_t _offset_base = 0x4000;
	int planes;
	const size_t shift = 5;
	int width;
	switch(dmode) {
	case DISPLAY_MODE_8_200L:
		_offset_base = 0x4000;
//		use_cmd = true;
		planes = 3;
		width = 80;
		break;
#if defined(_FM77AV_VARIANTS)
	case DISPLAY_MODE_4096:
		_offset_base = 0x2000;
		xzoom = 2;
		planes = 12;
		width = 40;
		break;
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77AV40)
	case DISPLAY_MODE_8_400L:
		_offset_base = 0x8000;
		planes = 3;
		width = 80;
		break;
#    if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	case DISPLAY_MODE_256k:
		_offset_base = 0x2000;
		xzoom = 2;
		planes = 20;
		width = 40;
		break;
#    endif
#  endif
#endif
	default:
		return;
		break;
	}
	scrntype_t *p = NULL;
	scrntype_t *pp = NULL;

	uint16_8_t* r_table = &(bit_trans_table_1[0]); // R
	uint16_8_t* g_table = &(bit_trans_table_0[0]); // G
	uint16_8_t* b_table = &(bit_trans_table_2[0]); // B
	const uint32_t base_address_b = (0 * _offset_base) + yoff_d;
	const uint32_t base_address_r = (1 * _offset_base) + yoff_d;
	const uint32_t base_address_g = (2 * _offset_base) + yoff_d;
	const uint32_t voffset = y * width;
	const uint32_t address_mask = _offset_base - 1;
	const uint32_t offset_mask = _offset_base - 1;
	const bool is_render_rgb[4] = { !(multimode_dispflags[1]), !(multimode_dispflags[2]), !(multimode_dispflags[0]), false };
	scrntype8_t* palette_ptr = (scrntype8_t*) dpalette_pixel;
	#if defined(USE_GREEN_DISPLAY)
	if(use_green_monitor) {
		palette_ptr = (scrntype8_t*)dpalette_pixel_green;
	}
	#endif
	
	if(dmode == DISPLAY_MODE_8_400L) {
#if defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77AV40)
		p = emu->get_screen_buffer(y);
		if(p == NULL) return;
		Render8Colors_Line<const size_t>(&(p[begin * 8]), NULL, gvram_shadow,
							begin, width,
							palette_ptr,
							r_table, g_table, b_table,
							/*scan_line*/ false,
							base_address_r, base_address_g, base_address_b,
							voffset, address_mask, offset_mask,
							is_render_rgb, shift, bytes);
#endif
	} else {
#if !defined(FIXED_FRAMEBUFFER_SIZE)
		p = emu->get_screen_buffer(y);
		pp = NULL;
#else
		p = emu->get_screen_buffer(y << 1);
		pp = emu->get_screen_buffer((y << 1) + 1);
#endif
		if(p == NULL) return;
		switch(dmode) {
		case DISPLAY_MODE_8_200L:
			{
				Render8Colors_Line<const size_t>(&(p[begin * 8]), (pp == NULL) ? NULL : &(pp[begin * 8]), gvram_shadow,
									begin, width,
									palette_ptr,
									r_table, g_table, b_table,
									(pp != NULL) ? scan_line : false,
									base_address_r, base_address_g, base_address_b,
									voffset, address_mask, offset_mask,
									is_render_rgb, shift, bytes);
			}
			break;
#if defined(_FM77AV_VARIANTS)
		case DISPLAY_MODE_4096:
			{
				if(pp != NULL) pp = &(pp[begin]);
				p = &(p[begin]);
				uint32_t yoff = y * 40 + begin;
				for(uint32_t x = begin; x < (begin + bytes); x++) {
					GETVRAM_4096(yoff, p, pp, window_inv, scan_line);
#    if defined(FIXED_FRAMEBUFFER_SIZE)
					p += 16;
					if(pp != NULL) pp += 16;
#    else
					p += 8;
#    endif
					yoff++;
				}
			}
			break;
#    if defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77AV40)
		case DISPLAY_MODE_256k:
			{
				uint32_t yoff = y * 40;
				if(pp != NULL) pp = &(pp[begin]);
				p = &(p[begin]);
				for(uint32_t x = begin; x < (begin + bytes); x++) {
					GETVRAM_256k(yoff + x, p, pp, scan_line);
#      if defined(FIXED_FRAMEBUFFER_SIZE)
					p += 16;
					if(pp != NULL) pp += 16;
#      else
					p += 8;
#      endif
				}
			}
			break;
#    endif
#  endif
		default:
			break;
		}
	}			
}

inline void DISPLAY::zoomed_store(SCRNTYPE8_SIMD data, scrntype_t* p, scrntype_t* px, const bool scan_line)
{
	__UNLIKELY_IF(p == NULL) return;
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class _d;
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmp_dd2[2];
	_d = data;
	// Zoom Horiz.
	__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 0; i < 8; i += 2, j++) {
		scrntype_t __tmp = _d.at<scrntype_t>(j);
		tmp_dd2[0].set<scrntype_t>(    i, __tmp);
		tmp_dd2[0].set<scrntype_t>(i + 1, __tmp);
	}
	__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 4; i < 8; i += 2, j++) {
		scrntype_t __tmp = _d.at<scrntype_t>(j);
		tmp_dd2[1].set<scrntype_t>(    i, __tmp);
		tmp_dd2[1].set<scrntype_t>(i + 1, __tmp);
	}
	for(size_t i = 0, j = 0; i < 2; i++, j += 8) {
		tmp_dd2[i].unalign_store(&(p[j]));
	}
	if(px != NULL) {
		if(scan_line) {
/* Fancy scanline */
#if defined(_RGB888) || defined(_RGBA888)
			const size_t __shift = 3;
#else
			const size_t __shift = 2;
#endif
			for(size_t i = 0; i < 2; i++) {
				tmp_dd2[i] >>= __shift;
			}
			__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class vmask((scrntype_t)RGBA_COLOR(31, 31, 31, 255));
			for(size_t i = 0; i < 2; i++) {
				tmp_dd2[i] &= vmask;
			}
		}
		for(size_t i = 0, j = 0; i < 2; i++, j += 8) {
			tmp_dd2[i].unalign_store(&(px[j]));
		}
	}
}
#if defined(_FM77L4)
void DISPLAY::draw_77l4_400l(bool ff)
{
	bool renderf = false;
	uint32_t naddr;
	uint8_t bitcode;
	uint8_t charcode;
	uint8_t attr_code;
	scrntype_t on_color;
	int xlim, ylim;
	bool do_green;
	uint8_t *regs = l4crtc->get_regs();
	cursor_start = (int)(regs[10] & 0x1f);
	cursor_end = (int)(regs[11] & 0x1f);
	cursor_type = (int)((regs[10] & 0x60) >> 5);
	text_xmax = (int)((uint16_t)regs[1] << 1);
	text_lines = (int)((regs[9] & 0x1f) + 1);
	text_ymax = (int)(regs[6] & 0x7f);
	int yoff = 0;
	scrntype_t *p;
	for(int y =0; y < 400; y+= 8) {
		renderf = false;
		if((y & 0x0f) == 0) {
			for(int yy = 0; yy < 16; yy++) renderf |= vram_draw_table[y + yy];
			renderf = renderf | ff;
			if(renderf) {
				for(int yy = 0; yy < 16; yy++) vram_draw_table[y + yy] = true;
			}
		}
		if(use_green_monitor) {
			for(int yy = 0; yy < 8; yy++) {
				if(!(vram_draw_table[y + yy] | ff)) continue;
				vram_draw_table[y + yy] = false;
				p = emu->get_screen_buffer(y + yy);
				if(p == NULL) continue;
				yoff = (y + yy) * 80;
				for(int x = 0; x < 10; x++) {
					for(int ii = 0; ii < 8; ii++) {
						GETVRAM_1_400L_GREEN(yoff + ii, p);
						p += 8;
					}
					yoff += 8;
				}
			}
			do_green = true;
		} else {
			for(int yy = 0; yy < 8; yy++) {
				if(!(vram_draw_table[y + yy] | ff)) continue;
				vram_draw_table[y + yy] = false;
				p = emu->get_screen_buffer(y + yy);
				if(p == NULL) continue;
				yoff = (y + yy) * 80;
				for(int x = 0; x < 10; x++) {
					for(int ii = 0; ii < 8; ii++) {
						GETVRAM_1_400L(yoff + ii, p);
						p += 8;
					}
					yoff += 8;
				}
			}
			do_green = false;
		}
		// Draw Text
		if(renderf) {
			bool reverse;
			bool display_char;
			int raster;
			bool cursor_rev;
			uint8_t bitdata;
			if(text_width40) {
				xlim = 40;
			} else {
				xlim = 80;
			}
				
			for(int x = 0; x < xlim; x++) {
				naddr = (text_start_addr.w.l + ((y / text_lines) * text_xmax + x) * 2) & 0x0ffe;
				charcode = text_vram[naddr];
				attr_code = text_vram[naddr + 1];
						
				on_color = GETVRAM_TEXTCOLOR(attr_code, do_green);
					
				display_char = ((attr_code & 0x10) == 0);
				reverse = ((attr_code & 0x08) != 0);
					
				for(int yy = 0; yy < 16; yy++) {
					raster = y % text_lines;
					bitdata = 0x00;
					p = emu->get_screen_buffer(y + yy);
					if(p == NULL) continue;
					if((raster < 16) && (display_char || text_blink)) {
						bitdata = subsys_cg_l4[(uint32_t)charcode * 16 + (uint32_t)raster];
					}
					cursor_rev = false;
					if((naddr == (uint32_t)(cursor_addr.w.l)) && (cursor_type != 1) &&
					   (text_blink || (cursor_type == 0))) {
						if((raster >= cursor_start) && (raster <= cursor_end)) {
							cursor_rev = true;
						}
					}
					bitdata = GETVRAM_TEXTPIX(bitdata, reverse, cursor_rev);
					if(bitdata != 0) {
						if(text_width40) {
							scrntype_t *pp = &(p[x * 2]); 
							for(int ii = 0; ii < 8; ii++) {
								if((bitdata & 0x80) != 0) {
									p[0] = on_color;
									p[1] = on_color;
								}
								bitdata <<= 1;
								p += 2;
							}										
						} else {
							scrntype_t *pp = &(p[x * 2]); 
							for(int ii = 0; ii < 8; ii++) {
								if((bitdata & 0x80) != 0) {
									p[0] = on_color;
								}
								bitdata <<= 1;
								p += 1;
							}										
						}
					}
				}
			}
		}
	}
}
#endif

void DISPLAY::draw_screen()
{
	uint16_t wx_begin = -1, wx_end = -1, wy_low = 1024, wy_high = -1;
	bool scan_line = config.scan_line;
	bool ff = force_update;
	int dmode = display_mode;
	yoff_d = 0;
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
		if(_flag != window_opened) {
			vram_wrote_shadow = true;
		}
	}
#endif
#if defined(_FM77AV_VARIANTS)
	yoff_d2 = 0;
	yoff_d1 = 0;
#else
	//if(!(vram_wrote_shadow)) return;
	yoff_d1 = yoff_d2 = offset_point;
#endif
	int ylines;
	int xpixels;
	switch(dmode) {
	case DISPLAY_MODE_8_200L:
		xpixels = 640;
		ylines = 200;
		break;
	case DISPLAY_MODE_1_400L:
	case DISPLAY_MODE_8_400L:
		xpixels = 640;
		ylines = 400;
		break;
	default:
		xpixels = 320;
		ylines = 200;
		break;
	}
# if !defined(FIXED_FRAMEBUFFER_SIZE)
	emu->set_vm_screen_size(xpixels, ylines, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
# endif
	emu->set_vm_screen_lines(ylines);
	if(!crt_flag) {
		if(crt_flag_bak) {
			clear_display(dmode, xpixels, ylines);
		}
		crt_flag_bak = crt_flag;
		if(ff) force_update = false;
		return;
	}
	crt_flag_bak = crt_flag;
	if(!(vram_wrote_shadow | ff)) return;
	vram_wrote_shadow = false;

	int wpixels = xpixels >> 3;
#if defined(_FM77L4)
	if(dmode == DISPLAY_MODE_1_400L) {
		draw_77l4_400l(ff);
		if(ff) force_update = false;
		return;
	}
#endif	
	for(int y = 0;  y < ylines; y += 8) {
		for(int yy = 0; yy < 8; yy++) {
			if(!(vram_draw_table[y + yy] | ff)) continue;
			vram_draw_table[y + yy] = false;
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77AV40)
				int dpage;
				dpage = vram_display_block;
				bool window_inv = false;
#    if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				if((window_opened && (wy_low <= (y + yy)) && (wy_high > (y + yy)))
				   && (dmode != DISPLAY_MODE_256k)) {
					if((wx_begin > 0) && (wx_begin < wx_end) && (wx_begin < wpixels)) {
						yoff_d = (dpage != 0) ? 0x18000 : 0x00000;
						if(display_page_bak == 1) yoff_d += 0xc000;
						draw_window(dmode, yy + y, 0, wx_begin,
									false, scan_line);
						yoff_d = (dpage != 0) ? 0x00000 : 0x18000;
						if(display_page_bak == 1) yoff_d += 0xc000;
						draw_window(dmode, yy + y,
									wx_begin, ((wx_end >= wpixels) ? wpixels : wx_end) - wx_begin,
									true, scan_line);
						if(wx_end < wpixels) {
							yoff_d = (dpage != 0) ? 0x18000 : 0x00000;
							if(display_page_bak == 1) yoff_d += 0xc000;
							draw_window(dmode, yy + y, wx_end,  wpixels - wx_end,
										false, scan_line);
						}
					} else {
						yoff_d = (dpage != 0) ? 0x00000 : 0x18000;
						if(display_page_bak == 1) yoff_d += 0xc000;
						draw_window(dmode, yy + y, 0, wx_end,
									false, scan_line);
						if(wx_end < wpixels) {
							yoff_d = (dpage != 0) ? 0x18000 : 0x00000;
							if(display_page_bak == 1) yoff_d += 0xc000;
							draw_window(dmode, yy + y, wx_end , wpixels - wx_end,
										true, scan_line);
						}
					}						
				} else
#    endif
				{
					
					yoff_d = (dpage != 0) ? 0x18000 : 0x00000;
					if(display_page_bak == 1) yoff_d += 0xc000;
					draw_window(dmode, yy + y, 0, wpixels, false, scan_line);
				}
				// Copy line
#elif defined(_FM77AV_VARIANTS)
				yoff_d = 0;
				if(display_page_bak == 1) yoff_d += 0xc000;
				draw_window(dmode, yy + y, 0, wpixels, false, scan_line);
#else
				yoff_d = 0;
				draw_window(dmode, yy + y, 0, wpixels, false, scan_line);
#endif
		}
	}
	if(ff) force_update = false;
	return;
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


#if defined(_FM77L4)
scrntype_t DISPLAY::GETVRAM_TEXTCOLOR(uint8_t attr, bool do_green)
{
	int color = attr & 0x07;
	int r, g, b;

	static const int green_g_table[16] = {0, 24, 48, 64, 80, 96, 112, 128,
										  140, 155, 175, 186, 210, 220, 240, 255};
	if(do_green) {
		if((attr & 0x20) != 0) color += 8;
		r = b = 0;
		g = green_g_table[color];
		if(color >= 10) {
			r = (color - 9) * 16;
			b = (color - 9) * 16;
		}
	} else {
		if((attr & 0x20) != 0) {
			g = ((color & 4) != 0) ? 255 : 0;
			r = ((color & 2) != 0) ? 255 : 0;
			b = ((color & 1) != 0) ? 255 : 0;
		} else {
			g = ((color & 4) != 0) ? 128 : 0;
			r = ((color & 2) != 0) ? 128 : 0;
			b = ((color & 1) != 0) ? 128 : 0;
		}
	}
	return RGBA_COLOR(r, g, b, 255);
}

uint8_t DISPLAY::GETVRAM_TEXTPIX(uint8_t bitdata, bool reverse, bool cursor_rev)
{
	uint8_t ret = bitdata;
	if(reverse) {
		ret = (uint8_t)(~ret);
	}
	if(cursor_rev) {
	    ret = (uint8_t)(~ret);
	}
	return ret;
}

void DISPLAY::GETVRAM_1_400L(int yoff, scrntype_t *p)
{
	uint8_t pixel;
	__UNLIKELY_IF(p == NULL) return;
	yoff_d = yoff & 0x7fff;
	pixel = gvram_shadow[yoff_d];
	uint16_8_t *ppx = (uint16_8_t *)___assume_aligned(&(bit_trans_table_0[0]), sizeof(uint16_vec8_t));
	
	__DECL_ALIGNED(16) simd_uint16_8 tmp_dl;
	__DECL_ALIGNED(16) simd_uint16_8 tmp_dr;
	__DECL_ALIGNED(16) const simd_uint16_8 mask((uint16_t)0x0001);
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmpdd;
	tmp_dl.align_load(&(ppx[pixel >> 4]));
	tmp_dr.align_load(&(ppx[pixel & 0x0f]));	

	// Note: SSE2 OPs is for LITTLE ENDIAN, not for BIG ENDIAN. - 20260518 K.O
	tmp_dl = simd_128bit::op_rshift_bytes<8>(tmp_dl.data().v);
	tmp_dl |= tmp_dr;
	tmp_dl >>= 5;
	tmp_dl &= mask;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.set_unsafe(i, dpalette_pixel[tmp_dl.at<uint16_t>(i)]); 
	}
	tmpdd.store(p);
}

void DISPLAY::GETVRAM_1_400L_GREEN(int yoff, scrntype_t *p)
{
	uint8_t pixel;
	__UNLIKELY_IF(p == NULL) return;
	yoff_d = yoff & 0x7fff;
	pixel = gvram_shadow[yoff_d];
	uint16_8_t *ppx = (uint16_8_t *)___assume_aligned(&(bit_trans_table_0[0]), sizeof(uint16_vec8_t));
	
	__DECL_ALIGNED(16) simd_uint16_8 tmp_dl;
	__DECL_ALIGNED(16) simd_uint16_8 tmp_dr;
	__DECL_ALIGNED(16) const simd_uint16_8 mask((uint16_t)0x0001);
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmpdd;
	tmp_dl.align_load(&(ppx[pixel >> 4]));
	tmp_dr.align_load(&(ppx[pixel & 0x0f]));	

	// Note: SSE2 OPs is for LITTLE ENDIAN, not for BIG ENDIAN. - 20260518 K.O
	tmp_dl = simd_128bit::op_rshift_bytes<8>(tmp_dl.data().v);
	tmp_dl |= tmp_dr;
	tmp_dl >>= 5;
	tmp_dl &= mask;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.set_unsafe(i, dpalette_pixel_green[tmp_dl.at<uint16_t>(i)]); 
	}
	tmpdd.store(p);

}
#endif


#if defined(_FM77AV_VARIANTS)
inline uint16_8_t DISPLAY::GETVRAM_4bit_from_vram(uint32_t base)
{
	uint8_t _d[4];
	__DECL_ALIGNED(16) simd_uint16_8 _r;
	uint32_t off1 = yoff_d1 + (uint32_t)base;
	uint32_t off2 = yoff_d2 + (uint32_t)base;

	_d[0] = gvram_shadow[off1 + 0x00000];
	_d[1] = gvram_shadow[off1 + 0x02000];
	
	_d[2] = gvram_shadow[off2 + 0x0c000];
	_d[3] = gvram_shadow[off2 + 0x0e000];

	uint16_8_t *vp[4] = {
		___assume_aligned(&(bit_trans_table_0[0]), sizeof(uint16_8_t)),
		___assume_aligned(&(bit_trans_table_1[0]), sizeof(uint16_8_t)),
		___assume_aligned(&(bit_trans_table_2[0]), sizeof(uint16_8_t)),
		___assume_aligned(&(bit_trans_table_3[0]), sizeof(uint16_8_t)),
	};
	_r =  Get4PixelsFromRGBI(_d[0], _d[1], _d[2], _d[3], vp[0], vp[1], vp[2], vp[3], true);
	_r |= Get4PixelsFromRGBI(_d[0], _d[1], _d[2], _d[3], vp[0], vp[1], vp[2], vp[3], false);
	return _r.data();
}
	
void DISPLAY::GETVRAM_4096(const uint32_t yoff, scrntype_t *p, scrntype_t *px,
						   const bool window_inv,
						   const bool scan_line)
{
	if(p == NULL) return;
	__DECL_ALIGNED(16) simd_uint16_8 pixels;
	
	yoff_d1 = yoff;
	yoff_d2 = yoff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int dpage = (vram_display_block != 0) ? 1 : 0;
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
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmp_dd;
	
	__DECL_ALIGNED(16) simd_uint16_8 tmp_r((uint16_t)0);
	__DECL_ALIGNED(16) simd_uint16_8 tmp_g((uint16_t)0);
	__DECL_ALIGNED(16) simd_uint16_8 tmp_b((uint16_t)0);

	if(!(multimode_dispflags[0])) {
		tmp_b = GETVRAM_4bit_from_vram(__offset_b);
		// ToDo: Big Endian.
		tmp_b >>= 4;
	}
	if(!(multimode_dispflags[1])) {
		tmp_r = GETVRAM_4bit_from_vram(__offset_r);
		// ToDo: Big Endian.
	}
	if(!(multimode_dispflags[2])) {
		tmp_g = GETVRAM_4bit_from_vram(__offset_g);
		// ToDo: Big Endian.
		tmp_g <<= 4;
	}
	
	pixels  = tmp_b;
	pixels |= tmp_r;
	pixels |= tmp_g;

//	scrntype_vec8_t *dp = (scrntype_vec8_t*)tmp_dd;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmp_dd.set<scrntype_t>(i, analog_palette_pixel[pixels.at<uint16_t>(i)]);
	}
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	tmp_dd.unalign_store(p);
#else
	zoomed_store(tmp_dd.data().v, p, px, scan_line);
#endif	
}
#endif

/* ToDo: Support 16bytes table. */
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)

inline uint16_8_t DISPLAY::GETVRAM_6bit_from_vram(uint32_t base)
{
	// get data
	__DECL_ALIGNED(8) union {
		uint8_t b[8];
		uint64_t q;
	} _d_l, _d_r;
	uint32_t off1 = yoff_d1 + (uint32_t)base;
	uint32_t off2 = yoff_d2 + (uint32_t)base;

	_d_l.q = 0;
	_d_l.b[0] = gvram_shadow[off1 + 0x00000];
	_d_l.b[1] = gvram_shadow[off1 + 0x02000];
	
	_d_l.b[2] = gvram_shadow[off2 + 0x0c000];
	_d_l.b[3] = gvram_shadow[off2 + 0x0e000];
	
	_d_l.b[4] = gvram_shadow[off1 + 0x18000];
	_d_l.b[5] = gvram_shadow[off1 + 0x1a000];

	_d_r.q = _d_l.q & 0x0f0f0f0f0f0f0f0full;
	_d_l.q >>= 4;
	_d_l.q &= 0x0f0f0f0f0f0f0f0full;
	
	__DECL_ALIGNED(16) simd_uint16_8 _left, _right, __tmp;
	
	uint16_8_t* tp[6] = {
		(uint16_8_t*)___assume_aligned(&(bit_trans_table_0[0]), sizeof(uint16_8_t)),
		(uint16_8_t*)___assume_aligned(&(bit_trans_table_1[0]), sizeof(uint16_8_t)),
		(uint16_8_t*)___assume_aligned(&(bit_trans_table_2[0]), sizeof(uint16_8_t)),
		(uint16_8_t*)___assume_aligned(&(bit_trans_table_3[0]), sizeof(uint16_8_t)),
		(uint16_8_t*)___assume_aligned(&(bit_trans_table_4[0]), sizeof(uint16_8_t)),
		(uint16_8_t*)___assume_aligned(&(bit_trans_table_5[0]), sizeof(uint16_8_t))
	};
	// left
	_left.align_load(&(tp[0][_d_l.b[0]]));
	for(size_t i = 1; i < 6; i++) {
		__tmp.align_load(&(tp[i][_d_l.b[i]]));
		_left |= __tmp;
	}
	// right
	_right.align_load(&(tp[0][_d_r.b[0]]));
	for(size_t i = 1; i < 6; i++) {
		__tmp.align_load(&(tp[i][_d_r.b[i]]));
		_right |= __tmp;
	}
	// Move left nibble
	// Note: SSE2 OPs is for LITTLE ENDIAN, not for BIG ENDIAN. - 20260518 K.O
	_left = simd_128bit::op_rshift_bytes<8>(_left.data().v);
	_left |= _right;

	// Boost luminance if non-zero value.
	__DECL_ALIGNED(16) simd_uint16_8 _non_zero_mask((uint16_t)0x0003);
	__DECL_ALIGNED(16) simd_uint16_8 _zeroval((uint16_t)0x0000);
	__DECL_ALIGNED(16) simd_uint16_8 _cmpresult(_left);

	_cmpresult.not_equals_i16(_zeroval);
	_cmpresult &= _non_zero_mask;
	_left |= _cmpresult;
	return _left.data();
}
	
void DISPLAY::GETVRAM_256k(const uint32_t yoff, scrntype_t *p, scrntype_t *px, const bool scan_line)
{
	if(p == NULL) return;
	
	yoff_d1 = yoff;
	yoff_d2 = yoff;

	__DECL_ALIGNED(16) simd_uint16_8 _btmp((uint16_t)0);
	__DECL_ALIGNED(16) simd_uint16_8 _rtmp((uint16_t)0);
	__DECL_ALIGNED(16) simd_uint16_8 _gtmp((uint16_t)0);
//	if(mask & 0x01) {
	__LIKELY_IF(!(multimode_dispflags[0])) {
		// B
		_btmp = GETVRAM_6bit_from_vram(__offset_b);
	}
	__LIKELY_IF(!(multimode_dispflags[1])) {
		//if(mask & 0x02) {
		// R
		_rtmp = GETVRAM_6bit_from_vram(__offset_r);
	}
	
	__LIKELY_IF(!(multimode_dispflags[2])) {
		//if(mask & 0x04) {
		// G
		_gtmp = GETVRAM_6bit_from_vram(__offset_g);
	}

	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmp_dd;
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmp_dd.set(i, RGBA_COLOR((scrntype_t)(_rtmp.at<uint16_t>(i)), (scrntype_t)(_gtmp.at<uint16_t>(i)), (scrntype_t)(_btmp.at<uint16_t>(i)), 0xff));
	}
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	tmp_dd.unlign_store(p);
#else	
	zoomed_store(tmp_dd.data().v, p, px, scan_line);
#endif	
}
#endif

}
