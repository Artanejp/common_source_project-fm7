/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.22 -

	[ Sprite ]
*/

#include "../../common.h"
#include "./towns_sprite.h"

namespace FMTOWNS {
	
void TOWNS_SPRITE::initialize(void)
{
	memset(index_ram, 0x00, sizeof(index_ram));
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	memset(color_ram, 0x00, sizeof(color_ram));
	
	for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
			memset(&(cache_pixels[i][0]) , 0x00, sizeof(uint16_t) * 16 * 16);
			memset(&(cache_masks[i][0]) , 0x00, sizeof(uint16_t) * 16 * 16);
			memset(&(cache_index[i]), 0x00, sizeof(sprite_cache_t));
			cache_index[i].is_use = false;
			cache_index[i].pixels = &(cache_pixels[i][0]);
			cache_index[i].masks  = &(cache_masks[i][0]);
	}
	last_put_cache_num = 0;
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	disp_page1 = false;
	disp_page0 = false;
	reg_spen = false;
	reg_addr = 0;
	memset(reg_data, 0x00, sizeof(reg_data));
	for(int i = 0; i < (sizeof(pattern_cached) / sizeof(bool)); i++) {
		pattern_cached[i] = false;
	}
	for(int i = 0; i < 256; i++) {
		color_cached[i] = false;
	}
	use_cache = false; // ToDo: Enable cache.
	render_num = 0;
	render_mod = 0;
	render_lines = 0;
	split_rendering = false;
	vram_buffer = NULL;
	mask_buffer = NULL;
}

void TOWNS_SPRITE::reset()
{
	// Clear RAMs?
	reg_ctrl = 0x0000; // REG#00, #01
	reg_voffset = 0x0000; // REG#02, #03
	reg_hoffset = 0x0000; // REG#04, #05
	disp_page1 = false;
	disp_page0 = false;
	reg_spen = false;
	reg_addr = 0;
	render_num = 0;
	render_mod = 0;
	render_lines = 0;
	memset(reg_data, 0x00, sizeof(reg_data)); // OK?
	// ToDo: Is these right?
	write_page = 1;
	display_page = 0;
	// Is clear cache?
	// Is clear buffers?
}
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

void TOWNS_SPRITE::set_sprite_attribute(int table_num, uint16_t num_attr)
{
	if((table_num < 0) || (table_num > 1023)) return;
	uint16_t num = num_attr & 0x3ff;
	uint8_t rotate_type = (uint8_t)((num_attr & 0x7000) >> 12);
	bool halfx = ((num_attr &  0x0400) != 0);
	bool halfy = ((num_attr &  0x0800) != 0);
	bool enable_offset = ((num_attr & 0x8000) != 0);
	
	sprite_table[table_num].num = num;
	sprite_table[table_num].rotate_type = rotate_type;
	sprite_table[table_num].is_halfx = halfx;
	sprite_table[table_num].is_halfy = halfy;
	sprite_table[table_num].offset = enable_offset;
	sprite_table[table_num].attribute = num_attr & 0x7fff;
}

void TOWNS_SPRITE::set_sprite_color(int table_num, uint16_t color_table_num)
{
	if((table_num < 0) || (table_num > 1023)) return;
	sprite_table[table_num].color = color_table_num & 0x0fff;
	sprite_table[table_num].is_32768 = ((color_table_num & 0x8000) == 0);
	sprite_table[table_num].is_impose = ((color_table_num & 0x4000) != 0);
	sprite_table[table_num].is_disp   = ((color_table_num & 0x2000) != 0);
}

