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

void TOWNS_VRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0xfff00000;
	switch(naddr) {
	case 0x80000000:
		// Main VRAM
		write_raw_vram8(addr - 0x80000000, data);
		return;
		break;
	case 0x80100000:
		// Main VRAM
		break;
	case 0x00000000:
		switch(addr & 0x000ff000) {
		case 0xc0000:
		case 0xc1000:
		case 0xc2000:
		case 0xc3000:
		case 0xc4000:
		case 0xc5000:
		case 0xc6000:
		case 0xc7000:
			write_plane_data8(addr, data);	// Plane access by I/O FF81h
			return;
			break;
		case 0xc8000:
			// Text VRAM
			break;
		case 0xc9000:
			// Reserved
			break;
		case 0xca000:
			// ANKCG1 / IO / RAM
			break;
		case 0xcb000:
			// CGROM
			break;
		default:
			return;
			break;
		}
		break;
	default:
		return;
		break;
	}		
	
}

void TOWNS_VRAM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0xfff00000;
	switch(naddr) {
	case 0x80000000:
		// Main VRAM
		write_raw_vram16(addr - 0x80000000, data);
		return;
		break;
	case 0x80100000:
		// Main VRAM
		break;
	case 0x00000000:
		switch(addr & 0x000ff000) {
		case 0xc0000:
		case 0xc1000:
		case 0xc2000:
		case 0xc3000:
		case 0xc4000:
		case 0xc5000:
		case 0xc6000:
		case 0xc7000:
			write_plane_data16(addr, data);	// Plane access by I/O FF81h
			return;
			break;
		case 0xc8000:
			// Text VRAM
			break;
		case 0xc9000:
			// Reserved
			break;
		case 0xca000:
			// ANKCG1 / IO / RAM
			break;
		case 0xcb000:
			// CGROM
			break;
		default:
			return;
			break;
		}
		break;
	default:
		return;
		break;
	}		
	
}
	
void TOWNS_VRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0xfff00000;
	switch(naddr) {
	case 0x80000000:
		// Main VRAM
		write_raw_vram32(addr - 0x80000000, data);
		return;
		break;
	case 0x80100000:
		// Main VRAM
		break;
	case 0x00000000:
		switch(addr & 0x000ff000) {
		case 0xc0000:
		case 0xc1000:
		case 0xc2000:
		case 0xc3000:
		case 0xc4000:
		case 0xc5000:
		case 0xc6000:
		case 0xc7000:
			write_plane_data32(addr, data);	// Plane access by I/O FF81h
			return;
			break;
		case 0xc8000:
			// Text VRAM
			break;
		case 0xc9000:
			// Reserved
			break;
		case 0xca000:
			// ANKCG1 / IO / RAM
			break;
		case 0xcb000:
			// CGROM
			break;
		default:
			return;
			break;
		}
		break;
	default:
		return;
		break;
	}		
	
}

uint32_t TOWNS_VRAM::read_memory_mapped_io8(uint32_t addr)
{
	uint32_t naddr = addr & 0xfff00000;
	switch(naddr) {
	case 0x80000000:
		// Main VRAM
		return read_raw_vram8(addr - 0x80000000);
		break;
	case 0x80100000:
		// Main VRAM
		break;
	case 0x00000000:
		switch(addr & 0x000ff000) {
		case 0xc0000:
		case 0xc1000:
		case 0xc2000:
		case 0xc3000:
		case 0xc4000:
		case 0xc5000:
		case 0xc6000:
		case 0xc7000:
			// Plane access by I/O FF81h
			return read_plane_data8(addr);	// Plane access by I/O FF81h
			break;
		case 0xc8000:
			// Text VRAM
			break;
		case 0xc9000:
			// Reserved
			break;
		case 0xca000:
			// ANKCG1 / IO / RAM
			break;
		case 0xcb000:
			// CGROM
			break;
		default:
			return 0xff;
		}
		break;
	default:
		return 0xff;
		break;
	}		
}

