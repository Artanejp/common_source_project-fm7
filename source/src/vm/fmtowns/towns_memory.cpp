/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.01 -

	[memory]
*/

#include "../../fileio.h"
#include "./towns_memory.h"
#include "./towns_vram.h"
#include "./serialrom.h"
#include "../i386.h"
#include "../pcm1bit.h"

namespace FMTOWNS {
#define EVENT_1US_WAIT 1
	
void TOWNS_MEMORY::initialize()
{
	if(initialized) return;
	initialized = true;
	
	extra_nmi_mask = true;
	extra_nmi_val = false;
	
	vram_wait_val = 6;
	mem_wait_val = 3;

	// Initialize R/W table
	memset(rd_dummy, 0xff, sizeof(rd_dummy));
	memset(wr_dummy, 0x00, sizeof(wr_dummy));
	memset(rd_table, 0x00, sizeof(rd_table));
	memset(wr_table, 0x00, sizeof(wr_table));
	for(int i = 0; i < (0x100000000 >> TOWNS_BANK_SHIFT); i++) {
		rd_table[i].dev = NULL;
		rd_table[i].memory = rd_dummy;
		rd_table[i].wait = 6;
	}
	for(int i = 0; i < (0x100000000 >> TOWNS_BANK_SHIFT); i++) {
		wr_table[i].dev = NULL;
		wr_table[i].memory = rd_dummy;
		wr_table[i].wait = 6;
	}
	
	extram_size = extram_size & 0x3ff00000;
	if(extram_size >= 0x00100000) {
		extra_ram = (uint8_t*)malloc(extram_size);
		if(extra_ram != NULL) {
			set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
			memset(extra_ram, 0x00, extram_size);
		}
	}		
	memset(ram_mmio, 0x00, sizeof(ram_mmio)); // ToDo: Move To Sprite.
	memset(ram_page0, 0x00, sizeof(ram_page0));
	memset(ram_pagef, 0x00, sizeof(ram_pagef));
	
	set_memory_rw(0x00000000, 0x000bffff, ram_page0);
	set_memory_rw(0x000f0000, 0x000f7fff, ram_pagef);
	set_memory_mapped_io_rw(0x000c8000, 0x000cffff, this);
	set_wait_values();
	// Another devices are blank
	
	// load rom image
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
}

void TOWNS_MEMORY::set_wait_values()
{
	set_wait_rw(0x00000000, 0x00100000 + (extram_size & 0x3ff00000) - 1, mem_wait_val);
	// ToDo: Extend I/O Slots
	set_wait_rw(0x80000000, 0x800fffff, vram_wait_val);
	set_wait_rw(0x80100000, 0x801fffff, vram_wait_val);
	// ToDo: pattern RAM
	// ToDo: ROM CARDS
	set_wait_rw(0xc2000000, 0xc213ffff, mem_wait_val);
	// ToDo: DICT RAM and PCM RAM
	set_wait_rw(0xfffc0000, 0xffffffff, mem_wait_val);
}

// Note: This contains SUBSET of MEMORY:: class (except read_bios()).	
void TOWNS_MEMORY::set_memory_r(uint32_t start, uint32_t end, uint8_t *memory)
{
	TOWNS_MEMORY::initialize(); // May overload initialize()
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	if(memory == NULL) {
		for(uint32_t i = start_bank; i <= end_bank; i++) {
			rd_table[i].dev = NULL;
			rd_table[i].memory = rd_dummy;
		}
	} else { 
		for(uint32_t i = start_bank; i <= end_bank; i++) {
			rd_table[i].dev = NULL;
			rd_table[i].memory = memory + bank_size * (i - start_bank);
		}
	}
}

void TOWNS_MEMORY::set_memory_w(uint32_t start, uint32_t end, uint8_t *memory)
{
	TOWNS_MEMORY::initialize(); // May overload initialize()
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	if(memory == NULL) {
		for(uint32_t i = start_bank; i <= end_bank; i++) {
			wr_table[i].dev = NULL;
			wr_table[i].memory = wr_dummy;
		}
	} else { 
		for(uint32_t i = start_bank; i <= end_bank; i++) {
			wr_table[i].dev = NULL;
			wr_table[i].memory = memory + bank_size * (i - start_bank);
		}
	}
}

void TOWNS_MEMORY::set_memory_rw(uint32_t start, uint32_t end, uint8_t *memory)
{
	TOWNS_MEMORY::initialize(); // May overload initialize()
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	if(memory == NULL) {
		for(uint32_t i = start_bank; i <= end_bank; i++) {
			wr_table[i].dev = NULL;
			wr_table[i].memory = wr_dummy;
			rd_table[i].dev = NULL;
			rd_table[i].memory = rd_dummy;
		}
	} else { 
		for(uint32_t i = start_bank; i <= end_bank; i++) {
			wr_table[i].dev = NULL;
			wr_table[i].memory = memory + bank_size * (i - start_bank);
			rd_table[i].dev = NULL;
			rd_table[i].memory = memory + bank_size * (i - start_bank);
		}
	}
}
	
void TOWNS_MEMORY::set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].dev = device;
	}
}

