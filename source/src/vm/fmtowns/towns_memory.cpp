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

#define ADDR_MASK (addr_max - 1)
#define BANK_MASK (bank_size - 1)
	
void TOWNS_MEMORY::config_page00()
{
	if(dma_is_vram) {
		 // OK? From TSUGARU
		set_memory_mapped_io_rw(0x000c0000, 0x000c7fff, d_planevram);
		set_memory_mapped_io_rw(0x000c8000, 0x000cbfff, d_sprite);
		set_memory_mapped_io_rw(0x000ca000, 0x000cafff, d_sprite);
		if(ankcg_enabled) {
			set_memory_mapped_io_r(0x000ca000, 0x000ca7ff, d_font);
			set_memory_r          (0x000ca800, 0x000cafff, rd_dummy);
			set_memory_mapped_io_r(0x000cb000, 0x000cbfff, d_font);
		}
		set_memory_rw          (0x000cc000, 0x000cffff, &(ram_pagec[0xc000]));
		set_memory_mapped_io_rw(0x000cfc00, 0x000cffff, this); // MMIO
	} else {
		set_memory_rw          (0x000c0000, 0x000cffff, ram_pagec);
	}
	if((select_d0_rom) && (select_d0_dict)) { 
		set_memory_mapped_io_rw(0x000d0000, 0x000dffff, d_dictionary);
	} else {
		set_memory_rw          (0x000d0000, 0x000dffff, ram_paged);
	}
	if(select_d0_rom) {
		set_memory_mapped_io_r (0x000f8000, 0x000fffff, d_sysrom);
//		set_memory_w	       (0x000f8000, 0x000fffff, &(ram_pagef[0x8000]));
//		set_memory_mapped_io_rw(0x000f8000, 0x000fffff, d_sysrom);
		set_memory_w	       (0x000f8000, 0x000fffff, wr_dummy);
	} else {
		set_memory_rw          (0x000f8000, 0x000fffff, &(ram_pagef[0x8000]));
	}		
	if(dma_is_vram) {
		set_wait_rw(0x000c0000, 0x000cffff, vram_wait_val);
	} else {
		set_wait_rw(0x000c0000, 0x000cffff, mem_wait_val);
	}

}
	
