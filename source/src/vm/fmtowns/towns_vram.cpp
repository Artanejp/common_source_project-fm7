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

namespace FMTOWNS {

void TOWNS_VRAM::initialize()
{
	for(int i = 0; i < 32768; i++) {
		uint8_t g = (i / (32 * 32)) & 0x1f;
		uint8_t r = (i / 32) & 0x1f;
		uint8_t b = i & 0x1f;
		table_32768c[i] = RGBA_COLOR(r << 3, g << 3, b << 3, 0xff);
		table_32768c[i + 32768] = table_32768c[i];
		alpha_32768c[i] = RGBA_COLOR(255, 255, 255, 255);
		alpha_32768c[i + 32768] = RGBA_COLOR(0, 0, 0, 0);
		mask_32768c[i] = 0xffff;
		mask_32768c[i + 32768] = 0x0000;
	}
	for(int i = 0; i < 256; i++) {
		int chigh = i & 0xf0;
		int clow  = i & 0x0f;
		uint8_t alpha;
		alpha_16c[ i << 2     ] = (chigh == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
		alpha_16c[(i << 2) + 1] = (clow  == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
		mask_16c[i] = ((chigh == 0) ? 0x00: 0xf0) | ((clow == 0) ? 0x00 : 0x0f);
	}
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_rendered[0][i] = false;
		line_rendered[1][i] = false;
	}

}
uint32_t TOWNS_VRAM::read_data8(uint32_t addr)
{
	// ToDo:Offset.
	// ToDo: Wait.
#ifdef __LITTLE_ENDIAN__
	uint8_t*p = (uint8_t*)vram;
	return (uint32_t)(p[addr & 0x7ffff]);
#else
	pair16_t d;
	d.w= vram[(addr & 0x7ffff) >> 1];
	if((addr & 0x01) == 0) { // Hi
		return (uint32_t)(d.b.h);
	} else { // Lo
		return (uint32_t)(d.b.l);
	}
#endif	
}



uint32_t TOWNS_VRAM::read_data16(uint32_t addr)
{
	// ToDo:Offset.
	// ToDo: Wait.
	// Host: Little endian
	// ToDo: Mask Register.
	uint32_t naddr = (addr & 0x7fffe) >> 1;
#ifdef __LITTLE_ENDIAN__
	uint16_t w = vram[naddr];
	return (uint32_t)w;
#else // __BIG_ENDIAN__	
	pair16_t nw;
	nw.w = vram[naddr];
	return (uint32_t)(nw.w);
#endif
}

uint32_t TOWNS_VRAM::read_data32(uint32_t addr)
{
	// ToDo: Wait.
	// ToDo: Mask Register.
#ifdef __LITTLE_ENDIAN__
	uint32_t*p = (uint32_t*)vram;
	uint32_t naddr = (addr & 0x7ffff) >> 2;
	uint32_t w = p[naddr];
	return w;
#else // __BIG_ENDIAN__	
	pair32_t nd;
	uint32_t naddr = (addr & 0x7ffff) >> 1;
	nd.w.l = vram[naddr];
	nd.w.h = vram[naddr + 1];;
	return nd.d;
#endif
}

void TOWNS_VRAM::write_data8(uint32_t addr, uint32_t data)
{
	// ToDo:Offset.
	// ToDo: Wait.
	// ToDo: Mask Register.
	bool dirty;
	uint32_t naddr = addr & 0x7ffff;
	
#ifdef __LITTLE_ENDIAN__
	uint8_t* p = (uint8_t*)vram;
	n = p[naddr];
#else
	pair16_t d;
	// ToDo: Mask register
	d.w = vram[naddr >> 1];
	if((addr & 0x01) != 0) { // Hi
		n = d.b.h;
		d.b.h = data;
	} else { // Lo
		n = d.b.l;
		d.b.l = data;
	}
#endif
	// ToDo: Mask register
	
	dirty =	((uint8_t)data != n) ? true : false;

	if(dirty) {
		dirty_flag[naddr >> 3] = true;
	// ToDo: Mask register
#ifdef __LITTLE_ENDIAN__
		p[naddr] = data;
#else
		vram[naddr >> 1] = d.w; 
#endif
	}
}

void TOWNS_VRAM::write_data16(uint32_t addr, uint32_t data)
{
	// ToDo:Offset.
	// ToDo: Wait.
	bool dirty;
	uint32_t naddr = (addr & 0x7fffe) >> 1;
	uint16_t d;
	// ToDo: Mask register
	d = vram[naddr];
    dirty = ((uint16_t)data != d) ? true : false;

	if(dirty) {
		dirty_flag[naddr >> 3] = true;
		// ToDo: Mask register
		vram[naddr] = data; 
	}
}

void TOWNS_VRAM::write_data32(uint32_t addr, uint32_t data)
{
	// ToDo:Offset.
	// ToDo: Wait.
	bool dirty;
	uint32_t naddr = (addr & 0x7fffc) >> 1;
	uint32_t *p = (uint32_t*)vram; 
	pair32_t nw;

	// ToDo: Mask register
#ifdef __LITTLE_ENDIAN__
	nw.d = p[naddr >> 1];
#else
	nw.w.l = vram[naddr + 0];
	nw.w.h = vram[naddr + 1];
#endif
	// ToDo: Mask register
    dirty = (data != nw.d) ? true : false;

	if(dirty) {
		// ToDo: Mask register
		dirty_flag[naddr >> 3] = true;

#ifdef __LITTLE_ENDIAN__
		p[naddr >> 1] = data;
#else
		nw.d = data;
		vram[naddr + 0] = nw.w.l;
		vram[naddr + 1] = nw.w.h;
#endif
	}	
}

// Check dirty and calc alpha by 8 bytes.
bool TOWNS_VRAM::check_dirty_and_calc_alpha(uint32_t addr)
{
	__DECL_ALIGNED(16) uint16_t pix_cache_16[4];
	__DECL_ALIGNED(16)  uint8_t pix_cache_8[8];
	__DECL_ALIGNED(16)  uint8_t pix_cache_4[16];
	
	__DECL_ALIGNED(4 * sizeof(scrntype_t)) scrntype_t alpha_cache_16[4];
	__DECL_ALIGNED(16) uint16_t mask_cache_16[4];
	__DECL_ALIGNED(16) uint16_t mask_cache_16_neg[4];
	__DECL_ALIGNED(16) uint8_t mask_cache_4[8];
	__DECL_ALIGNED(16) uint8_t mask_cache_4_neg[8];
	__DECL_ALIGNED(16) scrntype_t alpha_cache_4[16];
	bool dirty = dirty_flag[addr >> 3];
	// If Not dirty, alpha/mask already calced.
	if(dirty) {
		uint32_t layer = (addr >= 0x40000) ? 1 : 0;
		uint32_t naddr = addr & 0x3ffff;
		// alpha32768
		uint16_t *vptr = vram_ptr[layer];
		uint16_t *vptr8 = (uint8_t*)vptr;
		for(int i = 0; i < 4; i++) {
			pix_cache_16[i] = vptr[naddr >> 1];
		}
		for(int i = 0; i < 4; i++) {
			alpha_cache_16[i] = alpha_32768c[pix_cache_16[i]];
			mask_cache_16[i] =  mask_32768c[pix_cache_16[i]];
			mask_cache_16_neg[i] = ~mask_cache_16[i];
		}
		scrntype_t* palpha = &(alpha_buffer_32768[addr >> 1]);
		uint16_t* pmask16 = &(mask_buffer_32768[addr >> 1]);
		uint16_t* pmask16_neg = &(mask_buffer_32768_neg[addr >> 1]);
		for(int i = 0; i < 4; i++) {
			palpha[i]      = alpha_cache_16[i];
			pmask16[i]     = mask_cache_16[i];
			pmask16_neg[i] = mask_cache_16_neg[i];
		}
		
		for(int i = 0; i < 8; i++) {
			pix_cache_8[i] = vptr8[naddr];
		}
		// Alpha8
		
		for(int i = 0; i < 8; i++) {
			alpha_cache_4[i << 1]       = alpha_16c[pix_cache_8[i] << 1];
			alpha_cache_4[(i << 1) + 1] = alpha_16c[(pix_cache_8[i] << 1) + 1];
		}
		for(int i = 0; i < 8; i++) {
			mask_cache_4[i] = mask_16c[pix_cache_8[i]];
			mask_cache_4_neg[i] = ~mask_cache_4[i];
		}
		palpha = &(alpha_buffer_16[addr << 1]);
		uint16_t* pmask4 = &(mask_buffer_16[addr]);
		uint16_t* pmask4_neg = &(mask_buffer_16_neg[addr]);
		for(int i = 0; i < 16; i++) {
			palpha[i]     = alpha_cache_4[i];
		}
		for(int i = 0; i < 8; i++) {
			pmask4[i]     = mask_cache_4[i];
			pmask4_neg[i] = mask_cache_4_neg[i];
		}
	}
	return dirty;
}
			
		
uint32_t TOWNS_VRAM::read_plane_data8(uint32_t addr)
{
	// Plane Access
	pair_t data_p;
	uint32_t x_addr = 0;
	uint8_t *p = (uint8_t*)vram;
	uint32_t mod_pos;

	// ToDo: Writing plane.
	if(access_page1) x_addr = 0x40000; //?
	addr = (addr & 0x7fff) << 3;
	p = &(p[x_addr + addr]); 
	
	// 8bit -> 32bit
	uint32_t *pp = (uint32_t *)p;
	uint8_t tmp = 0;
	uint32_t tmp_d = *pp;
	
#ifdef __LITTLE_ENDIAN__
	uint32_t tmp_m1 = 0x000000f0;
	uint32_t tmp_m2 = 0x0000000f;
#else
	uint32_t tmp_m1 = 0xf0000000;
	uint32_t tmp_m2 = 0x0f000000;
#endif	
	uint32_t tmp_r;
	tmp_d = tmp_d & write_plane_mask;
	
	for(int i = 0; i < 4; i++) {
		tmp <<= 2;
		tmp = tmp | (((tmp_d & tmp_m1) != 0) ? 0x02 : 0x00);
		tmp = tmp | (((tmp_d & tmp_m2) != 0) ? 0x01 : 0x00);
		
#ifdef __LITTLE_ENDIAN__
		tmp_d <<= 8;
#else
		tmp_d >>= 8;
#endif		
	}
	return tmp;
}

uint32_t TOWNS_VRAM::read_plane_data16(uint32_t addr)
{
	pair16_t d;
	d.b.l = (uint8_t)(read_plane_data8(addr + 0));
	d.b.h = (uint8_t)(read_plane_data8(addr + 1));
	return (uint32_t)(d.w);
}

uint32_t TOWNS_VRAM::read_plane_data32(uint32_t addr)
{
	pair32_t d;
	d.b.l  = (uint8_t)(read_plane_data8(addr + 0));
	d.b.h  = (uint8_t)(read_plane_data8(addr + 1));
	d.b.h2 = (uint8_t)(read_plane_data8(addr + 2));
	d.b.h3 = (uint8_t)(read_plane_data8(addr + 3));
	return d.d;
}

void TOWNS_VRAM::write_plane_data8(uint32_t addr, uint32_t data)
{
	// Plane Access
	pair_t data_p;
	uint32_t x_addr = 0;
	uint8_t *p = (uint8_t*)vram;
	uint32_t mod_pos;

	// ToDo: Writing plane.
	if(access_page1) x_addr = 0x40000; //?
	addr = (addr & 0x7fff) << 3;
	x_addr = x_addr + addr;
	p = &(p[x_addr]); 
	
	// 8bit -> 32bit
	uint32_t *pp = (uint32_t *)p;
	uint32_t tmp = 0;
	uint32_t tmp_d = data & 0xff;
#ifdef __LITTLE_ENDIAN__
	uint32_t tmp_m1 = 0xf0000000 & write_plane_mask;
	uint32_t tmp_m2 = 0x0f000000 & write_plane_mask;
#else
	uint32_t tmp_m1 = 0x000000f0 & write_plane_mask;
	uint32_t tmp_m2 = 0x0000000f & write_plane_mask;
#endif	
	uint32_t tmp_r1;
	uint32_t tmp_r2;

	for(int i = 0; i < 4; i++) {
#ifdef __LITTLE_ENDIAN__
		tmp = tmp >> 8;
#else
		tmp = tmp << 8;
#endif
		tmp = tmp | (((tmp_d & 0x02) != 0) ? tmp_m1 : 0x00);
		tmp = tmp | (((tmp_d & 0x01) != 0) ? tmp_m2 : 0x00);
		tmp_d >>= 2;
	}
	tmp_r1 = *pp;
	tmp_r2 = tmp_r1;
	tmp_r1 = tmp_r1 & ~write_plane_mask;
	tmp_r1 = tmp_d | tmp_r1;
	if(tmp_r2 != tmp_r1) {
		*pp = tmp_r1;
		dirty_flag[x_addr >> 3] = true;
	}
}

void TOWNS_VRAM::write_plane_data16(uint32_t addr, uint32_t data)
{
	pair16_t d;
	d.w = (uint16_t)data;
	write_plane_data8(addr + 0, d.b.l);
	write_plane_data8(addr + 1, d.b.h);
}


void TOWNS_VRAM::write_plane_data32(uint32_t addr, uint32_t data)
{
	pair32_t d;
	d.d = data;
	write_plane_data8(addr + 0, d.b.l);
	write_plane_data8(addr + 1, d.b.h);
	write_plane_data8(addr + 2, d.b.h2);
	write_plane_data8(addr + 3, d.b.h3);
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
void TOWNS_CRTC::render_line_16_boundary(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t startaddr, uint32_t words)
{
	// Simplify render: Must set 16pixels
	uint32_t wordptr = 0;
	int nwords = (int)words / 16;
	int ip;
	uint32_t src;
	uint32_t src1, src2;
	__DECL_ALIGNED(8) uint8_t  srcdat[8];
	__DECL_ALIGNED(sizeof(scrntype_t) * 16) scrntype_t data_cache_16[16];
	
	scrntype_t *pdst = framebuffer;
	uint8_t *pp = (uint8_t *)vramptr;
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	uintptr_t offset;
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base]);
	
	for(ip = 0; ip < nwords; ip++) {
		if(!check_dirty_and_calc_alpha(offset_base)) {
			pdst += 16;
			pp += 8;
			offset_base += 8;
			continue;
		}
		dirty_flag[(offset_base >> 3)] = false;
		for(int ii = 0; ii < 8; ii++) {
			srcdat[ii << 1]       =  pp[ii];
			srcdat[(ii << 1) + 1] =  srcdat[ii << 1] & 0x0f;
			srcdat[ii << 1]       = (srcdat[ii << 1] & 0x0f) >> 4;
		}
		scrntype_t* aptr = alpha_buffer_16[offset_base << 1];
		for(int ii = 0; ii < 16; ii++) {
			data_cache_16[ii] = apalette_16[layer][srcdat1[ii]];
		}
		// ToDo: For hadrdware that does not support alpha blending(not RGBA32).
		for(int ii = 0; ii < 16; ii++) {
			pdst[ii] = data_cache_16[ii];
		}
		pdst += 16;
		pp += 8;
		offset_base += 8;
	}
}

void TOWNS_CRTC::render_line_16_not_boundary(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t startaddr, uint32_t words)
{
	// Simplify render: Must set 32pixels
	uint32_t wordptr = 0;
	int ip;
	uint32_t src;
	uint32_t src1, src2;
	scrntype_t *pdst = framebuffer;
	uint8_t *pp = (uint8_t *)vramptr;
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	uintptr_t offset;
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base]);
	
	if(check_dirty_and_calc_alpha(offset_base)) {
		//dirty_flag[(offset_base >> 3)] = false; // OK?
		for(ip = 0; ip < words; ip++) {
			uint8_t sdat = pp[ip];
			scrntype_t pdat = apalette_16[layer][sdat];
			pdst[ii] = pdat;
		}
	}
}

