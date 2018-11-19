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

#if 0		
		if((naddr & 1) != 0) {
			// Upper address: Change mask for 32768colors. 
			uint16_t nd = (n < 0x80) ? 0xffff : 0x0000;
			alpha_32768_byte[naddr >> 1] = (uint8_t)nd;
			alpha_32768_word[naddr >> 1] = nd;
			mask_32768_word[naddr >> 1]  = ~nd;
		}
		// ToDo: Mask/alpha16.
		uint16_t nu = ((n & 0xf0) != 0) ? 0x0000 : 0xffff;
		uint16_t nl = ((n & 0x0f) != 0) ? 0x0000 : 0xffff;
		naddr = naddr << 1;
		alpha_16_byte[naddr + 0] = (uint8_t)nu;
		alpha_16_byte[naddr + 1] = (uint8_t)nl;
		alpha_16_word[naddr + 0] = nu;
		alpha_16_word[naddr + 1] = nl;
		mask_16_byte[naddr + 0]  = ~nu;
		mask_16_byte[naddr + 1]  = ~nl;
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
		dirty_flag[naddr >> 2] = true;
		// ToDo: Mask register
		vram[naddr] = data; 
#if 0		
		uint16_t alphaw = (nw.w < 0x8000) ? 0xffff : 0x0000;
		alpha_32768_byte[naddr >> 1] = (uint8_t)alphaw;
		alpha_32768_word[naddr >> 1] = alphaw;
		mask_32768_word[naddr >> 1] = ~alphaw;

		
		uint16_t n0 = ((nw.b.h  & 0xf0) != 0) ? 0x0000 : 0xffff;
		uint16_t n1 = ((nw.b.h  & 0x0f) != 0) ? 0x0000 : 0xffff;
		uint16_t n2 = ((nw.b.l & 0xf0) != 0)  ? 0x0000 : 0xffff;
		uint16_t n3 = ((nw.b.l & 0x0f) != 0)  ? 0x0000 : 0xffff;
		naddr = naddr << 1;
		alpha_16_byte[naddr + 0] = (uint8_t)n0;
		alpha_16_byte[naddr + 1] = (uint8_t)n1;
		alpha_16_byte[naddr + 2] = (uint8_t)n2;
		alpha_16_byte[naddr + 3] = (uint8_t)n3;
		alpha_16_word[naddr + 0] = n0;
		alpha_16_word[naddr + 1] = n1;
		alpha_16_word[naddr + 2] = n2;
		alpha_16_word[naddr + 3] = n3;
		
		mask_16_byte[naddr + 0]  = ~n0;
		mask_16_byte[naddr + 1]  = ~n1;
		mask_16_byte[naddr + 2]  = ~n2;
		mask_16_byte[naddr + 3]  = ~n3;
#endif
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
		dirty_flag[(naddr >> 2) + 0] = true;

#ifdef __LITTLE_ENDIAN__
		p[naddr >> 1] = data;
#else
		nw.d = data;
		vram[naddr + 0] = nw.w.l;
		vram[naddr + 1] = nw.w.h;