void TOWNS_MEMORY::set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].dev = device;
	}
}
	
void TOWNS_MEMORY::set_memory_mapped_io_rw(uint32_t start, uint32_t end, DEVICE *device)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].dev = device;
		wr_table[i].dev = device;
	}
}

void TOWNS_MEMORY::unset_memory_r(uint32_t start, uint32_t end)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].dev = NULL;
		rd_table[i].memory = rd_dummy;
	}
}

void TOWNS_MEMORY::unset_memory_w(uint32_t start, uint32_t end)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].dev = NULL;
		wr_table[i].memory = wr_dummy;
	}
}
void TOWNS_MEMORY::unset_memory_rw(uint32_t start, uint32_t end)
{
	unset_memory_w(start, end);
	unset_memory_r(start, end);
}
	
void TOWNS_MEMORY::copy_table_w(uint32_t to, uint32_t start, uint32_t end)
{
	TOWNS_MEMORY::initialize();

	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	uint32_t to_bank = to >> addr_shift;
	int blocks = (int)((0xffffffff / bank_size) + 1);
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		if(to_bank >= blocks) break;
		wr_table[to_bank].dev = wr_table[i].dev;
		wr_table[to_bank].memory = wr_table[i].memory;
		wr_table[to_bank].wait = wr_table[i].wait;
		to_bank++;
	}
}

void TOWNS_MEMORY::copy_table_r(uint32_t to, uint32_t start, uint32_t end)
{
	TOWNS_MEMORY::initialize();

	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	uint32_t to_bank = to >> addr_shift;
	int blocks = (int)((0xffffffff / bank_size) + 1);

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		if(to_bank >= blocks) break;
		rd_table[to_bank].dev = rd_table[i].dev;
		rd_table[to_bank].memory = rd_table[i].memory;
		rd_table[to_bank].wait = rd_table[i].wait;
		to_bank++;
	}
}

void TOWNS_MEMORY::copy_table_rw(uint32_t to, uint32_t start, uint32_t end)
{
	copy_table_r(to, start, end);
	copy_table_w(to, start, end);
}	

void TOWNS_MEMORY::set_wait_w(uint32_t start, uint32_t end, int wait)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].wait = wait;
	}
}

void TOWNS_MEMORY::set_wait_r(uint32_t start, uint32_t end, int wait)
{
	TOWNS_MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].wait = wait;
	}
}

void TOWNS_MEMORY::set_wait_rw(uint32_t start, uint32_t end, int wait)
{
	set_wait_r(start, end, wait);
	set_wait_w(start, end, wait);
}

