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
	render_num = 0;
	render_mod = 0;
	render_lines = 0;
	split_rendering = true;

	max_sprite_per_frame = 224;
	frame_sprite_count = 0;	

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
//	render_lines = 0;
	sprite_enabled = false;
	now_transferring = false;
	max_sprite_per_frame = 224;
	frame_sprite_count = 0;
	tvram_enabled = false;
	tvram_enabled_bak = false;
	
	memset(reg_data, 0x00, sizeof(reg_data)); // OK?
//	ankcg_enabled = false;
}
#if 0
void TOWNS_SPRITE::clear_cache(int num)
{
	if(num >= TOWNS_SPRITE_CACHE_NUM) return;
	if(num < 0) return;
	memset(&(cache_index[num]), 0x00, sizeof(sprite_cache_t));
	memset(&(cache_pixels[num][0]) , 0x00, sizeof(uint16_t) * 16 * 16);
	memset(&(cache_masks[num][0]) , 0x00, sizeof(uint16_t) * 16 * 16);
	cache_index[num].is_use = false;
	cache_index[num].pixels = &(cache_pixels[num][0]);
	cache_index[num].masks  = &(cache_masks[num][0]);
}

bool TOWNS_SPRITE::check_cache(int num, sprite_cache_t** p)
{
	sprite_cache_t* q;
	sprite_table_t* t;
	if(p != NULL) *p = NULL;
	
	t = &(sprite_table[num]);
	if(use_cache) {
		for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
			q = &(cache_index[i]);
			if(!(q->is_use)) continue;
			if(q->attribute == t->attribute) {
				if(q->is_32768 == t->is_32768) {
					if((q->is_halfy == t->is_halfy) && (q->is_halfx == t->is_halfx)) {
						if(p != NULL) *p = q;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void TOWNS_SPRITE::render_sprite(int num,  int x, int y, uint16_t attr, uint16_t color)
{
	uint16_t sprite_limit = reg_index & 0x3ff;
	if(sprite_limit == 0) sprite_limit = 1024;
	if(num < 0) return;
	if(num >= sprite_limit) return;
	if(num >= 1024) return;
	if(stride <= 0) return;
	if(stride > 512) return;
	if(!(sprite_table[num].is_disp)) return;

	sprite_cache_t *cacheptr;
	bool c_stat = false;
	c_stat = check_cache(num, &cacheptr);
	if((c_stat) && (cacheptr != NULL)) {
		if((cacheptr->pixels != NULL) && (cacheptr->masks != NULL)) {
			render_zoomed_pixels(x, y, cacheptr->pixels, cacheptr->masks, cacheptr->is_halfx, cacheptr->is_halfy, dst_pixel, dst_mask);
			return;
		}
	}
	// Cache Not hit
	// ToDo: Implement Link counter.
	int target_num = -1;
	for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
		if(!(cache_index[i].is_use)) {
			target_num = i;
			break;
		}
	}
	if((target_num < 0) || (target_num >= TOWNS_SPRITE_CACHE_NUM)) {
		// Force erace a cache.
		target_num = (last_put_cache_num + 1) % TOWNS_SPRITE_CACHE_NUM;
	}
	last_put_cache_num = target_num;

	cacheptr = &(cache_index[target_num]);
	
	memset(cacheptr, 0x00, sizeof(sprite_cache_t));
	cacheptr->is_use = true;
	cacheptr->attribute = sprite_table[num].attribute;
	cacheptr->is_32768 = sprite_table[num].is_32768;
	cacheptr->is_halfx = sprite_table[num].is_halfx;
	cacheptr->is_halfy = sprite_table[num].is_halfy;
	cacheptr->pixels = (uint16_t*)(&(cache_pixels[target_num][0]));
	cacheptr->masks = (uint16_t*)(&(cache_masks[target_num][0]));
	cacheptr->color = sprite_table[num].color;
	cacheptr->num = sprite_table[num].num;
	cacheptr->rotate_type = sprite_table[num].rotate_type;
	
	if(!(cacheptr->is_32768)) {
		// ToDo
		color_cached[(cacheptr->color) & 0xff] = true;
	}
	pattern_cached[sprite_table[num].num] = true; // OK?

	switch((sprite_table[num].rotate) & 7) {
	case 0:
		rot_type = ROT_FMTOWNS_SPRITE_0;
		is_mirror = false;
		break;
	case 1:
		rot_type = ROT_FMTOWNS_SPRITE_180;
		is_mirror = true;
		break;
	case 2:
		rot_type = ROT_FMTOWNS_SPRITE_180;
		is_mirror = false;
		break;
	case 3:
		rot_type = ROT_FMTOWNS_SPRITE_0;
		is_mirror = true;
		break;
	case 4:
		rot_type = ROT_FMTOWNS_SPRITE_270;
		is_mirror = true;
		break;
	case 5:
		rot_type = ROT_FMTOWNS_SPRITE_90;
		is_mirror = false;
		break;
	case 6:
		rotate = false;
		rot_type = ROT_FMTOWNS_SPRITE_270;
		is_mirror = false;
		break;
	case 7:
		rot_type = ROT_FMTOWNS_SPRITE_90;
		is_mirror = true;
		break;
	}
	uint32_t index_num = cacheptr->attribute & 0x3ff;
	if(index_num < 128) return;
	
	uint8_t* src = &(pattern_ram[index_num << 7]);
	bool is_32768 = cacheptr->is_32768;
	bool is_halfx = cacheptr->is_halfx;
	bool is_halfy = cacheptr->is_halfy;
	
	if((cacheptr->pixels != NULL) || (cacheptr->masks != NULL)) return;
	switch(rot_type) {
	case ROT_FMTOWNS_SPRITE_00:
		rot_data_0(src, is_mirror, cacheptr->pixels, cacheptr->masks, is_32768, is_halfx, is_halfy);
		break;
	case ROT_FMTOWNS_SPRITE_90:
		rot_data_0(src, is_mirror, cacheptr->pixels, cacheptr->masks, is_32768, is_halfx, is_halfy);
		break;
	case ROT_FMTOWNS_SPRITE_180:
		rot_data_0(src, is_mirror, cacheptr->pixels, cacheptr->masks, is_32768, is_halfx, is_halfy);
		break;
	case ROT_FMTOWNS_SPRITE_270:
		rot_data_0(src, is_mirror, cacheptr->pixels, cacheptr->masks, is_32768, is_halfx, is_halfy);
		break;
	}
	// ToDo: wrap round.This is still bogus implement.
	// ToDo: Separate writing buffer and integrate cache.
	// copy cache to buffer
	render_zoomed_pixels(x, y, cacheptr->pixels, cacheptr->masks, cacheptr->is_halfx, cacheptr->is_halfy, dst_pixel, dst_mask);
	return;
}

void TOWNS_SPRITE::render_zoomed_pixels(int x, int y, int uint16_t* pixels, uint16_t* masks, bool is_halfx, bool is_halfy, uint16_t* dst_pixel, uint16_t* dst_mask)
{
	static const int stride = 256;
	uint16_t* pp = cache_index[target_num].pixels;
	uint16_t* pq = cache_index[target_num].masks;
	uint16_t* pd;
	uint16_t* pm;
	int w, h;
	int beginx, beginy;
	bool is_wrapx = false;
	bool is_wrapy = false;
	int offset;
	int ww, hh;
	w = 16;
	h = 16;
	beginx = 0;
	beginy = 0;
	if(is_halfx) {
		w = 8;
	}
	if(is_halfy) {
		h = 8;
	}
	if((x < 0) || (y < 0)) return;
	if((x > 511) || (y > 511)) return;
	if((x >= (512 - w)) && (x < 512)) {
		beginx = x - (512 - 16);
		ww = w - beginx; 
		is_wrapx = true;
	} else if((x >= (256 - w)) && (x < 256)) {
		beginx = 0;
		ww = 256 - x;
	} else {
		ww = w;
	}
	if((y >= (512 - h)) && (y < 512)) {
		beginy = y - (512 - 16);
		hh = h - beginy;
		is_wrapy = true;
	} else if((y >= 256 - h) && (y < 256)) {
		beginy = 0;
		hh = 256 - y;
	} else {
		hh = h;
	}
	if(!(is_wrapx) && !(is_wrapy)) {
		if((hh <= 0) || (ww <= 0)) return;
	}
	if(is_wrapx) { // Check Y
		if(y >= 256) return;
	}
	if(is_wrapy) { // Check Y
		if(x >= 256) return;
	}
	if(is_wrapy) {
		offset = 0 + (is_wrapx) ? 0 : x;
	} else if(is_wrapx) {
		offset = (y * stride) + 0;
	} else {
		if((x >= 256) || (y >= 256)) return;
		offset = (y * stride) + x;
	}
	// ToDo: Add offset registers.
	
	uint16_t cacheline[16];
	uint16_t mcacheline[16];
	uint16_t pcacheline[16];
	uint16_t mcacheline2[16];
	int cache_stride = (is_halfx) ? 3 : 4;

	pd = &(dst_pixels[offset]);
	pm = &(dst_mask[offset]);
	for(int y = beginy; y < (hh + beginy); y++) {
		uint16_t* rp = &(pp[(y << cache_stride) + beginx]);
		uint16_t* rq = &(pq[(y << cache_stride) + beginx]);
__DECL_VECTORIZED_LOOP						
		for(int x = 0; x < ww; x++) {
			cacheline[x] = rp[x];
			mcacheline[x] = rq[x];
			pcacheline[x] = pd[x];
		}
__DECL_VECTORIZED_LOOP
		for(int x = 0; x < ww; x++) {
			pm[x] = pm[x] | mcacheline[x]; // Fill mask what pixel is drawn.
			
			cacheline[x] = cacheline[x] & mcacheline[x];
			mcacheline[x] = ~mcacheline[x]; // Invert mask
			pcacheline[x] = pcacheline[x] & mcacheline[x];
			pd[x] = cacheline[x] | pcacheline[x];
		}
		pd = pd + stride;
	}
}

#else
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
	__DECL_ALIGNED(32) uint32_t lbuf[16];
	__DECL_ALIGNED(32) uint32_t mbuf[16];
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
		
	if(!(is_halfx) && !(is_halfy)) { // not halfed
		for(int yy = 0; yy < 16;  yy++) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx] = 0;
				mbuf[xx] = 0;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx] = sbuf[yy][xx];
			}
			// void __FASTCALL VRAM::write_sprite_data(int x, int y, int xoffset, int yoffset, uint16_t *ptr __assume_aligned(16), int width);
			if(d_vram != NULL) {
				//d_vram->write_sprite_data(x, y + yy, xoffset, yoffset, lbuf, 16);
			}
		}
	} else if(is_halfx) { // halfx only
		for(int yy = 0; yy < 16;  yy++) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx] = 0;
				mbuf[xx] = 0;
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx += 2) {
				lbuf[xx >> 1] += (sbuf[yy][xx] & 0x7fff);
				mbuf[xx >> 1] |= (sbuf[yy][xx] & 0x8000);
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 8; xx++) {
				lbuf[xx] = ((lbuf[xx] >> 2) & 0x7fff) | mbuf[xx];
			}
			if(d_vram != NULL) {
				//d_vram->write_sprite_data(x, y + yy, xoffset, yoffset, lbuf, 8);
			}
		}
	} else if(is_halfy) { // halfy only
		for(int yy = 0; yy < 16;  yy += 2) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx] = 0;
				mbuf[xx] = 0;
			}
			for(int yy2 = 0; yy2 < 2; yy2++) {
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx++) {
					lbuf[xx] += (sbuf[yy + yy2][xx] & 0x7fff);
					mbuf[xx] |= (sbuf[yy + yy2][xx] & 0x8000);
				}
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx] = ((lbuf[xx] >> 1) & 0x7fff) | mbuf[xx];
			}
			if(d_vram != NULL) {
				//d_vram->write_sprite_data(x, y + (yy >>1), xoffset, yoffset, lbuf, 16);
			}
		}
	} else { //halfx &&halfy
		for(int yy = 0; yy < 16;  yy += 2) {
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 16; xx++) {
				lbuf[xx] = 0;
				mbuf[xx] = 0;
			}
			for(int yy2 = 0; yy2 < 2; yy2++) {
__DECL_VECTORIZED_LOOP						
				for(int xx = 0; xx < 16; xx += 2) {
					lbuf[xx >> 1] += (sbuf[yy + yy2][xx] & 0x7fff);
					mbuf[xx >> 1] |= (sbuf[yy + yy2][xx] & 0x8000);
				}
			}
__DECL_VECTORIZED_LOOP						
			for(int xx = 0; xx < 8; xx++) {
				lbuf[xx] = ((lbuf[xx] >> 2) & 0x7fff) | mbuf[xx];
			}
			if(d_vram != NULL) {
				//d_vram->write_sprite_data(x, y + (yy >>1), xoffset, yoffset, lbuf, 8);
			}
		}
	}
	now_transferring = false;

}

