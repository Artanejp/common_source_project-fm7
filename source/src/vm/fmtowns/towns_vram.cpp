/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2017.01.16 Initial.
*/


#include "common.h"
#include "./towns_common.h"
#include "./towns_crtc.h"
#include "./towns_vram.h"

#define _CLEAR_COLOR RGBA_COLOR(0,0,0,0)

#if defined(_RGB888)
#define _USE_ALPHA_CHANNEL
#endif

namespace FMTOWNS {

void TOWNS_VRAM::initialize()
{
	memset(vram, 0x00, sizeof(vram));
}

void TOWNS_VRAM::reset()
{
	for(int i = 0; i < (sizeof(dirty_flag) / sizeof(bool)); i++) {
		dirty_flag[i] = true;
	}
	vram_access_reg_addr = 0;
	packed_pixel_mask_reg.d = 0xffffffff;
	write_plane_mask = 0xffffffff;
	
	sprite_busy = false;
	sprite_disp_page = false;
	mix_reg = 0xff;
	r50_readplane = 0x0; // OK?
	r50_ramsel = 0x0; // OK?
	r50_gvramsel = 0x0; // OK?
	
	apalette_code = 0;
	apalette_b = 0;
	apalette_r = 0;
	apalette_g = 0;
	for(int i = 0; i < 16; i++) {
		uint16_t r = ((i & 4) != 0) ? 0x700 : 0;
		uint16_t g = ((i & 2) != 0) ? 0x070 : 0;
		uint16_t b = ((i & 1) != 0) ? 0x007 : 0;
		if((i & 8) != 0) {
			r <<= 1;
			b <<= 1;
			g <<= 1;
		}
		apalette_16_rgb[0][i] = r | g | b;
		apalette_16_rgb[1][i] = r | g | b;
		apalette_16_pixel[0][i] = RGBA_COLOR((r >> 4) | 0x0f, g | 0x0f, (b << 4) | 0x0f, 0xff);
		apalette_16_pixel[1][i] = RGBA_COLOR((r >> 4) | 0x0f, g | 0x0f, (b << 4) | 0x0f, 0xff);
	}
	for(int i = 0; i < 256; i++) {
		apalette_256_rgb[i] = ((i & 0xc0) << 16) | ((i & 0x38) << 8) | (i & 0x07);
		apalette_256_pixel[i] = RGBA_COLOR((i & 0xc0) | 0x3f, ((i & 0x38) << 2) | 0x1f, ((i & 0x07) <<5) | 0x1f, 255);
	}

	for(int i = 0; i < 8; i++) {
		r50_digital_palette[i] = (uint8_t)i;
	}
	layer_display_flags[0] = layer_display_flags[1] = 0;
	r50_dpalette_updated = true;

#if 0	
	// For Debug
	for(uint32_t x = 0; x < (sizeof(vram) / sizeof(uint16_t)); x++) {
//		uint16_t r = ((x & 0x1ffff) / 3) & 0x1f;
//		uint16_t g = ((x & 0x1ffff) / 6) & 0x1f;
//		uint16_t b = ((x & 0x1ffff) / 9) & 0x1f;
//		uint16_t a =  ((x / 640) & 1) ? 0x0000 : 0x8000;
//		uint16_t c = (r << 10) | (g << 5) | b | a;
		uint16_t c = x & 0x7fff;
		if((x / 160) & 0x10) {
			c |= 0x8000;
			if((x / 160) & 0x08) c = 0;
		}		
		uint16_t* p = (uint16_t*)(&vram[x << 1]);
		*p = c;
	}
#endif	
}
	
void TOWNS_VRAM::make_dirty_vram(uint32_t addr, int bytes)
{
	if(bytes <= 0) return;
	uint32_t naddr1 = (addr & 0x7ffff) >> 3;
	uint32_t naddr2 = ((addr + bytes) & 0x7ffff) >> 3;
	if(naddr1 != naddr2) {
		for(uint32_t a = naddr1; a != naddr2;) {
			dirty_flag[a] = true;
			a = (a + 1) & (0x7ffff >> 3);
		}
		dirty_flag[naddr2] = true;
	} else {
		dirty_flag[naddr1] = true;
	}
}
	
void TOWNS_VRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0xfff80000;
	switch(naddr) {
	case 0x80000000:
	case 0x80100000:
		// ToDo: Upper 0x80x80000
		// Main VRAM
		write_raw_vram8(addr, data);
		return;
		break;
	case 0x00080000:
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
			
		case 0xc9000:
		case 0xca000:
		case 0xcb000:
		case 0xcc000:
		case 0xcd000:
		case 0xce000:
		case 0xcf000:
			// Reserved
			if((addr < 0xcff80) && (addr >= 0xcff88)) {
				if(d_sprite != NULL) {
					d_sprite->write_data8(addr & 0x7fff, data);
				}
			} else {
				write_mmio8(addr, data);
			}				
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
	uint32_t naddr = addr & 0xfff80000;
	switch(naddr) {
	case 0x80000000:
	case 0x80100000:
		// ToDo: Upper 0x80x80000
		// Main VRAM
		write_raw_vram16(addr, data);
		return;
		break;
	case 0x00080000:
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
		case 0xc9000:
		case 0xca000:
		case 0xcb000:
		case 0xcc000:
		case 0xcd000:
		case 0xce000:
		case 0xcf000:
			// Reserved
			if((addr < (0xcff80 - 1)) && (addr >= 0xcff88)) {
				if(d_sprite != NULL) {
					d_sprite->write_data16(addr & 0x7fff, data);
				}
			} else {
				pair32_t d;
				d.w.l = data;
				write_mmio8(addr, d.b.l);
				write_mmio8(addr + 1, d.b.h);
			}				
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
	
void TOWNS_VRAM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0xfff80000;
	switch(naddr) {
	case 0x80000000:
	case 0x80100000:
		// ToDo: Upper 0x80x80000
		// Main VRAM
		write_raw_vram32(addr, data);
		return;
		break;
	case 0x00080000:
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
		case 0xc9000:
		case 0xca000:
		case 0xcb000:
		case 0xcc000:
		case 0xcd000:
		case 0xce000:
		case 0xcf000:
			// Reserved
			if((addr < (0xcff80 - 3)) && (addr >= 0xcff88)) {
				if(d_sprite != NULL) {
					d_sprite->write_data32(addr & 0x7fff, data);
				}
			} else {
				pair32_t d;
				d.d = data;
				write_mmio8(addr, d.b.l);
				write_mmio8(addr + 1, d.b.h);
				write_mmio8(addr + 2, d.b.h2);
				write_mmio8(addr + 3, d.b.h3);
			}				
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
	uint32_t naddr = addr & 0xfff80000;
	switch(naddr) {
	case 0x80000000:
	case 0x80100000:
		// ToDo: Upper 0x80x80000
		// Main VRAM
		return read_raw_vram8(addr);
		break;
	case 0x00080000:
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
		case 0xc9000:
		case 0xca000:
		case 0xcb000:
		case 0xcc000:
		case 0xcd000:
		case 0xce000:
		case 0xcf000:
			// Reserved
			if((addr < 0xcff80) && (addr >= 0xcff88)) {
				if(d_sprite != NULL) {
					return d_sprite->read_data8(addr & 0x7fff);
				}
				return 0xff;
			} else {
				return read_mmio8(addr);
			}				
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
	uint32_t naddr = addr & 0xfff80000;
	switch(naddr) {
	case 0x80000000:
	case 0x80100000:
		// ToDo: Upper 0x80x80000
		// Main VRAM
		return read_raw_vram16(addr);
		break;
	case 0x00080000:
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
		case 0xc9000:
		case 0xca000:
		case 0xcb000:
		case 0xcc000:
		case 0xcd000:
		case 0xce000:
		case 0xcf000:
			// Reserved
			if((addr < (0xcff80 - 1)) && (addr >= 0xcff88)) {
				if(d_sprite != NULL) {
					return d_sprite->read_data16(addr & 0x7fff);
				}
				return 0xff;
			} else {
				pair16_t d;
				d.b.l = read_mmio8(addr);
				d.b.h = read_mmio8(addr + 1);
				return d.w;
			}				
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
	uint32_t naddr = addr & 0xfff80000;
	switch(naddr) {
	case 0x80000000:
	case 0x80100000:
		// ToDo: Upper 0x80x80000
		// Main VRAM
		return read_raw_vram32(addr);
		break;
	case 0x00080000:
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
		case 0xc9000:
		case 0xca000:
		case 0xcb000:
		case 0xcc000:
		case 0xcd000:
		case 0xce000:
		case 0xcf000:
			// Reserved
			if((addr < (0xcff80 - 3)) && (addr >= 0xcff88)) {
				if(d_sprite != NULL) {
					return d_sprite->read_data16(addr & 0x7fff);
				}
				return 0xff;
			} else {
				pair32_t d;
				d.b.l = read_mmio8(addr);
				d.b.h = read_mmio8(addr + 1);
				d.b.h2 = read_mmio8(addr + 2);
				d.b.h3 = read_mmio8(addr + 3);
				return d.d;
			}				
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
	return vram[addr & 0x7ffff];
}
	
uint32_t TOWNS_VRAM::read_raw_vram16(uint32_t addr)
{
	pair16_t a;
	bool is_wrap = false;
	uint32_t wrap_addr;
	if((addr & 0x3ffff) == 0x3ffff) {
		if(addr != 0x8013ffff) {
			is_wrap = true;
			wrap_addr = (addr == 0x8007ffff) ? 0x40000 : 0;
		}
	}
	addr = addr & 0x7ffff;
	if(is_wrap) {
		a.b.l = vram[addr];
		a.b.h = vram[wrap_addr];
	} else {
#ifdef __LITTLE_ENDIAN__
		// SAME endian
		uint16_t* p = (uint16_t *)(&vram[addr]);
		a.w = *p;
#else
		a.read_2bytes_le_from(&vram[addr]);
#endif
	}
	return (uint32_t)(a.w);
}

uint32_t TOWNS_VRAM::read_raw_vram32(uint32_t addr)
{
	pair32_t a;
	bool is_wrap = false;
	uint32_t wrap_addr = 0;
	uint32_t wrap_mask;
	
	if((addr & 0x3ffff) > 0x3fffc) {
		if((addr < 0x8013fffd) && (addr > 0x8013ffff)) {
			is_wrap = true;
			wrap_addr = ((addr <= 0x8007ffff) && (addr > 0x8007fffc)) ? 0x40000 : 0;
			wrap_mask = (addr >= 0x80100000) ? 0x7ffff : 0x3ffff;
		}
	}
	addr = addr & 0x7ffff;
	if(is_wrap) {
		a.b.l  = vram[addr];
		a.b.h  = vram[((addr + 1) & wrap_mask) | wrap_addr];
		a.b.h2 = vram[((addr + 2) & wrap_mask) | wrap_addr];
		a.b.h3 = vram[((addr + 3) & wrap_mask) | wrap_addr];
	} else {
#ifdef __LITTLE_ENDIAN__
		uint32_t* p = (uint32_t*)(&vram[addr]);
		a.d = *p;
#else
		a.read_4bytes_le_from(&vram[addr]);
#endif
	}
	return a.d;
}

void TOWNS_VRAM::write_raw_vram8(uint32_t addr, uint32_t data)
{
//	return;
	uint8_t mask;
	uint8_t d1;
	uint8_t d2;
	uint8_t d3;
	switch(addr & 0x03) {
	case 0:
		mask = packed_pixel_mask_reg.b.l;
		break;
	case 1:
		mask = packed_pixel_mask_reg.b.h;
		break;
	case 2:
		mask = packed_pixel_mask_reg.b.h2;
		break;
	case 3:
		mask = packed_pixel_mask_reg.b.h3;
		break;
	}
	addr = addr & 0x7ffff; // ToDo
	d1 = vram[addr];
	d2 = data;
	if(mask != 0xff) {
		d2 = d2 & mask;
		d3 = d1 & ~(mask);
		d2 = d2 | d3;
	}
	if(d1 != d2) {
		make_dirty_vram(addr, 1);
		vram[addr] = d2;
	}
}

void TOWNS_VRAM::write_raw_vram16(uint32_t addr, uint32_t data)
{
//	return;
	pair16_t a;
	pair16_t b;
	pair16_t c;
	uint16_t mask;
	bool is_wrap = false;
	uint32_t wrap_addr = 0;
	mask = ((addr & 0x02) == 0) ? packed_pixel_mask_reg.w.l : packed_pixel_mask_reg.w.h;
	a.w = data;
	
	if((addr & 0x3ffff) == 0x3ffff) {
		if(addr != 0x8013ffff) {
			is_wrap = true;
			wrap_addr = (addr == 0x8007ffff) ? 0x40000 : 0;
		}
	}
	addr = addr & 0x7ffff;
	if(is_wrap) {
		b.b.l = vram[addr];
		b.b.h = vram[wrap_addr];
		c.w = b.w;
		if(mask != 0xffff) {
			b.w = b.w & ~mask;
			a.w = a.w & mask;
			a.w = a.w | b.w;
		}
		if(a.w != c.w) {
			make_dirty_vram(addr, 1);
			make_dirty_vram(wrap_addr, 1);
			vram[addr] = a.b.l;
			vram[wrap_addr] = a.b.h;
		}
	} else {
#ifdef __LITTLE_ENDIAN__
		uint16_t* p = (uint16_t* )(&vram[addr]);
		b.w = *p;
#else
		b.read_2bytes_le_from(&vram[addr]);
#endif
		c.w = b.w;
		if(mask != 0xffff) {
			b.w = b.w & ~mask;
			a.w = a.w & mask;
			a.w = a.w | b.w;
		}
		if(a.w != c.w) {
			make_dirty_vram(addr, 2);
#ifdef __LITTLE_ENDIAN__
			*p = a.w;
#else
			a.write_2bytes_le_to(&vram[addr]);
#endif
		}
	}
	return;
}

void TOWNS_VRAM::write_raw_vram32(uint32_t addr, uint32_t data)
{
//	return; // For Debug
	pair32_t a;
	pair32_t b;
	pair32_t c;
	uint32_t mask;
	bool is_wrap = false;
	uint32_t wrap_addr = 0;
	uint32_t wrap_mask;
	mask = packed_pixel_mask_reg.d;
	a.d = data;
	
	if((addr & 0x3ffff) > 0x3fffc) {
		if((addr < 0x80100000) || (addr >= 0x80140000)) {
			is_wrap = true;
			wrap_addr = ((addr <= 0x8007ffff) && (addr > 0x8008fffc)) ? 0x40000 : 0;
			wrap_mask = (addr >= 0x80100000) ? 0x7ffff : 0x3ffff;
		}
	}
	addr = addr & 0x7ffff;
	if(is_wrap) {
		b.b.l  = vram[addr];
		b.b.h  = vram[((addr + 1) & wrap_mask) | wrap_addr];
		b.b.h2 = vram[((addr + 2) & wrap_mask) | wrap_addr];
		b.b.h3 = vram[((addr + 3) & wrap_mask) | wrap_addr];
		
		c.d = b.d;
		if(mask != 0xffffffff) {
			b.d = b.d & ~mask;
			a.d = a.d & mask;
			a.d = a.d | b.d;
		}
		if(a.d != c.d) {
			int leftbytes = 0x40000 - (addr & 0x3ffff);
			make_dirty_vram(addr, leftbytes);
			make_dirty_vram(wrap_addr, 4 - leftbytes);
			vram[addr] = a.b.l;
			vram[((addr + 1) & wrap_mask) | wrap_addr] = a.b.h;
			vram[((addr + 2) & wrap_mask) | wrap_addr] = a.b.h2;
			vram[((addr + 3) & wrap_mask) | wrap_addr] = a.b.h3;
		}
	} else {
#ifdef __LITTLE_ENDIAN__
		uint32_t* p = (uint32_t* )(&vram[addr]);
		b.d = *p;
#else
		b.read_4bytes_le_from(&vram[addr]);
#endif
		c.d = b.d;
		if(mask != 0xffffffff) {
			b.d = b.d & ~(mask);
			a.d = a.d & mask;
			a.d = a.d | b.d;
		}
		if(a.d != c.d) {
			make_dirty_vram(addr, 4);
#ifdef __LITTLE_ENDIAN__
			*p = a.d;
#else
			d.write_4bytes_le_to(&vram[addr]);
#endif
		}
	}
	return;
}

void TOWNS_VRAM::write_mmio8(uint32_t addr, uint32_t data)
{
	if((addr < 0xcff80) || (addr >= 0xcff88)) {
		if(d_sprite != NULL) {
			d_sprite->write_data8(addr & 0x7fff, data);
		}
		return;
	}
	switch(addr) {
	case 0xcff80:
		mix_reg = data & 0x28;
		break;
	case 0xcff81:
		r50_readplane = (data & 0xc0) >> 6;
		r50_ramsel = data & 0x0f;
		break;
	case 0xcff82:
		if(d_crtc != NULL) {
			d_crtc->write_signal(SIG_TOWNS_CRTC_MMIO_CF882H, data, 0xffffffff);
		}
		break;
	case 0xcff83:
		r50_gvramsel = (data & 0x10) >> 4;
		break;
	case 0xcff86:
		break;
	default:
		break;
	}
}

uint32_t TOWNS_VRAM::read_mmio8(uint32_t addr)
{
	if((addr < 0xcff80) || (addr >= 0xcff88)) {
		if(d_sprite != NULL) {
			return d_sprite->read_data8(addr & 0x7fff);
		}
		return 0xff;
	}
	switch(addr) {
	case 0xcff80:
		return mix_reg;
		break;
	case 0xcff81:
		return ((r50_readplane << 6) | r50_ramsel);
		break;
	case 0xcff82:
		return d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CF882H);
		break;
	case 0xcff83:
		return (r50_gvramsel << 4);
		break;
	case 0xcff86:
		{
			uint8_t d;
			d = (d_crtc->read_signal(SIG_TOWNS_CRTC_VSYNC) != 0) ? 0x04 : 0;
			d = d | ((d_crtc->read_signal(SIG_TOWNS_CRTC_HSYNC) != 0) ? 0x80 : 0);
			d = d | 0x10;
			return d;
		}
		break;
	default:
		break;
	}
	return 0xff;
}

uint32_t TOWNS_VRAM::read_plane_data8(uint32_t addr)
{
	// Plane Access
	uint32_t x_addr = 0;
	uint8_t *p = (uint8_t*)vram;

	// ToDo: Writing plane.
	if(r50_gvramsel != 0) x_addr = 0x20000; //?
	addr = (addr & 0x7fff) << 2;
	p = &(p[x_addr + addr]); 
	
	// 8bit -> 32bit
	uint8_t tmp = 0;
	uint8_t val = 0;
	uint8_t nmask[4] = {0x11, 0x22, 0x44, 0x88};
	uint8_t ntmp = nmask[r50_readplane & 3];
	for(int i = 0; i < 4; i++) {
		val <<= 2;
		tmp = *p++;
		tmp = tmp & ntmp;
		if((tmp & 0xf0) != 0) val |= 0x02;
		if((tmp & 0x0f) != 0) val |= 0x01;
	}
	return val;
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
	uint32_t x_addr = 0;
	uint8_t *p = (uint8_t*)vram;

	// ToDo: Writing plane.
	if(r50_gvramsel != 0) x_addr = 0x20000; //?
	addr = (addr & 0x7fff) << 2;
	x_addr = x_addr + addr;
	p = &(p[x_addr]); 
	
	// 8bit -> 32bit
	uint32_t *pp = (uint32_t *)p;
	uint32_t tmp = 0;
	uint32_t tmp_d = data & 0xff;
	uint8_t ntmp = r50_ramsel & 0x0f;
	
#ifdef __LITTLE_ENDIAN__
	uint32_t tmp_m1 = 0xf0000000/* & write_plane_mask*/;
	uint32_t tmp_m2 = 0x0f000000/* & write_plane_mask*/;
	tmp_m1 &= (((uint32_t)ntmp) << 28);
	tmp_m2 &= (((uint32_t)ntmp) << 24);
	
#else
	uint32_t tmp_m1 = 0x000000f0/* & write_plane_mask*/;
	uint32_t tmp_m2 = 0x0000000f/* & write_plane_mask*/;
	tmp_m1 &= (((uint32_t)ntmp) << 4);
	tmp_m2 &= (((uint32_t)ntmp) << 0);
#endif	
	uint32_t tmp_r1;
	uint32_t tmp_r2;
	uint32_t mask = 0;

	for(int i = 0; i < 4; i++) {
#ifdef __LITTLE_ENDIAN__
		tmp = tmp >> 8;
		mask = mask >> 8;
#else
		tmp = tmp << 8;
		mask = mask << 8;
#endif
		tmp = tmp | (((tmp_d & 0x80) != 0) ? tmp_m1 : 0x00);
		tmp = tmp | (((tmp_d & 0x40) != 0) ? tmp_m2 : 0x00);
		mask = mask | (tmp_m1 | tmp_m2);
		tmp_d <<= 2;
	}
	tmp_r1 = *pp;
	tmp_r2 = tmp_r1;
	tmp_r1 = tmp_r1 & ~mask;
//	tmp_r1 = tmp_r1 & ~write_plane_mask;
	tmp_r1 = tmp | tmp_r1;
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
void TOWNS_VRAM::calc_apalette16(int layer, int index)
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

void TOWNS_VRAM::calc_apalette256(int index)
{
	if(index < 0) return;
	if(index > 255) return;
	apalette_256_rgb[index] =
		((uint32_t)apalette_b) |
		((uint32_t)apalette_r << 8) |
		((uint32_t)apalette_g << 16);
	if(index == 0) {
		apalette_256_pixel[index] = _CLEAR_COLOR; // ??
	} else {
		apalette_256_pixel[index] = RGBA_COLOR(apalette_r, apalette_g, apalette_b, 0xff);
	}
}

void TOWNS_VRAM::set_apalette_r(int layer, uint8_t val)
{
	apalette_r = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_VRAM::set_apalette_g(int layer, uint8_t val)
{
	apalette_g = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_VRAM::set_apalette_b(int layer, uint8_t val)
{
	apalette_b = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_VRAM::set_apalette_num(uint8_t val)
{
	apalette_code = ((int)val) % 256;
}

void TOWNS_VRAM::write_signal(int id, uint32_t data, uint32_t mask)
{
	// ToDo
}
// Renderers

void TOWNS_VRAM::write_io8(uint32_t address,  uint32_t data)
{
	switch(address & 0xffff) {
	case 0x0458:
		vram_access_reg_addr = data & 3;
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			packed_pixel_mask_reg.b.l = data;
			break;
		case 1:
			packed_pixel_mask_reg.b.h2 = data;
			break;
		}			
		break;
	case 0x045b:
		switch(vram_access_reg_addr) {
		case 0:
			packed_pixel_mask_reg.b.h = data;
			break;
		case 1:
			packed_pixel_mask_reg.b.h3 = data;
			break;
		}			
		break;
	}
}

void TOWNS_VRAM::write_io16(uint32_t address,  uint32_t data)
{
	pair32_t d;
	d.d = data;
	switch(address & 0xffff) {
	case 0x0458:
		vram_access_reg_addr = data & 3;
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			packed_pixel_mask_reg.w.l = d.w.l;
			break;
		case 1:
			packed_pixel_mask_reg.w.h = d.w.l;
			break;
		}			
		break;
	}
}

uint32_t TOWNS_VRAM::read_io8(uint32_t address)
{
	switch(address & 0xffff) {
	case 0x0458:
		return vram_access_reg_addr;
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			return packed_pixel_mask_reg.b.l;
			break;
		case 1:
			return packed_pixel_mask_reg.b.h2;
			break;
		}			
		break;
	case 0x045b:
		switch(vram_access_reg_addr) {
		case 0:
			return packed_pixel_mask_reg.b.h;
			break;
		case 1:
			return packed_pixel_mask_reg.b.h3;
			break;
		}			
		break;
	}
	return 0xff;
}

uint32_t TOWNS_VRAM::read_io16(uint32_t address)
{
	switch(address & 0xffff) {
	case 0x0458:
		return vram_access_reg_addr;
		break;
	case 0x045a:
		switch(vram_access_reg_addr) {
		case 0:
			return packed_pixel_mask_reg.w.l;
			break;
		case 1:
			return packed_pixel_mask_reg.w.h;
			break;
		}			
		break;
	}
	return 0xffff;
}

}

#undef _CLEAR_COLOR