// MEMORY:: don't allow to override member functions , re-made.
// Because Towns's memory access rules are multiple depended by per device.
// 20191202 K.Ohta
uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int *wait)
{
	uint32_t bank = addr >> TOWNS_BANK_SHIFT;
	if(wait != NULL) {
		*wait = rd_table[bank].wait;
	}
	if(rd_table[bank].dev != NULL) {
//		return rd_table[bank].dev->read_data8w(addr, wait);
		return rd_table[bank].dev->read_memory_mapped_io8(addr);
	} else if(rd_table[bank].memory != NULL) {
		return rd_table[bank].memory[addr & (TOWNS_BANK_SIZE - 1)];
	}
	return 0xff;
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int *wait)
{
	uint32_t bank = addr >> TOWNS_BANK_SHIFT;
	if(wait != NULL) {
		*wait = rd_table[bank].wait;
	}
	if(rd_table[bank].dev != NULL) {
		return rd_table[bank].dev->read_data16w(addr, wait);
	} else if(rd_table[bank].memory != NULL) {
		// Internal memories may access with 32bit width.
		pair32_t nd;
		nd.b.l = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 0];
		nd.b.h = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 1];
		return nd.w.l;
	}
	return 0xffff;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int *wait)
{
	uint32_t bank = addr >> TOWNS_BANK_SHIFT;
	if(wait != NULL) {
		*wait = rd_table[bank].wait;
	}
	if(rd_table[bank].dev != NULL) {
		return rd_table[bank].dev->read_data16w(addr, wait);
	} else if(rd_table[bank].memory != NULL) {
		// Internal memories may access with 32bit width.
		pair32_t nd;
		nd.b.l  = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 0];
		nd.b.h  = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 1];
		nd.b.h2 = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 2];
		nd.b.h3 = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 3];
		return nd.d;
	}
	return 0xffffffff;
}

void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	uint32_t bank = addr >> TOWNS_BANK_SHIFT;
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	if(wr_table[bank].dev != NULL) {
		wr_table[bank].dev->write_data8w(addr, data, wait);
	} else if(wr_table[bank].memory != NULL) {
		// Internal memories may access with 32bit width.
		wr_table[bank].memory[addr & (TOWNS_BANK_SIZE - 1)] = data;
	}
}

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int *wait)
{
	uint32_t bank = addr >> TOWNS_BANK_SHIFT;
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	if(wr_table[bank].dev != NULL) {
		wr_table[bank].dev->write_data16w(addr, data, wait);
	} else if(wr_table[bank].memory != NULL) {
		// Internal memories may access with 32bit width.
		pair32_t nd;
		nd.d = data; 
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 0] = nd.b.l;
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 1] = nd.b.h;
	}
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int *wait)
{
	uint32_t bank = addr >> TOWNS_BANK_SHIFT;
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	if(wr_table[bank].dev != NULL) {
		wr_table[bank].dev->write_data32w(addr, data, wait);
	} else if(wr_table[bank].memory != NULL) {
		// Internal memories may access with 32bit width.
		pair32_t nd;
		nd.d = data; 
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 0] = nd.b.l;
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 1] = nd.b.h;
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 2] = nd.b.h2;
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 4)) + 3] = nd.b.h3;
	}
}

uint32_t TOWNS_MEMORY::read_dma_data8w(uint32_t addr, int* wait)
{
	int bank = (addr & (TOWNS_BANK_SIZE - 1)) >> addr_shift;
	if(rd_table[bank].dev != NULL) { 
//		return rd_table[bank].dev->read_dma_data8w(addr, wait);
	} else if(dma_is_vram) {
		if(d_vram != NULL) {
			return d_vram->read_dma_data8w(addr, wait);
		}
	} else if(rd_table[bank].memory != NULL) {
		if(wait != NULL) {
			*wait = mem_wait_val;
		}
		return rd_table[bank].memory[addr & (TOWNS_BANK_SIZE - 1)];
	}
	return 0xff;
}

