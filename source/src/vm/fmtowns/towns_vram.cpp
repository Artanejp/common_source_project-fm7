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
	return vram[addr & 0x7ffff];
}



uint32_t TOWNS_VRAM::read_data16(uint32_t addr)
{
	// ToDo:Offset.
	// ToDo: Wait.
	uint32_t naddr = addr & 0x7fffe;
	pair16_t nw;
	nw.l = vram[naddr + 0];
	nw.h = vram[naddr + 1];
	return (uint32_t)(nw.w);
}

uint32_t TOWNS_VRAM::read_data32(uint32_t addr)
{
	// ToDo: Wait.
	pair32_t nd;
	uint32_t naddr = addr & 0x7fffc;
	nd.w.l = read_data16(naddr + 0);
	nd.w.h = read_data16(naddr + 2);
	return nd.d;
}

void TOWNS_VRAM::write_data8(uint32_t addr, uint32_t data)
{
	// ToDo:Offset.
	// ToDo: Wait.
	bool dirty;
	uint32_t naddr = addr & 0x7ffff;
	uint8_t n = vram[naddr];
	dirty =	((uint8_t)data != n) ? true : false;
    dirty_flag[naddr] = dirty;
	if(dirty) {
		vram[naddr] = n;
		if((naddr & 1) != 0) {
			uint16_t nd = (n < 0x80) ? 0xffff : 0x0000;
			alpha_32768_byte[naddr >> 1] = (uint8_t)nd;
			alpha_32768_word[naddr >> 1] = nd;
			mask_32768_word[naddr >> 1]  = ~nd;
		}
		// ToDo: Mask/alpha16.
	}
}

void TOWNS_VRAM::write_data16(uint32_t addr, uint32_t data)
{
	// ToDo:Offset.
	// ToDo: Wait.
	bool dirty;
	uint32_t naddr = addr & 0x7fffe;
	uint16_t *vp = (uint16_t*)(&(vram[naddr]));
	pair16_t nw;
	nw.b.l = vram[naddr + 0];
	nw.b.h = vram[naddr + 1];
	
    dirty = ((uint16_t)data != nw.w) ? true : false;
	dirty_flag[naddr] = dirty;
	dirty_flag[naddr + 1] = dirty;

	if(dirty) {
		uint16_t alphaw = (nw.w < 0x8000) ? 0xffff : 0x0000;
		alpha_32768_byte[naddr >> 1] = (uint8_t)alphaw;
		alpha_32768_word[naddr >> 1] = alphaw;
		mask_32768_word[naddr >> 1] = ~alphaw;
		*vp = (uint16_t)data;
	}
}

