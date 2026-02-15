/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.03.27 -

	[ FM-Towns CRTC / Renderer ]
	History: 2025.03.27 Split from crtc.cpp .
*/

#include "../../vm.h"
#include "../../../common.h"
#include "../../../types/simd.h"

#include "../crtc.h"
#include "./crtc_utils.h"

namespace FMTOWNS {


	
inline void TOWNS_CRTC::transfer_pixels(scrntype_t* dst, scrntype_t* src, int w)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (w <= 0)) return;
//	for(int i = 0; i < w; i++) {
//		dst[i] = src[i];
//	}
	my_memcpy(dst, src, w * sizeof(scrntype_t));
}

void TOWNS_CRTC::draw_screen()
{
	int trans = display_linebuf.load() & display_linebuf_mask;
	
	bool do_alpha = false; // ToDo: Hardware alpha rendaring.
	// Don't need Locking because Already locking from OSD::doDraw() .
	__UNLIKELY_IF(d_vram == nullptr) {
		return;
	}
	int lines = vst[trans];
	int width = hst[trans];

	//bool is_single_tmp = is_single_layer[trans];
	bool is_single_tmp = is_single_mode_for_standard(control_cache[trans]);
	// Will remove.
	__UNLIKELY_IF(lines <= 0) lines = 1;
	__UNLIKELY_IF(width <= 16) width = 16;

	__UNLIKELY_IF(lines > TOWNS_CRTC_MAX_LINES) lines = TOWNS_CRTC_MAX_LINES;
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	osd->set_vm_screen_size(width, lines, 1024, lines, 1024, 768);
	//out_debug_log(_T("%s RENDER WIDTH=%d HEIGHT=%d"), __FUNCTION__, width, lines);
	osd->set_vm_screen_lines(lines / 2);

	int yskip[2] = {0};
	int ycount[2] = {0};

	int disp_y = 0;

	// Note: Start origin must be from 0.
	yskip[0] = 0;
	yskip[1] = 0;

	for(int y = 0; y < lines; y++) {
		int real_mode[2] = { DISPMODE_NONE };
		int real_y[2] = {0};
		int prio[2] = {0, 1};
		bool do_render[2] = {false};
		// Clear buffers per every lines.

		// ToDo: Enable to raster scrolling. 20240104 K.O
		for(int l = 0; l < 2; l++) {
			int __y = ycount[l];
			int tmp_num = linebuffers[trans][__y].num[l];
			prio[l] = tmp_num & 1;
			
			do_render[l] = (linebuffers[trans][__y].crtout[l] != 0) ? true : false;
			__LIKELY_IF(do_render[l]) {
				real_y[l] = __y;
				real_mode[l] = (do_render[l]) ? linebuffers[trans][__y].mode[l] : DISPMODE_NONE;
			}
			__LIKELY_IF(yskip[l] <= y) {
				ycount[l]++;
			}
		}
		if((do_render[0]) || (do_render[1])) {
			disp_y++;
			memset(lbuffer1, 0x00, sizeof(lbuffer1));
			memset(abuffer1, 0xff, sizeof(abuffer1));
			memset(lbuffer0, 0x00, sizeof(lbuffer0));
			memset(abuffer0, 0xff, sizeof(abuffer0));
		}/* else {
			continue;
			}*/
		scrntype_t* pix_array[2] = {lbuffer0, lbuffer1};
		scrntype_t* alpha_array[2] = {abuffer0, abuffer1};
		
		bool do_mix[2] = {false};
		int bitshift[2] = {0};
		bool is_hloop[2] = {false};
		int rendered_words[2] = {0};
		if(is_single_tmp) {
			do_render[1] = false;
		}
		for(int l = 0; l < 2; l++) {
			bool is_transparent = false;
			int prio_l = (is_single_tmp) ? 0 : (prio[l] & 1); // Will Fix
			is_transparent = ((do_render[0]) && (do_render[1]) && (prio_l == 0)) ? true : false;			
			if(do_render[l]) {
				scrntype_t* pix = pix_array[prio_l];
				scrntype_t* alpha = alpha_array[prio_l];
				int yyy = real_y[l]; // ToDo: Reduce rendering costs.
				bitshift[prio_l] = linebuffers[trans][yyy].bitoffset[l];
				is_hloop[prio_l] = (linebuffers[trans][yyy].is_hloop[l] != 0) ? true : false;
				
				switch(real_mode[l] & ~(DISPMODE_DUP)) {
				case DISPMODE_256:
					if(prio_l == 0) {
						do_mix[0] = render_256(trans, pix, yyy, rendered_words[0]);
					}
					break;
				case DISPMODE_32768:
					do_mix[prio_l] = render_32768(trans, pix, alpha, yyy, l, is_transparent, do_alpha, rendered_words[prio_l]);
					break;
				case DISPMODE_16:
					do_mix[prio_l] = render_16(trans, pix, alpha, yyy, l, is_transparent, do_alpha, rendered_words[prio_l]);
					break;
				default:
					break;
				}
			}
		}
//		if(y == 128) {
//			out_debug_log(_T("MIX: %d %d RENDER: %d %d WIDTH:%d %d SCREEN_WIDTH:%d"), do_mix[0], do_mix[1], do_render[0], do_render[1], rendered_words[0], rendered_words[1], width);
//		}
		
		__LIKELY_IF((do_mix[0]) && (do_mix[1])) {
			if(bitshift[0] > bitshift[1]) {
				bitshift[0] = bitshift[0] - bitshift[1];
				bitshift[1] = 0;
			} else if(bitshift[1] > bitshift[0]) {
				bitshift[0] = 0;
				bitshift[1] = bitshift[1] - bitshift[0];
			} else {
				bitshift[0] = 0;
				bitshift[1] = 0;
			}
//			__UNLIKELY_IF(is_transparent[1]) {
//				memset(abuffer1, 0xff, sizeof(abuffer1));
//			}
			//__LIKELY_IF((rendered_words[0] > 0)) {
			//	make_prefetch(lbuffer0, sizeof(scrntype_t) * rendered_words[0]);
			//	make_prefetch(abuffer0, sizeof(scrntype_t) * rendered_words[0]);
			//}
			//__LIKELY_IF((rendered_words[1] > 0)) {
			//	make_prefetch(lbuffer1, sizeof(scrntype_t) * rendered_words[1]);
			//	make_prefetch(abuffer1, sizeof(scrntype_t) * rendered_words[1]);
			//}
			mix_screen(y, width, do_mix[0], do_mix[1], bitshift[0], bitshift[1], rendered_words[0], rendered_words[1], is_hloop[0], is_hloop[1]);
		} else {
			__LIKELY_IF(do_mix[0]) {
				//memset(abuffer0, 0xff, sizeof(abuffer0));
				//__LIKELY_IF((rendered_words[0] > 0)) {
				//	make_prefetch(lbuffer0, sizeof(scrntype_t) * rendered_words[0]);
				//	//make_prefetch(abuffer0, sizeof(scrntype_t) * rendered_words[0]);
				//}
				mix_screen(y, width, do_mix[0], false, bitshift[0], 0, rendered_words[0], 0, is_hloop[0], false);
			} else if(do_mix[1]) {
				//memset(abuffer1, 0xff, sizeof(abuffer1));
				//__LIKELY_IF((rendered_words[1] > 0)) {
				//	make_prefetch(lbuffer1, sizeof(scrntype_t) * rendered_words[1]);
				//	//make_prefetch(abuffer1, sizeof(scrntype_t) * rendered_words[1]);
				//}
				mix_screen(y, width, false, do_mix[1], 0, bitshift[1], 0, rendered_words[1], false, is_hloop[1]);
			}
			// ToDo: Clear VRAM?
		}


	}

	return;
}