void TOWNS_SPRITE::build_sprite_table(void)
{
	uint16_t* p = index_ram;
	for(int i = 0; i < 1024; i++) {
		set_sprite_attribute(i, p[3]);
		set_sprite_color(i, p[3]);
		p = p + 4;
	}
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

void TOWNS_SPRITE::render_sprite(int num, uint16* dst_pixel, uint16_t* dst_mask, int x, int y)
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

// Q: Does split rendering per vline?
void TOWNS_SPRITE::render(uint16_t *buffer, uint16_t* mask)
{
	// ToDo: Implement Register #2-5
	uint16_t lot = reg_index & 0x3ff;
	if(lot == 0) lot = 1024;
	// Clear buffer?
	if((buffer == NULL) || (mask == NULL)) return;
	//memset(buffer, 0x00, 256 * 256 * sizeof(uint16_t));
	//memset(mask, 0x00, 256 * 256 * sizeof(uint16_t));
	// ToDo: Implement registers.
	if(reg_spen) {
		for(render_num = 0; render_num < (int)lot; render_num++) {
			uint16_t* index_base = &(index_ram[render_num << 2]);
			uint16_t xaddr = index_base[0] & 0x1ff;
			uint16_t yaddr = index_base[1] & 0x1ff;
			// ToDo: wrap round.This is still bogus implement.
			render_sprite(render_num, buffer, mask, (int)xaddr, (int)yaddr);
		}
	}
}

void TOWNS_SPRITE::render_part(uint16_t *buffer, uint16_t* mask, int start, int end)
{
	// ToDo: Implement Register #2-5
	uint16_t lot = reg_index & 0x3ff;
	if(lot == 0) lot = 1024;
	if((start < 0) || (end < 0)) return;
	if(end > lot) end = lot;
	if(start > end) return;
	// Clear buffer?
	if((buffer == NULL) || (mask == NULL)) return;
	// ToDo: Implement registers.
	if(reg_spen) {
		for(render_num = start; render_num < end; render_num++) {
			uint16_t* index_base = &(index_ram[render_num << 2]);
			uint16_t xaddr = index_base[0] & 0x1ff;
			uint16_t yaddr = index_base[1] & 0x1ff;
			// ToDo: wrap round.This is still bogus implement.
			render_sprite(render_num, buffer, mask, (int)xaddr, (int)yaddr);
		}
	}
}

// ToDo: Discard cache(s) if dirty color index and if used this cache at 16 colors.
// ToDo: Discard cache(s) if dirty 
void TOWNS_SPRITE::write_io8(uint32_t addr, uint32_t data)
{
	reg_addr = addr & 7;
	reg_data[reg_addr] = (uint8_t)data;
	
	switch(reg_addr) {
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
		disp_page0 = ((data & 0x01) != 0) ? true : false;
		disp_page1 = ((data & 0x10) != 0) ? true : false;
		break;
	default:
		break;
	}
}

uint32_t TOWNS_SPRITE::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	reg_addr = addr & 7;
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


uint32_t TOWNS_SPRITE::read_data8(uint32_t addr)
{
	uint32_t nbank;
	uint8_t* p8;
	uint16_t val;
	pair_t tval;
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		nbank = (addr & 0x1e000) >> 12;
	} else {
		nbank = 0; // OK?
	}
	switch(nbank) {
	case 0:
	case 1:
		tval.w.l = index_ram[(addr & 0x1ffe) >> 1];
		if((addr & 1) == 0) { // Lo
			val = (uint16_t)(tval.b.l);
		} else {
			val = (uint16_t)(tval.b.h);
		}
		break;
	case 2:
	case 3:
		tval.w.l = color_ram[(addr & 0x1ffe) >> 1];
		if((addr & 1) == 0) { // Lo
			val = (uint16_t)(tval.b.l);
		} else {
			val = (uint16_t)(tval.b.h);
		}
		break;
	default:
		p8 = &(pattern_ram[(addr & 0x1fffe) - 0x4000]);
		if((addr & 1) == 0) { // Lo
			val = p8[0];
		} else {
			val = p8[1];
		}
		break;
	}
	return (uint32_t)val;
}

uint32_t TOWNS_SPRITE::read_data16(uint32_t addr)
{
	uint32_t nbank;
	uint8_t* p8;
	pair_t tval;
	uint16_t val;
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		nbank = (addr & 0x1e000) >> 12;
	} else {
		nbank = 0; // OK?
	}
	switch(nbank) {
	case 0:
	case 1:
		val = (uint32_t)(index_ram[(addr & 0x1ffe) >> 1]);
		break;
	case 2:
	case 3:
		val = (uint32_t)(color_ram[(addr & 0x1ffe) >> 1]);
		break;
	default:
		p8 = &(pattern_ram[(addr & 0x1fffe) - 0x4000]);
		tval.b.l = p8[0];
		tval.b.h = p8[1];
		val = (uint32_t)(tval.w.l);
		break;
	}
	return (uint32_t)val;
}

uint32_t TOWNS_SPRITE::read_data32(uint32_t addr)
{
	uint32_t hi, lo = 0;
	lo = read_data16(addr);
	if(addr < 0x8101fffe) hi = read_data16(addr + 2);
	return ((hi << 16) & 0xffff0000) | (lo & 0x0000ffff);
}

