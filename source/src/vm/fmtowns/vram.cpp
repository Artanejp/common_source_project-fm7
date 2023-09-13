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
	const uint32_t naddr = calc_std_address_offset(addr);
	const uint8_t mask = packed_pixel_mask_reg[naddr & 3];

	lock();
	uint8_t nd = vram[naddr];
	uint8_t dd = data;
	dd &= mask;
	nd &= ~mask;
	dd |= nd;

	vram[naddr] = dd;
	unlock();

}

void TOWNS_VRAM::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	lock();
	const uint32_t maddr = addr & 1;
	const uint32_t paddr0 = calc_std_address_offset(addr);
	const uint32_t paddr1 = calc_std_address_offset(addr + 1);

	pair16_t dmask;
	pair16_t xdata;
	uint16_t ydata;

	dmask.b.l = packed_pixel_mask_reg[paddr0 & 7];
	dmask.b.h = packed_pixel_mask_reg[paddr1 & 7];

	xdata.b.l = vram[paddr0];
	xdata.b.h = vram[paddr1];

	ydata = data;

	xdata.w &= ~(dmask.w);
	ydata   &= dmask.w;

	xdata.w |= ydata;

	vram[paddr0] = xdata.b.l;
	vram[paddr1] = xdata.b.h;

	unlock();
	return;
}

void TOWNS_VRAM::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	lock();
	uint32_t maddr = addr & 3;
	uint32_t paddr0, paddr1, paddr2, paddr3;
	pair32_t dmask;
	pair32_t xdata;
	uint32_t ydata;
	__LIKELY_IF(maddr == 0) { // Aligned
		paddr0 = calc_std_address_offset(addr);
		dmask.read_4bytes_le_from(&(packed_pixel_mask_reg[0]));
		xdata.read_4bytes_le_from(&(vram[paddr0]));
	} else {
		// Unaligned
		paddr0 = calc_std_address_offset(addr);
		paddr1 = calc_std_address_offset(addr + 1);
		paddr2 = calc_std_address_offset(addr + 2);
		paddr3 = calc_std_address_offset(addr + 3);

		dmask.b.l  = packed_pixel_mask_reg[paddr0 & 7];
		dmask.b.h  = packed_pixel_mask_reg[paddr1 & 7];
		dmask.b.h2 = packed_pixel_mask_reg[paddr2 & 7];
		dmask.b.h3 = packed_pixel_mask_reg[paddr3 & 7];

		xdata.b.l  = vram[paddr0];
		xdata.b.h  = vram[paddr1];
		xdata.b.h2 = vram[paddr2];
		xdata.b.h3 = vram[paddr3];
	}

	ydata = data;

	xdata.d &= ~(dmask.d);
	ydata   &= dmask.d;
	xdata.d |= ydata;

	__LIKELY_IF(maddr == 0) { // Aligned
		xdata.write_4bytes_le_to(&(vram[paddr0]));
	} else {
		// Unaligned
		vram[paddr0] = xdata.b.l;
		vram[paddr1] = xdata.b.h;
		vram[paddr2] = xdata.b.h2;
		vram[paddr3] = xdata.b.h3;
	}
	unlock();
	return;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io8(uint32_t addr)
{
	lock();
	const uint32_t naddr = calc_std_address_offset(addr);
	uint32_t n = vram[naddr];
	unlock();
	return n;
}

uint32_t TOWNS_VRAM::read_memory_mapped_io16(uint32_t addr)
{
	const uint32_t paddr0 = calc_std_address_offset(addr);
	const uint32_t paddr1 = calc_std_address_offset(addr + 1);
	//const uint32_t maddr = addr & 1;
	pair16_t data;

	lock();
	data.b.l = vram[paddr0];
	data.b.h = vram[paddr1];
	unlock();
	return (uint32_t)(data.w);
}

uint32_t TOWNS_VRAM::read_memory_mapped_io32(uint32_t addr)
{
	pair32_t data;
	__LIKELY_IF((addr & 3) == 0) { // Aligned
		const uint32_t paddr = calc_std_address_offset(addr);
		data.read_4bytes_le_from(&(vram[paddr]));
	} else {
		const uint32_t paddr0 = calc_std_address_offset(addr);
		const uint32_t paddr1 = calc_std_address_offset(addr + 1);
		const uint32_t paddr2 = calc_std_address_offset(addr + 2);
		const uint32_t paddr3 = calc_std_address_offset(addr + 3);
		data.b.l  = vram[paddr0];
		data.b.h  = vram[paddr1];
		data.b.h2 = vram[paddr2];
		data.b.h3 = vram[paddr3];
	}
	return data.d;
}

void TOWNS_VRAM::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // WAIT SETS by TOWNS_MEMORY:: .
	}
	write_memory_mapped_io8(addr, data);
}

