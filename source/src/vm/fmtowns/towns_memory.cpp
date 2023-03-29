/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.01 -

	[memory]
*/

#include "../../fileio.h"
#include "./towns_memory.h"
#include "./dmac.h"
#include "./vram.h"
#include "./planevram.h"
#include "./sprite.h"
#include "./fontroms.h"
#include "./serialrom.h"
#include "./crtc.h"
#include "./timer.h"

#include "../i386_np21.h"
//#include "../i386.h"

#include <math.h>

namespace FMTOWNS {

// Note: Bank width will be 0x2000 bytes.
void TOWNS_MEMORY::config_page_c0()
{
	if(dma_is_vram) {
		 // OK? From TSUGARU
		set_memory_mapped_io_rw(0x000c0000, 0x000c7fff, d_planevram);
		set_memory_mapped_io_rw(0x000c8000, 0x000c9fff, d_sprite);
		if(ankcg_enabled) {
			set_memory_mapped_io_r(0x000ca000, 0x000cbfff, d_font);
			unset_memory_w        (0x000ca000, 0x000cbfff); // OK?
		} else {
			set_memory_mapped_io_rw(0x000ca000, 0x000cbfff, d_sprite);
		}
		set_memory_rw          (0x000cc000, 0x000cdfff, &(ram_pagec[0xc000]));
		set_memory_mapped_io_rw(0x000ce000, 0x000cffff, this); // MMIO and higher RAM.
		// ToDo: Correctness wait value.
		set_wait_rw(0x000c0000, 0x000cffff, vram_wait_val);
	} else {
		#if 1
		set_memory_rw          (0x000c0000, 0x000cffff, ram_pagec);
		#else
		//set_memory_rw          (0x000c0000, 0x000cdfff, ram_pagec);
		//set_memory_mapped_io_rw(0x000ce000, 0x000cffff, this); // MMIO and higher RAM.
		#endif
		// ToDo: Correctness wait value.
		set_wait_rw(0x000c0000, 0x000cffff, mem_wait_val);
	}
}

void TOWNS_MEMORY::config_page_d0_e0()
{
	// Change as of
	// TownsPhysicalMemory::UpdateSysROMDicROMMappingFlag(bool , bool )
	// at src/towns/memory/pysmem.cpp, TSUGARU.
	// -- 20220125 K.O
	if(!(dma_is_vram)) {
		set_memory_rw          (0x000d0000, 0x000effff, ram_paged);
	} else {
		if(select_d0_dict) {
			set_memory_mapped_io_rw(0x000d0000, 0x000d9fff, d_dictionary);
			unset_memory_rw        (0x000da000, 0x000effff);
		} else {
			//set_memory_rw        (0x000d0000, 0x000dffff, ram_paged);
			unset_memory_rw        (0x000d0000, 0x000d9fff);
			unset_memory_rw        (0x000da000, 0x000effff);
		}
	}
}

void TOWNS_MEMORY::config_page_f8_rom()
{
	if(select_d0_rom) {
		set_memory_mapped_io_r (0x000f8000, 0x000fffff, d_sysrom);
		unset_memory_w         (0x000f8000, 0x000fffff);
	} else {
		set_memory_rw          (0x000f8000, 0x000fffff, &(ram_pagef[0x8000]));
	}
}

void TOWNS_MEMORY::config_page00()
{
	config_page_c0();
	config_page_d0_e0();
	config_page_f8_rom();
}

void TOWNS_MEMORY::initialize()
{
	if(initialized) return;
	MEMORY::initialize();
//	DEVICE::initialize();
	update_machine_features();

	extra_nmi_mask = true;
	extra_nmi_val = false;
	poff_status = false;
	reset_happened = false;

	vram_wait_val = 6;
	mem_wait_val = 3;
	if((cpu_id == 0x01) || (cpu_id == 0x03)) {
		wait_register = 0x03;
	} else {
		wait_register = 0x83;
	}
	cpu_clock_val = 16000 * 1000;
	extram_size = extram_size & 0x3ff00000;
	set_extra_ram_size(extram_size >> 20); // Check extra ram size.

	if(extram_size >= 0x00100000) {
		__UNLIKELY_IF(extra_ram == NULL) {
			extra_ram = (uint8_t*)malloc(extram_size);
			__LIKELY_IF(extra_ram != NULL) {
				set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
				memset(extra_ram, 0x00, extram_size);
			}
		}
	}
	memset(ram_page0, 0x00, sizeof(ram_page0));
	memset(ram_pagec, 0x00, sizeof(ram_pagec));
	memset(ram_paged, 0x00, sizeof(ram_paged));
	memset(ram_pagef, 0x00, sizeof(ram_pagef));

	select_d0_dict = false;
	select_d0_rom = true;

	dma_is_vram = true;
	// Lower 100000h
	set_memory_rw          (0x00000000, 0x000bffff, ram_page0);
	set_memory_rw          (0x000f0000, 0x000f7fff, ram_pagef);
	config_page00();

	set_memory_mapped_io_rw(0x80000000, 0x8007ffff, d_vram);
	set_memory_mapped_io_rw(0x80100000, 0x8017ffff, d_vram);
	set_memory_mapped_io_rw(0x81000000, 0x8101ffff, d_sprite);

	set_memory_mapped_io_rw(0xc0000000, 0xc0ffffff, d_iccard[0]);
	set_memory_mapped_io_rw(0xc1000000, 0xc1ffffff, d_iccard[1]);
//	set_wait_rw(0x00000000, 0xffffffff,  vram_wait_val);

	set_memory_mapped_io_r (0xc2000000, 0xc207ffff, d_msdos);
	set_memory_mapped_io_r (0xc2080000, 0xc20fffff, d_dictionary);
	set_memory_mapped_io_r (0xc2100000, 0xc213ffff, d_font);
	set_memory_mapped_io_rw(0xc2140000, 0xc2141fff, d_dictionary);
	if(d_font_20pix != NULL) {
		set_memory_mapped_io_r (0xc2180000, 0xc21fffff, d_font_20pix);
	}
	set_memory_mapped_io_rw(0xc2200000, 0xc2200fff, d_pcm);
	set_memory_mapped_io_r (0xfffc0000, 0xffffffff, d_sysrom);

	set_wait_values();
	// Another devices are blank

	// load rom image
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
	initialized = true;
}


void TOWNS_MEMORY::set_wait_values()
{
	uint32_t waitfactor = 0;
	if(cpu_clock_val < get_cpu_clocks(d_cpu)) {
		waitfactor = (uint32_t)(((double)get_cpu_clocks(d_cpu) / (double)cpu_clock_val) * 65536.0);
	}
	d_cpu->write_signal(SIG_CPU_WAIT_FACTOR, waitfactor, 0xffffffff);

	set_wait_rw(0x00000000, 0x000bffff, mem_wait_val);
	set_wait_rw(0x000d0000, 0x000fffff, mem_wait_val);
	if(dma_is_vram) {
		set_wait_rw(0x000c0000, 0x000cffff, vram_wait_val);
	} else {
		set_wait_rw(0x000c0000, 0x000cffff, mem_wait_val);
	}
	set_wait_rw(0x00100000, 0x00100000 + (extram_size & 0x3ff00000) - 1, mem_wait_val);

	// ToDo: Extend I/O Slots
	set_wait_rw(0x80000000, 0x800fffff, vram_wait_val);
	set_wait_rw(0x80100000, 0x801fffff, vram_wait_val);
	set_wait_rw(0x81000000, 0x8101ffff, vram_wait_val);
	// ToDo: ROM CARDS
	if(d_iccard[0] != NULL) {
		set_wait_rw(0xc0000000, 0xc0ffffff, mem_wait_val); // OK?
	}
	if(d_iccard[0] != NULL) {
		set_wait_rw(0xc1000000, 0xc1ffffff, mem_wait_val); // OK?
	}
	set_wait_rw(0xc2000000, 0xc2141fff, mem_wait_val);
	set_wait_rw(0xc2200000, 0xc2200fff, mem_wait_val);
	set_wait_rw(0xfffc0000, 0xffffffff, mem_wait_val);
}

void TOWNS_MEMORY::release()
{
//	if(rd_table != NULL) free(rd_table);
//	if(rd_dummy != NULL) free(rd_dummy);
//	if(wr_table != NULL) free(wr_table);
//	if(wr_dummy != NULL) free(wr_dummy);

	if(extra_ram != NULL) {
		free(extra_ram);
		extra_ram = NULL;
	}
	MEMORY::release();

}
void TOWNS_MEMORY::reset()
{
	// reset memory
	// ToDo
	MEMORY::reset();
	is_compatible = true;
	reset_happened = false;
	dma_is_vram = true;
	nmi_vector_protect = false;
	ankcg_enabled = false;
	nmi_mask = false;
	select_d0_dict = false;
	select_d0_rom = true;
	config_page00();

	set_wait_values();
#if 1
	__LIKELY_IF(d_cpu != NULL) {
		d_cpu->set_address_mask(0xffffffff);
	}
	if(d_dmac != NULL) {
		d_dmac->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, 0xffffffff, 0xffffffff);
		uint8_t wrap_val = 0xff; // WRAP OFF
		if(machine_id >= 0x0b00) { // After MA/MX/ME
			wrap_val = 0x00;
		}
		d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP_REG, wrap_val, 0xff);
	}