uint32_t TOWNS_MEMORY::read_dma_data16w(uint32_t addr, int* wait)
{
	int bank = (addr & (TOWNS_BANK_SIZE - 2)) >> addr_shift;
	if(rd_table[bank].dev != NULL) { 
//		return rd_table[bank].dev->read_dma_data16w(addr, wait);
	} else if(dma_is_vram) {
		if(d_vram != NULL) {
			return d_vram->read_dma_data16w(addr, wait);
		}
	} else if(rd_table[bank].memory != NULL) {
		if(wait != NULL) {
			*wait = mem_wait_val;
		}
		pair16_t nd;
		nd.b.l = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 0];
		nd.b.h = rd_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 1];
		return nd.w;
	}
	return 0xffff;
}


void TOWNS_MEMORY::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	int bank = (addr & (TOWNS_BANK_SIZE - 1)) >> addr_shift;
	if(wr_table[bank].dev != NULL) { 
//		rd_table[bank].dev->write_dma_data8w(addr, data, wait);
		return;
	} else if(dma_is_vram) {
		if(d_vram != NULL) {
			d_vram->write_dma_data8w(addr, data, wait);
			return;
		}
	} else if(wr_table[bank].memory != NULL) {
		if(wait != NULL) {
			*wait = mem_wait_val;
		}
		wr_table[bank].memory[addr & (TOWNS_BANK_SIZE - 1)] = data;
	}
	return;
}

void TOWNS_MEMORY::write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
{
	int bank = (addr & (TOWNS_BANK_SIZE - 2)) >> addr_shift;
	if(wr_table[bank].dev != NULL) { 
//		rd_table[bank].dev->write_dma_data8w(addr, data, wait);
		return;
	} else if(dma_is_vram) {
		if(d_vram != NULL) {
			d_vram->write_dma_data16w(addr, data, wait);
			return;
		}
	} else if(wr_table[bank].memory != NULL) {
		if(wait != NULL) {
			*wait = mem_wait_val;
		}
		pair32_t nd;
		nd.d = data;
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 0] = nd.b.l;
		wr_table[bank].memory[(addr & (TOWNS_BANK_SIZE - 2)) + 1] = nd.b.h;
	}
	return;
}
uint32_t TOWNS_MEMORY::read_dma_data8(uint32_t addr)
{
	int dummy;
	return read_dma_data8w(addr, &dummy);
}

uint32_t TOWNS_MEMORY::read_dma_data16(uint32_t addr)
{
	int dummy;
	return read_dma_data16w(addr, &dummy);
}

void TOWNS_MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	int dummy;
	return write_dma_data8w(addr, data, &dummy);
}

void TOWNS_MEMORY::write_dma_data16(uint32_t addr, uint32_t data)
{
	int dummy;
	return write_dma_data16w(addr, data, &dummy);
}

void TOWNS_MEMORY::release()
{
	if(extra_ram != NULL) {
		free(extra_ram);
		extra_ram = NULL;
	}
}
void TOWNS_MEMORY::reset()
{
	// reset memory
	// ToDo
	d_cpu->set_address_mask(0xffffffff);
	dma_is_vram = false;
	nmi_vector_protect = false;
	set_wait_values();
}

uint32_t TOWNS_MEMORY::read_data8(uint32_t addr)
{
	int dummy;
	return read_data8w(addr, &dummy);
}

uint32_t TOWNS_MEMORY::read_data16(uint32_t addr)
{
	int dummy;
	return read_data16w(addr, &dummy);
}

uint32_t TOWNS_MEMORY::read_data32(uint32_t addr)
{
	int dummy;
	return read_data32w(addr, &dummy);
}

void TOWNS_MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	int dummy;
	return write_data8w(addr, data, &dummy);
}

void TOWNS_MEMORY::write_data16(uint32_t addr, uint32_t data)
{
	int dummy;
	return write_data16w(addr, data, &dummy);
}

