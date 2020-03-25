/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.01 -

	[memory]
*/

#include "../../fileio.h"
#include "./towns_memory.h"
#include "./towns_dmac.h"
#include "./towns_vram.h"
#include "./towns_sprite.h"
#include "./fontroms.h"
#include "./serialrom.h"
#include "./towns_crtc.h"

#include "../i386_np21.h"
#include "../pcm1bit.h"

#include <math.h>

namespace FMTOWNS {

#define ADDR_MASK (addr_max - 1)
#define BANK_MASK (bank_size - 1)
	
void TOWNS_MEMORY::config_page00()
{
	if(dma_is_vram) {
		set_memory_rw          (0x00000000, 0x000bffff, ram_page0);
//		set_memory_mapped_io_r (0x000b0000, 0x000bffff, d_msdos); // OK? <- for compatible ROM.
		set_memory_rw          (0x000c0000, 0x000cffff, ram_pagec);
		set_memory_mapped_io_rw(0x000c0000, 0x000c7fff, d_vram);
		set_memory_mapped_io_rw(0x000c8000, 0x000c8fff, d_sprite);
		set_memory_mapped_io_rw(0x000ca000, 0x000cafff, d_sprite);
		if(ankcg_enabled) {
//			set_memory_mapped_io_r(0x000ca000, 0x000ca7ff, d_font);
			set_memory_mapped_io_r(0x000cb000, 0x000cbfff, d_font);
		}
		set_memory_mapped_io_rw(0x000cf000, 0x000cffff, this);
		set_memory_rw          (0x000d0000, 0x000d7fff, ram_paged);
		set_memory_mapped_io_rw(0x000d8000, 0x000d9fff, d_dictionary); // CMOS
	} else {
		set_memory_rw          (0x00000000, 0x000bffff, ram_page0);
		set_memory_rw          (0x000c0000, 0x000cffff, ram_pagec);
		set_memory_rw          (0x000d0000, 0x000d9fff, ram_paged);
	}		
	set_memory_rw          (0x000da000, 0x000effff, ram_pagee);
	set_memory_rw          (0x000f0000, 0x000f7fff, ram_pagef);
	set_memory_mapped_io_rw(0x000f8000, 0x000fffff, d_sysrom);
}
	
void TOWNS_MEMORY::initialize()
{
//	if(initialized) return;
	MEMORY::initialize();
//	DEVICE::initialize();
	
	extra_nmi_mask = true;
	extra_nmi_val = false;
	poff_status = false;

//	vram_wait_val = 6;
//	mem_wait_val = 3;
	vram_wait_val = 6;
	mem_wait_val = 3;
	mem_wait_val >>= 1;
	vram_wait_val >>= 1;

	// Initialize R/W table
	_MEMORY_DISABLE_DMA_MMIO = osd->check_feature(_T("MEMORY_DISABLE_DMA_MMIO"));
	if(!(addr_max_was_set) && osd->check_feature(_T("MEMORY_ADDR_MAX"))) {
		addr_max = osd->get_feature_uint64_value(_T("MEMORY_ADDR_MAX"));
	}
	if(!(bank_size_was_set) && osd->check_feature(_T("MEMORY_BANK_SIZE"))) {
		bank_size = osd->get_feature_uint64_value(_T("MEMORY_BANK_SIZE"));
	}

	bank_mask = BANK_MASK;
	addr_mask = ADDR_MASK;
	
	initialized = true;
	extram_size = extram_size & 0x3ff00000;
	set_extra_ram_size(extram_size >> 20); // Check extra ram size.
	if(extram_size >= 0x00100000) {
		extra_ram = (uint8_t*)malloc(extram_size);
		if(extra_ram != NULL) {
			set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
			memset(extra_ram, 0x00, extram_size);
		}
	}		
	memset(ram_page0, 0x00, sizeof(ram_page0));
	memset(ram_pagec, 0x00, sizeof(ram_pagec));
	memset(ram_paged, 0x00, sizeof(ram_paged));
	memset(ram_pagee, 0x00, sizeof(ram_pagee));
	memset(ram_pagef, 0x00, sizeof(ram_pagef));
	
	dma_is_vram = true;
	config_page00();
	
	set_memory_mapped_io_rw(0x80000000, 0x81ffffff, d_vram);
//	set_memory_mapped_io_rw(0xc0000000, 0xc0ffffff, d_iccard[0]);
//	set_memory_mapped_io_rw(0xc1000000, 0xc1ffffff, d_iccard[1]);
	set_memory_mapped_io_r (0xc2000000, 0xc207ffff, d_msdos);
	set_memory_mapped_io_r (0xc2080000, 0xc20fffff, d_dictionary);
	set_memory_mapped_io_r (0xc2100000, 0xc213ffff, d_font);
	set_memory_mapped_io_rw(0xc2140000, 0xc2141fff, d_dictionary);
	if(d_font_20pix != NULL) {
		set_memory_mapped_io_r (0xc2180000, 0xc21fffff, d_font_20pix);
	}
	set_memory_mapped_io_r (0xfffc0000, 0xffffffff, d_sysrom);
	set_wait_values();
	// Another devices are blank
	
	// load rom image
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int* wait)
{
	int dummy;
	int bank = (addr & ADDR_MASK) >> addr_shift;
	uint32_t val = MEMORY::read_data16w(addr, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int* wait)
{
	int dummy;
	int bank = (addr & ADDR_MASK) >> addr_shift;
	uint32_t val = MEMORY::read_data32w(addr, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	return val;
}

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	int dummy;
	int bank = (addr & ADDR_MASK) >> addr_shift;
	MEMORY::write_data16w(addr, data, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	int dummy;
	int bank = (addr & ADDR_MASK) >> addr_shift;
	MEMORY::write_data32w(addr, data, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
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

void TOWNS_MEMORY::release()
{
	if(rd_table != NULL) free(rd_table);
	if(rd_dummy != NULL) free(rd_dummy);
	if(wr_table != NULL) free(wr_table);
	if(wr_dummy != NULL) free(wr_dummy);
	
	if(extra_ram != NULL) {
		free(extra_ram);
		extra_ram = NULL;
	}
}
void TOWNS_MEMORY::reset()
{
	// reset memory
	// ToDo
	is_compatible = true;
#if 1	
	if(d_cpu != NULL) {
		d_cpu->set_address_mask(0xffffffff);
	}
	if(d_dmac != NULL) {
		d_dmac->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, 0xffffffff, 0xffffffff);
	}
#endif
	dma_is_vram = true;
	nmi_vector_protect = false;
	ankcg_enabled = false;
	nmi_mask = false;
	config_page00();
	set_wait_values();
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
	uint32_t val = 0x00;  // MAY NOT FILL to "1" for unused bit 20200129 K.O
	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		if(d_cpu != NULL) {
			val = ((software_reset) ? 1 : 0) | ((d_cpu->get_shutdown_flag() != 0) ? 2 : 0);
		}
		software_reset = false;
		if(d_cpu != NULL) {
			d_cpu->set_shutdown_flag(0);
		}
		if((machine_id >= 0x0300) & ((machine_id & 0xff00) != 0x0400)) { // After UX
			val = val | ((poff_status) ? 0x04 : 0x00);
		}
		break;
	case 0x0022:
//		if(d_dmac != NULL) {
//			val = d_dmac->read_signal(SIG_TOWNS_DMAC_ADDR_REG);
//		}
		break;
	case 0x0024:
		// CPU MISC3
		val = 0xff;
		if(machine_id >= 0x0700) { // After HR/HG
			val &= ~0x08; // POFFEN (
		}
		if(machine_id >= 0x0700) { // After HR/HG
			val &= ~0x10; // Free run counter
		}
		if(machine_id >= 0x0700) { // After HR/HG
			val &= ~0x20; // CRTPOWOFF (0022h)
		}
		if(machine_id >= 0x0700) { // After HR/HG
			val &= ~0x40; // RCREN
		}
		if(machine_id >= 0x0700) { // After HR/HG
			val &= ~0x80; // ENPOFF
		}
		break;
	case 0x0025:
		// CPU MISC4
		if(machine_id >= 0x0500) { // After CX
			val = 0x7f;
		} else {
			val = 0xff;
		}
		break;
	case 0x0028:
		// NMI MASK
		if(machine_id >= 0x0500) { // After CX
			val = (nmi_mask) ? 0x01 : 0x00;
		}
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
	case 0x0400: // Resolution:
//		val = 0xfe;
		val = 0x00;
		break;
	case 0x0404: // System Status Reg.
//		val = (dma_is_vram) ? 0x7f : 0xff;
		val = (dma_is_vram) ? 0x00 : 0x80;
		break;
	case 0x05c0:
//		val = (extra_nmi_mask) ? 0xf7 : 0xff;
		val = (extra_nmi_mask) ? 0x00 : 0x08;
		break;
	case 0x05c2:
//		val = (extra_nmi_val) ? 0xff : 0xf7;
		val = (extra_nmi_val) ? 0x08 : 0x00;
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
				val = 0x00; // ???
				break;
			}
		}
		break;
	   
	case 0x05ec:
		if(machine_id >= /*0x0500*/0x0200) { // Towns2 CX : Is this hidden register after Towns 1F/2F/1H/2H? -> Yes
		   val = 0x00 | ((mem_wait_val > 0) ? 0x01 : 0x00); 
		}
		break;
	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			return (is_compatible) ? 0x00 : 0x01;
		} else {
			return 0x00;
		}
	default:
		break;
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_io16(uint32_t addr)
{
	{
			// OK?
		pair16_t n;
		n.b.l = read_io8((addr & 0xfffe) + 0);
		n.b.h = read_io8((addr & 0xfffe) + 1);
		return n.w;
	}
	return 0xffff;
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
			poff_status = true;
//			if(d_cpu != NULL) {
//				d_cpu->set_shutdown_flag(1);
//			}
			// Todo: Implement true power off.
//			emu->power_off();
		} else {
			poff_status = false;
			if(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(0);
			}
		}
		
		if(software_reset) {
			if(d_cpu != NULL) {
				d_cpu->reset();
			}
		}
		// Towns SEEMS to not set addreess mask (a.k.a A20 mask). 20200131 K.O	
		if(d_dmac != NULL) {
			d_dmac->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, 0xffffffff, 0xffffffff);
		}
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
//			if(d_cpu != NULL) {
//				d_cpu->set_shutdown_flag(1);
//			}
			// Todo: Implement true power off.
//			emu->power_off();
		}
		if(d_dmac != NULL) {
			d_dmac->write_signal(SIG_TOWNS_DMAC_ADDR_REG, data, 0xff);
		}
		// Power register
		break;
	case 0x0024:
