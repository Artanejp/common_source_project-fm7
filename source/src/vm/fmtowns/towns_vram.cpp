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
#include "./towns_planevram.h"

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
	packed_pixel_mask_reg.d = 0xffffffff;
	vram_access_reg_addr = 0;
	sprite_busy = false;
	sprite_disp_page = false;
	
	layer_display_flags[0] = layer_display_flags[1] = 0;
}
	
void TOWNS_VRAM::make_dirty_vram(uint32_t addr, int bytes)
{
	uint32_t amask = 0x7ffff;
	uint32_t naddr1 = (addr & amask) >> 3;
	switch(bytes) {
	case 1:
		dirty_flag[naddr1] = true;
		break;
	case 2:
	case 3:
	case 4:
		{
			uint32_t naddr2 = ((addr + bytes - 1) & amask) >> 3;
			dirty_flag[naddr1] = true;
			dirty_flag[naddr2] = true;
		}
		break;
	}
}
	
void TOWNS_VRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	__DECL_ALIGNED(8) uint8_t mask[4];
	packed_pixel_mask_reg.write_4bytes_le_to(mask);
	uint8_t rmask = mask[addr & 3];
	uint32_t naddr = addr & 0x7ffff;
	uint8_t nd = vram[naddr];
	data = data & rmask;
	nd = nd & ~rmask;
	vram[naddr] = nd | data;
}

void TOWNS_VRAM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	__DECL_ALIGNED(8) uint8_t mask[4];
	uint32_t naddr = addr & 0x7ffff;
	pair16_t nd;
	pair16_t xmask;
	if((naddr & 1) != 0) {
		packed_pixel_mask_reg.write_4bytes_le_to(mask);
		xmask.b.l = mask[addr & 3];
		xmask.b.h = mask[(addr + 1) & 3];
		if(addr == 0x8003ffff) {
			nd.b.l = vram[naddr];
			nd.b.h = vram[0x00000];
		} else 	if(addr == 0x8007ffff) {
			nd.b.l = vram[naddr];
			nd.b.h = vram[0x40000];
		} else 	if(addr == 0x8017ffff) {
			// ToDo: Extra VRAM.
			write_memory_mapped_io8(addr, data);
			return;
		} else {
			nd.read_2bytes_le_from(&(vram[naddr]));
		}
		data = data & xmask.w;
		nd.w = nd.w & ~(xmask.w);
		nd.w = nd.w | data;
		if(addr == 0x8003ffff) {
			vram[naddr] = nd.b.l;
			vram[0x00000] = nd.b.h;
		} else 	if(addr == 0x8007ffff) {
			vram[naddr] = nd.b.l;
			vram[0x40000] = nd.b.h;
		} else {
			// ToDo: Extra VRAM.
			nd.write_2bytes_le_to(&(vram[naddr]));
			return;
		}
	} else {
		xmask.w = ((addr & 2) == 0) ? packed_pixel_mask_reg.w.l : packed_pixel_mask_reg.w.h;
		uint16_t *p = (uint16_t*)(&(vram[naddr]));
		uint16_t n = *p;
		data = data & xmask.w;
		n = n & ~(xmask.w);
		n = n | data;
		*p = n;
	}		
	return;
}
	