uint32_t TOWNS_VRAM::read_memory_mapped_io16(uint32_t addr)
{
	uint32_t naddr = addr & 0xfff00000;
	switch(naddr) {
	case 0x80000000:
		// Main VRAM
		return read_raw_vram16(addr - 0x80000000);
		break;
	case 0x80100000:
		// Main VRAM
		break;
	case 0x00000000:
		switch(addr & 0x000ff000) {
		case 0xc0000:
		case 0xc1000:
		case 0xc2000:
		case 0xc3000:
		case 0xc4000:
		case 0xc5000:
		case 0xc6000:
		case 0xc7000:
			// Plane access by I/O FF81h
			return read_plane_data16(addr);	// Plane access by I/O FF81h
			break;
		case 0xc8000:
			// Text VRAM
			break;
		case 0xc9000:
			// Reserved
			break;
		case 0xca000:
			// ANKCG1 / IO / RAM
			break;
		case 0xcb000:
			// CGROM
			break;
		default:
			return 0xffff;
		}
		break;
	default:
		return 0xffff;
		break;
	}
	return 0xffff;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io32(uint32_t addr)
{
	uint32_t naddr = addr & 0xfff00000;
	switch(naddr) {
	case 0x80000000:
		// Main VRAM
		return read_raw_vram32(addr - 0x80000000);
		break;
	case 0x80100000:
		// Main VRAM
		break;
	case 0x00000000:
		switch(addr & 0x000ff000) {
		case 0xc0000:
		case 0xc1000:
		case 0xc2000:
		case 0xc3000:
		case 0xc4000:
		case 0xc5000:
		case 0xc6000:
		case 0xc7000:
			// Plane access by I/O FF81h
			return read_plane_data32(addr);	// Plane access by I/O FF81h
			break;
		case 0xc8000:
			// Text VRAM
			break;
		case 0xc9000:
			// Reserved
			break;
		case 0xca000:
			// ANKCG1 / IO / RAM
			break;
		case 0xcb000:
			// CGROM
			break;
		default:
			return 0xffffffff;
		}
		break;
	default:
		return 0xffffffff;
		break;
	}		
	return 0xffffffff;
}

uint32_t TOWNS_VRAM::read_raw_vram8(uint32_t addr)
{
	return vram[addr];
}
	
uint32_t TOWNS_VRAM::read_raw_vram16(uint32_t addr)
{
	if(addr == 0x7ffff) {
		pair16_t a;
		a.b.l = vram[addr];
		a.b.h = vram[0];
		return (uint32_t)(a.w);
	} else {
#ifdef __LITTLE_ENDIAN__
		// SAME endian
		uint16_t* p = (uint16_t *)(&vram[addr]);
		return (uint32_t)(*p);
#else
		pair16_t a;
		a.b.l = vram[addr];
		a.b.h = vram[addr + 1];
		return (uint32_t)(a.w);
#endif
	}
}

uint32_t TOWNS_VRAM::read_raw_vram32(uint32_t addr)
{
	if(addr > 0x7fffc) {
		pair32_t a;
		a.b.l  = vram[addr];
		a.b.h  = vram[(addr + 1) & 0x7ffff];
		a.b.h2 = vram[(addr + 2) & 0x7ffff];
		a.b.h3 = vram[(addr + 3) & 0x7ffff];
		return a.d;
	} else {
#ifdef __LITTLE_ENDIAN__
		// SAME endian
		uint32_t* p = (uint32_t *)(&vram[addr]);
		return *p;
#else
		pair32_t a;
		a.b.l  = vram[addr];
		a.b.h  = vram[addr + 1];
		a.b.h2 = vram[addr + 2];
		a.b.h3 = vram[addr + 3];
#endif
		return a.d;
	}
}

void TOWNS_VRAM::write_raw_vram8(uint32_t addr, uint32_t data)
{
	if(vram[addr] != data) {
		make_dirty_vram(addr, 1);
		vram[addr] = data;
	}
}

void TOWNS_VRAM::write_raw_vram16(uint32_t addr, uint32_t data)
{
	if(addr == 0x7ffff) {
		pair16_t a;
		a.w = data;
		if(vram[addr] != a.b.l) {
			make_dirty_vram(addr, 1);
			vram[addr] = a.b.l;
		}
		if(vram[0] != a.b.h) {
			make_dirty_vram(0, 1);
			vram[0] = a.b.h;
		}
		return;
	} else {
#ifdef __LITTLE_ENDIAN__
		// SAME endian
		uint16_t* p = (uint16_t *)(&vram[addr]);
		if(*p != (uint16_t)data) {
			make_dirty_vram(addr, 2);
			*p = data;
		}
		return;
#else
		pair16_t a;
		a.w = data;
		uint16_t* p = (uint16_t *)(&vram[addr]);
		uint16_t b = *p;
		if(a.w != b) {
			make_dirty_vram(addr, 2);
			vram[addr] = a.b.l;
			vram[addr + 1] = a.b.h;
		}
#endif
	}
}

void TOWNS_VRAM::write_raw_vram32(uint32_t addr, uint32_t data)
{
	if(addr > 0x7fffc) {
		pair32_t a;
		a.d = data;
		if(vram[addr] != a.b.l) {
			make_dirty_vram(addr & 0x7ffff, 1);
			vram[addr & 0x7ffff] = a.b.l;
		}
		if(vram[(addr + 1) & 0x7ffff] != a.b.h) {
			make_dirty_vram((addr + 1) & 0x7ffff, 1);
			vram[(addr + 1) & 0x7ffff] = a.b.h;
		}
		if(vram[(addr + 2) & 0x7ffff] != a.b.h2) {
			make_dirty_vram((addr + 2) & 0x7ffff, 1);
			vram[(addr + 2) & 0x7ffff] = a.b.h2;
		}
		if(vram[(addr + 3) & 0x7ffff] != a.b.h3) {
			make_dirty_vram((addr + 3) & 0x7ffff, 1);
			vram[(addr + 3) & 0x7ffff] = a.b.h3;
		}
		return;
	} else {
#ifdef __LITTLE_ENDIAN__
		// SAME endian
		uint32_t* p = (uint32_t *)(&vram[addr]);
		if(*p != data) {
			make_dirty_vram(addr, 4);
			*p = data;
		}
		return;
#else
		pair32_t a;
		a.d = data;
		uint32_t* p = (uint32_t *)(&vram[addr]);
		uint32_t b = *p;
		if(a.d != b) {
			make_dirty_vram(addr, 4);
			vram[addr] = a.b.l;
			vram[addr + 1] = a.b.h;
			vram[addr + 2] = a.b.h2;
			vram[addr + 3] = a.b.h3;
		}
#endif
	}
}



void TOWNS_CRTC::write_io16(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0448:
		voutreg_num = data & 0x01;
		break;
	case 0x044a:
		if(voutreg_num == 0) {
			voutreg_ctrl = data & 0x10;
		} else if(voutreg_num == 1) {
			voutreg_prio = data & 0x10;
		}			
		break;
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



}
#undef _CLEAR_COLOR

