/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.03.27 -

	[ FM-Towns CRTC around Line buffer ]
	History: 2025.03.27 Split from crtc.cpp .
*/

#include "../../vm.h"
#include "../../../common.h"
#include "../../../types/simd.h"

#include "../crtc.h"
#include "../vram.h"

namespace FMTOWNS {

void TOWNS_CRTC::transfer_line(int layer, int line)
{
	int l = layer;
	bool to_disp = true; // Dummy
	static const uint32_t address_add[2] =  {0x00000000, 0x00040000};
	uint8_t ctrl, prio;
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];
	
	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;
	__UNLIKELY_IF(d_vram == nullptr) return;
	__UNLIKELY_IF(layer < 0) return;
	__UNLIKELY_IF(layer > 1) return;
	__UNLIKELY_IF(!(frame_in[l])) return;
	__UNLIKELY_IF(linebuffers[trans] == nullptr) return;
	if((is_single_tmp) && (layer != 0)) return;
	// Update some parameters per line. 20230731 K.O
	uint32_t __vstart_addr  = vstart_addr[l];
	uint32_t __line_offset  = line_offset[l];
	uint32_t address_mask = 0x0003ffff;

	uint32_t lo = __line_offset;
	//uint32_t hscroll_mask = ((lo == 128) || (lo == 256)) ? lo : 0xffffffff;
	uint32_t hscroll_mask = 0xffffffff; // ToDo.
	uint32_t magx = zoom_factor_horiz[l];

	int hoffset   = hoffset_val[l];
	int bit_shift = max(0, hbitshift_val[l]);
	int64_t hwidth = hwidth_val[l];
	
	// FAx
	// Note: Re-Wrote related by Tsugaru. 20230806 K.O
	if(is_single_tmp) {
		address_mask = 0x0007ffff;
	}

	uint32_t skip_zoom = zoom_raw_horiz[l];
	int64_t pixels = hwidth * ((is_single_tmp) ? 2 : 1);
	int64_t bit_shift64 = bit_shift;
	
