/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2017.01.16 Initial.
*/

#include "common.h"
#include "./towns_vram.h"

#define CLEAR_COLOR RGBA_COLOR(0,0,0,0)

#if defined(_RGB888)
#define _USE_ALPHA_CHANNEL
#endif

void TOWNS_VRAM::initialize()
{
#ifdef _USE_ALPHA_CHANNEL
	for(int i = 0; i < 32768; i++) {
		uint8_t g = (i / (32 * 32)) & 0x1f;
		uint8_t r = (i / 32) & 0x1f;
		uint8_t b = i & 0x1f;
		table_32768c[i] = RGBA_COLOR(r << 3, g << 3, b << 3, 0xff);
	}
	for(int i = 32768; i < 65536; i++) {
		table_32768c[i] = _CLEAR_COLOR;
	}
#endif
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_rendered[0][i] = false;
		line_rendered[1][i] = false;
	}

}
uint32_t TOWNS_VRAM::read_data8(uint32_t addr)
{
	uint8_t *p;
	uint8_t tmp_m, tmp_d;
	uint8_t mask;

	if((addr & 1) == 0) {
		mask = packed_access_mask_lo;
	} else {
		mask = packed_access_mask_hi;
	}
	p = &(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xffffffff]);
	tmp_m = *p;
	//tmp_m = tmp_m & ~mask;
	tmp_d = tmp_m & mask; // Is unreaderble bits are '0'?
	return (uint32_t) tmp_d;
}

uint32_t TOWNS_VRAM::read_data16(uint32_t addr)
{
	uint16_t *p;
	uint16_t tmp_m, tmp_d;
	uint16_t mask;
	pair_t n;

	n.b.l = packed_access_mask_lo;
	n.b.h = packed_access_mask_hi;
	mask = n.w.l;

	p = &(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xfffffffe]);
	tmp_m = *p;
	//tmp_m = tmp_m & ~mask;
	tmp_d = tmp_m & mask; // Is unreaderble bits are '0'?
	return (uint32_t) tmp_d;
}

uint32_t TOWNS_VRAM::read_data32(uint32_t addr)
{
	uint32_t *p;
	uint32_t tmp_m, tmp_d;
	uint32_t mask;
	pair_t n;

	n.b.l = packed_access_mask_lo;
	n.b.h = packed_access_mask_hi;
	n.b.h2 = packed_access_mask_lo;
	n.b.h3 = = packed_access_mask_hi;
	
	mask = n.d;
	p = &(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xfffffffc]);
	tmp_m = *p;
	//tmp_m = tmp_m & ~mask;
	tmp_d = tmp_m & mask; // Is unreaderble bits are '0'?
	return tmp_d;
}

void TOWNS_VRAM::write_data8(uint32_t addr, uint32_t data)
{
	uint8_t *p;
	uint8_t tmp_m, tmp_d;
	uint8_t mask;

	if((addr & 1) == 0) {
		mask = packed_access_mask_lo;
	} else {
		mask = packed_access_mask_hi;
	}
	p = &(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xffffffff]);
	tmp_m = *p;
	tmp_m = tmp_m & ~mask;
	tmp_d = data & mask;
	*p = tmp_d | tmp_m;
}

void TOWNS_VRAM::write_data16(uint32_t addr, uint32_t data)
{
	uint16_t *p;
	uint16_t tmp_m, tmp_d;
	uint16_t mask;
	pair_t n;

	n.b.l = packed_access_mask_lo;
	n.b.h = packed_access_mask_hi;
	mask = n.w.l;
	p = (uint16_t *)(&(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xfffffffe]));
	tmp_m = *p;
	tmp_m = tmp_m & ~mask;
	tmp_d = data & mask;
	*p = tmp_d | tmp_m;
}

void TOWNS_VRAM::write_data32(uint32_t addr, uint32_t data)
{
	uint32_t *p;
	uint32_t tmp_m, tmp_d;
	uint32_t mask;
	pair_t n;

	n.b.l = packed_access_mask_lo;
	n.b.h = packed_access_mask_hi;
	n.b.h2 = packed_access_mask_lo;
	n.b.h3 = = packed_access_mask_hi;
	
	mask = n.d;
	p = (uint32_t *)(&(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xfffffffc]));
	tmp_m = *p;
	tmp_m = tmp_m & ~mask;
	tmp_d = data & mask;
	*p = tmp_d | tmp_m;
}