void TOWNS_VRAM::write_data32(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0x7fffc; // OK?
	write_data16(naddr + 0, (uint16_t)data);
	write_data16(naddr + 2, (uint16_t)(data >> 16));
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
	int nwords = (int)words / 16;
	int ip;
	uint32_t src;
	uint32_t src1, src2;
	uint8_t  srcdat[16];
	scrntype_t tmpdat[16];
	scrntype_t *pdst = framebuffer;
	uint8_t *pp = (uint8_t *)vramptr;
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	for(ip = 0; ip < nwords; ip++) {
		for(int ii = 0; ii < 8; ii++) {
			srcdat[ii << 1]       =  pp[ii];
			srcdat[(ii << 1) + 1] =  srcdat[ii << 1] & 0x0f;
			srcdat[ii << 1]       = (srcdat[ii << 1] & 0x0f) >> 4;
		}
		
		for(int ii = 0; ii < 16; ii++) {
			uint16_t c = (srcdat[ii] == 0) ? 0x0000 : 0xffff;
			alpha_cache_16_word[ii] = c;
			alpha_cache_16_byte[ii] = (uint8_t)c;
			mask_cache_16_word[ii] = ~c;
		}
		if(upper_layer) {
			for(int ii = 0; ii < 16; i++) {
				tmpdat[ii] = (alpha_cache_16_byte[ii] != 0) ? apalette_16[layer][srcdat1[ii]] : RGBA_COLOR(0, 0, 0, 0);
			}
		} else {
			for(int ii = 0; ii < 16; i++) {
				tmpdat[ii] = (alpha_cache_16_byte[ii] != 0) ? apalette_16[layer][srcdat1[ii]] : RGBA_COLOR(0, 0, 0, 255);
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
	uint16_t c1 = 0xffff;
	uint16_t c2 = 0xffff;
	
	if(mod_words > 0) {
		for(int ii = 0; ii < mod_words; i++) {
			if((ii & 1) == 0) {
				sdat1 = *pp;
				sdat2 = sdat1 & 0x0f;
				sdat1 = (sdat1 & 0xf0) >> 4;
				c1 = (sdat1 == 0) ? 0x0000 : 0xffff;
				c2 = (sdat2 == 0) ? 0x0000 : 0xffff;
			}
			if((ii & 1) == 0) {
				alpha_cache_16_word[ii] = c1;
				alpha_cache_16_byte[ii] = (uint8_t)c1;
				mask_cache_16_word[ii]  = ~c1;
			} else {
				alpha_cache_16_word[ii] = c2;
				alpha_cache_16_byte[ii] = (uint8_t)c2;
				mask_cache_16_word[ii]  = ~c2;
			}	
			
			if((ii & 1) == 0) {
				if(upper_layer) {
					tmpdat[ii] = (c1 != 0) ? apalette_16_pixel[layer][sdat1] : RGBA_COLOR(0, 0, 0, 0);
				} else {
					tmpdat[ii] = (c1 != 0) ? apalette_16_pixel[layer][sdat1] : RGBA_COLOR(0, 0, 0, 255);
				}
			} else {
				if(upper_layer) {
					tmpdat[ii] = (c2 != 0) ? apalette_16_pixel[layer][sdat2] : RGBA_COLOR(0, 0, 0, 0);
				} else {
					tmpdat[ii] = (c2 != 0) ? apalette_16_pixel[layer][sdat2] : RGBA_COLOR(0, 0, 0, 255);
				}
			}
			pp++;
		}
		for(int ii = 0; ii < mod_words; i++) {
			pdst[ii] = tmpdat[ii];
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
void TOWNS_CRTC::render_line_32768(int layer, scrntype_t *pixcache, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint16_t src16;
	scrntype_t *pdst = pixcache;
	uint8_t *pp = (uint8_t *)vramptr;
	uint16_t *cachep = (uint16_t *)srcdat;
	pair16_t rgb[8];
	uint8_t a[8], r[8], g[8], b[8];
	scrntype_t dcache[8];

	int i = 0;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;

	union {
		uint16_t u16[8];
		pair16_t p16[8];
		uint8_t  u8[16];
	} srcdat;
	for(ip = 0; ip < nwords; ip++) {
		for(int i = 0; i < 16; i++) {
			srcdat.u8[i] = pp[i];
		}
		for(int i = 0; i < 8; i++) {
			dcache[i] = _CLEAR_COLOR;
		}

		for(int ii = 0; ii < 8; ii++) {
#if defined(__LITTLE_ENDIAN__)
			rgb[ii].w = srcdat.w[ii];
#else
			rgb[ii].b.l = srcdat.u8[ii << 1];
			rgb[ii].b.h = srcdat.u8[(ii << 1) + 1];
#endif
		}
		for(int ii = 0; ii < 8; ii++) {
			//g5r5b5
			g[ii] = (rgb[ii].w & 0x7c00) >> 10;
			r[ii] = (rgb[ii].w & 0x03e0) >> 5;
			b[ii] = (rgb[ii].w & 0x001f);
			a[ii] = (rgb[ii].w < 0x8000) ? 0xff : 0x00;
		}
		for(int ii = 0; ii < 8; ii++) {
			dcache[ii] = RGBA_COLOR(r[ii], g[ii], b[ii], a[ii]);
		}
		for(int ii = 0; ii < 8; ii++) {
			pdst[ii] = dcache[ii];
		}
		pdst += 8;
		pp += 16;
	}
	for(int i = 0; i < 8; i++) {
		dcache[i] = _CLEAR_COLOR;
	}
	int mod_words = words - nwords * 8;
	for(ip = 0; ip < mod_words; ip++) {
		srcdat.u8[ip << 1] = pp[ip << 1];
		srcdat.u8[(ip << 1) + 1] = pp[(ip << 1) + 1];
#if defined(__LITTLE_ENDIAN__)
		rgb[ip].w = srcdat.w[ip];
#else
		rgb[ip].b.l = srcdat.u8[ip << 1];
		rgb[ip].b.h = srcdat.u8[(ip << 1) + 1];
#endif
		//g5r5b5
		g[ip] = (rgb[ip].w & 0x7c00) >> 10;
		r[ip] = (rgb[ip].w & 0x03e0) >> 5;
		b[ip] = (rgb[ip].w & 0x001f);
		a[ip] = (rgb[ip].w < 0x8000) ? 0xff : 0x00;
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