void TOWNS_VRAM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{

	__DECL_ALIGNED(8) uint8_t mask[4];
	uint32_t naddr = addr & 0x7ffff;
	pair32_t nd;
	pair32_t xmask;
	if((naddr & 3) != 0) {
		packed_pixel_mask_reg.write_4bytes_le_to(mask);
		xmask.b.l  = mask[addr & 3];
		xmask.b.h  = mask[(addr + 1) & 3];
		xmask.b.h2 = mask[(addr + 1) & 3];
		xmask.b.h3 = mask[(addr + 1) & 3];
		if((addr >= 0x8003fffd) && (addr <= 0x8003ffff)) {
			nd.b.l  = vram[(naddr & 0x3ffff)];
			nd.b.h  = vram[((naddr + 1) & 0x3ffff)];
			nd.b.h2 = vram[((naddr + 2) & 0x3ffff)];
			nd.b.h3 = vram[((naddr + 3) & 0x3ffff)];
		} else 	if((addr >= 0x8007fffd) && (addr <= 0x8007ffff)) {
			nd.b.l  = vram[(naddr & 0x3ffff) + 0x40000];
			nd.b.h  = vram[((naddr + 1) & 0x3ffff) + 0x40000];
			nd.b.h2 = vram[((naddr + 2) & 0x3ffff) + 0x40000];
			nd.b.h3 = vram[((naddr + 3) & 0x3ffff) + 0x40000];
		} else 	if(addr >= 0x8017fffd) {
			// ToDo: Extra VRAM.
			__DECL_ALIGNED(8) uint8_t tmp[4];
			int i = 0;
			for(uint32_t na = (addr & 0x7ffff); na < 0x80000; na++, i++) {
				tmp[i] = vram[na];
			}
			nd.read_4bytes_le_from(tmp);
		} else {
			nd.read_4bytes_le_from(&(vram[naddr]));
		}
		data = data & xmask.d;
		nd.d = nd.d & ~(xmask.d);
		nd.d = nd.d | data;
		if((addr >= 0x8003fffd) && (addr <= 0x8003ffff)) {
			vram[(naddr & 0x3ffff)] = nd.b.l;
			vram[((naddr + 1) & 0x3ffff)] = nd.b.h;
			vram[((naddr + 2) & 0x3ffff)] = nd.b.h2;
			vram[((naddr + 3) & 0x3ffff)] = nd.b.h3;
		} else 	if((addr >= 0x8007fffd) && (addr <= 0x8007ffff)) {
			vram[(naddr & 0x3ffff) + 0x40000] = nd.b.l;
			vram[((naddr + 1) & 0x3ffff) + 0x40000] = nd.b.h;
			vram[((naddr + 2) & 0x3ffff) + 0x40000] = nd.b.h2;
			vram[((naddr + 3) & 0x3ffff) + 0x40000] = nd.b.h3;
		} else if((addr >= 0x8017fffd)) {
			__DECL_ALIGNED(8) uint8_t tmp[4];
			nd.write_4bytes_le_to(tmp);
			int i = 0;
			for(uint32_t na = (addr & 0x7ffff); na < 0x80000; na++, i++) {
				vram[na] = tmp[i];
			}
		} else {
			// ToDo: Extra VRAM.
			nd.write_4bytes_le_to(&(vram[naddr]));
		}
	} else {
		xmask.d = packed_pixel_mask_reg.d;
		uint32_t *p = (uint32_t*)(&(vram[naddr]));
		uint32_t n = *p;
		data = data & xmask.d;
		n = n & ~(xmask.d);
		n = n | data;
		*p = n;
	}		
	return;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io8(uint32_t addr)
{
	return vram[addr & 0x7ffff];
}

uint32_t TOWNS_VRAM::read_memory_mapped_io16(uint32_t addr)
{
	pair16_t a;
	bool is_wrap = false;
	uint32_t wrap_addr;
	if((addr & 0x8013ffff) == 0x8003ffff) {
		is_wrap = true;
		wrap_addr = (addr == 0x8007ffff) ? 0x40000 : 0;
	} else if(addr > 0x8017fffe) {
		is_wrap = true;
		wrap_addr = 0;
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

uint32_t TOWNS_VRAM::read_memory_mapped_io32(uint32_t addr)
{
	pair32_t a;
	bool is_wrap = false;
	uint32_t wrap_addr = 0;
	uint32_t wrap_mask;
	if((addr & 0x8013fffc) == 0x8003fffc) {
		if((addr & 0x8003ffff) != 0x8003fffc) {
			is_wrap = true;
			wrap_addr = (addr >= 0x80040000) ? 0x40000 : 0;
		}
	} else if(addr > 0x8017fffc) {
		is_wrap = true;
		wrap_addr = 0;
	}		
	wrap_mask = (addr >= 0x80100000) ? 0x7ffff : 0x3ffff;
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
//		out_debug_log(_T("VRAM ACCESS(0458h)=%02X"), data);
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
//		out_debug_log(_T("VRAM MASK(045Ah)=%08X"), packed_pixel_mask_reg.d);
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
//		out_debug_log(_T("VRAM MASK(045Bh)=%08X"), packed_pixel_mask_reg.d);
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
//		out_debug_log(_T("VRAM ACCESS(0458h)=%02X"), data);
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
//		out_debug_log(_T("VRAM MASK(045Ah)=%08X"), packed_pixel_mask_reg.d);
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

#define STATE_VERSION	1

bool TOWNS_VRAM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}

	state_fio->StateValue(access_page1);
	state_fio->StateArray(dirty_flag, sizeof(dirty_flag), 1);
	state_fio->StateArray(layer_display_flags, sizeof(layer_display_flags), 1);

	state_fio->StateValue(sprite_busy);
	state_fio->StateValue(sprite_disp_page);

	state_fio->StateValue(vram_access_reg_addr);
	state_fio->StateValue(packed_pixel_mask_reg);

	state_fio->StateArray(vram, sizeof(vram), 1);
	
	return true;
}


#undef _CLEAR_COLOR
}