bool TOWNS_CRTC::render_32768(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = (do_alpha) ? NULL : mask;
	
	__UNLIKELY_IF(pwidth <= 0) return false;
	
	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	csp_vector8<uint16_t> magx_tmp;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		magx_tmp.set(i, (magx + (i & 1)) / 2);
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2+ 16 * magx);
	if(width <= 0) return false;
	
	rendered_pixels = 0;
	if((pwidth * magx) > width) {
		__UNLIKELY_IF(magx < 1) {
			pwidth = width;
		} else {
			pwidth = width / magx;
			if((width % magx) > 1) {
				pwidth++;
			}
		}
	} else {
		__LIKELY_IF(magx > 0) {
			if((pwidth % magx) > 1) {
				pwidth = pwidth / magx + 1;
			} else {
				pwidth = pwidth / magx;
			}
		}
	}
	magx = magx / 2;
	__UNLIKELY_IF(magx < 1) return false;
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 0) return false;
	if(y == 128) {
		//out_debug_log("RENDER_32768 Y=%d LAYER=%d PWIDTH=%d WIDTH=%d DST=%08X MASK=%08X ALPHA=%d", y, layer, pwidth, width, dst, mask, do_alpha);
	}
	csp_vector8<uint16_t> pbuf;
	csp_vector8<uint16_t> rbuf;
	csp_vector8<uint16_t> gbuf;
	csp_vector8<uint16_t> bbuf;
	csp_vector8<uint16_t> pixture_mask((uint16_t)0x001f);
	
	csp_vector8<scrntype_t> sbuf[TOWNS_CRTC_MAX_PIXELS / 8];
	csp_vector8<scrntype_t> abuf[TOWNS_CRTC_MAX_PIXELS / 8];
	csp_vector8<bool> pix_transparent;
	csp_vector8<uint16_t> a2buf;

	pair16_t ptmp16;
	int rwidth = pwidth & 7;

	int k = 0;

	size_t width_tmp = (hst[trans] > TOWNS_CRTC_MAX_PIXELS) ? TOWNS_CRTC_MAX_PIXELS : hst[trans];
	__UNLIKELY_IF(width_tmp == 0) return false;
	size_t width_tmp1 = width_tmp;
	size_t width_tmp2 = width_tmp;
	size_t words = 0;
	for(int x = 0; x < pwidth ; x += 8) {
		int xx = x >> 3;
		pix_transparent.fill(false);
		__UNLIKELY_IF(xx >= (TOWNS_CRTC_MAX_PIXELS / 8)) {
			break;
		}

		__UNLIKELY_IF(xx == (pwidth >> 3)) {
			pbuf.fill(0x8000);
			pair16_t tmp;
			size_t x0 = 0;
			for(int i = x; i < pwidth; i++, x0++) {
				tmp.read_2bytes_le_from(p);
				pbuf.set(x0, tmp.w);
				p += 2;
			}
		} else {
			pbuf.load_from_le(p);
			p += 16; // 8 * 2bytes.
		}

		rbuf = pbuf;
		gbuf = pbuf;
		bbuf = pbuf;

		rbuf >>= 5;
		gbuf >>= 10;

		rbuf &= pixture_mask;
		gbuf &= pixture_mask;
		bbuf &= pixture_mask;

		rbuf <<= 3;
		gbuf <<= 3;
		bbuf <<= 3;

		if(is_transparent) {
			pbuf.check_any_bits(pix_transparent, 0x8000);
		}
		__UNLIKELY_IF(do_alpha) {
			if(is_transparent) {
				a2buf.set_cond(pix_transparent, 0, 255);
			} else {
				a2buf.fill(255);
			}
			make_rgba_vec8(sbuf[xx], rbuf, gbuf, bbuf, a2buf);
		} else {
			if(is_transparent) {
				abuf[xx].set_cond(pix_transparent, RGBA_COLOR(0, 0, 0, 0), RGBA_COLOR(255, 255, 255,255));
			} else {
				abuf[xx].fill(RGBA_COLOR(255, 255, 255, 255));
			}
			make_rgb_vec8(sbuf[xx], rbuf, gbuf, bbuf); // ToDo
		}
		words++;
	}
	
	size_t __l = 0;
	__LIKELY_IF(!(odd_mag)) {
		__LIKELY_IF(q != NULL) {
			__l = scaling_store(q, sbuf, magx, words, width_tmp1);
			q = &(q[__l]);
		}
		rendered_pixels += __l;
		k += __l;
		__LIKELY_IF(r2 != NULL) {
			__l = scaling_store(r2, abuf, magx, words, width_tmp2);
			r2 = &(r2[__l]);
		}
	} else {
		__LIKELY_IF(q != NULL) {
			__l = scaling_store_by_map(q, sbuf, magx_tmp, words, width_tmp1);
			q = &(q[__l]);
		}
		rendered_pixels += __l;
		k += __l;
		__LIKELY_IF(r2 != NULL) {
			__l = scaling_store_by_map(r2, abuf, magx_tmp, words, width_tmp2);
			r2 = &(r2[__l]);
		}
	}
	return (rendered_pixels > 0) ? true : false;
}

