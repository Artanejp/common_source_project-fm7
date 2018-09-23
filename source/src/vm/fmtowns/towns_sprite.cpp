/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.22 -

	[ Sprite ]
*/

#include "../../common.h"
#include "./towns_sprite.h"

void TOWNS_SPRITE::initialize(void)
{
	memset(index_ram, 0x00, sizeof(index_ram));
	memset(pattern_ram, 0x00, sizeof(pattern_ram));
	memset(color_ram, 0x00, sizeof(color_ram));
	
	for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
			memset(&(cache_index[i]), 0x00, sizeof(sprite_cache_t));
			cache_index[i].is_use = false;
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
	memset(reg_data, 0x00, sizeof(reg_data));
	
}
void TOWNS_SPRITE::clear_cache(int num)
{
	if(num >= TOWNS_SPRITE_CACHE_NUM) return;
	if(num < 0) return;
	memset(&(cache_index[num]), 0x00, sizeof(sprite_cache_t));
	cache_index[num].is_use = false;
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

void TOWNS_SPRITE::render_not_rotate(int num, uint16* dst_pixel, int width, int height, int stride)
{
	uint16_t* qp = (uint16_t*)(sprite_table[num].pixels);
	uint8_t* addr;
	uint16_t *cache_pixel = &(cache_pixels[last_put_cache_num][0]);
	uint16_t *cache_mask = cache_masks[last_put_cache_num][0];
	if(sprite_table[num].is_32768) {
	} else {
		addr = &(pattern_ram[(sprite_table[num].num - 128) << 3]);
		int ystep = (cache_index[num].is_halfy) ? 2 : 1;
		int xstep = (cache_index[num].is_halfx) ? 2 : 1;
		int w = (cache_index[num].is_halfy) ? 8 : 16;
		int h = (cache_index[num].is_halfx) ? 8 : 16;
		int color = sprite_table[num].color;
		if(color < 256) return;
		if(color > 511) return;
		color = color - 256;
		uint16_t* p;
		uint16_t* m;
		uint16_t* q;
		uint16_t* dp;
		int yy = 0;
		uint8_t pixels[16];
		uint8_t pixels_lo[16];
		uint8_t pixels_hi[16];
		uint16_t masks[16];
		uint16_t tpixels[16];
		uint16_t t2pixels[16];
		uint16_t* color_index = (uint16_t*)(&color_index_ram[color << 5]);
		for(int y = 0; y < 16; y += ystep) {
			p = &(cache_pixel[yy << 4]);
			m = &(cache_mask[yy << 4]);
			q = (uint8_t*)qp;
			q = &q[y << 3];
			dp = &(dst_pixel[yy * stride]);
			if(cache_index[num].is_halfx) {
				for(int x = 0; x < 8; x++) {
					pixels[x] = q[x];
				}
				for(int x = 0; x < 8; x++) {
					pixels[x] = pixels[x] & 0xf0;
				}
				
				for(int x = 0; x < 8; x++) {
					masks[x] = (pixels[x] == 0) ? 0xffff : 0x0000;
				}
				for(int x = 0; x < 8; x++) {
					p[x] = color_index[pixels[x]];
				}
				for(int x = 0; x < 8; x++) {
					m[x] = masks[x];
				}
				// Draw to buffer
				for(int x = 0; x < 8; x++) {
					tpixels[x] = dp[x] & masks[x];
				}
				for(int x = 0; x < 8; x++) {
					pixels[x] = p[x] & ~masks[x];
				}
				for(int x = 0; x < 8; x++) {
					dp[x] = pixels[x] | tpixels[x];
				}
			} else {
				for(int x = 0; x < 8; x++) {
					pixels[x] = q[x];
				}
				for(int x = 0; x < 8; x++) {
					pixels_hi[x] = pixels[x] & 0xf0;
				}
				for(int x = 0; x < 8; x++) {
					pixels_lo[x] = pixels[x] & 0x0f;
				}
				
				for(int x = 0; x < 8; x++) {
					masks[x << 1] = (pixels_hi[x] == 0) ? 0xffff : 0x0000;
				}
				for(int x = 0; x < 8; x++) {
					masks[(x << 1) + 1] = (pixels_lo[x] == 0) ? 0xffff : 0x0000;
				}
				for(int x = 0; x < 8; x++) {
					p[x << 1] = color_index[pixels_hi[x]];
					p[(x << 1) + 1] = color_index[pixels_lo[x]];
				}
				for(int x = 0; x < 16; x++) {
					m[x] = masks[x];
				}
				// Draw to buffer
				for(int x = 0; x < 16; x++) {
					tpixels[x] = dp[x] & masks[x];
				}
				for(int x = 0; x < 16; x++) {
					t2pixels[x] = p[x] & ~masks[x];
				}
				for(int x = 0; x < 16; x++) {
					dp[x] = t2pixels[x] | tpixels[x];
				}
			}
			yy++;
		}
	}
}


void TOWNS_SPRITE::render_sprite(int num, uint16* dst_pixel, int width, int height, int stride)
{
	if(num < 0) return;
	if(num >= sprite_limit) return;
	if(num >= 1024) return;
	if(stride <= 0) return;
	if(stride > 512) return;
	if(!(sprite_table[num].is_disp)) return;
	
	for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
		if(!(cache_index[i].is_use)) continue;
		if(cache_index[i].attribute == sprite_table[num].attribute) {
			if(cache_index[i].is_32768 == sprite_table[num].is_32768) {
				if(cache_index[i].is_32768) {
					//
					int h = (cache_index[i].is_halfy) ? 8 : 16;
					int w = (cache_index[i].is_halfx) ? 8 : 16;
					if(height < h) h = height;
					if(width < w)  w = width;
					uint16_t* pp = (uint16_t*)dst_pixel;
					uint16_t* qp = (uint16_t*)(cache_index[i].pixels);
					uint16_t* qm = (uint16_t*)(cache_index[i].masks);
					uint16_t* ppp = pp;
					int np = 0;
					
					for(int y = 0; y < h; y++) {
						uint16_t* rp = &qp[y << 4];
						uint16_t* rm = &qm[y << 4];
						ppp = pp;
						for(x = 0; x < w; x++) {
							uint16_t pixel = *rp;
							uint16_t dpixel = *ppp;
							pixel = pixel & *rm;
							dpixel = dpixel & ~(*rm);
							dpixel = dpixel | pixel;
							*tp = dpixel;
							ppp++;
						}
						pp = pp + stride;
					}
					return;
				} else { // 16 colors
					if(cache_index[i].color != sprite_table[num].color) continue;
					if(cache_index[i].color < 256) continue;
					if(cache_index[i].color > 511) continue;
					//
					int h = (cache_index[i].is_halfy) ? 8 : 16;
					int w = (cache_index[i].is_halfx) ? 8 : 16;
					if(height < h) h = height;
					if(width < w)  w = width;
					uint16_t* pp = (uint16_t*)dst_pixel;
					uint16_t* qp = (uint16_t*)(cache_index[i].pixels);
					uint16_t* qm = (uint16_t*)(cache_index[i].masks);
					uint16_t* ppp = pp;
					int np = 0;
					
					for(int y = 0; y < h; y++) {
						uint16_t* rp = &qp[y << 4];
						uint16_t* rm = &qm[y << 4];
						ppp = pp;
						for(x = 0; x < w; x++) {
							uint16_t pixel = *rp;
							uint16_t dpixel = *ppp;
							pixel = pixel & *rm;
							dpixel = dpixel & ~(*rm);
							dpixel = dpixel | pixel;
							*tp = dpixel;
							ppp++;
						}
						pp = pp + stride;
					}
					return;
				}
			}
		}
	}
	// Cache Not hit
	int target_num = -1;
	for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
		if(!(cache_index[i].is_use)) {
			target_num = i;
			break;
		}
	}
	if(target_num < 0) {
		target_num = (last_put_cache_num + 1) % TOWNS_SPRITE_CACHE_NUM;
	}
	last_put_cache_num = target_num; 
	memset(&(cache_index[target_num]), 0x00, sizeof(sprite_cache_t));
	cache_index[target_num].is_use = true;
	cache_index[target_num].attribute = sprite_table[num].attribute;
	cache_index[target_num].is_32768 = sprite_table[num].is_32768;
	cache_index[target_num].is_halfx = sprite_table[num].is_halfx;
	cache_index[target_num].is_halfy = sprite_table[num].is_halfy;
	cache_index[target_num].pixels = (uint16_t*)(&(cache_pixels[target_num][0]));
	cache_index[target_num].masks = (uint16_t*)(&(cache_masks[target_num][0]));
	cache_index[target_num].color = sprite_table[num].color;
	cache_index[target_num].num = sprite_table[num].num;
	color_cached[(cache_index[target_num].color) & 0xff] = true;
	pattern_cached[sprite_table[num].num] = true; // OK?

	render_base(num, dst_pixel, width, height, stride);
}

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
		tval.w.l = index_ram[(addr & 0x1ffe) >> 1];
		if((addr & 1) == 0) { // Lo
			tval.b.l = (uint8_t)(data & 0xff);
		} else {
			tval.b.h = (uint8_t)(data & 0xff);
		}
		index_ram[(addr & 0x1ffe) >> 1] = tval.w.l;
		break;
	case 2:
	case 3:
		// ToDO: Discard cache
		tval.w.l = color_ram[(addr & 0x1ffe) >> 1];
		if((addr & 1) == 0) { // Lo
			tval.b.l = (uint8_t)(data & 0xff);
		} else {
			tval.b.h = (uint8_t)(data & 0xff);
		}
		color_ram[(addr & 0x1ffe) >> 1] = tval.w.l;
		if(color_cached[(addr & 0x1ffe) >> 5]) {
			uint32_t nnum = (addr & 0x1ffe) >> 5;
			for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
				if(cache_index[i].color == (uint16_t)(nnum + 256)) {
					if(cache_index[i].is_use) {
						clear_cache(i);
					}
				}
			}
			color_cached[nnum] = false;
		}
		break;
	default:
		// ToDO: Discard cache
		p8 = &(pattern_ram[(addr & 0x1ffff) - 0x4000]);
		*p8 = (uint8_t)(data & 0xff);
		if(pattern_cached[((addr & 0x1fffe) - 0x4000) >> 7]) {
			uint32_t nnum = ((addr & 0x1fffe) - 0x4000) >> 7;
			for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
				if(cache_index[i].num == (uint16_t)nnum) {
					if(cache_index[i].is_use) {
						clear_cache(i);
					}
				}
			}
			pattern_cached[((addr & 0x1fffe) - 0x4000) >> 7] = false;
		}
		break;
	}
	return;
}