void TOWNS_MEMORY::initialize()
{
//	if(initialized) return;
	MEMORY::initialize();
//	DEVICE::initialize();
	
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
	
	select_d0_dict = false;
	select_d0_rom = true;
	
	dma_is_vram = true;
	// Lower 100000h
	set_memory_rw          (0x00000000, 0x000bffff, ram_page0);
	set_memory_rw          (0x000d0000, 0x000dffff, ram_paged);
	set_memory_rw          (0x000e0000, 0x000effff, ram_pagee);
	set_memory_rw          (0x000f0000, 0x000f7fff, ram_pagef);
	set_memory_mapped_io_rw(0x000f8000, 0x000fffff, d_sysrom);
	config_page00();
	
	set_memory_mapped_io_rw(0x80000000, 0x8007ffff, d_vram);
	set_memory_mapped_io_rw(0x80100000, 0x8017ffff, d_vram);
	set_memory_mapped_io_rw(0x81000000, 0x8101ffff, d_sprite);
	
//	set_memory_mapped_io_rw(0xc0000000, 0xc0ffffff, d_iccard[0]);
//	set_memory_mapped_io_rw(0xc1000000, 0xc1ffffff, d_iccard[1]);
	set_wait_rw(0x00000000, 0xffffffff,  vram_wait_val);

	set_memory_mapped_io_rw(0xc0000000, 0xc0ffffff, d_iccard[0]);
	set_memory_mapped_io_rw(0xc1000000, 0xc1ffffff, d_iccard[1]);
	
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
}

	
void TOWNS_MEMORY::set_wait_values()
{
	uint32_t waitfactor = 1 << 16;
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
	if(d_cpu != NULL) {
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
//	uint32_t val = 0x00;  // MAY NOT FILL to "1" for unused bit 20200129 K.O
	uint32_t val = 0xff;  //
	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		val = ((software_reset) ? 1 : 0) | ((reset_happened) ? 2 : 0);
		reset_happened = false;
		software_reset = false;
		if(d_cpu != NULL) {
			d_cpu->set_shutdown_flag(0);
		}
		if((machine_id >= 0x0300) && ((machine_id & 0xff00) != 0x0400)) { // After UX
			val = val | ((poff_status) ? 0x04 : 0x00);
		}
		break;
	case 0x0022:
//		val = 0xff;
//		if(d_dmac != NULL) {
//			val = d_dmac->read_signal(SIG_TOWNS_DMAC_ADDR_REG);
//		}
		break;
	case 0x0024:
		// CPU MISC3
		return reg_misc3;
		break;
	case 0x0025:
		// CPU MISC4
		return reg_misc4;
		break;
	case 0x0028:
		// NMI MASK
		if(machine_id >= 0x0500) { // After CX
			val = (nmi_mask) ? 0x01 : 0x00;
		}
		break;
	case 0x0030:
		// 20210227 K.O
		// From FMTowns::MachineID()  of TSUGARU,
		// git 83d4ec2309ac9fcbb8c01f26061ff0d49c5321e4.
//		if((config.dipswitch & TOWNS_DIPSW_PRETEND_I386) != 0) {
//			val = ((machine_id & 0xf8) | 0x01);
//		} else {
			val = ((machine_id & 0xf8) | (cpu_id & 7));
//		}
		break;
	case 0x0031:
		val = ((machine_id >> 8) & 0xff);
		break;
	case 0x0032:
		{
			//bool __cs = (d_serialrom->read_signal(SIG_SERIALROM_CS) == 0);
			bool __clk = (d_serialrom->read_signal(SIG_SERIALROM_CLK) != 0);
			bool __reset = (d_serialrom->read_signal(SIG_SERIALROM_RESET) != 0);
			bool __dat = (d_serialrom->read_signal(SIG_SERIALROM_DATA) != 0);
			val = ((__reset) ? 0x80 : 0x00) | ((__clk) ? 0x40 : 0x00) | /*0x3e |*/ ((__dat) ? 0x01 : 0x00);
		}
		break;
	case 0x00c0: // Cache
		val = 0x00;
		if((cpu_id == 0x02) || (cpu_id >= 0x04)) { // i486 SX/DX or After Pentium.
			// ToDo: Implement around cache.
			// Modified by this register and (05ECh:bit0 / Wait register).
			// Now, cache is always disabled.
			// RPNH = 0 (Disabled) : Bit1
			// CMEN = 0 (Disabled) : Bit0
			val = 0x00;
		}
		break;
	case 0x00c2: // Cache Diagnostic
		val = 0x00;
		if((cpu_id == 0x02) || (cpu_id >= 0x04)) { // i486 SX/DX or After Pentium.
			// ToDo: Implement cache disgnostic.
			// SDMOD (Not diagnostic) : Bit3
			val = 0x00;
		}
		break;
	case 0x0400: // Resolution:
		val = 0xfe;
//		val = 0x00;
		break;
	case 0x0404: // System Status Reg.
//		val = (dma_is_vram) ? 0x7f : 0xff;
		val = (dma_is_vram) ? 0x00 : 0x80;
		break;
	case 0x0480:
		val  =  (select_d0_dict) ? 0x01 : 0x00;
		val |= ((select_d0_rom) ? 0x00 : 0x02);
		break;
	case 0x05c0:
//		val = (extra_nmi_mask) ? 0xf7 : 0xff;
		val = (extra_nmi_mask) ? 0x00 : 0x08;
		break;
	case 0x05c2:
//		val = (extra_nmi_val) ? 0xff : 0xf7;
		val = (extra_nmi_val) ? 0x08 : 0x00;
		break;
	case 0x05e0:
		if(machine_id < 0x0200) { // Towns 1/2
			return wait_register;
		}
		break;
	case 0x05e2:
		if(machine_id >= 0x0200) { // i386
			return wait_register;
		}
		break;
	case 0x05e8:
		// After Towns1F/2F/1H/2H
		{
			uint16_t nid = machine_id & 0xff00;
			if(nid >= 0x1000) {
				val = (extram_size >> 20) & 0x7f; // MAX 128MB
			} else if(nid >= 0x0900) { // UR,MA,MX,ME,MF
				val = (extram_size >> 20) & 0x1f; // MAX 32MB
			} else if(nid == 0x0800) { // HG
				val = (extram_size >> 20) & 0x0f; // MAX 15MB
			} else if(nid == 0x0700) { // HR
				val = (extram_size >> 20) & 0x1f; // MAX 32MB
			} else if(nid >= 0x0200) { // 2nd GEN,3rd Gen, UX/UG, CX
				val = (extram_size >> 20) & 0x0f; // MAX 15MB
			} else {
				val = 0xff; // NOT SUPPORTED
			}
		}
		break;
	   
	case 0x05ec:
		if(machine_id >= 0x0200) { // Towns2H/2F : Is this hidden register after Towns 1F/2F/1H/2H? -> Yes
			val = 0x00;
			if(mem_wait_val < 1) val |= 0x01;
		} else {
			val = 0xff;
		}
		break;
	case 0x05ed:
		if(machine_id >= 0x0700) { // After HR/HG
			uint32_t clk = get_cpu_clocks(d_cpu);
			clk = clk / (1000 * 1000);
			if(clk < 16) clk = 16;
			if(clk > 127) clk = 127; // ToDo
			val = 0x00 | clk;
		} else {
			val = 0xff;
		}
		break;
	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			return (is_compatible) ? 0x00 : 0x01;
		} else {
			return 0x00;
		}
		break;
	case 0xff88:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_crtc != NULL) {
				val = d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CFF82H);
			}
		} else  if(d_planevram != NULL) {
			val = d_planevram->read_io8(addr);
		}
		break;
	case 0xff94:
		return 0x80;
		break;
	case 0xff95:
		break;
	case 0xff96:
		if(d_font != NULL) {
			return d_font->read_signal(SIG_TOWNS_FONT_KANJI_DATA_LOW);
		}
		break;
	case 0xff97:
		if(d_font != NULL) {
			return d_font->read_signal(SIG_TOWNS_FONT_KANJI_DATA_HIGH);
		}
		break;
	case 0xff98:
		if(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 1, 1);
		}
		break;
	case 0xff99:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			val = (ankcg_enabled) ? 0x01 : 0x00;
		} else if(d_planevram != NULL) {
			val = d_planevram->read_memory_mapped_io8(addr);
		}
		break;
	case 0xff9c:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_HIGH);
			}
		} else if(d_planevram != NULL) {
			val = d_planevram->read_io8(addr);
		}
		break;
	case 0xff9d:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_LOW);
			}
		} else if(d_planevram != NULL) {
			val = d_planevram->read_io8(addr);
		}
		break;
	case 0xff9e:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				val = d_font->read_signal(SIG_TOWNS_FONT_KANJI_ROW);
			}
		} else if(d_planevram != NULL) {
			val = d_planevram->read_io8(addr);
		}
		break;
	default:
		if(d_planevram != NULL) {
			val = d_planevram->read_io8(addr);
		}
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
			if(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(1);
			}
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
			uint8_t wrap_val = 0xff; // WRAP OFF
			if(machine_id >= 0x0b00) { // After MA/MX/ME
				wrap_val = 0x00;
			}
			if(d_dmac != NULL) {
				d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP_REG, wrap_val, 0xff);
			}
		}
		// Towns SEEMS to not set addreess mask (a.k.a A20 mask). 20200131 K.O	
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
			if(d_cpu != NULL) {
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
		{
			d_serialrom->write_signal(SIG_SERIALROM_CS, ~data, 0x20);
			d_serialrom->write_signal(SIG_SERIALROM_CLK, data, 0x40);
			d_serialrom->write_signal(SIG_SERIALROM_RESET, data, 0x80);
		}
		break;
	case 0x0404: // System Status Reg.
		dma_is_vram = ((data & 0x80) == 0);
		config_page00();
		break;
	case 0x0480:
		select_d0_dict = ((data & 0x01) != 0) ? true : false;
		select_d0_rom = ((data & 0x02) == 0) ? true : false;
		config_page00();
		break;
	case 0x05c0:
		extra_nmi_mask = ((data & 0x08) == 0);
		break;
	case 0x05e0:
		// From AB.COM
		if(machine_id < 0x0200) { // Towns 1/2
			uint8_t nval = data & 7;
			if(nval < 1) nval = 1;
			if(nval > 5) nval = 5;
			mem_wait_val = nval + 1;
			vram_wait_val = nval + 3 + 1;
			wait_register = nval;
			set_wait_values();
		}
		break;
	case 0x05e2:
		if(machine_id >= 0x0200) { // After Towns 1H/2F. Hidden wait register.
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
			set_wait_values();
		}
		break;
	case 0x05ec:
		if(machine_id >= 0x0500) { // Towns2 CX :
			vram_wait_val = ((data & 0x01) != 0) ? 3 : 6;
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 3;
			cpu_clock_val = ((data & 0x01) != 0) ? (get_cpu_clocks(d_cpu)) : (16 * 1000 * 1000);
			set_wait_values();
		}
		break;
	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			is_compatible = ((data & 0x01) == 0x00) ? true : false;
			if(d_crtc != NULL) {
				d_crtc->write_signal(SIG_TOWNS_CRTC_COMPATIBLE_MMIO, (is_compatible) ? 0xffffffff : 0x00000000, 0xffffffff);
			}
		}
		break;
	case 0xff94:
		if(d_font != NULL) {
			d_font->write_signal(SIG_TOWNS_FONT_KANJI_HIGH, data, 0xff);
		}
		break;
	case 0xff95:
		if(d_font != NULL) {
			d_font->write_signal(SIG_TOWNS_FONT_KANJI_LOW, data, 0xff);
		}
		break;
	case 0xff96:
		break;
	case 0xff97:
		break;
	case 0xff98:
		if(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 0, 1);
		}
		break;
	case 0xff99:
		ankcg_enabled = ((data & 1) != 0) ? true : false;
		config_page00();
		break;
	case 0xff9e:
		if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
			if(d_font != NULL) {
				d_font->write_signal(SIG_TOWNS_FONT_KANJI_ROW, data, 0xff);
			}
		} else if(d_planevram != NULL) {
			d_planevram->write_io8(addr , data);
		}
		break;
	default:
		if(d_planevram != NULL) {
			d_planevram->write_io8(addr , data);
		}
		break;
	}
	return;
}
/*
// At page 000C0000h - 000CFFFFh : Maybe return real memory for word/dword access.
uint32_t TOWNS_MEMORY::read_memory_mapped_io16(uint32_t addr)
{
	if((addr >= 0xc0000) && (addr <= 0xcffff)) {
		pair32_t n;
		n.d = 0;
		n.b.l = ram_pagec[(addr & 0xffff) + 0];
		n.b.h = ram_pagec[(addr & 0xffff) + 1];
		return n.d;
	}
	return 0xffff;
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io32(uint32_t addr)
{
	if((addr >= 0xc0000) && (addr <= 0xcffff)) {
		pair32_t n;
		n.d = 0;
		n.b.l  = ram_pagec[(addr & 0xffff) + 0];
		n.b.h  = ram_pagec[(addr & 0xffff) + 1];
		n.b.h2 = ram_pagec[(addr & 0xffff) + 2];
		n.b.h3 = ram_pagec[(addr & 0xffff) + 3];
		return n.d;
	}
	return 0xffffffff;
}
*/
uint32_t TOWNS_MEMORY::read_memory_mapped_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	if((addr >= 0xc0000) && (addr <= 0xc7fff)) {
		if(d_planevram != NULL) {
			return d_planevram->read_memory_mapped_io8(addr);
		}
		return 0xff;
	} else if((addr >= 0xc8000) && (addr <= 0xc9fff)) {
		if(d_sprite != NULL) {
			d_sprite->read_memory_mapped_io8(addr);
		} else {
			return 0xff;
		}
	} else if((addr >= 0xca000) && (addr <= 0xcbfff)) {
		if(ankcg_enabled) {
			if((addr >= 0xca000) && (addr <= 0xca7ff)) {
				if(d_font != NULL) {
					d_font->read_memory_mapped_io8(addr);
				} else {
					return 0xff;
				}
			} else if((addr >= 0xcb000) && (addr <= 0xcbfff)) {
				if(d_font != NULL) {
					d_font->read_memory_mapped_io8(addr);
				} else {
					return 0xff;
				}
			} else if((addr >= 0xca800) && (addr <= 0xcafff)) {
				return 0xff;
			}
		}
		if(d_sprite != NULL) {
			d_sprite->read_memory_mapped_io8(addr);
		} else {
			return 0xff;
		}
	}
	
	if((addr < 0xcfc00) || (addr >= 0xd0000)) return 0xff;
	switch(addr) {
	case 0xcff88:
	case 0xcff94:
	case 0xcff96:
	case 0xcff97:
	case 0xcff98:
	case 0xcff99:
	case 0xcff9c:
	case 0xcff9d:
	case 0xcff9e:
		val = read_io8(addr & 0xffff);
		break;
	default:
		if((addr < 0xcff80) || (addr > 0xcffbb)) {
			return ram_pagec[addr & 0xffff];
		}
		if(d_planevram != NULL) {
			val = d_planevram->read_io8(addr & 0xffff);
		}
		break;
	}
	return (uint32_t)val;
}
/*
void TOWNS_MEMORY::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	if((addr >= 0xc0000) && (addr <= 0xcffff)) {
		pair16_t n;
		n.w = data;
		ram_pagec[(addr & 0xffff) + 0] = n.b.l;
		ram_pagec[(addr & 0xffff) + 1] = n.b.h;
	}
}

void TOWNS_MEMORY::write_memory_mapped_io32(uint32_t addr, uint32_t data)
{
	if((addr >= 0xc0000) && (addr <= 0xcffff)) {
		pair32_t n;
		n.d = data;
		ram_pagec[(addr & 0xffff) + 0] = n.b.l;
		ram_pagec[(addr & 0xffff) + 1] = n.b.h;
		ram_pagec[(addr & 0xffff) + 2] = n.b.h2;
		ram_pagec[(addr & 0xffff) + 3] = n.b.h3;
	}
}
*/