bool TOWNS_CRTC::render_256(int trans, scrntype_t* dst, int y, int& rendered_pixels)
{
	// 256 colors
	__UNLIKELY_IF(dst == nullptr) return false;
	int magx = linebuffers[trans][y].mag[0];
	int pwidth = linebuffers[trans][y].pixels[0];
	uint8_t *p = linebuffers[trans][y].pixels_layer[0];

	scrntype_t* q = dst;
	__UNLIKELY_IF(pwidth <= 0) return false;
	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	csp_vector8<uint16_t> magx_tmp;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		magx_tmp.set(i, (magx + (i & 1)) / 2);
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2 + 16 * magx);

	rendered_pixels = 0;
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) > 1) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) > 1) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}
	magx = magx / 2;
	__UNLIKELY_IF(magx < 1) return false;
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 0) return false;

	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype_t apal256[256];
	scrntype_t* __app = &(linebuffers[trans][y].palettes[0].pixels[0]);
	
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t __t;
	for(size_t i = 0; i < 256; i += 8) {
		__t = load8_unaligned(&(__app[i]));
		store8_aligned(&(apal256[i]), __t);
	}


//	out_debug_log(_T("Y=%d MAGX=%d WIDTH=%d pWIDTH=%d"), y, magx, width, pwidth);
	__UNLIKELY_IF(pwidth < 1) pwidth = 1;
	union {
		SIMDE_ALIGN_TO_32 uint8_t u8[8];
		SIMDE_ALIGN_TO_32 uint64_t u64;
	} pbuf;
	//csp_vector8<uint8_t> pbuf;
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t sbuf[TOWNS_CRTC_MAX_PIXELS / 8];
	
	size_t rwidth = pwidth & 7;
	size_t width_tmp1 = (hst[trans] > TOWNS_CRTC_MAX_PIXELS) ? TOWNS_CRTC_MAX_PIXELS : hst[trans];
	__UNLIKELY_IF(width_tmp1 == 0) return false;

	size_t words = 0;

	TOWNS_CRTC_SCRNTYPE8_ALIGN const scrntype8_t zero_value = zero_scrntype8_t();
	
	for(size_t x = 0; x < pwidth; x += 8) {
		size_t xx = x >> 3;
		__UNLIKELY_IF(xx >= (TOWNS_CRTC_MAX_PIXELS / 8)) {
			break;
		}
		__UNLIKELY_IF(xx == (pwidth >> 3)) {
			pbuf.u64 = 0;
			sbuf[xx].v = zero_value;
			__LIKELY_IF(rwidth != 0) {
				for(size_t _ii = 0; _ii < rwidth; _ii++) {
					pbuf.u8[_ii] = p[_ii];
				}
				p += rwidth;
				for(size_t _ii = 0; _ii < rwidth; _ii++) {
					sbuf[xx].s[_ii] = apal256[pbuf.u8[_ii]];
				}
			}
		} else {
			pbuf.u64 = *((uint64_t*)p);
			SIMDE_VECTORIZE
			for(size_t _ii = 0; _ii < 8; _ii++) {
				sbuf[xx].s[_ii] = apal256[pbuf.u8[_ii]]);
			}
			p += 8;
		}
		words++;
	}
	size_t __l = 0;
	__LIKELY_IF(!(odd_mag)) {
		__LIKELY_IF(q != NULL) {
			__l = scaling_store(q, sbuf, magx, words, width_tmp1);
			q = &(q[__l]);
		}
		rendered_pixels += __l;
	} else {
		__LIKELY_IF(q != NULL) {
			__l = scaling_store_by_map(q, sbuf, magx_tmp, words, width_tmp1);
			q = &(q[__l]);
		}
		rendered_pixels += __l;
	}
	return (rendered_pixels > 0) ? true : false;
}