void TOWNS_CRTC::render_line_256_boundary(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t startaddr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	__DECL_ALIGNED(16) uint8_t src[8];
	scrntype_t *pdst  = __builtin_assume_aligned(framebuffer, sizeof(scrntype_t));
	uint8_t *pp = (uint8_t *)vramptr;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base >> 2]);
	
	for(ip = 0; ip < nwords; ip++) {
		if(check_dirty_and_calc_alpha(offset_base)) {
			dirty_flag[offset_base >> 3] = false;
			for(int i = 0; i < 8; i++) {
				src[i] = pp[i];
			}
			// ToDo: Super Impose.
			for(int i = 0; i < 8; i++) {
				pdst[i] = apalette_256_pixel[src[i]];
			}
		}
		offset_base += 8;
		pp += 8;
		pdst += 8;
	}
}

void TOWNS_CRTC::render_line_256_not_boundary(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t startaddr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint8_t *pp = (uint8_t *)vramptr;
	uint8_t src;
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base >> 2]);
	
	for(ip = 0; ip < nwords; ip++) {
		if(check_dirty_and_calc_alpha(offset_base + ip)) {
			// Keep dirty?
			src = pp[ip];
			// ToDo: Super Impose.
			pdst[ip] = apalette_256_pixel[src];
		}
	}
}