void TOWNS_VRAM::write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // WAIT SETS by TOWNS_MEMORY:: .
	}
	write_memory_mapped_io16(addr, data);
}

void TOWNS_VRAM::write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // WAIT SETS by TOWNS_MEMORY:: .
	}
	write_memory_mapped_io32(addr, data);
}

uint32_t TOWNS_VRAM::read_dma_data8w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // WAIT SETS by TOWNS_MEMORY:: .
	}
	return read_memory_mapped_io8(addr);
}

uint32_t TOWNS_VRAM::read_dma_data16w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // WAIT SETS by TOWNS_MEMORY:: .
	}
	return read_memory_mapped_io16(addr);
}

uint32_t TOWNS_VRAM::read_dma_data32w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // WAIT SETS by TOWNS_MEMORY:: .
	}
	return read_memory_mapped_io32(addr);
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

void TOWNS_VRAM::get_data_from_vram(bool is_single, uint32_t offset, uint32_t bytes, uint8_t* dst)
{
	__UNLIKELY_IF((bytes == 0) || (bytes > (TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t))) || (dst == nullptr)) {
		return;
	}
	uint32_t addr = offset & TOWNS_VRAM_ADDR_MASK;
	uint8_t* p = ___assume_aligned(dst, 16);

	lock();
	if(is_single) {
		addr |= (1 << (TOWNS_VRAM_ADDR_SHIFT + 1));
		__DECL_ALIGNED(32) uint8_t cache[16];
		__DECL_ALIGNED(32) uint8_t cache2[16];
		for(int i = bytes; i >= 16; i -= 16) {
			for(int j = 0; j < 16; j++) {
				cache[j] = vram[calc_std_address_offset(addr + j)];
			}
			__DECL_VECTORIZED_LOOP
			for(int j = 0; j < 16; j++) {
				p[j] = cache[j];
			}
			p += 16;
			addr += 16;
		}
		bytes = bytes & 0x0f; // MOD
		for(int j = 0; j < bytes; j++) {
			cache2[j] = vram[calc_std_address_offset(addr + j)];
		}
		for(int j = 0; j < bytes; j++) {
			p[j] = cache2[j];
		}
	} else {
		__LIKELY_IF((addr + bytes) <= (TOWNS_VRAM_ADDR_MASK + 1)) {
			// Not Wrapped.
			memcpy(p, &(vram[addr]), bytes);
		} else {
			uint32_t nb = (addr + bytes) - (TOWNS_VRAM_ADDR_MASK + 1);
			__LIKELY_IF(nb < bytes) {
				memcpy(p, &(vram[addr]), bytes - nb);
				__LIKELY_IF(nb > 0) {
					memcpy(&(p[bytes - nb]), &(vram[0]), nb);
				}
			} else {
				// Fallthrough.
				memcpy(p, &(vram[addr]), bytes - nb);
			}
		}
	}
	unlock();
}

bool TOWNS_VRAM::set_buffer_to_vram(uint32_t offset, uint8_t *buf, int words)
{
//		uint32_t offset2 = calc_std_address_offset(offset);
	const uint32_t offset2 = offset & TOWNS_VRAM_ADDR_MASK;
//		if(words > 16) return false;
	__UNLIKELY_IF(words <= 0) return false;
	uint8_t* p = &(vram[offset2]);

	lock();
	__LIKELY_IF((offset2 + (words << 1)) <= (TOWNS_VRAM_ADDR_MASK + 1)) {
		memcpy(p, buf, words << 1);
	} else {
		int nb = (TOWNS_VRAM_ADDR_MASK + 1) - offset2;
		memcpy(p, buf, nb);
		int nnb = (words << 1) - nb;
		__LIKELY_IF(nnb > 0) {
			memcpy(vram, &(buf[nb]), nnb);
		}
	}
	unlock();
	return true;
}

bool TOWNS_VRAM::get_vram_to_buffer(uint32_t offset, uint8_t *buf, int words)
{
	//uint32_t offset2 = calc_std_address_offset(offset);
	const uint32_t offset2 = offset & TOWNS_VRAM_ADDR_MASK;
//		if(words > 16) return false;
	__UNLIKELY_IF(words <= 0) return false;

	lock();
	uint8_t* p = &(vram[offset2]);
	__LIKELY_IF((offset2 + (words << 1)) <= (TOWNS_VRAM_ADDR_MASK + 1)) {
		memcpy(buf, p, words << 1);
	} else {
		uint32_t nb = (TOWNS_VRAM_ADDR_MASK + 1) - offset2;
		memcpy(buf, p, nb);
		int nnb = (words << 1) - nb;
		__LIKELY_IF(nnb > 0) {
			memcpy(&(buf[nb]), vram, nnb);
		}
	}
	unlock();
	return true;
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
}