bool TOWNS_CRTC::render_16(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	TOWNS_CRTC_SCRNTYPE8_ALIGN static const scrntype_t maskdata_transparent[16] = {
		RGBA_COLOR(0, 0, 0, 0),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),

		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),

		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),

		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255)
	};	
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = (do_alpha) ? NULL : mask;
	__UNLIKELY_IF(pwidth <= 0) return false;

	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	SIMDE_ALIGN_TO_16 uint16_8_t magx_tmp;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 8; i++) {
		magx_tmp.u16[i] = (magx + (i & 1)) / 2;
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2 + 16 * magx);
	rendered_pixels = 0;

	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) > 1) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) > 1) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}
	magx = magx / 2;
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 1) return false;
	__UNLIKELY_IF(magx < 1) return false;


	SIMDE_ALIGN_TO_16 uint16_8_t hlbuf;
	SIMDE_ALIGN_TO_8  uint16_8_t mbuf;
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t sbuf[TOWNS_CRTC_MAX_PIXELS / 8];
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t abuf[TOWNS_CRTC_MAX_PIXELS / 8];
	
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype_t palbuf[16];
	
	uint8_t pmask = linebuffers[trans][y].r50_planemask[layer] & 0x0f;
	SIMD_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		mbuf.u8[n] = pmask;
	}

	scrntype_t *pal = &(linebuffers[trans][y].palettes[layer].pixels[0]);
	simd_copy(palbuf, pal, 16);
	// Clear palbuf[0]?
	if((do_alpha) && (is_transparent)) {
		palbuf[0] &= RGBA_COLOR(255, 255, 255, 0); // OK?
	} else if(!is_transparent) {
		TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _s;
		SIMDE_VECTORIZE
		for(size_t n = 0; n < 8; n++) {
			_s.s[n] = RGBA_COLOR(255, 255, 255, 255;
		}
		SIMDE_VECTORIZE
		for(size_t x = 0; x < (TOWNS_CRTC_MAX_PIXELS / 8); x++) {
			abuf[x] = _s;
		}
	}
	int k = 0;

	size_t width_tmp = (hst[trans] > TOWNS_CRTC_MAX_PIXELS) ? TOWNS_CRTC_MAX_PIXELS : hst[trans];
	__UNLIKELY_IF(width_tmp == 0) return false;
//	size_t width_tmp1 = pwidth << 1;
//	size_t width_tmp2 = pwidth << 1;
	size_t width_tmp1 = width_tmp;
	size_t width_tmp2 = width_tmp;
	size_t words = 0;
	size_t pptr = 0;

	TOWNS_CRTC_SCRNTYPE8_ALIGN uint16_8_t bytes_mask;
	for(int i = 0; i < 16; i += 2) {
		bytes_mask.u8[i + 0] = 0x0f;
		bytes_mask.u8[i + 1] = 0xf0;
	}
	TOWNS_CRTC_SCRNTYPE8_ALIGN const scrntype8_t zero_value = zero_scrntype8_t();
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t white_value;
	SIMDE_VECTORIZE
	for(int n = 0; n < 8; n++) {	
		white_value.s[n] = RGBA_COLOR(255, 255, 255, 255);
	}
	
	for(int x = 0; x < pwidth ; x++) {
		size_t xx = x >> 3;
		__UNLIKELY_IF((pptr + 1) >= (TOWNS_CRTC_MAX_PIXELS / 8)) {
			break;
		}
		__UNLIKELY_IF(xx == (pwidth >> 3)) {
//			sbuf[pptr] = zero_value;
//			sbuf[pptr + 1] = zero_value;
			SIMDE_VECTORIZE
			for(int i = x; i < pwidth; i++, j += 2) {
				hlbuf.u8[j + 0] = *p;
				hlbuf.u8[j + 1] = *p;
				p++;
			}
		} else {
			SIMDE_VECTORIZE
			for(int i = 0; i < 8; i++, j += 2) {
				hlbuf.u8[j + 0] = p[i];
				hlbuf.u8[j + 1] = p[i];
			}
			p += 8;
		}

		hlbuf.v = simde_mm_and_ps(bytes_mask.v, hlbuf.v);

		SIMDE_VECTORIZE
		for(int j = 1; j < 16; j+= 2) {
			hlbuf.u8[j] >>= 4;
		}
		hlbuf.v = simde_mm_and_ps(mbuf.v, hlbuf.v);

		sbuf[pptr] = zero_value;
		SIMDE_VECTORIZE
		for(size_t n = 0; n < 8; n++) {
			sbuf[pptr].s[n] = palbuf[hlbuf.u8[n]];
		}
		
		abuf[pptr].v = white_value.v;
		if(!(do_alpha) && (is_transparent)) {
			SIMDE_VECTORIZE
			for(size_t n = 0; n < 8; n++) {
				abuf[pptr].s[n] = maskdata_transparent[hlbuf.u8[n]];
			}
		}
		words++;
		pptr++;
		__UNLIKELY_IF(pptr >= (width_tmp / 8)) {
			break;
		}
		sbuf[pptr] = zero_value;
		SIMDE_VECTORIZE
		for(size_t n = 0; n < 8; n++) {
			sbuf[pptr].s[n] = palbuf[hlbuf.u8[n + 8]];
		}
		
		abuf[pptr].v = white_value.v;
		if(!(do_alpha) && (is_transparent)) {
			SIMDE_VECTORIZE
			for(size_t n = 0; n < 8; n++) {
				abuf[pptr].s[n] = maskdata_transparent[hlbuf.u8[n + 8]];
			}
		}
		words++;
		pptr++;
		__UNLIKELY_IF(pptr >= (width_tmp / 8)) {
			break;
		}
	}
	size_t __l = 0;
	__LIKELY_IF(!(odd_mag)) {
		__LIKELY_IF(q != NULL) {
			__l = scaling_store(q, sbuf, magx, words, width_tmp1);
			q = &(q[__l]);
		}
		rendered_pixels += __l;
		k += __l;
		__LIKELY_IF(r2 != NULL) {
			__l = scaling_store(r2, abuf, magx, words, width_tmp2);
			r2 = &(r2[__l]);
		}
	} else {
		__LIKELY_IF(q != NULL) {
			__l = scaling_store_by_map(q, sbuf, magx_tmp, words, width_tmp1);
			q = &(q[__l]);
		}
		rendered_pixels += __l;
		k += __l;
		__LIKELY_IF(r2 != NULL) {
			__l = scaling_store_by_map(r2, abuf, magx_tmp, words, width_tmp2);
			r2 = &(r2[__l]);
		}
	}
	return (rendered_pixels > 0) ? true : false;
}