	recalc_width_by_clock(magx, pixels);
	recalc_offset_by_clock(skip_zoom, hoffset, bit_shift64);
	uint32_t address_shift;
	int32_t  hskip_bytes;
	switch(linebuffers[trans][line].mode[l]) {
	case DISPMODE_32768:
		if(is_single_tmp) {
			address_shift = 3; // FM-Towns Manual P.145
			hskip_bytes = bit_shift64 << 1;
		} else {
			address_shift = 2; // FM-Towns Manual P.145
			hskip_bytes = bit_shift64 << 1;
		}
		break;
	case DISPMODE_256:
		address_shift = 3; // FM-Towns Manual P.145
		hskip_bytes = bit_shift64;
		break;
	case DISPMODE_16:
	default:
		address_shift = 2; //  Default is 16 colors; FM-Towns Manual P.145		
		hskip_bytes = bit_shift64 >> 1;
		break;
	}
	//! Note:
	//! - Below is from Tsugaru, commit 1a442831 .
	//! - I wonder sprite offset effects every display mode at page1,
	//!   and FMR's address offset register effects every display mode at page0
	//! - -- 20230715 K.O
	uint32_t page_offset;
	if(l == 0) {
		page_offset = ((r50_pagesel != 0) ? 0x20000 : 0);
	} else {
		page_offset = sprite_offset;
	}
	__UNLIKELY_IF(pixels < 0) {
		pixels = 0;
	} else if(pixels > TOWNS_CRTC_MAX_PIXELS) {
		pixels = TOWNS_CRTC_MAX_PIXELS;
	}
	__LIKELY_IF((linebuffers[trans][line].crtout[l] != 0)){
		__LIKELY_IF(/*(pixels >= magx) && */(magx != 0)){
			//__UNLIKELY_IF(pixels >= TOWNS_CRTC_MAX_PIXELS) pixels = TOWNS_CRTC_MAX_PIXELS;
			uint32_t pixels_bak = pixels;
			linebuffers[trans][line].bitoffset[l] =  hoffset;
			linebuffers[trans][line].pixels[l] = pixels;
			linebuffers[trans][line].mag[l] = magx; // ToDo: Real magnif
			linebuffers[trans][line].is_hloop[l] = NOT_LOOP;
			pixels = ((pixels_bak << 16) / skip_zoom) >> 15;
			bool is_256 = false;
			uint32_t tr_bytes = 0;
			switch(linebuffers[trans][line].mode[l]) {
			case DISPMODE_32768:
				tr_bytes = pixels << 1;
				//lo = lo << 1;
				break;
			case DISPMODE_16:
				tr_bytes = pixels >> 1;
				//lo = lo >> 1;
				break;
			case DISPMODE_256:
				is_256 = true;
				tr_bytes = pixels;
				break;
			default:
				to_disp = false;
				break;
			}
			if(to_disp) {
				uint32_t offset = __vstart_addr; // ToDo: Larger VRAM
				uint32_t head_tmp = head_address[l];
				offset = offset + head_tmp;
				bool is_xwrap = false;
				offset <<= address_shift;
				offset += hskip_bytes;
				uint32_t lo2 = lo * ((is_single_tmp) ? 4 : 8);
				linebuffers[trans][line].prev_y[l] = line;

				__UNLIKELY_IF((lo2 == 1024) || (lo2 == 512)) {
					__LIKELY_IF(!(is_256)) {
						hscroll_mask = lo2 - 1;
						is_xwrap = true;
					}
				}

				__UNLIKELY_IF(is_interlaced[l]) {
					if(odd_field) { // odd field
						offset = offset + (frame_offset_bak[l] << address_shift);
					}
				}

				offset = ((offset + page_offset) & address_mask);
				// ToDo: FO1 Offset Value.
				// ToDo: Will Fix
				__LIKELY_IF(!(is_single_tmp)) {
					offset = offset + address_add[l];
				}
				d_vram->get_data_from_vram(((is_single_tmp) || (is_256)), offset, tr_bytes, &(linebuffers[trans][line].pixels_layer[l][0]), hscroll_mask);
				__LIKELY_IF(is_xwrap) {
					linebuffers[trans][line].is_hloop[l] = IS_LOOP;
				}
			}
		}
	} else {
		to_disp = false;
	}
	if(zoom_count_vert[l] > 0) {
		zoom_count_vert[l]--;
	}
	if(zoom_count_vert[l] <= 0) {
		zoom_count_vert[l] = zoom_factor_vert[l];
		// ToDo: Interlace
		//if((to_disp)) {
			head_address[l] += lo;
			//head_address[l] &= (address_mask >> address_shift);
		//}
	}
}

void TOWNS_CRTC::copy_line(const int trans, int layer, const int from_y, const int to_y)
{
	__UNLIKELY_IF(from_y == to_y) {
		return;
	}
	__UNLIKELY_IF(from_y < 0) {
		return;
	}
	__UNLIKELY_IF(to_y < 0) {
		return;
	}
	layer &= 1;
	linebuffers[trans][to_y].mode[layer] = 	linebuffers[trans][from_y].mode[layer];
	linebuffers[trans][to_y].is_hloop[layer] = linebuffers[trans][from_y].is_hloop[layer];
	linebuffers[trans][to_y].mag[layer] = linebuffers[trans][from_y].mag[layer];
	linebuffers[trans][to_y].r50_planemask[layer] = linebuffers[trans][from_y].r50_planemask[layer];
	linebuffers[trans][to_y].crtout[layer] = linebuffers[trans][from_y].crtout[layer];
	linebuffers[trans][to_y].pixels[layer] = linebuffers[trans][from_y].pixels[layer];
	linebuffers[trans][to_y].num[layer] = linebuffers[trans][from_y].num[layer];
	linebuffers[trans][to_y].bitoffset[layer] = linebuffers[trans][from_y].bitoffset[layer];

	
	__DECL_ALIGNED(32) uint8_t cache[32]; // 8 * 2
	for(int x = 0; (x < TOWNS_CRTC_MAX_PIXELS * 2); x += 32) {
		__DECL_VECTORIZED_LOOP
		for(int xx = 0; xx < 32; xx++) {
			cache[xx] = linebuffers[trans][from_y].pixels_layer[layer][x + xx];
		}
		__DECL_VECTORIZED_LOOP
		for(int xx = 0; xx < 32; xx++) {
			linebuffers[trans][to_y].pixels_layer[layer][x + xx] = cache[xx];
		}
	}
	memcpy(&(linebuffers[trans][to_y].palettes[layer]),
		   &(linebuffers[trans][from_y].palettes[layer]),
		   sizeof(palette_backup_t));

}
	