#endif
}

void TOWNS_MEMORY::update_machine_features()
{
	// 0024h: MISC3
	reg_misc3 = 0xff;
	if(machine_id >= 0x0b00) { // After MA/MX/ME
		reg_misc3 &= ~0x04; // DMACMD
	}
	if(machine_id >= 0x0700) { // After HR/HG
		reg_misc3 &= ~0x08; // POFFEN
	}
	if(machine_id >= 0x0700) { // After HR/HG
		reg_misc3 &= ~0x10; // Free run counter
	}
	if(machine_id >= 0x0700) { // After HR/HG
		reg_misc3 &= ~0x20; // CRTPOWOFF (0022h)
	}
	if(machine_id >= 0x0700) { // After HR/HG
		reg_misc3 &= ~0x40; // RCREN
	}
	if(machine_id >= 0x0700) { // After HR/HG
		reg_misc3 &= ~0x80; // ENPOFF
	}
	// 0025h: NMICNT
	if(machine_id >= 0x0500) { // After CX
		reg_misc4 = 0x7f;
	} else {
		reg_misc4 = 0xff;
	}
}

uint8_t TOWNS_MEMORY::read_fmr_ports8(uint32_t addr)
{
	uint8_t val = 0xff;
	__UNLIKELY_IF((addr & 0xffff) < 0xff80) {
		return 0xff;
	}
	__LIKELY_IF((addr & 0xffff) < 0xff88) {
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		return val;
	} else if(((addr & 0xffff) >= 0xff94) && ((addr & 0xffff) < 0xff98)) {
		__LIKELY_IF(d_font != NULL) {
			val = d_font->read_io8(addr & 0xffff);
		}
		return val;
	}
	switch(addr & 0xffff) {
	case 0xff88:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			__LIKELY_IF(d_crtc != NULL) {
				val = d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CFF82H);
			}
		} else  __LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	case 0xff98:
		__LIKELY_IF(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 1, 1);
		}
		break;
	case 0xff99:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			val = (ankcg_enabled) ? 0x01 : 0x00;
		} else __LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	case 0xff9c:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			__LIKELY_IF(d_font != NULL) {
				val = d_font->read_io8(addr & 0xffff);
			}
		} else __LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	case 0xff9d:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			__LIKELY_IF(d_font != NULL) {
				val = d_font->read_io8(addr & 0xffff);
			}
		} else __LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	case 0xff9e:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			__LIKELY_IF(d_font != NULL) {
				val = d_font->read_io8(addr & 0xffff);
			}
		} else __LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	default:
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	}
	return val;
}
uint16_t TOWNS_MEMORY::read_sys_ports16(uint32_t addr)
{
	pair16_t val;
	val.w = 0xffff;
	switch(addr & 0x0ffe) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		val.b.l = ((software_reset) ? 1 : 0) | ((reset_happened) ? 2 : 0);
		reset_happened = false;
		software_reset = false;
		__UNLIKELY_IF(d_cpu != NULL) {
			d_cpu->set_shutdown_flag(0);
		}
		if((machine_id >= 0x0300) && ((machine_id & 0xff00) != 0x0400)) { // After UX
			val.b.l = val.b.l | ((poff_status) ? 0x04 : 0x00);
		}
		break;
	case 0x0022:
//		val.b.l = 0xff;
//		if(d_dmac != NULL) {
//			val = d_dmac->read_signal(SIG_TOWNS_DMAC_ADDR_REG);
//		}
		break;
	case 0x0024:
		// 0024, 0025 : MISC3 + MISC4
		val.w = (((uint32_t)reg_misc4) << 8) | ((uint32_t)reg_misc3);
		break;
	case 0x0028:
		// NMI MASK
		if(machine_id >= 0x0500) { // After CX
			val.b.l = (nmi_mask) ? 0x01 : 0x00;
		}
		break;
	case 0x0030:
		// 0030, 0031 : CPUID + MACHINE ID
		val.w = (cpu_id & 7) | (machine_id & 0xfff8);
		break;
	case 0x0032:
		{
			//bool __cs = (d_serialrom->read_signal(SIG_SERIALROM_CS) == 0);
			bool __clk = (d_serialrom->read_signal(SIG_SERIALROM_CLK) != 0);
			bool __reset = (d_serialrom->read_signal(SIG_SERIALROM_RESET) != 0);
			bool __dat = (d_serialrom->read_signal(SIG_SERIALROM_DATA) != 0);
			val.b.l = ((__reset) ? 0x80 : 0x00) | ((__clk) ? 0x40 : 0x00) | /*0x3e |*/ ((__dat) ? 0x01 : 0x00);
		}
		break;
	case 0x00c0: // Cache
		val.b.l = 0x00;
		if((cpu_id == 0x02) || (cpu_id >= 0x04)) { // i486 SX/DX or After Pentium.
			// ToDo: Implement around cache.
			// Modified by this register and (05ECh:bit0 / Wait register).
			// Now, cache is always disabled.
			// RPNH = 0 (Disabled) : Bit1
			// CMEN = 0 (Disabled) : Bit0
			val.b.l = 0x00;
		}
		break;
	case 0x00c2: // Cache Diagnostic
		val.b.l = 0x00;
		if((cpu_id == 0x02) || (cpu_id >= 0x04)) { // i486 SX/DX or After Pentium.
			// ToDo: Implement cache disgnostic.
			// SDMOD (Not diagnostic) : Bit3
			val.b.l = 0x00;
		}
		break;
	case 0x0400: // Resolution:
		val.b.l = 0xfe;
//		val = 0x00;
		break;
	case 0x0404: // System Status Reg.
//		val = (dma_is_vram) ? 0x7f : 0xff;
		val.b.l = (dma_is_vram) ? 0x00 : 0x80;
		break;
	case 0x0480:
		val.b.l  =  (select_d0_dict) ? 0x01 : 0x00;
		val.b.l |= ((select_d0_rom) ? 0x00 : 0x02);
		break;
	case 0x05c0:
//		val = (extra_nmi_mask) ? 0xf7 : 0xff;
		val.b.l = (extra_nmi_mask) ? 0x00 : 0x08;
		break;
	case 0x05c2:
//		val = (extra_nmi_val) ? 0xff : 0xf7;
		val.b.l = (extra_nmi_val) ? 0x08 : 0x00;
		break;
	case 0x05e0:
		if(machine_id < 0x0200) { // Towns 1/2
			val.b.l =  wait_register;
		}
		break;
	case 0x05e2:
		if(machine_id >= 0x0200) { // i386
			val.b.l = wait_register;
		}
		break;
	case 0x05e8:
		// After Towns1F/2F/1H/2H
		{
			uint16_t nid = machine_id & 0xff00;
			if(nid >= 0x1000) {
				val.b.l = (extram_size >> 20) & 0x7f; // MAX 128MB
			} else if(nid >= 0x0900) { // UR,MA,MX,ME,MF
				val.b.l = (extram_size >> 20) & 0x1f; // MAX 32MB
			} else if(nid == 0x0800) { // HG
				val.b.l = (extram_size >> 20) & 0x0f; // MAX 15MB
			} else if(nid == 0x0700) { // HR
				val.b.l = (extram_size >> 20) & 0x1f; // MAX 32MB
			} else if(nid >= 0x0200) { // 2nd GEN,3rd Gen, UX/UG, CX
				val.b.l = (extram_size >> 20) & 0x0f; // MAX 15MB
			} else {
				val.b.l = 0xff; // NOT SUPPORTED
			}
		}
		break;
	case 0x05ec:
		// 05ec, 05ed
		if(machine_id >= 0x0200) { // 05ec
			val.b.l = ((mem_wait_val < 1) ? 0x01 : 0x00);
		}
		if(machine_id >= 0x0700) { // 05ed
			uint32_t clk = get_cpu_clocks(d_cpu);
			clk = clk / (1000 * 1000);
			__UNLIKELY_IF(clk < 16) clk = 16;
			__UNLIKELY_IF(clk > 127) clk = 127; // ToDo
			val.b.h = 0x00 | clk;
		}
		break;
	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			val.b.l = (is_compatible) ? 0x00 : 0x01;
		} else {
			val.b.l = 0x00;
		}
		break;
	default:
		break;
	}
	return val.w;
}
// Address (TOWNS BASIC):
// 0x0020 - 0x0022, 0x0030-0x0031,
// 0x0400 - 0x0404,
// 0x0480 - 0x0484
// 0x05c0 - 0x05c2
// 0x05ec (Wait register)
// 0x05ed (CPU SPEED REGISTER)
// Is set extra NMI (0x05c0 - 0x05c2)?
uint32_t TOWNS_MEMORY::read_io8(uint32_t addr)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		return read_fmr_ports8(addr & 0xffff);
	}
	pair16_t val;
	val.w = read_sys_ports16(addr);
	return ((addr & 1) != 0) ? val.b.h : val.b.l;
}