// This function does alpha-blending.
// If CSP support hardware-accelalations, will support.
// (i.e: Hardware Alpha blending, Hardware rendaring...)
void TOWNS_CRTC::mix_screen(int y, int width, bool do_mix0, bool do_mix1, int bitshift0, int bitshift1, int words0, int words1, bool is_hloop0, bool is_hloop1)
{
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(width <= 0) return;

	scrntype_t *pp = osd->get_vm_screen_buffer(y);
	if(y == 128) {
		//out_debug_log(_T("MIX_SCREEN Y=%d WIDTH=%d DST=%08X"), y, width, pp);
	}

	bool pix_cached = false;
	bool alpha_cached = false;
	bool pix0_cached = false;
	__LIKELY_IF(pp != nullptr) {
		int left0 = words0;
		int left1 = words1;
		__UNLIKELY_IF(words0 <= 0) {
			do_mix0 = false;
		}
		__UNLIKELY_IF(words1 <= 0) {
			do_mix1 = false;
		}
		// Clear cache
		TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t blank;
		TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t blank_alpha;

		SIMDE_VECTORIZE
		for(size_t n = 0; n < 8; n++) {
			blank.s[n] = RGBA_COLOR(0, 0, 0, 255);
		}
		SIMDE_VECTORIZE
		for(size_t n = 0; n < 8; n++) {
			blank_alpha.s[n] = RGBA_COLOR(0, 0, 0, 0);
		}
		//csp_vector8<scrntype_t> blank(RGBA_COLOR(0, 0, 0, 255));
		//csp_vector8<scrntype_t> blank_alpha(RGBA_COLOR(0, 0, 0, 0));

		if(do_mix1) {
			simd_fill(pix_cache, blank, width);
			make_prefetch(pix_cache, sizeof(pix_cache));
			pix_cached = true;
		}
		if((do_mix0) && (bitshift0 != 0)) {
			simd_fill(pix_cache0, blank, width);
			make_prefetch(pix_cache0, sizeof(pix_cache0));
			pix0_cached = true;
		}
		bool got_0 = false;
		bool got_1 = false;
		__LIKELY_IF(do_mix1) {
			__UNLIKELY_IF(words1 >= TOWNS_CRTC_MAX_PIXELS) {
				words1 = TOWNS_CRTC_MAX_PIXELS;
			}
			if(is_hloop1) {
				// COPY 0 to (words)
				if(bitshift1 == 0) {
					simd_copy(&(pix_cache[0]), &(lbuffer1[0]), words1);
					got_1 = true;
				} else if(bitshift1 < 0) {
					int x10 = -bitshift1;
					int w10 = words1 - x10;
					__LIKELY_IF((x10 >= 0) && (w10 <= words1)){
						simd_copy(&(pix_cache[0]), &(lbuffer1[x10]), w10);
						left1 -= w10;
						got_1 = true;
					}
					__LIKELY_IF((left1 > 0) && (w10 >= 0)) {
						simd_copy(&(pix_cache[w10]), &(lbuffer1[0]), left1);
						got_1 = true;
					}
				} else {
					int x10 = bitshift1;
					int w10 = words1 - x10;
					__LIKELY_IF((x10 >= 0) && (x10 <= words1) && (w10 > 0)){
						simd_copy(&(pix_cache[x10]), &(lbuffer1[0]), w10);
						left1 -= w10;
						got_1 = true;
					}
					__LIKELY_IF((left1 > 0) && (x10 >= 0)) {
						simd_copy(&(pix_cache[0]), &(lbuffer1[w10]), left1);
						got_1 = true;
					}
				}
			} else {
				if(bitshift1 == 0) {
					simd_copy(&(pix_cache[0]), &(lbuffer1[0]), words1);
					got_1 = true;
				} else if(bitshift1 < 0) {
					int x10 = -bitshift1;
					int w10 = words1;
					__LIKELY_IF(x10 >= 0) {
						__UNLIKELY_IF((x10 + w10) >= TOWNS_CRTC_MAX_PIXELS) {
							w10 = TOWNS_CRTC_MAX_PIXELS - x10;
						}
						__LIKELY_IF(w10 > 0) {
							simd_copy(&(pix_cache[0]), &(lbuffer1[x10]), w10);
							left1 -= w10;
							got_1 = true;
						}
					}
				} else {
					int x10 = bitshift1;
					int w10 = words1;
					__LIKELY_IF(x10 >= 0) {
						__UNLIKELY_IF((x10 + w10) >= TOWNS_CRTC_MAX_PIXELS) {
							w10 = TOWNS_CRTC_MAX_PIXELS - x10;
						}
						__LIKELY_IF(w10 > 0) {
							simd_copy(&(pix_cache[x10]), &(lbuffer1[0]), w10);
							left1 -= w10;
							got_1 = true;
						}
					}
				}
			}
		}
		if((got_1) && (do_mix0)) {
			simd_fill(alpha_cache, blank_alpha, width);
			make_prefetch(alpha_cache, sizeof(alpha_cache));
			alpha_cached = true;
		}
		__LIKELY_IF(do_mix0) {
			__UNLIKELY_IF(words0 >= TOWNS_CRTC_MAX_PIXELS) {
				words0 = TOWNS_CRTC_MAX_PIXELS;
			}
			make_prefetch(lbuffer0, sizeof(lbuffer0));
			if((is_hloop0) && (bitshift0 != 0)) {
				ssize_t of00 = 0;
				ssize_t of01 = 0;
				if(bitshift0 > 0) {
					of00 = bitshift0;
					of01 = 0;
				} else if(bitshift0 < 0) {
					of00 = 0;
					of01 = -bitshift0;
				}
				ssize_t w00 = words0 - of01;
				ssize_t w01 = words0 - of00;
				
				__LIKELY_IF((of00 < width) && (of01 < width)) {
					__LIKELY_IF(w00 > 0) {
						simd_copy(&(pix_cache0[of00]), &(lbuffer0[of01]), w00);
						if(got_1) {
							simd_copy(&(alpha_cache[of00]), &(abuffer0[of01]), w00);
						}
						got_0 = true;
					}
					__LIKELY_IF(w01 > 0) {
					    simd_copy(&(pix_cache0[of01]), &(lbuffer0[of00]), w01);
						if(got_1) {
							simd_copy(&(alpha_cache[of01]), &(abuffer0[of00]), w01);
						}
						got_0 = true;
					}
				}
			} else if(bitshift0 != 0) {
				ssize_t of00 = 0;
				ssize_t of01 = 0;
				ssize_t w00 = words0;
				if(bitshift0 > 0) {
					of00 = bitshift0;
					of01 = 0;
					__UNLIKELY_IF((w00 + of00) >= width) {
						w00 = width - of00;
					}
				} else if(bitshift0 < 0) {
					of00 = 0;
					of01 = -bitshift0;
					__UNLIKELY_IF((w00 + of01) >= width) {
						w00 = width - of01;
					}
				}
				
				__LIKELY_IF((of00 < width) && (of01 < width)) {
					__LIKELY_IF(w00 > 0) {
						simd_copy(&(pix_cache0[of00]), &(lbuffer0[of01]), w00);
						if(got_1) {
							simd_copy(&(alpha_cache[of00]), &(abuffer0[of01]), w00);
						}
						words0 = w00;
						got_0 = true;
					}
				}
			} else {
				__LIKELY_IF(words0 > 0) {
					got_0 = true;
				}
			}
		}
		
		if((got_0) && (got_1)) {
			scrntype_t p0;
			scrntype_t p1;
			scrntype_t mf;
			scrntype_t mb;
			//csp_vector8<scrntype_t> pix00;
			//csp_vector8<scrntype_t> pix01;
			//csp_vector8<scrntype_t> pix10;
			//csp_vector8<scrntype_t> pix11;
			//csp_vector8<scrntype_t> mask_front0;
			//csp_vector8<scrntype_t> mask_front1;
			//csp_vector8<scrntype_t> mask_back0;
			//csp_vector8<scrntype_t> mask_back1;

			TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t pix0[2];
			TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t pix1[2];
			TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t mask_front[2];
			TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t mask_back[2];

			__LIKELY_IF(bitshift0 == 0) {
				__LIKELY_IF(width > 15) {
					for(size_t xx = 0; xx < width; xx += 16) {
						//pix00.load_aligned(&(lbuffer0[xx]));
						//pix01.load_aligned(&(lbuffer0[xx + 8]));
						//pix10.load_aligned(&(pix_cache[xx]));
						//pix11.load_aligned(&(pix_cache[xx + 8]));
						pix0[0] = load8_aligned(&(lbuffer0[xx]));
						pix0[1] = load8_aligned(&(lbuffer0[xx + 8]));
						pix1[0] = load8_aligned(&(pix_cache[xx]));
						pix1[1] = load8_aligned(&(pix_cache[xx + 8]));
						
						//mask_front0.load_aligned(&(abuffer0[xx]));
						//mask_front1.load_aligned(&(abuffer0[xx + 8]));
						//mask_back0 = mask_front0;
						//mask_back1 = mask_front1;
						mask_front[0] = load8_aligned(&(abuffer0[xx]));
						mask_front[1] = load8_aligned(&(abuffer0[xx + 8]));
						//mask_back[0].v = mask_front[0].v;
						//mask_back[1].v = mask_front[1].v;
						
						//mask_back0.bitwise_not();
						//mask_back1.bitwise_not();
						//pix00 &= mask_front0;
						//pix01 &= mask_front1;
						//pix10 &= mask_back0;
						//pix11 &= mask_back1;
						#if defined(_RGB555) || defined(_RGB565)
						pix0[0].v = simde_mm_and_ps(mask_front[0].v, pix0[0].v);
						pix0[1].v = simde_mm_and_ps(mask_front[1].v, pix0[1].v);
						
						pix1[0].v = simde_mm_andnot_ps(mask_front[0].v, pix1[0].v);
						pix1[1].v = simde_mm_andnot_ps(mask_front[1].v, pix1[1].v);
						#else
						pix0[0].v = simde_mm256_and_ps(mask_front[0].v, pix0[0].v);
						pix0[1].v = simde_mm256_and_ps(mask_front[1].v, pix0[1].v);
						
						pix1[0].v = simde_mm256_andnot_ps(mask_front[0].v, pix1[0].v);
						pix1[1].v = simde_mm256_andnot_ps(mask_front[1].v, pix1[1].v);
						#endif
						
						//pix00 |= pix10;
						//pix01 |= pix11;
						//pix00.store(&(pp[xx]));
						//pix01.store(&(pp[xx + 8]));
						#if defined(_RGB555) || defined(_RGB565)
						pix0[0].v = simde_mm_or_ps(pix0[0].v, pix1[0].v);
						pix0[1].v = simde_mm_or_ps(pix0[1].v, pix1[1].v);
						#else
						pix0[0].v = simde_mm256_or_ps(pix0[0].v, pix1[0].v);
						pix0[1].v = simde_mm256_or_ps(pix0[1].v, pix1[1].v);
						#endif
						store8_pix(&(pp[xx]), pix0[0]);
						store8_pix(&(pp[xx + 8]), pix0[1]);
					}
				}
				if((width & 15) != 0) {
					for(size_t xx = (width & ~(15)); xx < width; xx++) {
						p0 = lbuffer0[xx];
						mf = abuffer0[xx];
						p1 = pix_cache[xx];
						mb = ~mf;
						p0 &= mf;
						p1 &= mb;
						p0 |= p1;
						pp[xx] = p0;
					}
				}
			} else {
				__LIKELY_IF(width > 15) {
					for(size_t xx = 0; xx < width; xx += 16) {
						//pix00.load_aligned(&(pix_cache0[xx]));
						//pix01.load_aligned(&(pix_cache0[xx + 8]));
						//pix10.load_aligned(&(pix_cache[xx]));
						//pix11.load_aligned(&(pix_cache[xx + 8]));
						
						pix0[0] = load8_aligned(&(pix_cache0[xx]));
						pix0[1] = load8_aligned(&(pix_cache0[xx + 1]));
						pix1[0] = load8_aligned(&(pix_cache[xx]));
						pix1[1] = load8_aligned(&(pix_cache[xx + 1]));
						
						//mask_front0.load_aligned(&(alpha_cache[xx]));
						//mask_front1.load_aligned(&(alpha_cache[xx + 8]));
						mask_front[0] = load8_aligned(&(alpha_cache[xx]));
						mask_front[1] = load8_aligned(&(alpha_cache[xx + 1]));
						
						//mask_back0 = mask_front0;
						//mask_back1 = mask_front1;
						//mask_back0.bitwise_not();
						//mask_back1.bitwise_not();
						//pix00 &= mask_front0;
						//pix01 &= mask_front1;
						//pix10 &= mask_back0;
						//pix11 &= mask_back1;
						#if defined(_RGB555) || defined(_RGB565)
						pix0[0].v = simde_mm_and_ps(mask_front[0].v, pix0[0].v);
						pix0[1].v = simde_mm_and_ps(mask_front[1].v, pix0[1].v);
						
						pix1[0].v = simde_mm_andnot_ps(mask_front[0].v, pix1[0].v);
						pix1[1].v = simde_mm_andnot_ps(mask_front[1].v, pix1[1].v);
						#else
						pix0[0].v = simde_mm256_and_ps(mask_front[0].v, pix0[0].v);
						pix0[1].v = simde_mm256_and_ps(mask_front[1].v, pix0[1].v);
						
						pix1[0].v = simde_mm256_andnot_ps(mask_front[0].v, pix1[0].v);
						pix1[1].v = simde_mm256_andnot_ps(mask_front[1].v, pix1[1].v);
						#endif

						//pix00 |= pix10;
						//pix01 |= pix11;
						//pix00.store(&(pp[xx]));
						//pix01.store(&(pp[xx + 8]));
						#if defined(_RGB555) || defined(_RGB565)
						pix0[0].v = simde_mm_or_ps(pix0[0].v, pix1[0].v);
						pix0[1].v = simde_mm_or_ps(pix0[1].v, pix1[1].v);
						#else
						pix0[0].v = simde_mm256_or_ps(pix0[0].v, pix1[0].v);
						pix0[1].v = simde_mm256_or_ps(pix0[1].v, pix1[1].v);
						#endif
						store8_pix(&(pp[xx]), pix0[0]);
						store8_pix(&(pp[xx + 8]), pix0[1]);
					}
				}
				if((width & 15) != 0) {
					for(size_t xx = (width & ~(15)); xx < width; xx++) {
						p0 = pix_cache0[xx];
						mf = alpha_cache[xx];
						p1 = pix_cache[xx];
						mb = ~mf;
						p0 &= mf;
						p1 &= mb;
						p0 |= p1;
						pp[xx] = p0;
					}
				}
			}
			
		} else if(got_1) {
			simd_copy(pp, pix_cache, width);
		} else if(got_0) {
			if(bitshift0 == 0) {
				simd_copy(pp, lbuffer0, width);
			} else {
				simd_copy(pp, pix_cache0, width);
			}
		} else {
			// Clear ONLY
			if((do_mix0) || (do_mix1)) {
				TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t pix;
				SIMDE_VECTORIZE
				for(size_t n = 0; n < 8; n++) {
					pix.s[n] = RGBA_COLOR(0, 0, 0, 255);
				}
				simd_fill(pp, pix, width);
			}
		}
		if(do_mix0) {
			flush_cache(lbuffer0, sizeof(lbuffer0));
		}			
		if(pix_cached) {
			flush_cache(pix_cache, sizeof(pix_cache));
		}
		if(pix0_cached) {
			flush_cache(pix_cache0, sizeof(pix_cache0));
		}
		if(alpha_cached) {
			flush_cache(alpha_cache, sizeof(alpha_cache));
		}
	}
}

}