//		if(d_dmac != NULL) {
//			d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP_REG, data, 0xff);
//		}
		break;
	case 0x0032:
		{
			d_serialrom->write_signal(SIG_SERIALROM_CS, ~data, 0x20);
			d_serialrom->write_signal(SIG_SERIALROM_CLK, data, 0x40);
			d_serialrom->write_signal(SIG_SERIALROM_RESET, data, 0x80);
		}
		break;
		break;
	case 0x0404: // System Status Reg.
		dma_is_vram = ((data & 0x80) == 0);
		config_page00();
		break;
	case 0x05c0:
		extra_nmi_mask = ((data & 0x08) == 0);
		break;
	case 0x05ec:
		if(machine_id >= /*0x0500*/0x0200) { // Towns2 CX : Is this hidden register after Towns 1F/2F/1H/2H? -> Yes
			vram_wait_val = ((data & 0x01) != 0) ? 3 : 6;
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 3;
			mem_wait_val >>= 1;
			vram_wait_val >>= 1;
		}
		set_wait_values();
		break;
	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			is_compatible = ((data & 0x01) == 0x00) ? true : false;
			if(d_crtc != NULL) {
				d_crtc->write_signal(SIG_TOWNS_CRTC_COMPATIBLE_MMIO, (is_compatible) ? 0xffffffff : 0x00000000, 0xffffffff);
			}
		}
		break;
	default:
		break;
	}
	return;
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	if((addr < 0xcc000) || (addr >= 0xd0000)) return 0xff;
	if(addr < 0xcff80) {
		val = ram_pagec[addr & 0xffff];
		return val;
	}
	switch(addr & 0x7f) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x04:
		val = 0x7f; // Reserve.FIRQ
		break;
	case 0x06:
		if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x08:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_crtc != NULL) {
				val = d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CF882H);
			}
		} else if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x14:
		val = 0x80;
		break;