void TOWNS_CRTC::clear_line(const int trans, int layer, const int y)
{
	layer &= 1;

	union {
		uint16_t w;
		uint8_t b[2];
	} n;

	uint16_t *p = (uint16_t*)(&(linebuffers[trans][y].pixels_layer[layer][0]));
	uint16_t *q = (uint16_t*)(___assume_aligned(p, 16));
	if((linebuffers[trans][y].mode[layer] & ~(DISPMODE_DUP)) == DISPMODE_32768) {
		n.b[0] = 0x00;
		n.b[1] = 0x80;
	} else {
		n.w = 0x0000;
	}
	csp_vector8<uint16_t> clrdata(n.w);
	for(size_t x = 0; x < TOWNS_CRTC_MAX_PIXELS; x += 8) {
		clrdata.store(&(q[x]));
	}
}

void TOWNS_CRTC::pre_transfer_line(int layer, int line)
{
	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;

	layer &= 1;
	uint8_t prio;
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];

	prio = video_out_regs[FMTOWNS::VOUTREG_PRIO];
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i += 2) {
		linebuffers[trans][line].mode[i + layer] = DISPMODE_NONE;
		linebuffers[trans][line].pixels[i + layer] = 0;
		linebuffers[trans][line].is_hloop[i + layer] = NOT_LOOP;
		linebuffers[trans][line].mag[i + layer] = 0;
		linebuffers[trans][line].num[i + layer] = layer;
	}
	linebuffers[trans][line].r50_planemask[layer] = r50_planemask;
	int disp_prio[2] = {0, 1};
	if(!(is_single_tmp)) {
		if((prio & 0x01) == 0) {
			disp_prio[0] = 0; // Front
			disp_prio[1] = 1; // Back
		} else {
			disp_prio[0] = 1;
			disp_prio[1] = 0;
		}
	}
//	out_debug_log("LINE %d CTRL=%02X \n", line, ctrl);
	
	int mode_tmp = real_display_mode[layer];
	bool disp = frame_in[layer];
	if(is_single_tmp) {
		// ToDo: High resolution.
		if(layer != 0) {
			disp = false;
		} else if(mode_tmp == DISPMODE_16) {
			disp = false;
		}
	} else {
		if(mode_tmp == DISPMODE_256) {
			disp = false;
		}
	}
	
	linebuffers[trans][line].num[layer] = disp_prio[layer];
	//linebuffers[trans][line].crtout[layer] = ((crtout_fmr[layer]) /*|| (crtout_towns[layer])*/) ? 0xff : 0x00;
	linebuffers[trans][line].crtout[layer] = (((crtout_fmr[layer]) /*|| (crtout_towns[layer])*/) && (disp)) ? 0xff : 0x00;
	linebuffers[trans][line].mode[layer] = mode_tmp;
	// Copy (sampling) palettes
	clear_line(trans, layer, line);
	
	switch(mode_tmp) {
	case DISPMODE_256:
		memcpy(&(linebuffers[trans][line].palettes[layer].raw[0][0]), &(apalette_256_rgb[0][0]), sizeof(uint8_t) * 256 * 4); // Copy RAW
		memcpy(&(linebuffers[trans][line].palettes[layer].pixels[0]), &(apalette_256_pixel[0]), sizeof(scrntype_t) * 256); // Copy Pixels
		break;
	case DISPMODE_16:
		memcpy(&(linebuffers[trans][line].palettes[layer].raw[0][0]), &(apalette_16_rgb[layer][0][0]), sizeof(uint8_t) * 16 * 4); // Copy RAW
		memcpy(&(linebuffers[trans][line].palettes[layer].pixels[0]), &(apalette_16_pixel[layer][0]), sizeof(scrntype_t) * 16); // Copy Pixels
		break;
	default:
		memset(&(linebuffers[trans][line].palettes[layer].raw[0][0]), 0x00, sizeof(uint8_t) * 256 * 4);
		memset(&(linebuffers[trans][line].palettes[layer].pixels[0]), 0x00, sizeof(scrntype_t) * 256);
		break;
	}
	// Fill by skelton colors;

}

}