uint32_t TOWNS_MEMORY::read_io8w(uint32_t addr, int *wait)
{
//	uint32_t val = 0x00;  // MAY NOT FILL to "1" for unused bit 20200129 K.O
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		*wait = 6; // ToDo: will io_wait_val.
		return read_fmr_ports8(addr & 0xffff);
	}
	*wait = 6; // ToDo: will io_wait_val.
	pair16_t val;
	val.w = read_sys_ports16(addr);
	return ((addr & 1) != 0) ? val.b.h : val.b.l;
}

uint32_t TOWNS_MEMORY::read_io16(uint32_t addr)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		// OK? This may be bus width has 8bit ?
		pair16_t val;
		val.b.l = read_fmr_ports8(addr & 0xfffe);
		val.b.h = read_fmr_ports8((addr & 0xfffe) + 1);
		return val.w;
	}
	return read_sys_ports16(addr);
}

uint32_t TOWNS_MEMORY::read_io16w(uint32_t addr, int *wait)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		// OK? This may be bus width has 8bit ?
		*wait = 6 * 2; // ToDo: will io_wait_val.
		pair16_t val;
		val.b.l = read_fmr_ports8(addr & 0xfffe);
		val.b.h = read_fmr_ports8((addr & 0xfffe) + 1);
		return val.w;
	}
	*wait = 6; // ToDo: will io_wait_val.
	return read_sys_ports16(addr);
}