void TOWNS_SPRITE::write_data8(uint32_t addr, uint32_t data)
{
	uint32_t nbank;
	uint32_t uaddr;
	uint16_t tmp16;
	uint8_t tmp8;
	uint8_t* p8;
	pair_t tval;
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		nbank = (addr & 0x1e000) >> 12;
	} else {
		nbank = 0; // OK?
	}
	
	switch(nbank) {
	case 0:
	case 1:
		uaddr = (addr & 0x1ffe) >> 1;
		tval.w.l = index_ram[uaddr];
		tmp16 = tval.w.l;
		if((addr & 1) == 0) { // Lo
			tval.b.l = (uint8_t)(data & 0xff);
		} else {
			tval.b.h = (uint8_t)(data & 0xff);
		}
		if(use_cache) {
			if(uaddr == 2) { // ATTR 
				if((tmp16 & 0x7c00) != (tval.w.l & 0x7c00)) {
					// Search cache and Discard cache
				}
			} else if(uaddr == 3) {
				if((tmp16 & 0x8fff) != (tval.w.l & 0x8fff)) {
					// Search cache and Discard cache
				}
			}
		}
		index_ram[uaddr] = tval.w.l;
		break;
	case 2:
	case 3:
		// ToDO: Discard cache
		uaddr = (addr & 0x1ffe) >> 1;
		tval.w.l = color_ram[uaddr];
		tmp16 = tval.w.l;
		if((addr & 1) == 0) { // Lo
			tval.b.l = (uint8_t)(data & 0xff);
		} else {
			tval.b.h = (uint8_t)(data & 0xff);
		}
		if(use_cache) {
			if(tmp16 != tval.w.l) { // Dirty color table
				uint32_t nnum = uaddr >> 4;
				color_ram[uaddr] = tval.w.l;
				if(color_cached[nnum]) {
					for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
						if(cache_index[i].color == (uint16_t)(nnum + 256)) {
							if((cache_index[i].is_use) && !(cache_index[i].is_32768)) {
								clear_cache(i);
							}
						}
					}
					color_cached[nnum] = false;

				}
			} else {
				color_ram[uaddr] = tval.w.l;
			}
			break;
		default:
			// ToDO: Discard cache
			uaddr = (addr & 0x1ffff) - 0x4000;
			p8 = &(pattern_ram[uaddr]);
			tmp8 = *p8;
			if(use_cache) {
				if((uint8_t)(data & 0xff) != tmp8) { // Dirty pattern memory.
					*p8 = (uint8_t)(data & 0xff);
					uint32_t nnum = uaddr >> 7;
					uint32_t nnum_bak = nnum;
					if(pattern_cached[nnum]) {  // ToDo: Search another number.
						for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
							if(cache_index[i].is_32768) {
								if(cache_index[i].num == (uint16_t)nnum) {
									if(cache_index[i].is_use) {
										clear_cache(i);
									}
								}
							} else {
								uint32_t begin;
								uint32_t end;
								uint32_t clen = 0;
								// OK?
								begin = (nnum <= (128 + 3)) ? (128 + 3) : nnum - 3;
								end = (nnum <= 128) ? 128 : nnum + 3;
								if(begin < 1024) {
									if(end > 1023) end = 1023;
									if((cache_index[i].num >= begin) && (cache_index[i].num <= end)) { 
										clen = end - begin + 1;
										for(uint32_t j = 0; j < clen; j++) {
											if(cache_index[i].num == (uint16_t)(begin + j)) {
												if(cache_index[i].is_use) {
													clear_cache(i);
												}
											}
										}
									}
								}
							}
						}
						pattern_cached[nnum] = false;
					}
				
				}
			} else {
				*p8 = (uint8_t)(data & 0xff);
			}		
		}
		break;
	}
	return;
}
void TOWNS_SPRITE::write_data16(uint32_t addr, uint32_t data)
{
	pair_t t;
	t.d = data;
	write_data8(addr, (uint32_t)(t.b.l));
	write_data8(addr + 1, (uint32_t)(t.b.h));
}
				
void TOWNS_SPRITE::write_data32(uint32_t addr, uint32_t data)
{
	pair_t t;
	t.d = data;
	write_data8(addr, (uint32_t)(t.b.l));
	write_data8(addr + 1, (uint32_t)(t.b.h));
	if(addr < 0x8101fffe) {
		write_data8(addr + 2, (uint32_t)(t.b.h2));
		write_data8(addr + 3, (uint32_t)(t.b.h3));
	}
}

void FMTOWNS_SPRITE::event_frame()
{
	write_page = display_page & 1;
	display_page = (displae_page + 1) & 1;
	render_num = 0;
	render_mod = 0;
	render_lines = 0;

	// Set split_rendering from DIPSW.
	// Set cache_enabled from DIPSW.
	if(vram_head != NULL) {
		vram_buffer = vram_head->get_vram_buffer_sprite(write_page);
		mask_buffer = vram_head->get_mask_buffer_sprite(write_page);
		render_lines = vram_head->get_sprite_display_lines();
	}
	memset(vram_buffer, 0x00, w * h * sizeof(uint16_t));
	memset(vram_mask, 0x00, w * h * sizeof(uint16_t));
	if(!split_rendering) render(vram_buffer, mask_buffer);
}