void TOWNS_MEMORY::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if((addr >= 0xc0000) && (addr <= 0xc7fff)) {
		if(d_planevram != NULL) {
			d_planevram->write_memory_mapped_io8(addr, data);
		}
		return;
	} else if((addr >= 0xc8000) && (addr <= 0xc9fff)) {
		if(d_sprite != NULL) {
			d_sprite->write_memory_mapped_io8(addr, data);
		}
		return;
	} else if((addr >= 0xca000) && (addr <= 0xcbfff)) {
		if(d_sprite != NULL) {
			d_sprite->write_memory_mapped_io8(addr, data);
		}
		return;
	}
	if((addr < 0xcfc00) || (addr >= 0xd0000)) return;
	switch(addr) {
	case 0xcff94:
	case 0xcff95:
	case 0xcff98:
	case 0xcff99:
	case 0xcff9e:
		write_io8(addr & 0xffff, data);
		break;
	default:
		if((addr < 0xcff80) || (addr > 0xcffbb)) {
			ram_pagec[addr & 0xffff] = data;
			return;
		}
		if(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
		break;
	}
	return;
}

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	int bank = (addr & addr_mask) >> addr_shift;
	int dummy;
	MEMORY::write_data16w(addr, data, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	int bank = (addr & addr_mask) >> addr_shift;
	int dummy;
	MEMORY::write_data32w(addr, data, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
}


uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int* wait)
{
	int bank = (addr & addr_mask) >> addr_shift;
	int dummy;
	uint32_t val = MEMORY::read_data16w(addr, &dummy);
	// Note: WAIT valus may be same as 1 bytes r/w.
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int* wait)
{
	int bank = (addr & addr_mask) >> addr_shift;
	// Note: WAIT valus may be same as 1 bytes r/w.
	int dummy;
	uint32_t val = MEMORY::read_data32w(addr, &dummy);
	if(wait != NULL) {
		*wait = wr_table[bank].wait;
	}
	return val;
	
}