void TOWNS_MEMORY::write_fmr_ports8(uint32_t addr, uint32_t data)
{
	__UNLIKELY_IF((addr & 0xffff) < 0xff80) {
		return;
	}
	__LIKELY_IF((addr & 0xffff) < 0xff88) {
		__LIKELY_IF(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
		return;
	} else if(((addr & 0xffff) >= 0xff94) && ((addr & 0xffff) < 0xff98)) {
		__LIKELY_IF(d_font != NULL) {
			d_font->write_io8(addr & 0xffff, data);
		}
		return;
	}
	switch(addr & 0xffff) {
	case 0xff98:
		__LIKELY_IF(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 0, 1);
		}
		break;
	case 0xff99:
		{
			bool _b = ankcg_enabled;
			ankcg_enabled = ((data & 1) != 0) ? true : false;
			//if((_b != ankcg_enabled) && (dma_is_vram)) {
				config_page_c0();
			//}
		}
		break;
	case 0xff9e:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			__LIKELY_IF(d_font != NULL) {
				d_font->write_io8(addr & 0xffff, data);
			}
		} else __LIKELY_IF(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
		break;
	default:
		__LIKELY_IF(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
		break;
	}
}
void TOWNS_MEMORY::write_sys_ports8(uint32_t addr, uint32_t data)
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
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(1);
			}
			// Todo: Implement true power off.
			emu->notify_power_off();