// To be used table??
void TOWNS_CRTC::render_line_32768_boundary(int layer, scrntype_t *pixcache, uint16_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 4;
	int ip;
	uint16_t src16;
	scrntype_t *pdst = pixcache;
	uint16_t *pp = (uint16_t *)vramptr;
	uint16_t *cachep = (uint16_t *)srcdat;
	__DECL_ALIGNED(16) uint16_t rgb[4];
	__DECL_ALIGNED(sizeof(scrntype_t) * 4) scrntype_t dcache[4];
	int i = 0;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base >> 1]);

	for(ip = 0; ip < nwords; ip++) {
		if(check_dirty_and_calc_alpha(offset_base)) {
			dirty_flag[(offset_base >> 3)] = false;
			for(int i = 0; i < 4; i++) {
				rgb[i] = pp[i];
			}
			for(int ii = 0; ii < 4; ii++) {
				dcache[ii] = table_32768c[rgb[ii]];
			}
			for(int ii = 0; ii < 4; ii++) {
				pdst[ii] = dcache[ii];
			}
		}
		offset_base += 8;
		pdst += 4;
		pp += 4;
	}
}

void TOWNS_CRTC::render_line_32768_not_boundary(int layer, scrntype_t *pixcache, uint16_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 4;
	int ip;
	uint16_t src16;
	scrntype_t *pdst = pixcache;
	uint16_t *pp = (uint16_t *)vramptr;
	uint16_t *cachep = (uint16_t *)srcdat;
	int i = 0;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base >> 1]);

	for(ip = 0; ip < words; ip++) {
		if(check_dirty_and_calc_alpha(offset_base)) {
			// Keep dirty?
			pdst[ip] = table_32768c[pp[ip]];
		}
		offset_base++;
	}
}
// ToDo: Super impose.
void TOWNS_CRTC::mix_layers(scrntype* dst, scrntype_t* upper, scrntype_t *upper_alpha, scrntype_t* lower, int width)
{
	scrntype_t* src1 = upper;
	scrntype_t* src2 = lower;
	
	__DECL_ALIGNED(sizeof(scrntype_t) * 8) scrntype_t dcache1[8];
	__DECL_ALIGNED(sizeof(scrntype_t) * 8) scrntype_t dcache2[8];
	__DECL_ALIGNED(sizeof(scrntype_t) * 8) scrntype_t dcache3[8];
	__DECL_ALIGNED(sizeof(scrntype_t) * 8) scrntype_t acache1[8];
	__DECL_ALIGNED(sizeof(scrntype_t) * 8) scrntype_t acache2[8];
	if(src1 == NULL) {
		if(src2 == NULL) {
			for(int i = 0; i < width; i++) {
				dst[i] = RGBA_COLOR(0, 0, 0, 255);
			}
		} else {
			// Upper don't display, lower displayed.
			int iwidth = width / 8;
			for(int i = 0; i < iwidth; i++) {
				for(int ii = 0; ii < 8; ii++) {
					dcache2[ii] = src2[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					dst[ii] = dcache2[ii];
				}
				src2 += 8;
				dst += 8;
			}
			width = width - (iwidth * 8);
			if(width > 0) {
				for(int i = 0; i < width; i++) {
					sdt[i] = src2[i];
				}
			}
		}
	} else { 
		if(src2 == NULL) { // upper only
			int iwidth = width / 8;
			for(int i = 0; i < iwidth; i++) {
				for(int ii = 0; ii < 8; ii++) {
					dcache1[ii] = src1[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					dst[ii] = dcache1[ii];
				}
				src1 += 8;
				dst += 8;
			}
			width = width - (iwidth * 8);
			if(width > 0) {
				for(int i = 0; i < width; i++) {
					sdt[i] = src2[i];
				}
			}
		} else { // Both lives
			if(upper_alpha == NULL) {
				for(int i = 0; i < 8; i++) {
					acache1[i] = RGBA_COLOR(255, 255, 255, 255);
				}
			}
			int iwidth = width / 8;
			for(int i = 0; i < iwidth; i++) {
				if(upper_alpha != NULL) {
					for(int ii = 0; ii < 8; i++) {
						acache1[ii] = upper_alpha[ii];
					}
					upper_alpha += 8;
				}
				for(int ii = 0; ii < 8; i++) {
					acache2[ii] = ~acache1[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					dcache1[ii] = src1[ii];
					dcache2[ii] = src2[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					dcache2[ii] = dcache2[ii] * acache1[ii]; // Mask Upper
					//dcache3[ii] = (dcache2[ii] == RGBA_COLOR(0, 0, 0, 0)) ? dcache1[ii] : dcache2[ii];
					dcache1[ii] = dcache1[ii] * acache2[ii]; // Mask Lower.
					dcache3[ii] = dcache1[ii] | dcache2[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					dst[ii] = dcache3[ii];
				}
				src1 += 8;
				src2 += 8;
				dst += 8;
			}
			width = width - (iwidth * 8) ;
			scrntype_t d1, d2, a1, a2;
			for(int i = 0; i < width; i++) {
				if(upper_alpha != NULL) {
					a1 = upper_alpha[i];
				} else {
					a1 = RGBA_COLOR(255, 255, 255, 255);
				}
				a2 = ~a1;
				d1 = src1[i];
				d2 = src2[i];
				d2 = d2 * a1;
				d1 = d1 * a2;
				dst[i] = d1 | d2;
			}
		}
	}
}

void TOWNS_CRTC::vzoom_pixel(scrntype_t*src, int width, int srclines, scrntype_t* dst, int stride, int dstlines, float vzoom)
{
	if(src == dst) return;
	if(vzoom <= 0.0f) return;
	if((src == NULL) || (dst == NULL)) return;
	if(width <= 0) return;
	if(srclines <= 0) return;
	if(dstlines <= 0) return;

	scrntype_t *psrc = src;
	scrntype_t *pdst = dst;

	float vzoom_int = floor(vzoom);
	float vzoom_mod = vzoom - vzoom_int;
	float nmod = 0.0f;
	int nint = (int)vzoom_int;
	int srcline = 0;
	int linecount = 0;
	for(srcline = 0; srcline < srclines; srcline++) {
		int ifactor = nint;
		if(nmod >= 1.0f) {
			nmod = nmod - 1.0f;
			ifactor++;
		}
		for(int ii = 0; ii < ifactor; ii++) {
			// ToDo: Mask, blend.
			memcpy(pdst, psrc, width * sizeof(scrntype_t));
			linecount++;
			if(linecount >= dstlines) goto _exit0;;
			pdst += stride;
		}
		psrc += stride;
		nmod = nmod + vzoom_mod;
	}
_exit0:	
	return;
}
		
void TOWNS_CRTC::hzoom_pixel(scrntype_t* src, int width, scrntype_t* dst, int dst_width, float hzoom)
{
	if(src == dst) return;
	if(hzoom <= 0.0f) return;
	if((src == NULL) || (dst == NULL)) return;
	if(width <= 0) return;
	if(dst_width <= 0) return;

	scrntype_t* pdst = dst;
	float hzoom_int = floor(hzoom);
	float hzoom_mod = hzoom - hzoom_int;
	float nmod = 0.0f;
	int nint = (int)hzoom_int;
	if((nint == 1) && (hzoom_mod <= 0.0f)) {
		// x1.0
		scrntype_t *sp = src;
		scrntype_t pcache[4];
		int nw = width / 4;
		int dx = 0;
		int ycount = 0;
		for(int ii = 0; ii < nw; ii++) {
			if((dx + 3) >= dst_width) break;
			for(int j = 0; j < 4; j++) {
				pcache[j] = sp[j];
			}
			// ToDo: Mask/Alpha
			for(int j = 0; j < 4; j++) {
				pdst[j] = pcache[j];
			}
			dx += 4;
			pdst += 4;
			sp += 4;
		}
		nw = width - dx;
		if(dx >= dst_width) return;
		for(int ii = 0; ii < nw; ii++) {
			if(dx >= dst_width) goto __exit_0;
			// ToDo: Mask/Alpha
			pcache[ii] = sp[ii];
			pdst[ii] = pcache[ii];
			dx++;
		}
	} else {
		scrntype_t *sp = src;
		int dx = 0;
		int xcount = 0;
		for(int i = 0; i < width; i++) {
			// ToDo : Storage via SIMD.
			if(dx >= dst_width) goto __exit_0;
			
			int ifactor = nint;
			if(nmod >= 1.0f) {
				ifactor++;
				nmod = nmod - 1.0f;
			}
			scrntype_t pix = *sp++;
			scrntype_t pcache[4];
			// ToDo: MASK/ALPHA
		
			int jfactor = ifactor / 4;
			xcount = 0;
			for(int k = 0; k < 4; k++) {
				pcache[k] = pix;
			}
			for(int j = 0; j < jfactor; j++) {
				if((dx + 3) >= dst_width) break; 
				for(int k = 0; k < 4; k++) {
					pdst[k] = pcache[k];
				}
				xcount += 4;
				pdst += 4;
				dx += 4;
			}
			ifactor = ifactor - xcount;
			if(ifactor < 0) ifactor = 0;
			for(int k = 0; k < ifactor; k++) {
				if(dx >= dst_width) goto __exit_0;
				pdst[k] = pcache[k];
				dx++;
			}
			nmod = nmod + hzoom_mod;
			
			pdst += ifactor;
		}		
	}
__exit_0:
	return;
}

void TOWNS_VRAM::draw_screen()
{
	// Note: This renderer will contain three types (at least:)
	// 1. Software Zoom, Hardware stack (Using GPU for stack, Using CPU for zoom)
	// 2. Softare Zoom,  Software stack (Using only host-CPU).
	// 3. Hardware Zoom, Hardware stack (Using GPU and compute shader?).
	// To implement 1., Zooming all screen don't wish to use CPU, per raster Zooming wish to use GPU.
	// At first, will implement 2.As of CSP platform implement only single frame buffer area.
	//
	// Answer values(excepts 2.):
	//  Note: Upper layer = layer1, lower layer = layer2.
	// a. Pixel Layers of 1/2.
	// b. Alpha Layers of 1.
	// c. Layer using flags both layer1, layer2.
	// d. Data of Layers (1/2).
	// e. Source screen width and height per layer.
	// f. Offset sourcfe-pixel-address per raster.
	// g. Source raster width per layer and scanline.
	// h. Offset dst-pixel-position in raster per layer.
	// - 20190110 K.Ohta.
}


}
#undef _CLEAR_COLOR