void TOWNS_MEMORY::write_data32(uint32_t addr, uint32_t data)
{
	int dummy;
	return write_data32w(addr, data, &dummy);
}

// Address (TOWNS BASIC):
// 0x0020 - 0x0022, 0x0030-0x0031,
// 0x0400 - 0x0404,
// 0x0480 - 0x0484
// 0x05c0 - 0x05c2
// 0x05ec (Wait register)
// Is set extra NMI (0x05c0 - 0x05c2)?
uint32_t TOWNS_MEMORY::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		val = ((software_reset) ? 1 : 0) | ((d_cpu->get_shutdown_flag() != 0) ? 2 : 0);
		software_reset = false;
		d_cpu->set_shutdown_flag(0);
		val =  val | 0x7c;
		break;
	case 0x0022:
		// Power register
		val = 0xff;
		break;
	case 0x0030:
		val = (((machine_id & 0x1f) << 3) | (cpu_id & 7));
		// SPEED: bit0/Write
		break;
	case 0x0031:
		val = ((machine_id >> 5) & 0xff);
		break;
	case 0x0032:
		{
			//bool __cs = (d_serialrom->read_signal(SIG_SERIALROM_CS) == 0);
			bool __clk = (d_serialrom->read_signal(SIG_SERIALROM_CLK) != 0);
			bool __reset = (d_serialrom->read_signal(SIG_SERIALROM_RESET) != 0);
			bool __dat = (d_serialrom->read_signal(SIG_SERIALROM_DATA) != 0);
			val = ((__reset) ? 0x80 : 0x00) | ((__clk) ? 0x40 : 0x00) | 0x3e | ((__dat) ? 0x01 : 0x00);
		}
		break;
	case 0x006c: // Wait register.
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			val = 0x7f;
		}
		break;
	case 0x0400: // Resolution:
		val = 0xfe;
		break;
	case 0x0404: // System Status Reg.
		val = (dma_is_vram) ? 0x7f : 0xff;
		break;
	case 0x05c0:
		val = (extra_nmi_mask) ? 0xf7 : 0xff;
		break;
	case 0x05c2:
		val = (extra_nmi_val) ? 0xff : 0xf7;
		break;
	case 0x05e8:
		// After Towns1F/2F/1H/2H
		{
			switch(machine_id & 0xff00) {
			case 0x0000:
			case 0x0100:
				val = 0xff;
				break;
			case 0x0200:
			case 0x0300:
			case 0x0400:
			case 0x0500:
			case 0x0600:
			case 0x0700:
			case 0x0800:
			case 0x0a00:
				val = ((extram_size >> 20) & 0x1f);
				break;
			case 0x0b00:
			case 0x0c00:
			case 0x0d00:
			case 0x0f00:
				val = ((extram_size >> 20) & 0x7f);
				break;
			default:
				val = 0xff; // ???
				break;
			}
		}
		break;
	   
	case 0x05ec:
		if(machine_id >= 0x0500) { // Towns2 CX : Is this hidden register after Towns 1F/2F/1H/2H?
		   val = 0x00 | ((mem_wait_val > 0) ? 0x01 : 0x00); 
		}
		break;
	default:
		break;
	}
	return val;
}

