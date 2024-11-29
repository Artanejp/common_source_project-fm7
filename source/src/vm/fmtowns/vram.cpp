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
	pair16_t dmask;
	pair16_t xdata;
	__LIKELY_IF(maddr == 0) { // Aligned
		const uint32_t paddr = calc_std_address_offset(addr);
		
		dmask.read_2bytes_le_from(&(packed_pixel_mask_reg[paddr & 3]));
		xdata.read_2bytes_le_from(&(vram[paddr]));
		xdata.w &= ~(dmask.w);
		data    &= dmask.w;
		xdata.w |= data;
		xdata.write_2bytes_le_to(&(vram[paddr]));
	} else {
		const uint32_t paddr0 = calc_std_address_offset(addr);
		const uint32_t paddr1 = calc_std_address_offset(addr + 1);

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

	}
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
	pair16_t data2;
	switch(address & 0xffff) {
	case 0x0458:
		vram_access_reg_addr = data & 1;
//		out_debug_log(_T("VRAM ACCESS(0458h)=%02X"), data);
		break;
	case 0x045a:
		packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 0] = data;
		packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 4] = data;
//		out_debug_log(_T("VRAM MASK(045Ah)=%08X"), packed_pixel_mask_reg.d);
		break;
	case 0x045b:
		data2.set_2bytes_le_from(data);
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
		vram_access_reg_addr = data & 1;
		break;
	case 0x045a:
		{
			pair16_t w;
			w.set_2bytes_le_from(data);
			packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 0] = w.w;
			packed_pixel_mask_reg[(vram_access_reg_addr << 1) + 4] = w.w;
		}
		break;
	case 0x5ee:
		{
			pair16_t n;
			n.w = data;
			write_io8(0x05ee, n.b.l);
		}
		break;
	default:
		{
			pair16_t w;
			w.w = data;
			write_io8(address, w.b.l);
			write_io8(address + 1, w.b.h);
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
			w.read_2bytes_le_from(&(packed_pixel_mask_reg[(vram_access_reg_addr << 1)]));
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
	default:
		{
			pair16_t w;
			w.b.l = read_io8(address);
			w.b.h = read_io8(address + 1);
			return w.w;
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
}