void TOWNS_SPRITE::write_data16(uint32_t addr, uint32_t data)
{
	uint32_t nbank;
	uint16_t* p;
	pair_t tval;
	if((addr >= 0x81000000) && (addr < 0x81020000)) {
		nbank = (addr & 0x1e000) >> 12;
	} else {
		nbank = 0; // OK?
	}
	switch(nbank) {
	case 0:
	case 1:
		index_ram[(addr & 0x1ffe) >> 1] = (uint16_t)(data & 0xffff);
		break;
	case 2:
	case 3:
		// ToDO: Discard cache
		color_ram[(addr & 0x1ffe) >> 1] = (uint16_t)(data & 0xffff);
		if(color_cached[(addr & 0x1ffe) >> 5]) {
			uint32_t nnum = (addr & 0x1ffe) >> 5;
			for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
				if(cache_index[i].color == (uint16_t)(nnum + 256)) {
					if(cache_index[i].is_use) {
						clear_cache(i);
					}
				}
			}
			color_cached[nnum] = false;
		}
		break;
	default:
		// ToDO: Discard cache
		tval.w.l = (uint16_t)data;
		pattern_ram[(addr & 0x1fffe) - 0x4000] = tval.b.l;		
		pattern_ram[(addr & 0x1fffe) - 0x4000 + 1] = tval.b.h;
		if(pattern_cached[((addr & 0x1fffe) - 0x4000) >> 7]) {
			uint32_t nnum = ((addr & 0x1fffe) - 0x4000) >> 7;
			for(int i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
				if(cache_index[i].num == (uint16_t)nnum) {
					if(cache_index[i].is_use) {
						clear_cache(i);
					}
				}
			}
			pattern_cached[((addr & 0x1fffe) - 0x4000) >> 7] = false;
		}
		break;
	}
	return;
}

