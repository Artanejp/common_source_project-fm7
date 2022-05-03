/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns VRAM ]
	History: 2017.01.16 Initial.
*/


#include "common.h"
#include "./towns_common.h"
#include "./crtc.h"
#include "./vram.h"
#include "./planevram.h"

#define _CLEAR_COLOR RGBA_COLOR(0,0,0,0)


namespace FMTOWNS {

void TOWNS_VRAM::initialize()
{
	memset(vram, 0x00, sizeof(vram));
}

void TOWNS_VRAM::reset()
{
	lock();
	
__DECL_VECTORIZED_LOOP						
	for(int i = 0; i < 8; i++) {
		packed_pixel_mask_reg[i] = 0xff;
	}
	vram_access_reg_addr = 0;
	unlock();
}
	
	
void TOWNS_VRAM::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0x7ffff;
	uint8_t mask = packed_pixel_mask_reg[naddr & 3];

	lock();
	uint8_t nd = vram[naddr];
	uint8_t dd = data;
	dd = dd & mask;
	nd = nd & ~mask;

	vram[naddr] = nd | dd;
	unlock();	
	
}

void TOWNS_VRAM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{

	lock();
	
	uint32_t naddr = addr & 0x7ffff;
	uint32_t maddr = addr & 3;
	pair16_t dmask;
	pair16_t xdata;
	pair16_t ydata;

	dmask.b.l = packed_pixel_mask_reg[maddr + 0];
	dmask.b.h = packed_pixel_mask_reg[maddr + 1];
	
	xdata.b.l = vram[naddr + 0];
	xdata.b.h = vram[naddr + 1];

	ydata.w = data;

	xdata.w = xdata.w & ~(dmask.w);
	ydata.w = ydata.w & dmask.w;

	xdata.w = xdata.w | ydata.w;

	vram[naddr + 0] = xdata.b.l;
	vram[naddr + 1] = xdata.b.h;
	
	unlock();
	return;
}
	
void TOWNS_VRAM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{

	lock();

	__DECL_ALIGNED(8) uint8_t mask[8];
	uint32_t naddr = addr & 0x7ffff;
	uint32_t maddr = addr & 3;
	pair32_t dmask;
	pair32_t xdata;
	pair32_t ydata;

	dmask.b.l  = packed_pixel_mask_reg[maddr + 0];
	dmask.b.h  = packed_pixel_mask_reg[maddr + 1];
	dmask.b.h2 = packed_pixel_mask_reg[maddr + 2];
	dmask.b.h3 = packed_pixel_mask_reg[maddr + 3];

	ydata.d = data;
	xdata.b.l = vram[naddr + 0];
	xdata.b.h = vram[naddr + 1];
	xdata.b.h2= vram[naddr + 2];
	xdata.b.h3= vram[naddr + 3];

	xdata.d = xdata.d & ~(dmask.d);
	ydata.d = ydata.d & dmask.d;
	xdata.d = xdata.d | ydata.d;
	
	vram[naddr + 0] = xdata.b.l;
	vram[naddr + 1] = xdata.b.h;
	vram[naddr + 2] = xdata.b.h2;
	vram[naddr + 3] = xdata.b.h3;

	unlock();
	return;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io8(uint32_t addr)
{
	lock();
	uint32_t n = vram[addr & 0x7ffff];
	unlock();
	return n;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io16(uint32_t addr)
{
	uint32_t naddr = addr & 0x7ffff;
	pair16_t data;

	lock();	
	__LIKELY_IF(naddr != 0x7ffff) {
		data.b.l = vram[naddr + 0];
		data.b.h = vram[naddr + 1];
	} else {
		data.b.l = vram[naddr + 0];
		data.b.h = 0xff;
	}	
	unlock();	
	return (uint32_t)(data.w);
}

uint32_t TOWNS_VRAM::read_memory_mapped_io32(uint32_t addr)
{
	uint32_t naddr = addr & 0x7ffff;
	pair32_t data;
	
	lock();	
	data.b.l  = vram[naddr + 0];
	data.b.h  = vram[naddr + 1];
	data.b.h2 = vram[naddr + 2];
	data.b.h3 = vram[naddr + 3];
	unlock();	
		
	__UNLIKELY_IF(naddr > 0x7fffc) {
		static const uint32_t mask[4] = {0x00000000, 0xff000000, 0xffff0000, 0xffffff00};
		data.d |= mask[naddr & 3];
	}
	return data.d;
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
		packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 0] = data;
		packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 4] = data;
//		out_debug_log(_T("VRAM MASK(045Ah)=%08X"), packed_pixel_mask_reg.d);
		break;
	case 0x045b:
		packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 1] = data;
		packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 5] = data;
//		out_debug_log(_T("VRAM MASK(045Bh)=%08X"), packed_pixel_mask_reg.d);
		break;
	case 0x05ee:
		// ToDo: Implement around VRAM cache.
		// VCMEN (Disabled) : Bit0
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
		{
			pair16_t w;
			w.w = data;
			packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 0] = w.b.l;
			packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 1] = w.b.h;
			packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 4] = w.b.l;
			packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 5] = w.b.h;
		}
		break;
	case 0x5ee:
		{
			pair16_t n;
			n.w = data;
			write_io8(0x05ee, n.b.l);
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
		{
			uint8_t v = packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 0]; 
			return (uint32_t)v;
		}
		break;
	case 0x045b:
		{
			uint8_t v = packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 1]; 
			return (uint32_t)v;
		}
		break;
	case 0x5ee:
		// ToDo: Implement around VRAM cache.
		// Bit7 = 0 if ready to turn on/off VRAM cache.
		// VCMEN (Disabled) : Bit0
		if((cpu_id == 0x02) || (cpu_id >= 0x04)) { // i486 SX/DX and after Pentium.
			// Still Disabled VRAM feature and disable VCMEN.
			return 0xff;
		}
		return 0xff;
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
		{
			pair16_t w;
			w.b.l = packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 0]; 
			w.b.h = packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 1]; 
			return (uint32_t)(w.w);
		}
		break;
	case 0x05ee:
		{
			pair16_t n;
			n.b.l = read_io8(0x05ee);
			n.b.h = 0xff;
			return n.w;
		}
		break;
	}
	return 0xffff;
}

#define STATE_VERSION	3

bool TOWNS_VRAM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}

	lock();
	
	state_fio->StateValue(vram_access_reg_addr);
	state_fio->StateArray(packed_pixel_mask_reg, sizeof(packed_pixel_mask_reg), 1);
	
	state_fio->StateArray(vram, sizeof(vram), 1);
	
	unlock();
	return true;
}


#undef _CLEAR_COLOR
}