//			emu->power_off();
		} else {
			poff_status = false;
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(0);
			}
		}

		if(software_reset) {
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->reset();
			}
			uint8_t wrap_val = 0xff; // WRAP OFF
			if(machine_id >= 0x0b00) { // After MA/MX/ME
				wrap_val = 0x00;
			}
			__LIKELY_IF(d_dmac != NULL) {
				d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP_REG, wrap_val, 0xff);
			}
		}
		// Towns SEEMS to not set addreess mask (a.k.a A20 mask). 20200131 K.O
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(1);
			}
			// Todo: Implement true power off.
//			emu->power_off();
		}
		// Power register
		break;
	case 0x0024:
//		if(d_dmac != NULL) {
//			d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP_REG, data, 0xff);
//		}
		break;
	case 0x0032:
		d_serialrom->write_signal(SIG_SERIALROM_CS, ~data, 0x20);
		d_serialrom->write_signal(SIG_SERIALROM_CLK, data, 0x40);
		d_serialrom->write_signal(SIG_SERIALROM_RESET, data, 0x80);
		break;
	case 0x0404: // System Status Reg.
		{
			//bool _b = dma_is_vram;
			dma_is_vram = ((data & 0x80) == 0);
			//if((_b != dma_is_vram)/* || (dma_is_vram)*/) {
				config_page_c0();
				config_page_d0_e0();
//				config_page00();
			//}
		}
		break;
	case 0x0480:
		{
			bool _dict = select_d0_dict;
			bool _rom = select_d0_rom;
			select_d0_dict = ((data & 0x01) != 0) ? true : false;
			select_d0_rom = ((data & 0x02) == 0) ? true : false;
			if(_dict != select_d0_dict) {
				config_page_d0_e0();
			}
			if(_rom != select_d0_rom) {
				config_page_f8_rom();
			}
//			if((_rom != select_d0_rom) ||(_dict != select_d0_dict)) {
//				config_page00();
//			}
//			config_page00();
		}
		break;
	case 0x05c0:
		extra_nmi_mask = ((data & 0x08) == 0);
		break;
	case 0x05e0:
		// From AB.COM
		if(machine_id < 0x0200) { // Towns 1/2
			uint8_t nval = data & 7;
			uint8_t val_bak = mem_wait_val;
			if(nval < 1) nval = 1;
			if(nval > 5) nval = 5;
			mem_wait_val = nval + 1;
			vram_wait_val = nval + 3 + 1;
			wait_register = nval;
			if(val_bak != mem_wait_val) {
				set_wait_values();
			}
		}
		break;
	case 0x05e2:
		if(machine_id >= 0x0200) { // After Towns 1H/2F. Hidden wait register.
			uint8_t val_bak = mem_wait_val;
			if(data != 0x83) {
				uint8_t nval = data & 7;
				if(machine_id <= 0x0200) { // Towns 1H/2F.
					if(nval < 1) nval = 1;
				}
				if(nval > 5) nval = 5;
				mem_wait_val = nval;
				vram_wait_val = nval + 3;
				wait_register = nval;
			} else {
				mem_wait_val = 3;
				vram_wait_val = 6;
				wait_register = data;
			}
			if(val_bak != mem_wait_val) {
				set_wait_values();
			}
		}
		break;
	case 0x05ec:
		// ToDo: 0x05ed
		if(machine_id >= 0x0500) { // Towns2 CX :
			uint8_t val_bak = mem_wait_val;
			uint32_t clk_bak = cpu_clock_val;
			vram_wait_val = ((data & 0x01) != 0) ? 3 : 6;
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 3;
			cpu_clock_val = ((data & 0x01) != 0) ? (get_cpu_clocks(d_cpu)) : (16 * 1000 * 1000);
			if((val_bak != mem_wait_val) || (cpu_clock_val != clk_bak)) {
				set_wait_values();
			}
		}
		break;

	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			is_compatible = ((data & 0x01) == 0x00) ? true : false;
			__LIKELY_IF(d_crtc != NULL) {
				d_crtc->write_signal(SIG_TOWNS_CRTC_COMPATIBLE_MMIO, (is_compatible) ? 0xffffffff : 0x00000000, 0xffffffff);
			}
		}
		break;
	default:
		break;
	}
}
void TOWNS_MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		write_fmr_ports8(addr & 0xffff, data);
		return;
	}
	write_io16(addr, data & 0xff);
}