void TOWNS_SPRITE::write_data32(uint32_t addr, uint32_t data)
{
	pair_t t;
	t.d = data;
	write_data16(addr, (uint32_t)(t.w.l));
	if(addr < 0x8101fffe) write_data16(addr + 2, (uint32_t)(t.w.h));
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
	
	DECL_STATE_ENTRY_INT32(sprite_limit);
	
	DECL_STATE_ENTRY_1D_ARRAY(index_ram, sizeof(index_ram) / sizeof(uint16_t));
	DECL_STATE_ENTRY_1D_ARRAY(pattern_ram, sizeof(pattern_ram) / sizeof(uint8_t));
	DECL_STATE_ENTRY_1D_ARRAY(color_ram, sizeof(color_ram) / sizeof(uint16_t));

	DECL_STATE_ENTRY_INT32(last_put_cache_num);		
	DECL_STATE_ENTRY_2D_ARRAY(cache_pixels, TOWNS_SPRITE_CACHE_NUM, 16 * 16);
	DECL_STATE_ENTRY_2D_ARRAY(cache_masks, TOWNS_SPRITE_CACHE_NUM, 16 * 16);
	DECL_STATE_ENTRY_1D_ARRAY(pattern_cached, (sizeof(pattern_cached) / sizeof(bool)));
	DECL_STATE_ENTRY_1D_ARRAY(color_cached, (sizeof(pattern_cached) / sizeof(bool)));
	
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_use), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT16_STRIDE((cache_index[0].attribute), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_32768), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_halfx), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_BOOL_STRIDE((cache_index[0].is_halfy), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT16_STRIDE((cache_index[0].color), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	DECL_STATE_ENTRY_UINT16_STRIDE((cache_index[0].num), TOWNS_SPRITE_CACHE_NUM, sizeof(sprite_cache_t));
	
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
	// If load state, clear cache.
	for(i = 0; i < TOWNS_SPRITE_CACHE_NUM; i++) {
		cache_index[i].pixels = (uint16_t*)(&(cache_pixels[i][0]));
		cache_index[i].masks = (uint16_t*)(&(cache_masks[i][0]));
	}
	return true;
}