#endif
	}	
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
void TOWNS_CRTC::render_line_16(int layer, bool upper_layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t startaddr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 16;
	int ip;
	uint32_t src;
	uint32_t src1, src2;
	uint8_t  srcdat[16]  __attribute__((__aligned(16)));
	scrntype_t tmpdat[16] __attribute__((__aligned(sizeof(scrntype_t) * 8)));
	
	scrntype_t alpha_cache_16[16] __attribute__((__aligned(16)));
	scrntype_t data_cache_16[16] __attribute__((__aligned(16)));
	
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
		if(!dirty_flag[(offset_base >> 3)]) {
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
		
		for(int ii = 0; ii < 16; ii++) {
			data_cache_16[ii] = apalette_16[layer][srcdat1[ii]];
		}
		if(upper_layer) {
			// ToDo: For hadrdware that does not upport alpha blending(not RGBA32).
			for(int ii = 0; ii < 16; i++) {
				alpha_cache_16[ii] = (srcdat[ii] == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
			}
			for(int ii = 0; ii < 16; i++) {
				tmpdat[ii] = alpha_cache_16_byte[ii] * data_cache_16[ii];
			}		
		} else {
			// ToDo: For hadrdware that does not upport alpha blending(not RGBA32).
			for(int ii = 0; ii < 16; i++) {
				alpha_cache_16[ii] = (srcdat[ii] == 0) ? RGBA_COLOR(0,0,0,255) : RGBA_COLOR(255, 255, 255, 255);
			}
			for(int ii = 0; ii < 16; i++) {
				tmpdat[ii] = alpha_cache_16_byte[ii] * data_cache_16[ii];
			}		
		}
		for(int ii = 0; ii < 16; ii++) {
			pdst[ii] = tmpdat[ii];
		}
		pdst += 16;
		pp += 8;
	}
	int mod_words = words - (nwords * 16);
	uint8_t sdat1, sdat2;
	scrntype_t c1, c2;
	
	if(mod_words > 0) {
		offset_base = (uintptr_t)startaddr;
		// offet_base = (offset_base + [value_of_offset_register]) & mask.
		offset_base = ((offet_base + nwords) & 0x3ffff) + base;
		if(dirty_flag[offset_base >> 3]) {
			dirty_flag[offset_base >> 3] = false;
			for(int ii = 0; ii < mod_words; i++) {
				if((ii & 1) == 0) {
					sdat1 = *pp;
					sdat2 = sdat1 & 0x0f;
					sdat1 = (sdat1 & 0xf0) >> 4;
					if(upper_layer) {
						c1 = (sdat1 == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
						c2 = (sdat2 == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
					} else {
						c1 = (sdat1 == 0) ? RGBA_COLOR(0, 0, 0, 255) : RGBA_COLOR(255, 255, 255, 255);
						c2 = (sdat2 == 0) ? RGBA_COLOR(0, 0, 0, 255) : RGBA_COLOR(255, 255, 255, 255);
					}
				}
				if((ii & 1) == 0) {
					tmpdat[ii] = c1 * apalette_16_pixel[layer][sdat1];
				} else {
					tmpdat[ii] = c2 * apalette_16_pixel[layer][sdat2];
				}
				pp++;
			}
			for(int ii = 0; ii < mod_words; i++) {
				pdst[ii] = tmpdat[ii];
			}
		}
	}
}

void TOWNS_CRTC::render_line_256(int layer, bool upper_layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t startaddr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint32_t src[2] __attribute__((aligned(8)));
	uint8_t  srcdat[8] __attribute__((aligned(16)));;
	scrntype_t *pdst  = __builtin_assume_aligned(framebuffer, sizeof(scrntype_t) * 4);
	uint32_t *pp = (uint32_t *)vramptr;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base >> 2]);
	
	for(ip = 0; ip < nwords; ip++) {
		if(dirty_flag[offset_base >> 3]) {
			src[0] = pp[0];
			src[1] = pp[1];
#if defined(__LITTLE_ENDIAN__)
			srcdat[0] = (uint8_t)((src[0] & 0xff000000) >> 24);
			srcdat[1] = (uint8_t)((src[0] & 0x00ff0000) >> 16);
			srcdat[2] = (uint8_t)((src[0] & 0x0000ff00) >> 8);
			srcdat[3] = (uint8_t) (src[0] & 0x000000ff);
			
			srcdat[4] = (uint8_t)((src[1] & 0xff000000) >> 24);
			srcdat[5] = (uint8_t)((src[1] & 0x00ff0000) >> 16);
			srcdat[6] = (uint8_t)((src[1] & 0x0000ff00) >> 8);
			srcdat[7] = (uint8_t) (src[1] & 0x000000ff);
#else
			srcdat[0] = (uint8_t)( src[0] & 0x000000ff);
			srcdat[1] = (uint8_t)((src[0] & 0x0000ff00) >> 8);
			srcdat[2] = (uint8_t)((src[0] & 0x00ff0000) >> 16);
			srcdat[3] = (uint8_t)((src[0] & 0xff000000) >> 24);
			
			srcdat[4] = (uint8_t)( src[1] & 0x000000ff);
			srcdat[5] = (uint8_t)((src[1] & 0x0000ff00) >> 8);
			srcdat[6] = (uint8_t)((src[1] & 0x00ff0000) >> 16);
			srcdat[7] = (uint8_t)((src[1] & 0xff000000) >> 24);
#endif
			// ToDo: Super Impose.
			for(int i = 0; i < 8; i++) {
				pdst[i] = apalette_256_pixel[srcdat[i]];
			}
		}
		pp += 2;
		pdst += 8;
	}
	int mod_words = words - (nwords * 4);
	if(mod_words > 0) {
		uint8_t src8;
		uint8_t *p8 = (uint8_t *)(&pp[ip]);
		for(int i = 0; i < mod_words; i++) {
			src8 = p8[i];
			// ToDo: Super Impose.
			pdst[i] = apalette_256_pixel[src8];
		}
	}
}

// To be used table??
void TOWNS_CRTC::render_line_32768(int layer, bool do_impose, scrntype_t *pixcache, uint16_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint16_t src16;
	scrntype_t *pdst = pixcache;
	uint16_t *pp = (uint16_t *)vramptr;
	uint16_t *cachep = (uint16_t *)srcdat;
	pair16_t rgb[8] __attribute__((aligned(16)));
	uint8_t a[8] __attribute__((aligned(16)));
	uint8_t r[8] __attribute__((aligned(16)));
	uint8_t g[8] __attribute__((aligned(16)));
	uint8_t b[8] __attribute__((aligned(16)));
	scrntype_t dcache[8] __attribute__((aligned(sizeof(scrntype_t) * 8)));

	int i = 0;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	uintptr_t offset_base = (uintptr_t)startaddr;
	uintptr_t base = ((layer == 0) ? 0x00000 : 0x40000);
	// offet_base = (offset_base + [value_of_offset_register]) & mask.
	offset_base = (offet_base & 0x3ffff) + base;
	pp = &(pp[offset_base >> 1]);
	for(ip = 0; ip < nwords; ip++) {
		if(dirty_flag[offset_base >> 3]) {
			dirty_flags[offset_base >> 3] = false;
			for(int i = 0; i < 8; i++) {
				rgb[i].w = pp[i];
			}
			for(int i = 0; i < 8; i++) {
				dcache[i] = _CLEAR_COLOR;
			}
			
			for(int ii = 0; ii < 8; ii++) {
				//g5r5b5
				g[ii] = rgb[ii].b.h & 0x7c;
				b[ii] = rgb[ii].b.l & 0x1f;
			}
			for(int ii = 0; ii < 8; ii++) {
				//g5r5b5
				b[ii] = b[ii] << 3;
			}
			for(int ii = 0; ii < 8; ii++) {
				r[ii] = ((rgb[ii].b.h & 0x03) << 6) | ((rgb[ii].b.l & 0xe0) >> 2);
			}
			if(do_impose) {
				for(int ii = 0; ii < 8; ii++) {
					a[ii] =  (rgb[ii].h < 0x80) ? 0xff : 0x00;
				}
			} else {
				for(int ii = 0; ii < 8; ii++) {
					a[ii] = 0xff;
				}
			}
			for(int ii = 0; ii < 8; ii++) {
				dcache[ii] = RGBA_COLOR(r[ii], g[ii], b[ii], a[ii]);
			}
			for(int ii = 0; ii < 8; ii++) {
				pdst[ii] = dcache[ii];
			}
		}
		offset_base += 8;
		pdst += 8;
		pp += 16;
	}
	for(int i = 0; i < 8; i++) {
		dcache[i] = _CLEAR_COLOR;
	}
	int mod_words = words - nwords * 8;
	for(ip = 0; ip < mod_words; ip++) {
		rgb[ip].w = pp[ip];
		//g5r5b5
		g[ip] = rgb[ip].b.h & 0x7c;
		r[ip] = ((rgb[ip].b.h & 0x03)  << 6) | ((rgb[ip].b.l & 0xe0) >> 2);
		b[ip] = (rgb[ip].b.l & 0x1f)   << 3;
		if(do_impose) {
			a[ip] = (rgb[ip].b.h < 0x80) ? 0xff : 0x00;
		} else {
			a[i] = 0xff;
		}
		dcache[ip] = RGBA_COLOR(r[ip], g[ip], b[ip], a[ip]);
	}
	for(ip = 0; ip < mod_words; ip++) {
		pdst[ip] = dcache[ip];
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


#undef _CLEAR_COLOR

