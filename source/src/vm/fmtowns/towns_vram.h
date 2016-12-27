/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2016.12.28 Initial.
*/

#ifndef _TOWNS_VRAM_H_
#define _TOWNS_VRAM_H_

#include "../vm.h"
#include "../emu.h"
#include "device.h"

// Older Towns.
#define TOWNS_VRAM_ADDR_MASK 0x7ffff
// VRAM DIRECT ACCESS: For Sprite. You should access with 16bit
// You can write raw data, drawing with colorkey is automatically.
#define SIG_TOWNS_TRANSFER_SPRITE_DATA 0x100000
#define SIG_TOWNS_SET_SPRITE_BANK      0x140000
#define SIG_TOWNS_CLEAR_SPRITE_BUFFER  0x140001
// Do render with any mode. You should set vline to arg.
#define SIG_TOWNS_RENDER_RASTER    0x01
#define SIG_TOWNS_RENDER_FULL      0x02
#define SIG_TOWNS_VRAM_VSTART      0x03
#define SIG_TOWNS_VRAM_VBLANK      0x04
#define SIG_TOWNS_VRAM_VSYNC       0x05
#define SIG_TOWNS_VRAM_HSYNC       0x06
#define SIG_TOWNS_VRAM_SET_VLINE   0x07
#define SIG_TOWNS_RENDER_FLAG      0x08

class TOWNS_VRAM : public DEVICE
{
protected:
	scrntype_t *render_buffer;
	uint32_t page_modes[4];
	uint32_t masks;
	uint8_t vram[0x80000]; // Related by machine.
public:
	TOWNS_VRAM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(vram, 0x00, sizeof(vram));
		render_buffer = NULL;
		page_modes[0] = page_modes[1] = page_modes[2] = page_modes[3] = 0;
		masks = 0;
	}
	~TOWNS_VRAM() {}
	
	uint32_t read_data8(uint32_t addr)
	{
		if(addr < 0x80000000) {
			// Plane Access
			uint32_t data32;
			uint8_t data8;
			uint32_t n_plane = (addr & 0x18000) / 0x8000;
			uint32_t n_addr = addr & 0x7fff;
			uint32_t x_addr = n_addr << 2;
			uint32_t *p;
			if(plane_page1) x_addr += 0x20000;
			p = (uint32_t *)(&(vram[x_addr]));
			data32 = *p;
			data32 >>= (3 - n_plane);
			data8 =
				((data32 & 0x10000000) ? 0x80 : 0) |
				((data32 & 0x01000000) ? 0x40 : 0) | 
				((data32 & 0x00100000) ? 0x20 : 0) | 
				((data32 & 0x00010000) ? 0x10 : 0) | 
				((data32 & 0x00001000) ? 0x08 : 0) |
				((data32 & 0x00000100) ? 0x04 : 0) | 
				((data32 & 0x00000010) ? 0x02 : 0) | 
				((data32 & 0x00000001) ? 0x01 : 0);
			return (uint32_t) data8;
		} else {
			return vram[addr & TOWNS_VRAM_ADDR_MASK];
		}
	};
				
	uint32_t read_data16(uint32_t addr)
	{
		pair_t n;
		n.d = 0;
		if(addr < 0x80000000) {
			addr = addr & 0x1fffe;
			n.b.h = read_data8(addr + 1);
			n.b.l = read_data8(addr);
		} else {
			addr = addr & (TOWNS_VRAM_ADDR_MASK & 0xfffffffe);
			n.b.h = vram[addr + 1];
			n.b.l = vram[addr];
		}
		return n.d;
	}

	uint32_t read_data32(uint32_t addr)
	{
		if(addr < 0x80000000) {
			addr = addr & 0x1fffc;
			n.b.h3 = read_data8(addr + 3);
			n.b.h2 = read_data8(addr + 2);
			n.b.h  = read_data8(addr + 1);
			n.b.l  = read_data8(addr);
		} else {
			addr = addr & (TOWNS_VRAM_ADDR_MASK & 0xfffffffc);
			n.b.h3 = vram[addr + 3];
			n.b.h2 = vram[addr + 2];
			n.b.h  = vram[addr + 1];
			n.b.l  = vram[addr];
		}
		return n.d;
	}

	void write_data8(uint32_t addr, uint32_t data)
	{
		if(addr < 0x80000000) {
			// Plane Access
			pair_t data_p;
			uint32_t n_plane = (addr & 0x18000) / 0x8000;
			uint32_t n_addr = addr & 0x7fff;
			uint32_t x_addr = n_addr << 2;
			
			pair_t *p;
			if(plane_page1) x_addr += 0x20000;
			p = (pair_t *)(&(vram[x_addr]));
			data_p.d = *p;
			// ToDo
		} else {
			// ToDo: Scroll
			uint32_t mask2 = (masks >> ((3 - (addr & 3)) * 8)) & 0xff;
			uint32_t d_s = vram[addr & TOWNS_VRAM_ADDR_MASK] & ~mask2;
			uint32_t d_d = data & mask2;
			vram[addr & TOWNS_VRAM_ADDR_MASK] = d_s | d_d;
		}
	}
	
	void write_data16(uint32_t addr, uint32_t data)
	{
		if(addr < 0x80000000) {
			// Plane Access
			pair_t data_p;
			uint32_t n_plane = (addr & 0x18000) / 0x8000;
			uint32_t n_addr = addr & 0x7fff;
			uint32_t x_addr = n_addr << 2;
			
			pair_t *p;
			if(plane_page1) x_addr += 0x20000;
			p = (pair_t *)(&(vram[x_addr]));
			data_p.d = *p;
			// ToDo
		} else {
			pair_t n;
			uint16_t *p;
			n.d = 0;
			p = (uint16_t *)(&(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xfffffffe]));
			n.sw.l = *p;
			uint32_t mask2 = (masks >> ((2 - (addr & 2)) * 16)) & 0xffff;
			uint32_t d_s = n.d & ~mask2;
			uint32_t d_d = data & mask2;
			// ToDo: Scroll
			n.d = d_s | d_d;
			*p = n.sw.l;
		}
	}

	void write_data32(uint32_t addr, uint32_t data)
	{
		if(addr < 0x80000000) {
			// Plane Access
			pair_t data_p;
			uint32_t n_plane = (addr & 0x18000) / 0x8000;
			uint32_t n_addr = addr & 0x7fff;
			uint32_t x_addr = n_addr << 2;
			
			pair_t *p;
			if(plane_page1) x_addr += 0x20000;
			p = (pair_t *)(&(vram[x_addr]));
			data_p.d = *p;
			// ToDo
		} else {
			uint32_t n;
			uint32_t *p;

			p = (uint32_t *)(&(vram[addr & TOWNS_VRAM_ADDR_MASK & 0xfffffffc]));
			n = *p;
			uint32_t mask2 = masks;
			uint32_t d_s = n & ~mask2;
			uint32_t d_d = data & mask2;
			// ToDo: Scroll
			*p = d_s | d_d;
		}
	}

	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	void draw_screen();
	void write_signal(int id, uint32_t data, uint32_t mask); // Do render

	void set_context_renderbuffer(scrntype_t *p){
		render_buffer = p;
	};
};

#endif