void TOWNS_MEMORY::write_io8w(uint32_t addr, uint32_t data, int *wait)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		*wait = 6; // ToDo: will io_wait_val.
		write_fmr_ports8(addr & 0xffff, data);
		return;
	}
	write_io16w(addr, data & 0xff, wait);
	return;
}

void TOWNS_MEMORY::write_sys_ports16(uint32_t addr, uint32_t data)
{
	// ToDo: WORD ACCESS.
	write_sys_ports8(addr & 0xfffe, data & 0xff);
}

void TOWNS_MEMORY::write_io16(uint32_t addr, uint32_t data)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		// OK? This may be bus width has 8bit ?
		pair16_t val;
		val.w = data;
		write_fmr_ports8(addr & 0xffff, val.b.l);
		write_fmr_ports8((addr & 0xffff) + 1, val.b.h);
		return;
	}
	write_sys_ports16(addr, data);
}

void TOWNS_MEMORY::write_io16w(uint32_t addr, uint32_t data, int *wait)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		// OK? This may be bus width has 8bit ?
		pair16_t val;
		*wait = 6 * 2; // Temporally value.
		val.w = data;
		write_fmr_ports8(addr & 0xffff, val.b.l);
		write_fmr_ports8((addr & 0xffff) + 1, val.b.h);
		return;
	} else {
		*wait = 6; // Temporally value.
		write_sys_ports16(addr, data);
	}
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io8(uint32_t addr)
{
	int wait = 0;
	return read_memory_mapped_io8w(addr, &wait);
}
uint32_t TOWNS_MEMORY::read_memory_mapped_io8w(uint32_t addr, int *wait)
{
	__LIKELY_IF(addr < 0xcff80) {
		*wait = mem_wait_val;
		return ram_pagec[addr & 0xffff];
	}
	__UNLIKELY_IF(addr >= 0xd0000) {
		*wait = mem_wait_val;
		return 0xff;
	}
	*wait = 6; // Maybe 6.
	return read_fmr_ports8(addr);
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io16(uint32_t addr)
{
	int wait = 0;
	return read_memory_mapped_io16w(addr, &wait);
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io16w(uint32_t addr, int *wait)
{
	__LIKELY_IF(addr < 0xcff80) {
		pair16_t v;
		__LIKELY_IF((addr & 1) == 0) {
			*wait = mem_wait_val;
			v.read_2bytes_le_from(&(ram_pagec[addr & 0xfffe]));
		} else {
			*wait = mem_wait_val * 2;
			v.b.l = ram_pagec[addr & 0xffff];
			v.b.h = ram_pagec[(addr + 1) & 0xffff];
		}
		return v.w;
	}
	__UNLIKELY_IF(addr >= 0xd0000) {
		*wait = mem_wait_val;
		return 0xff;
	}
	// OK? This may be bus width has 8bit ?
	*wait = 6; // Maybe 6.
	pair16_t val;
	val.b.l = read_fmr_ports8(addr & 0xffff);
	val.b.h = read_fmr_ports8((addr & 0xffff) + 1);
	return val.w;
}

void TOWNS_MEMORY::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_memory_mapped_io8w(addr, data, &wait);
}

void TOWNS_MEMORY::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int *wait)
{
	__LIKELY_IF(addr < 0xcff80) {
		*wait = mem_wait_val;
		ram_pagec[addr & 0xffff] = data;
		return;
	}
	__LIKELY_IF(addr < 0xd0000) {
		*wait = 6; // Maybe 6.
		write_fmr_ports8(addr, data);
	}
	*wait = mem_wait_val;
	return;
}