void TOWNS_MEMORY::write_io8(uint32_t addr, uint32_t data)
{

	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		if((data & 0x80) != 0) {
			nmi_vector_protect = true;
		} else {
			nmi_vector_protect = false;
		}
		if((data & 0x01) != 0) {
			software_reset = true;
		} else {
			software_reset = false;
		}
		if((data & 0x40) != 0) {
			d_cpu->set_shutdown_flag(1);
			emu->power_off();
		}
		if(software_reset) {
			d_cpu->reset();
		}
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
			d_cpu->set_shutdown_flag(1);
			emu->power_off();
		}
		// Power register
		break;
	case 0x0032:
		{
			d_serialrom->write_signal(SIG_SERIALROM_CS, ~data, 0x20);
			d_serialrom->write_signal(SIG_SERIALROM_CLK, data, 0x40);
			d_serialrom->write_signal(SIG_SERIALROM_RESET, data, 0x80);
		}
		break;
	case 0x006c: // Wait register.
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			if(event_wait_1us != -1) cancel_event(this, event_wait_1us);
			register_event(this, EVENT_1US_WAIT, 1.0, false, &event_wait_1us);
			d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		}
		break;
	case 0x0404: // System Status Reg.
		dma_is_vram = ((data & 0x80) != 0);
		break;
	case 0x05c0:
		extra_nmi_mask = ((data & 0x08) == 0);
		break;
	case 0x05ec:
		if(machine_id >= 0x0500) { // Towns2 CX : Is this hidden register after Towns 1F/2F/1H/2H?
			vram_wait_val = ((data & 0x01) != 0) ? 3 : 6;
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 3;
		}
		set_wait_values();
		break;
	default:
		break;
	}
	return;
}

void TOWNS_MEMORY::event_callback(int id, int err)
{
	switch(id) {
	case EVENT_1US_WAIT:
		event_wait_1us = -1;
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		}
		break;
	default:
		break;
	}
	
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io8(uint32_t addr)
{
	if(addr >= 0x000d0000) return 0xff;
	if(addr <  0x000c8000) return 0xff;
	if(addr <  0x000cff80) {
		return ram_mmio[addr & 0x7fff];
	}
	uint32_t val = 0xff;
	switch(addr & 0x7f) {
	case 0x00:
		if(d_vram != NULL) {
			//val = d_vram->read_io8(FMTOWNS_VRAM_IO_CURSOR);
		}
		break;
	case 0x01:
		if(d_vram != NULL) {
			//val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_RAMSELECT);
		}
		break;
	case 0x02:
		if(d_vram != NULL) {
			//val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_DISPMODE);
		}
		break;
	case 0x03:
		if(d_vram != NULL) {
			//val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_PAGESEL);
		}
		break;
	case 0x04:
		val = 0x7f; // Reserve.FIRQ
		break;
	case 0x06:
		if(d_vram != NULL) {
			//val = d_vram->read_io8(FMTOWNS_VRAM_IO_SYNC_STATUS);
		}
		break;
	//case 0x14:
	//case 0x15:
	case 0x16:
	case 0x17:
		if(d_vram != NULL) {
			//val = d_vram->read_io8(FMTOWNS_VRAM_KANJICG + (addr & 3));
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			//d_beep->write_signal(SIG_BEEP_ON, 1, 1);
		}
		break;
	case 0x19:
		val = val & ((ankcg_enabled) ? 0x00 : 0x01);
		break;
	case 0x20:
		val = 0xff;
		val = val & 0x7f;
		break;
	default:
		break;
	}
	return (uint32_t)val;
}