uint32_t TOWNS_MEMORY::read_dma_data8(uint32_t addr)
{
	int bank = (addr & addr_mask) >> addr_shift;
	
	if(rd_table[bank].device != NULL) {
		return rd_table[bank].device->read_memory_mapped_io8(addr);
	} else {
		return rd_table[bank].memory[addr & bank_mask];
	}
}

uint32_t TOWNS_MEMORY::read_dma_data16(uint32_t addr)
{
	int bank = (addr & addr_mask) >> addr_shift;
	
	if(rd_table[bank].device != NULL) {
		return rd_table[bank].device->read_memory_mapped_io16(addr);
	} else {
		uint32_t naddr = addr & bank_mask;
		pair32_t n;
		n.d = 0;

		if((naddr + 1) > bank_mask) {
			n.b.l = rd_table[bank].memory[naddr];
			n.b.h = read_dma_data8(addr + 1);
		} else {
			n.b.l = rd_table[bank].memory[naddr + 0];
			n.b.h = rd_table[bank].memory[naddr + 1];
		}
		return n.d;
	}
}

uint32_t TOWNS_MEMORY::read_dma_data32(uint32_t addr)
{
	int bank = (addr & addr_mask) >> addr_shift;
	
	if(rd_table[bank].device != NULL) {
		return rd_table[bank].device->read_memory_mapped_io32(addr);
	} else {
		uint32_t naddr = addr & bank_mask;
		pair32_t n;
		n.d = 0;
		
		if((naddr + 3) > bank_mask) {
			n.b.l  = rd_table[bank].memory[naddr];
			n.b.h  = read_dma_data8(addr + 1);
			n.b.h2 = read_dma_data8(addr + 2);
			n.b.h3 = read_dma_data8(addr + 3);
		} else {
			n.b.l  = rd_table[bank].memory[naddr + 0];
			n.b.h  = rd_table[bank].memory[naddr + 1];
			n.b.h2 = rd_table[bank].memory[naddr + 2];
			n.b.h3 = rd_table[bank].memory[naddr + 3];
		}
		return n.d;
	}
}