void TOWNS_MEMORY::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_memory_mapped_io16w(addr, data, &wait);
}

void TOWNS_MEMORY::write_memory_mapped_io16w(uint32_t addr, uint32_t data, int *wait)
{
	__LIKELY_IF(addr < 0xcff80) {
		pair16_t v;
		v.w = data;
		__LIKELY_IF((addr & 1) == 0) {
			*wait = mem_wait_val;
			v.write_2bytes_le_to(&(ram_pagec[addr & 0xffff]));
		} else {
			*wait = mem_wait_val * 2;
			ram_pagec[(addr & 0xffff) + 0] = v.b.l;
			ram_pagec[(addr & 0xffff) + 1] = v.b.h;
		}
		return;
	}
	__LIKELY_IF(addr < 0xd0000) {
		*wait = 6; // Maybe 6.
		write_fmr_ports8(addr, data);
	}
	*wait = mem_wait_val;
	return;
}


void TOWNS_MEMORY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_MEMORY_EXTNMI) {
		extra_nmi_val = ((data & mask) != 0);
		if(!(extra_nmi_mask)) {
			// Not MASK
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->write_signal(SIG_CPU_NMI, data, mask);
			}
		}
	} else if(ch == SIG_CPU_NMI) {
		// Check protect
		if(!(nmi_mask)) {
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->write_signal(SIG_CPU_NMI, data, mask);
			}
		}
	} else if(ch == SIG_CPU_IRQ) {
		__LIKELY_IF(d_cpu != NULL) {
			d_cpu->write_signal(SIG_CPU_IRQ, data, mask);
		}
	} else if(ch == SIG_CPU_BUSREQ) {
		__LIKELY_IF(d_cpu != NULL) {
			d_cpu->write_signal(SIG_CPU_BUSREQ, data, mask);
		}
	} else if(ch == SIG_I386_A20) {
		__LIKELY_IF(d_cpu != NULL) {
			d_cpu->write_signal(SIG_I386_A20, data, mask);
		}
	} else if(ch == SIG_FMTOWNS_NOTIFY_RESET) {
		out_debug_log("RESET FROM CPU!!!\n");
		reset_happened = true;
		dma_is_vram = true;
		nmi_vector_protect = false;
		ankcg_enabled = false;
		nmi_mask = false;
		select_d0_dict = false;
		select_d0_rom = true;
		config_page00();
		set_wait_values();

		__LIKELY_IF(d_cpu != NULL) {
			d_cpu->set_address_mask(0xffffffff);
		}
		__LIKELY_IF(d_dmac != NULL) {
			d_dmac->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, 0xffffffff, 0xffffffff);
			uint8_t wrap_val = 0xff; // WRAP OFF
			if(machine_id >= 0x0b00) { // After MA/MX/ME
				wrap_val = 0x00;
			}
			d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP_REG, wrap_val, 0xff);
		}
	} else if(ch == SIG_FMTOWNS_RAM_WAIT) {
		uint8_t _bak = mem_wait_val;
		mem_wait_val = (int)data;
		if(_bak != mem_wait_val) {
			set_wait_values();
		}
	} else if(ch == SIG_FMTOWNS_ROM_WAIT) {
//		mem_wait_val = (int)data;
		set_wait_values();
	} else if(ch == SIG_FMTOWNS_VRAM_WAIT) {
		uint8_t _bak = vram_wait_val;
		vram_wait_val = (int)data;
		if(_bak != vram_wait_val) {
			set_wait_values();
		}
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
	__LIKELY_IF(d_cpu != NULL) {
		d_cpu->set_intr_line(line, pending, bit);
	}
}