void FMTOWNS_SPRITE::event_vline(int v, int clock)
{
	int lot = reg_index & 0x3ff;
	if(!split_rendering) return;
	if(lot == 0) lot = 1024;
	if((sprite_enabled) && (render_lines > 0)) {
		int nf = lot / render_lines;
		int nm = lot % render_lines;
		render_mod += nm;
		if(render_mod >= render_lines) {
			nf++;
			render_mod -= render_lines;
		}
		if((nf > 1) && (render_num < lot)) render_part(vram_buffer, mask_buffer, render_num, render_num + nf);
	}
}
// Q: Is changing pages syncing to Frame?
// ToDo: Implement VRAM.
void FMTOWNS_SPRITE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_FMTOWNS_SPRITE_CACHE_ENABLE) {
		cache_enabled = ((data & mask) != 0);
	} else if(id == SIG_FMTOWNS_SPRITE_SWAP_BUFFER) {
		write_page = display_page & 1;
		display_page = (displae_page + 1) & 1;
	}
	
}
#define STATE_VERSION	1

#include "../../statesub.h"

void TOWNS_SPRITE::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT8(reg_addr);
	DECL_STATE_ENTRY_1D_ARRAY(reg_data, 8);
	
	DECL_STATE_ENTRY_BOOL(reg_spen);
	DECL_STATE_ENTRY_UINT16(reg_index);
	DECL_STATE_ENTRY_UINT16(reg_voffset);
	DECL_STATE_ENTRY_UINT16(reg_hoffset);
	DECL_STATE_ENTRY_BOOL(disp_page0);
	DECL_STATE_ENTRY_BOOL(disp_page1);

	DECL_STATE_ENTRY_BOOL(sprite_enabled);
	DECL_STATE_ENTRY_BOOL(use_cache);
	DECL_STATE_ENTRY_UINT8(write_page);
	DECL_STATE_ENTRY_UINT8(display_page);
	
	DECL_STATE_ENTRY_BOOL(split_rendering);
	DECL_STATE_ENTRY_INT32(render_num);
	DECL_STATE_ENTRY_INT32(render_mod);
	DECL_STATE_ENTRY_INT32(render_lines);
	

	// RAMs
	DECL_STATE_ENTRY_1D_ARRAY(index_ram, sizeof(index_ram) / sizeof(uint16_t));
	DECL_STATE_ENTRY_1D_ARRAY(pattern_ram, sizeof(pattern_ram) / sizeof(uint8_t));
	DECL_STATE_ENTRY_1D_ARRAY(color_ram, sizeof(color_ram) / sizeof(uint16_t));

	// Q: Is save/load caches?
	// Flags around cache.
	DECL_STATE_ENTRY_1D_ARRAY(pattern_cached, (sizeof(pattern_cached) / sizeof(bool)));
	DECL_STATE_ENTRY_1D_ARRAY(color_cached, (sizeof(pattern_cached) / sizeof(bool)));

	// Around cache.
	DECL_STATE_ENTRY_INT32(last_put_cache_num);
	
	DECL_STATE_ENTRY_2D_ARRAY(cache_pixels, TOWNS_SPRITE_CACHE_NUM, 16 * 16);
	DECL_STATE_ENTRY_2D_ARRAY(cache_masks, TOWNS_SPRITE_CACHE_NUM, 16 * 16);
	
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_use), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT16_STRIDE((cache_index[0].num), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT16_STRIDE((cache_index[0].attribute), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT8_STRIDE((cache_index[0].rotate_type), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_32768), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_halfx), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_halfy), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT16_STRIDE((cache_index[0].color), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	
	leave_decl_state();
	
}
bool TOWNS_SPRITE::save_state(FILEIO *state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
}

bool TOWNS_SPRITE::load_state(FILEIO *state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
		this->out_debug_log(_T("Load State: SPRITE: id=%d stat=%s\n"), this_device_id, (mb) ? _T("OK") : _T("NG"));
		if(!mb) return false;
	}
	// Post Process
	build_sprite_table();
	// Render?
	if(vram_head != NULL) {
		write_page = vram_head->get_sprite_write_page();
		display_page = (write_page + 1) & 1;
		// Restore cache buffer.
		for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
			cache_index[i].pixels = &(cache_pixels[i][0]);
			cache_index[i].masks  = &(cache_masks[i][0]);
		}			
		vram_buffer = vram_head->get_vram_buffer_sprite(write_page);
		mask_buffer = vram_head->get_mask_buffer_sprite(write_page);
		render(vram_buffer, mask_buffer);
	}
	return true;
}

}