void TOWNS_VRAM::write_plane_data8(uint32_t addr, uint32_t data)
{
	// Plane Access
	pair_t data_p;
	uint32_t x_addr = 0;
	uint8_t *p;
	uint32_t mod_pos;

	if(access_page1) x_addr = 0x20000; //?
	p = &(vram[x_addr]); 

	// 8bit -> 32bit
	uint32_t *pp = (uint32_t *)p;
	uint32_t tmp = 0;
	uint32_t tmp_d = data & 0xff;
	uint32_t tmp_m = 0xf0000000 & write_plane_mask;
	uint32_t tmp_r;

	for(int i = 0; i < 8; i++) {
		if((tmp_d & 0x80)) tmp |= tmp_m;
		tmp_d << 1;
		tmp_m >>= 4;
	}
	tmp_r = pp[addr];
	tmp_r = tmp_r & ~write_plane_mask;
	tmp_r = tmp_d | tmp_r;
	pp[addr] = tmp_r;
}

void TOWNS_VRAM::write_plane_data16(uint32_t addr, uint32_t data)
{
	pair_t d;
	d.d = data;
	write_plane_data8(addr, d.b.l);
	write_plane_data8(addr + 1, d.b.h);
}

void TOWNS_VRAM::write_plane_data16(uint32_t addr, uint32_t data)
{
	write_plane_data8(addr, data & 0xff);
	write_plane_data8(addr + 1, (data >> 8) & 0xff);
}

void TOWNS_VRAM::write_plane_data32(uint32_t addr, uint32_t data)
{
	write_plane_data8(addr, data & 0xff);
	write_plane_data8(addr + 1, (data >> 8) & 0xff);
	write_plane_data8(addr + 2, (data >> 16) & 0xff);
	write_plane_data8(addr + 3, (data >> 24) & 0xff);
}

// I/Os
// Palette.
void TOWNS_CRTC::calc_apalette16(int layer, int index)
{
	if(index < 0) return;
	if(index > 15) return;
	apalette_16_rgb[layer][index] =
		((uint16_t)(apalette_b & 0x0f)) |
		((uint16_t)(apalette_r & 0x0f) << 4) |
		((uint16_t)(apalette_g & 0x0f) << 8);
	if(index == 0) {
		apalette_16_pixel[layer][index] = _CLEAR_COLOR; // ??
	} else {
		apalette_16_pixel[layer][index] = RGBA_COLOR((apalette_r & 0x0f) << 4, (apalette_g & 0x0f) << 4, (apalette_b & 0x0f) << 4, 0xff);
	}
}

void TOWNS_CRTC::calc_apalette256(int index)
{
	if(index < 0) return;
	if(index > 255) return;
	apalette_256_rgb[layer][index] =
		((uint32_t)apalette_b) |
		((uint32_t)apalette_r << 8) |
		((uint32_t)apalette_g << 16);
	if(index == 0) {
		apalette_256_pixel[index] = _CLEAR_COLOR; // ??
	} else {
		apalette_256_pixel[index] = RGBA_COLOR(apalette_r, apalette_g, apalette_b, 0xff);
	}
}