//	case 0x15:
	case 0x16:
		if(d_font != NULL) {
			val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_DATA_LOW);
		}
		break;
	case 0x17:
		if(d_font != NULL) {
			val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_DATA_HIGH);
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			d_beep->write_signal(SIG_PCM1BIT_ON, 1, 1);
		}
		break;
	case 0x19:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			val = (ankcg_enabled) ? 0x01 : 0x00;
		} else if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x1c:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_HIGH);
			}
		} else if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x1d:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_LOW);
			}
		} else if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x1e:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_ROW);
			}
		} else if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	case 0x20:
		val = 0xff;
		val = val & 0x7f;
		break;
	default:
		if(d_vram != NULL) {
			val = d_vram->read_memory_mapped_io8(addr);
		}
		break;
	}
	return (uint32_t)val;
}

void TOWNS_MEMORY::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if((addr < 0xcc000) || (addr >= 0xd0000)) return;
	if(addr < 0xcff80) {
		ram_pagec[addr & 0xffff] = data;
		return;
	}
	switch(addr & 0x7f) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		if(d_vram != NULL) {
			d_vram->write_memory_mapped_io8(addr , data);
		}
		break;
	case 0x04:
		break;
	case 0x06:
		break;
	case 0x14:
		if(d_font != NULL) {
			d_font->write_signal(SIG_TOWNS_FONT_KANJI_HIGH, data, 0xff);
		}
		break;
	case 0x15:
		if(d_font != NULL) {
			d_font->write_signal(SIG_TOWNS_FONT_KANJI_LOW, data, 0xff);
		}
		break;
		/*
	case 0x16:
	case 0x17:
			// KANJI ROM/RAM? WRITE
		break;
		*/
	case 0x18:
		if(d_beep != NULL) {
			d_beep->write_signal(SIG_PCM1BIT_ON, 0, 1);
		}
		break;
	case 0x19:
		ankcg_enabled = ((data & 1) == 0) ? true : false;
		config_page00();
		break;
	case 0x1e:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				d_font->write_signal(SIG_TOWNS_FONT_KANJI_ROW, data, 0xff);
			}
		} else if(d_vram != NULL) {
			d_vram->write_memory_mapped_io8(addr , data);
		}
		break;
		break;
	case 0x20:
		break;
	default:
		if(d_vram != NULL) {
			d_vram->write_memory_mapped_io8(addr , data);
		}
		break;
	}
	return;
}
void TOWNS_MEMORY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_MEMORY_EXTNMI) {
		extra_nmi_val = ((data & mask) != 0);
		if(!(extra_nmi_mask)) {
			// Not MASK
			if(d_cpu != NULL) {
				d_cpu->write_signal(SIG_CPU_NMI, data, mask);
			}
		}			
	} else if(ch == SIG_CPU_NMI) {
		// Check protect
		if(!(nmi_mask)) {
			if(d_cpu != NULL) {
				d_cpu->write_signal(SIG_CPU_NMI, data, mask);
			}
		}
	} else if(ch == SIG_CPU_IRQ) {
		if(d_cpu != NULL) {
			d_cpu->write_signal(SIG_CPU_IRQ, data, mask);
		}
	} else if(ch == SIG_CPU_BUSREQ) {
		if(d_cpu != NULL) {
			d_cpu->write_signal(SIG_CPU_BUSREQ, data, mask);
		}
	} else if(ch == SIG_I386_A20) {
		if(d_cpu != NULL) {
			d_cpu->write_signal(SIG_I386_A20, data, mask);
		}
	} else if(ch == SIG_FMTOWNS_NOTIFY_RESET) {
		out_debug_log("RESET FROM CPU!!!\n");
		if(d_cpu != NULL) {
			d_cpu->set_address_mask(0xffffffff);
		}
		if(d_dmac != NULL) {
			d_dmac->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, 0xffffffff, 0xffffffff);
		}
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