void TOWNS_MEMORY::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if(addr >= 0x000d0000) return;
	if(addr <  0x000c8000) return;
	if(addr <  0x000cff80) {
		ram_mmio[addr & 0x7fff] = data;
		return;
	}
	switch(addr & 0x7f) {
	case 0x00:
		if(d_vram != NULL) {
			//d_vram->write_io8(FMTOWNS_VRAM_IO_CURSOR, data);
		}
		break;
	case 0x01:
		if(d_vram != NULL) {
			//d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_RAMSELECT, data);
		}
		break;
	case 0x02:
		if(d_vram != NULL) {
			//d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_DISPMODE, data);
		}
		break;
	case 0x03:
		if(d_vram != NULL) {
			//d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_PAGESEL, data);
		}
		break;
	case 0x04:
		break;
	case 0x06:
		break;
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		if(d_vram != NULL) {
			//d_vram->write_io8(FMTOWNS_VRAM_KANJICG + (addr & 3), data);
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			//d_beep->write_signal(SIG_BEEP_ON, 0, 1);
		}
		break;
	case 0x19:
	    ankcg_enabled = ((data & 1) == 0);
		break;
	case 0x20:
		break;
	default:
		break;
	}
	return;
}
void TOWNS_MEMORY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_MEMORY_EXTNMI) {
		extra_nmi_val = ((data & mask) != 0);
	} else if(ch == SIG_CPU_NMI) {
		// Check protect
		d_cpu->write_signal(SIG_CPU_NMI, data, mask);
	} else if(ch == SIG_CPU_IRQ) {
		d_cpu->write_signal(SIG_CPU_IRQ, data, mask);
	} else if(ch == SIG_CPU_BUSREQ) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, data, mask);
	} else if(ch == SIG_I386_A20) {
		d_cpu->write_signal(SIG_I386_A20, data, mask);
	} else if(ch == SIG_FMTOWNS_RAM_WAIT) {
		mem_wait_val = (int)data;
		set_wait_values();
	} else if(ch == SIG_FMTOWNS_ROM_WAIT) {
//		mem_wait_val = (int)data;
		set_wait_values();
	} else if(ch == SIG_FMTOWNS_VRAM_WAIT) {
		vram_wait_val = (int)data;
		set_wait_values();
	}
}

uint32_t TOWNS_MEMORY::read_signal(int ch)
{
	if(ch == SIG_FMTOWNS_MACHINE_ID) {
		uint16_t d = (machine_id & 0xfff8) | ((uint16_t)(cpu_id & 0x07));
		return (uint32_t)d;
	} else if(ch == SIG_FMTOWNS_RAM_WAIT) {
		return (uint32_t)mem_wait_val;
	} else if(ch == SIG_FMTOWNS_ROM_WAIT) {
		return 6; // OK?
	} else if(ch == SIG_FMTOWNS_VRAM_WAIT) {
		return (uint32_t)vram_wait_val;
	} 
	return 0;
}
// ToDo: DMA

#define STATE_VERSION	1

bool TOWNS_MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);
	
	state_fio->StateValue(dma_is_vram);
	state_fio->StateValue(nmi_vector_protect);
	state_fio->StateValue(software_reset);

	state_fio->StateValue(ankcg_enabled);
	state_fio->StateValue(event_wait_1us);
	state_fio->StateValue(extra_nmi_val);
	state_fio->StateValue(extra_nmi_mask);
	
	state_fio->StateArray(ram_page0, sizeof(ram_page0), 1);
	state_fio->StateArray(ram_page0, sizeof(ram_mmio), 1);
	state_fio->StateArray(ram_pagef, sizeof(ram_pagef), 1);
	if(loading) {
		uint32_t length_tmp = state_fio->FgetUint32_LE();
		if(extra_ram != NULL) {
			free(extra_ram);
			extra_ram = NULL;
		}
		length_tmp = length_tmp & 0x3ff00000;
		extram_size = length_tmp;
		if(length_tmp > 0) {
			extra_ram = (uint8_t*)malloc(length_tmp);
		}
		unset_memory_rw(0x00100000, 0x3fffffff);
		if(extra_ram == NULL) {
			extram_size = 0;
			return false;
		} else {
			state_fio->Fread(extra_ram, extram_size, 1);
			set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
		}
		set_wait_values();
	} else {
		// At saving
		if(extra_ram == NULL) {
			state_fio->FputUint32_LE(0);
		} else {
			state_fio->FputUint32_LE(extram_size & 0x3ff00000);
			state_fio->Fwrite(extra_ram, extram_size, 1);
		}
	}
			
	state_fio->StateValue(vram_wait_val);
	state_fio->StateValue(mem_wait_val);
	state_fio->StateValue(vram_size);

	// ToDo: Do save ROMs?
	return true;
}

}