void TOWNS_CRTC::set_apalette_r(int layer, uint8_t val)
{
	apalette_r = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_CRTC::set_apalette_g(int layer, uint8_t val)
{
	apalette_g = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_CRTC::set_apalette_b(int layer, uint8_t val)
{
	apalette_b = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_CRTC::set_apalette_num(int layer, uint8_t val)
{
	apalette_code = ((int)val) % 256;
}

// Renderers
void TOWNS_CRTC::render_line_16(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint32_t src;
	uint32_t src1, src2;
	uint8_t  srcdat1[4], srcdat2[1];
	scrntype_t *pdst = framebuffer;
	uint32_t *pp = (uint32_t *)vramptr;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	for(ip = 0; ip < nwords; ip++) {
		src = pp[ip];
		src1 = (src & 0xf0f0f0f0) >> 4;
		src2 = src & 0x0f0f0f0f;
		
		srcdat1[0] = (uint8_t)(src1 >> 24);
		srcdat1[1] = (uint8_t)(src1 >> 16);
		srcdat1[2] = (uint8_t)(src1 >> 8);
		srcdat1[3] = (uint8_t)(src1 >> 0);
		
		srcdat2[0] = (uint8_t)(src2 >> 24);
		srcdat2[1] = (uint8_t)(src2 >> 16);
		srcdat2[2] = (uint8_t)(src2 >> 8);
		srcdat2[3] = (uint8_t)(src2 >> 0);
		/*
		srcdat[0] = (uint8_t)((src & 0xf0000000) >> 28);
		srcdat[1] = (uint8_t)((src & 0x0f000000) >> 24);
		srcdat[2] = (uint8_t)((src & 0x00f00000) >> 20);
		srcdat[3] = (uint8_t)((src & 0x000f0000) >> 16);
		srcdat[4] = (uint8_t)((src & 0x0000f000) >> 12);
		srcdat[5] = (uint8_t)((src & 0x00000f00) >> 8);
		srcdat[6] = (uint8_t)((src & 0x000000f0) >> 4);
		srcdat[7] = (uint8_t)(src & 0x0f);
		for(int i = 0; i < 8; i++) {
			pdst[i] = apalette_16_pixel[layer][srcdat[i]];
		}
		pdst += 8;
		*/
		for(int i = 0; i < 4; i++) {
			pdst[0] = apalette_16_pixel[layer][srcdat1[i]];
			pdst[1] = apalette_16_pixel[layer][srcdat2[i]];
			pdst += 2;
		}
	}
	int mod_words = words - (nwords * 8);
	if(mod_words > 0) {
		uint8_t *ppp = (uint8_t *)(&pp[ip]);
		uint8_t hi, lo;
		for(int i = 0; i < mod_words / 2; i++) {
			uint8_t d = ppp[i];
			hi = (d & 0xf0) >> 4;
			lo = d & 0x0f;
			*pdst++ = apalette_16_pixel[layer][hi];
			*pdst++ = apalette_16_pixel[layer][lo];
		}
		if((mod_words & 1) != 0) {
			hi = (ppp[mod_words / 2] & 0xf0) >> 4;
			*pdst++ = apalette_16_pixel[layer][hi];
		}
	}
}

void TOWNS_CRTC::render_line_256(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 4;
	int ip;
	uint32_t src;
	uint8_t  srcdat[4];
	scrntype_t *pdst = framebuffer;
	uint32_t *pp = (uint32_t *)vramptr;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	for(ip = 0; ip < nwords; ip++) {
		src = pp[ip];
		srcdat[0] = (uint8_t)((src & 0xff000000) >> 24);
		srcdat[1] = (uint8_t)((src & 0x00ff0000) >> 16);
		srcdat[2] = (uint8_t)((src & 0x0000ff00) >> 8);
		srcdat[3] = (uint8_t) (src & 0x000000ff);
		for(int i = 0; i < 4; i++) {
			pdst[i] = apalette_256_pixel[srcdat[i]];
		}
		pdst += 4;
	}
	int mod_words = words - (nwords * 4);
	if(mod_words > 0) {
		uint8_t src8;
		uint8_t *p8 = (uint8_t *)(&pp[ip]);
		for(int i = 0; i < mod_words; i++) {
			src8 = p8[i];
			pdst[i] = apalette_256_pixel[src8];
		}
	}
}

// To be used table??
void TOWNS_CRTC::render_line_32768(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint16_t src16;
	uint32_t  srcdat[4];
	scrntype_t *pdst = framebuffer;
	uint32_t *pp = (uint32_t *)vramptr;
	uint16_t *cachep = (uint16_t *)srcdat;
	int i = 0;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	
	for(ip = 0; ip < nwords; ip++) {
		for(int i = 0; i < 4; i++) {
			srcdat[i] = pp[i];
		}
		pp = pp + 4;
		scrntype_t dcache[8];
		for(int i = 0; i < 8; i++) {
			dcache[i] = _CLEAR_COLOR;
		}
		for(int i = 0; i < 8; i++) {
			uint16_t v = cachep[i];
#ifdef _USE_ALPHA_CHANNEL
#ifdef __BIG_ENDIAN__
			pair_t n;
			n.d = 0;
			n.b.l = v & 0xff;
			n.b.h = (v & 0xff00) >> 8;
			dcache[i] = table_32768c[n.sw.l];
#else
			dcache[i] = table_32768c[v];
#endif
#else
			if((v & 0x8000) == 0) {
				dcache[i] = RGBA_COLOR((v & 0x03e0) >> 2, (v & 0x7c00) >> 7, (v & 0x001f) << 3, 0xff); // RGB555 -> PIXEL
			}
#endif
		}
		for(int i = 0; i < 8; i++) {
			pdst[i] = dcache[i];
		}
		pdst += 8;
	}
	int mod_words = words - nwords * 8;
	if(mod_words > 0) {
		uint16_t *vp = (uint16_t *)pp;
		scrntype_t dc;
		for(int i = 0; i < mod_words; i++) {
			src16 = vp[i];
#ifdef _USE_ALPHA_CHANNEL
#ifdef __BIG_ENDIAN__
			pair_t n;
			n.d = 0;
			n.b.l = src16 & 0xff;
			n.b.h = (src16 & 0xff00) >> 8;
			dc = table_32768c[n.sw.l];
#else
			dc = table_32768c[src16];
#endif
#else
			dc = _CLEAR_COLOR;
			if((src16 & 0x8000) == 0) {
				dc = RGBA_COLOR((src16 & 0x03e0) >> 2, (src16 & 0x7c00) >> 7, (src16 & 0x001f) << 3, 0xff); // RGB555 -> PIXEL
			}
#endif
			pdst[i] = dc;
		}
	}
}
#undef _CLEAR_COLOR