void TOWNS_MEMORY::set_intr_line(bool line, bool pending, uint32_t bit)
{
	if(d_cpu != NULL) {
		d_cpu->set_intr_line(line, pending, bit);
	}
}

// ToDo: DMA

#define STATE_VERSION	2

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
	state_fio->StateValue(is_compatible);
	
	state_fio->StateValue(dma_is_vram);
	state_fio->StateValue(nmi_vector_protect);
	state_fio->StateValue(software_reset);
	state_fio->StateValue(poff_status);
	
	state_fio->StateValue(extra_nmi_val);
	state_fio->StateValue(extra_nmi_mask);
	state_fio->StateValue(nmi_mask);
	
	state_fio->StateArray(ram_page0,  sizeof(ram_page0), 1);
	state_fio->StateArray(ram_pagec,  sizeof(ram_pagec), 1);
	state_fio->StateArray(ram_paged,  sizeof(ram_paged), 1);
	state_fio->StateArray(ram_pagee,  sizeof(ram_pagee), 1);
	state_fio->StateArray(ram_pagef,  sizeof(ram_pagef), 1);
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
	state_fio->StateValue(ankcg_enabled);
	
	state_fio->StateValue(vram_wait_val);
	state_fio->StateValue(mem_wait_val);
	state_fio->StateValue(vram_size);

	// ToDo: Do save ROMs?
	return true;
}

}