void TOWNS_MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	int bank = (addr & addr_mask) >> addr_shift;
	
	if(wr_table[bank].device != NULL) {
		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		wr_table[bank].memory[addr & bank_mask] = data;
	}
}

void TOWNS_MEMORY::write_dma_data16(uint32_t addr, uint32_t data)
{
	int bank = (addr & addr_mask) >> addr_shift;
	
	if(wr_table[bank].device != NULL) {
		wr_table[bank].device->write_memory_mapped_io16(addr, data);
	} else {
		uint32_t naddr = addr & bank_mask;
		pair32_t n;
		n.d = data;
		
		if((naddr + 1) > bank_mask) {
			wr_table[bank].memory[naddr] = n.b.l;
			write_dma_data8(addr + 1, n.b.h);
		} else {
			wr_table[bank].memory[naddr + 0] = n.b.l;
			wr_table[bank].memory[naddr + 1] = n.b.h;
		}
	}
}

void TOWNS_MEMORY::write_dma_data32(uint32_t addr, uint32_t data)
{
	int bank = (addr & addr_mask) >> addr_shift;
	
	if(wr_table[bank].device != NULL) {
		wr_table[bank].device->write_memory_mapped_io32(addr, data);
	} else {
		uint32_t naddr = addr & bank_mask;
		pair32_t n;
		n.d = data;
		
		if((naddr + 3) > bank_mask) {
			wr_table[bank].memory[naddr] = n.b.l;
			write_dma_data8(addr + 1, n.b.h);
			write_dma_data8(addr + 2, n.b.h2);
			write_dma_data8(addr + 3, n.b.h3);
		} else {
			wr_table[bank].memory[naddr + 0] = n.b.l;
			wr_table[bank].memory[naddr + 1] = n.b.h;
			wr_table[bank].memory[naddr + 2] = n.b.h2;
			wr_table[bank].memory[naddr + 3] = n.b.h3;
		}
	}
}