// ToDo: DMA

#define STATE_VERSION	5

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

	state_fio->StateValue(mem_wait_val);
	state_fio->StateValue(vram_wait_val);
	state_fio->StateValue(wait_register);

	state_fio->StateValue(dma_is_vram);
	state_fio->StateValue(nmi_vector_protect);
	state_fio->StateValue(software_reset);
	state_fio->StateValue(poff_status);
	state_fio->StateValue(reset_happened);

	state_fio->StateValue(extra_nmi_val);
	state_fio->StateValue(extra_nmi_mask);
	state_fio->StateValue(nmi_mask);

	state_fio->StateArray(ram_page0,  sizeof(ram_page0), 1);
	state_fio->StateArray(ram_pagec,  sizeof(ram_pagec), 1);
	state_fio->StateArray(ram_paged,  sizeof(ram_paged), 1);
	state_fio->StateArray(ram_pagef,  sizeof(ram_pagef), 1);

	state_fio->StateValue(select_d0_rom);
	state_fio->StateValue(select_d0_dict);
	state_fio->StateValue(ankcg_enabled);

	state_fio->StateValue(vram_wait_val);
	state_fio->StateValue(mem_wait_val);
	state_fio->StateValue(vram_size);
	state_fio->StateValue(cpu_clock_val);

	if(loading) {
		update_machine_features(); // Update MISC3, MISC4 by MACHINE ID.

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
		config_page00();
	} else {
		// At saving
		if(extra_ram == NULL) {
			state_fio->FputUint32_LE(0);
		} else {
			state_fio->FputUint32_LE(extram_size & 0x3ff00000);
			state_fio->Fwrite(extra_ram, extram_size, 1);
		}
	}

	// ToDo: Do save ROMs?
	return true;
}

}