#endif
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
			render_sprite(render_num, xaddr, yaddr, _nattr.w, _ncol.w);
			frame_sprite_count++;
			if((frame_sprite_count >= max_sprite_per_frame) && (max_sprite_per_frame > 0)) break;
		}
	}
}

void TOWNS_SPRITE::render_part(int start, int end)
{
	// ToDo: Implement Register #2-5
	uint16_t lot = reg_index & 0x3ff;
	if(lot == 0) lot = 1024;
	if((start < 0) || (end < 0)) return;
	if(end > lot) end = lot;
	if(start > end) return;
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
		reg_addr = addr & 7;
	} else if(addr != 0x0452) {
		return;
	}
	write_reg(reg_addr, data);
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
	_TCHAR sstr[32] = {0};
	my_stprintf_s(sstr, 32, _T("TEXT VRAM:%s \n"), ((tvram_enabled) || (tvram_enabled_bak)) ? _T("WROTE") : _T("NOT WROTE"));
	memset(sstr, 0x00, sizeof(sstr));
	my_stprintf_s(sstr, 32, _T("A:%02X \n"), reg_addr & 0x07);
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
	if(reg_spen && !(sprite_enabled)) {
		sprite_enabled = true;
		render_num = 0;
	}
	if(lot == 0) lot = 1024;
	frame_sprite_count = 0;
	if(sprite_enabled){
		if(d_vram != NULL) {
			if(d_vram->read_signal(SIG_TOWNS_VRAM_FRAMEBUFFER_READY) != 0) {
				if(render_num >= lot) {
					d_vram->write_signal(SIG_TOWNS_VRAM_SWAP_FRAMEBUFFER, 0xffffffff, 0xffffffff);
					render_num = 0;
					render_mod = 0;
				}
				// Set split_rendering from DIPSW.
				// Set cache_enabled from DIPSW.
				if(!split_rendering) {
					render_full();
				}
			} else {
				render_num = 0;
				render_mod = 0;
				sprite_enabled = false;
			}
		} else {
			render_num = 0;
			render_mod = 0;
			sprite_enabled = false;
		}
	}
}

void TOWNS_SPRITE::do_vline_hook(int line)
{
	int lot = reg_index & 0x3ff;
	if(!split_rendering) return;
	if(lot == 0) lot = 1024;
	if((max_sprite_per_frame > 0) && (max_sprite_per_frame < lot)) lot = max_sprite_per_frame;
	
	if((sprite_enabled) && (render_lines > 0)) {
		int nf = lot / render_lines;
		int nm = lot % render_lines;
		render_mod += nm;
		if(render_mod >= render_lines) {
			nf++;
			render_mod -= render_lines;
		}
		if((nf >= 1) && (render_num < lot)) render_part(render_num, render_num + nf);
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
	 } else */ if(id == SIG_TOWNS_SPRITE_TVRAM_ENABLED) {
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