uint32_t TOWNS_MEMORY::read_dma_data8w(uint32_t addr, int* wait)
{
	uint32_t val = read_dma_data8(addr);
	
	if(wait != NULL) {
		int bank = (addr & addr_mask) >> addr_shift;
		*wait = wr_table[bank].wait;
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_dma_data16w(uint32_t addr, int* wait)
{
	uint32_t val = read_dma_data16(addr);
	
	if(wait != NULL) {
		int bank = (addr & addr_mask) >> addr_shift;
		*wait = wr_table[bank].wait;
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_dma_data32w(uint32_t addr, int* wait)
{
	uint32_t val = read_dma_data32(addr);
	
	if(wait != NULL) {
		int bank = (addr & addr_mask) >> addr_shift;
		*wait = wr_table[bank].wait;
	}
	return val;
}

void TOWNS_MEMORY::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	write_dma_data8(addr, data);
	
	if(wait != NULL) {
		int bank = (addr & addr_mask) >> addr_shift;
		*wait = wr_table[bank].wait;
	}

}

void TOWNS_MEMORY::write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
{
	write_dma_data16(addr, data);
	
	if(wait != NULL) {
		int bank = (addr & addr_mask) >> addr_shift;
		*wait = wr_table[bank].wait;
	}

}

void TOWNS_MEMORY::write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
{
	write_dma_data32(addr, data);
	
	if(wait != NULL) {
		int bank = (addr & addr_mask) >> addr_shift;
		*wait = wr_table[bank].wait;
	}

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
		reset_happened = true;
		dma_is_vram = true;
		nmi_vector_protect = false;
		ankcg_enabled = false;
		nmi_mask = false;
		select_d0_dict = false;
		select_d0_rom = true;
		config_page00();
		set_wait_values();

		if(d_cpu != NULL) {
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
	state_fio->StateArray(ram_pagee,  sizeof(ram_pagee), 1);
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
