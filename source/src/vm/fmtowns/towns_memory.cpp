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

void TOWNS_MEMORY::initialize()
{
	if(initialized) return;
	//MEMORY::initialize();

	update_machine_features();
	extra_nmi_mask = true;
	extra_nmi_val = false;
	poff_status = false;
	reset_happened = false;

	vram_wait_val = 6;
	mem_wait_val = 3;
//	if((cpu_id == 0x01) || (cpu_id == 0x03)) {
//		wait_register_older = vram_wait_val;
//		wait_register_vram = vram_wait_val;
//		wait_register_ram = mem_wait_val;
//	} else {
		wait_register_older  = 3;
		wait_register_vram = 0x06;
		wait_register_ram  = 0x03;
//	}
	//cpu_clock_val = 16 * 1000 * 1000;
	//cpu_clock_val = get_cpu_clocks(d_cpu);
	set_cpu_clock_by_wait();
	extram_size = extram_size & 0x3ff00000;
	set_extra_ram_size(extram_size >> 20); // Check extra ram size.

	if(extram_size >= 0x00100000) {
		__UNLIKELY_IF(extra_ram == NULL) {
			extra_ram = (uint8_t*)malloc(extram_size);
			#if 0
			__LIKELY_IF(extra_ram != NULL) {
				set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
				memset(extra_ram, 0x00, extram_size);
			}
			#endif
		}
	}
	memset(ram_page0, 0x00, sizeof(ram_page0));
	memset(ram_pagec, 0x00, sizeof(ram_pagec));

	select_d0_dict = false;
	select_d0_rom = true;

	dma_is_vram = true;
	initialized = true;

	// Lower 100000h
	#if 0
	set_memory_rw          (0x00000000, 0x000bffff, ram_page0);
	set_memory_rw          (0x000f0000, 0x000f7fff, &(ram_paged[0x20000]));
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
	#endif
	set_wait_values();
	// Another devices are blank

	// load rom image
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
}

bool TOWNS_MEMORY::set_cpu_clock_by_wait()
{
	uint32_t cpu_bak = cpu_clock_val;
	cpu_clock_val = (is_faster_wait()) ?
		get_cpu_clocks(d_cpu) : (16 * 1000 * 1000);
	return ((cpu_clock_val != cpu_bak) ? true : false);
}
void TOWNS_MEMORY::set_wait_values()
{
	uint32_t waitfactor = 0;
	if(cpu_clock_val < get_cpu_clocks(d_cpu)) {
		waitfactor = (uint32_t)(((double)get_cpu_clocks(d_cpu) / (double)cpu_clock_val) * 65536.0);
	}
	d_cpu->write_signal(SIG_CPU_WAIT_FACTOR, waitfactor, 0xffffffff);
	#if 0
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
	#endif
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

}
void TOWNS_MEMORY::reset()
{
	// reset memory
	// ToDo
	update_machine_features(); // Update MISC3, MISC4 by MACHINE ID.
	is_compatible = true;
	reset_happened = false;
	dma_is_vram = false;
	nmi_vector_protect = false;
	ankcg_enabled = false;
	nmi_mask = false;
	select_d0_dict = false;
	select_d0_rom = true;
	//config_page00();

	set_cpu_clock_by_wait();
	set_wait_values();
#if 1
	__LIKELY_IF(d_cpu != NULL) {
		d_cpu->set_address_mask(0xffffffff);
	}
	if(d_dmac != NULL) {
		uint8_t wrap_val = 0xff; // WRAP ON
		d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP, wrap_val, 0xff);
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
		return ram_pagec[addr & 0xffff];
	}
#if 1
	__LIKELY_IF((addr & 0xffff) < 0xff88) {
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_memory_mapped_io8(addr & 0xffff);
		}
		return val;
	} else if(((addr & 0xffff) >= 0xff94) && ((addr & 0xffff) < 0xff98)) {
		__LIKELY_IF(d_font != NULL) {
			val = d_font->read_io8(addr & 0xffff);
		}
		return val;
	}
#endif
	if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
		switch(addr & 0xffff) {
		case 0xff88:
			__LIKELY_IF(d_crtc != NULL) {
				val = d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CFF82H);
			}
			return val;
			break;
		case 0xff99:
			return (ankcg_enabled) ? 0x01 : 0x00;
			break;
		case 0xff9c:
		case 0xff9d:
		case 0xff9e:
			__LIKELY_IF(d_font != NULL) {
				val = d_font->read_io8(addr & 0xffff);
			}
			return val;
			break;
		default:

			break;
		}
	}
	switch(addr & 0xffff) {
	case 0xff88:
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_io8(addr);
		}
		break;
	case 0xff95:
		val = 0x80;
		break;
	case 0xff96:
		__LIKELY_IF(d_font != NULL) {
			return d_font->read_signal(SIG_TOWNS_FONT_KANJI_DATA_LOW);
		}
		break;
	case 0xff97:
		__LIKELY_IF(d_font != NULL) {
			return d_font->read_signal(SIG_TOWNS_FONT_KANJI_DATA_HIGH);
		}
		break;
	case 0xff98:
		__LIKELY_IF(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 1, 1);
		}
		break;
	case 0xff99:
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_memory_mapped_io8(addr);
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
uint8_t TOWNS_MEMORY::read_sys_ports8(uint32_t addr)
{
    uint8_t val;
	val = 0xff;
	switch(addr & 0xffff) {
	case 0x0020: // Software reset ETC.
		// reset cause register
		val = ((software_reset) ? 1 : 0) | ((reset_happened) ? 2 : 0);
		reset_happened = false;
		software_reset = false;
		__UNLIKELY_IF(d_cpu != NULL) {
			d_cpu->set_shutdown_flag(0);
		}
		if((machine_id >= 0x0300) && ((machine_id & 0xff00) != 0x0400)) { // After UX
			val = val | ((poff_status) ? 0x04 : 0x00);
		}
		break;
	case 0x0022:
//		val.b.l = 0xff;
//		if(d_dmac != NULL) {
//			val = d_dmac->read_signal(SIG_TOWNS_DMAC_ADDR_REG);
//		}
		break;
		// 0024, 0025 : MISC3 + MISC4
	case 0x0024:
		val = reg_misc3;
		break;
	case 0x0025:
		val = reg_misc4;
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
//			val = ((machine_id & 0xf8) | 1);
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
		break;
	case 0x0404: // System Status Reg.
		val = (dma_is_vram) ? 0x7f : 0xff;
//		val = (dma_is_vram) ? 0x00 : 0x80;
		break;
	case 0x0480:
		val  =  (select_d0_dict) ? 0x01 : 0x00;
		val |=  ((select_d0_rom) ? 0x00 : 0x02);
		break;
	case 0x05c0:
		val = (extra_nmi_mask) ? 0xf7 : 0xff;
//		val = (extra_nmi_mask) ? 0x00 : 0x08;
		break;
	case 0x05c2:
		val = (extra_nmi_val) ? 0xff : 0xf7;
//		val = (extra_nmi_val) ? 0x08 : 0x00;
		break;
	case 0x05e0:
		if(machine_id < 0x0200) { // Towns 1/2
			val =  wait_register_older;
		}
		break;
	case 0x05e2:
		if(machine_id >= 0x0200) { // i386
			val = wait_register_ram;
		}
		break;
	case 0x05e6:
		if(machine_id >= 0x0200) { // i386
			val = wait_register_vram;
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
		// 05ec, 05ed
		if(machine_id >= 0x0200) { // 05ec
			val = ((is_faster_wait()) ? 0x01 : 0x00);
		}
		break;
	case 0x05ed:
		if(machine_id >= 0x0700) { // 05ed
			uint32_t clk = get_cpu_clocks(d_cpu);
			clk = clk / (1000 * 1000);
			__UNLIKELY_IF(clk < 16) clk = 16;
			__UNLIKELY_IF(clk > 127) clk = 127; // ToDo
			val = 0x00 | clk;
		}
		break;
	case 0xfda4:
		if(machine_id >= 0x0700) { // After HR/HG
			val = (is_compatible) ? 0x00 : 0x01;
		} else {
			val = 0x00;
		}
		break;
	default:
		break;
	}
	return val;
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
	return read_sys_ports8(addr);
}

uint32_t TOWNS_MEMORY::read_io8w(uint32_t addr, int *wait)
{
//	uint32_t val = 0x00;  // MAY NOT FILL to "1" for unused bit 20200129 K.O
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		*wait = 6; // ToDo: will io_wait_val.
		return read_fmr_ports8(addr & 0xffff);
	}
	*wait = 6; // ToDo: will io_wait_val.
	return read_sys_ports8(addr);
}


void TOWNS_MEMORY::write_fmr_ports8(uint32_t addr, uint32_t data)
{
	__UNLIKELY_IF((addr & 0xffff) < 0xff80) {
		ram_pagec[addr & 0xffff] = data;
		return;
	}
#if 1
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
#endif
	if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
		switch(addr & 0xffff) {
		case 0xff9e:
			__LIKELY_IF(d_font != NULL) {
				d_font->write_io8(addr & 0xffff, data);
			}
			return;
		default:
			break;
		}
	}
	switch(addr & 0xffff) {
	case 0xff94:
		__LIKELY_IF(d_font != NULL) {
			d_font->write_signal(SIG_TOWNS_FONT_KANJI_HIGH, data, 0xff);
		}
		break;
	case 0xff95:
		__LIKELY_IF(d_font != NULL) {
			d_font->write_signal(SIG_TOWNS_FONT_KANJI_LOW, data, 0xff);
		}
		break;
	case 0xff96:
	case 0xff97:
		break;
	case 0xff98:
		__LIKELY_IF(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 0, 1);
		}
		break;
	case 0xff99:
		{
			bool _b = ankcg_enabled;
			ankcg_enabled = ((data & 1) != 0) ? true : false;
		}
		break;
	case 0xffa0:
		__LIKELY_IF(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
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
//			__LIKELY_IF(d_cpu != NULL) {
//				d_cpu->set_shutdown_flag(1);
//			}
			// Todo: Implement true power off.
//			emu->notify_power_off();
//			emu->power_off();
//			break;
		} else {
			poff_status = false;
//			__LIKELY_IF(d_cpu != NULL) {
//				d_cpu->set_shutdown_flag(0);
//			}
		}

		if((software_reset) || (poff_status)){
//			__LIKELY_IF(d_cpu != NULL) {
//				d_cpu->reset();
//			}
			uint8_t wrap_val = 0xff; // WRAP ON
			__LIKELY_IF(d_dmac != NULL) {
				d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP, wrap_val, 0xff);
			}
			if(poff_status) {
				__LIKELY_IF(d_cpu != NULL) {
					d_cpu->set_shutdown_flag(1);
				}
				// Todo: Implement true power off.
				 emu->notify_power_off();
				// emu->power_off();
			}
			vm->reset();
		}
		// Towns SEEMS to not set addreess mask (a.k.a A20 mask). 20200131 K.O
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(1);
			}
			// Todo: Implement true power off.
			poff_status = true;
			emu->notify_power_off();
//			emu->power_off();
			vm->reset();
		}
		// Power register
		break;
	case 0x0024:
		//if((d_dmac != NULL) && (machine_id >= 0x0b00)) { // After MA/MX/ME
		//	d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP, data, 0xff);
		//}
		break;
	case 0x0032:
		d_serialrom->write_signal(SIG_SERIALROM_CS, ~data, 0x20);
		d_serialrom->write_signal(SIG_SERIALROM_CLK, data, 0x40);
		d_serialrom->write_signal(SIG_SERIALROM_RESET, data, 0x80);
		break;
	case 0x0404: // System Status Reg.
		{
			dma_is_vram = ((data & 0x80) == 0);
		}
		break;
	case 0x0480:
		{
			select_d0_dict = ((data & 0x01) != 0) ? true : false;
			select_d0_rom = ((data & 0x02) == 0) ? true : false;
		}
		break;
	case 0x05c0:
		extra_nmi_mask = ((data & 0x08) == 0);
		break;
	case 0x05e0:
		// From AB.COM
		if(machine_id < 0x0200) { // Towns 1/2
			uint8_t nval_bak = wait_register_older & 0x07;
			uint8_t nval = data & 0x07;
			if(nval < 1) nval = 1;
			mem_wait_val = nval;
			vram_wait_val = nval + 3; // OK?
			if(vram_wait_val > 6) {
				vram_wait_val = 6;
			}
			wait_register_older = (data & 0xf8) | nval;
			cpu_clock_val = 16 * 1000 * 1000;
			if(nval_bak != nval) {
				set_cpu_clock_by_wait();
				set_wait_values();
			}
		}
		break;
	case 0x05e2:
		if(machine_id >= 0x0200) { // After Towns 1H/2F. Hidden wait register.
			uint8_t vram_bak = vram_wait_val;
			uint8_t mem_bak = mem_wait_val;
			if(data != 0x83) {
				uint8_t nval = data & 7;
				if(machine_id <= 0x0200) { // Towns 1H/2F.
					if(nval < 1) nval = 1;
				}
				if(nval > 6) nval = 6;
				mem_wait_val = nval;
				wait_register_ram = (data & 0xf8) | nval;
			} else {
				mem_wait_val = 3;
				vram_wait_val = 6;
				wait_register_ram = data;
			}
			if((vram_bak != vram_wait_val) || (mem_bak != mem_wait_val)) {
				set_cpu_clock_by_wait();
				set_wait_values();
			}
		}
		break;
	case 0x05e6:
		if(machine_id >= 0x0200) { // After Towns 1H/2F. Hidden wait register.
			uint8_t mem_bak = mem_wait_val;
			uint8_t vram_bak = vram_wait_val;
			if(data != 0x83) {
				uint8_t nval = data & 7;
				if(machine_id <= 0x0200) { // Towns 1H/2F.
					if(nval < 1) nval = 1;
				}
				if(nval > 6) nval = 6;
				vram_wait_val = nval;
				wait_register_vram = (data & 0xf8) | nval;
			} else {
				mem_wait_val = 3;
				vram_wait_val = 3;
				wait_register_vram = data;
			}
			if((vram_bak != vram_wait_val) || (mem_bak != mem_wait_val)) {
				set_cpu_clock_by_wait();
				set_wait_values();
			}
		}
		break;
	case 0x05ec:
		// ToDo: 0x05ed
		if(machine_id >= 0x0500) { // Towns2 CX :
			uint8_t mem_bak = mem_wait_val;
			uint8_t vram_bak = vram_wait_val;
			vram_wait_val = ((data & 0x01) != 0) ? 0 : 6;
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 6;
			wait_register_ram = mem_wait_val;
			wait_register_vram = vram_wait_val;
			if((mem_bak != mem_wait_val) || (vram_bak != vram_wait_val)) {
				set_cpu_clock_by_wait();
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
	write_sys_ports8(addr, data);
}

void TOWNS_MEMORY::write_io8w(uint32_t addr, uint32_t data, int *wait)
{
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		*wait = 6; // ToDo: will io_wait_val.
		write_fmr_ports8(addr & 0xffff, data);
		return;
	}
	*wait = 6; // ToDo: will io_wait_val.
	write_sys_ports8(addr, data);
	return;
}

DEVICE* TOWNS_MEMORY::select_bank_memory_mpu(uint32_t addr, const bool is_dma, const bool is_read, bool& is_exists, uintptr_t& memptr, uint32_t& offset, int& waitval, bool& boundary2, bool& boundary4)
{
	memptr = UINTPTR_MAX;
	offset = UINT32_MAX;
	waitval = mem_wait_val;
	is_exists = false;
	boundary2 = false;
	boundary4 = false;
	__LIKELY_IF(addr < (0x00100000 + extram_size)) {
		__LIKELY_IF(addr < 0x00100000) {
			return select_bank_lower_memory(addr, is_dma, is_read, is_exists, memptr, offset, waitval, boundary2, boundary4);
		}
		check_boundaries(addr, (extram_size + 0x00100000), boundary2, boundary4);
		__LIKELY_IF(extra_ram != NULL) {
			is_exists = true;
			offset = addr - 0x00100000;
			memptr = (uintptr_t)extra_ram;
		}
		return NULL;
	}
	// I/O, ROMs
	switch(addr >> 24) {
	case 0x80: // 80xxxxxx : VRAM
		// ToDo: High RESOLUTION VRAM
		__LIKELY_IF(addr < 0x80080000) {
			check_boundaries(addr, 0x80080000, boundary2, boundary4);
			__LIKELY_IF(d_vram != NULL) {
				//offset = addr - 0x80000000; // OK?
				is_exists = true;
				waitval = vram_wait_val;
			}
			return d_vram;
		} else if(addr < 0x80100000) {
			check_boundaries(addr, 0x80100000, boundary2, boundary4);
			return NULL;
		}
		__LIKELY_IF(addr < 0x80180000) {
			check_boundaries(addr, 0x80180000, boundary2, boundary4);
			__LIKELY_IF(d_vram != NULL) {
				//offset = addr - 0x80000000; // OK?
				is_exists = true;
				waitval = vram_wait_val;
			}
			return d_vram;
		}
		// Fallthrough
		check_boundaries(addr, 0x81000000, boundary2, boundary4);
		return NULL;
		break;
	case 0x81: // 81000000 - 8101ffff : SPRITE
		check_boundaries(addr, 0x82000000, boundary2, boundary4);
		__LIKELY_IF((d_sprite != NULL) && (addr < 0x81020000)) {
			//offset = addr - 0x80000000; // OK?
			bool boundary2b;
			bool boundary4b;
			check_boundaries(addr, 0x81020000, boundary2b, boundary4b);
			boundary2 |= boundary2b;
			boundary4 |= boundary4b;
			is_exists = true;
			waitval = vram_wait_val; // OK?
			return d_sprite;
		}
		break;
	case 0xc0: // c0xxxxxx : ICCARD
	case 0xc1: // c1xxxxxx : ICCARD
		check_boundaries(addr, (addr & 0xff000000) + 0x01000000, boundary2, boundary4);
		__LIKELY_IF(d_iccard[(addr >> 24) & 1] != NULL) {
			is_exists = true;
			offset = addr & 0x00ffffff;
			return d_iccard[(addr >> 24) & 1];
		}
		break;
	case 0xc2: // ROMs
	{
		switch((addr >> 20) & 0x0f) {
		case 0: // MSDOS or DICTIONARY
			if(is_read) {
				if(addr >= 0xc2080000) {
					// DICTIONARY : 0xc2080000 - 0xc20fffff
					check_boundaries(addr, 0xc2100000, boundary2, boundary4);
					is_exists = (d_dictionary != NULL) ? true : false;
					//offset = addr - 0xc2080000; // OK?
					return d_dictionary;
				} else {
					// MSDOS : 0xc2000000 - 0xc207ffff
					check_boundaries(addr, 0xc2080000, boundary2, boundary4);
					is_exists = (d_msdos != NULL) ? true : false;
					//offset = addr - 0xc2000000; // OK?
					return d_msdos;
				}
			}
			// For WRITING
			check_boundaries(addr, 0xc2100000, boundary2, boundary4);
			return NULL;
			break;
		case 1: // FONT or CMOS
			__LIKELY_IF((is_read) && (addr < 0xc2140000)) {
				// FONT
				check_boundaries(addr, 0xc2140000, boundary2, boundary4);
				is_exists = (d_font != NULL) ? true : false;
				// offset = addr - 0xc2100000;
				return d_font;
			}
			__LIKELY_IF((addr >= 0xc2140000) && (addr < 0xc2142000)) {
				// CMOS
				check_boundaries(addr, 0xc2142000, boundary2, boundary4);
				is_exists = (d_dictionary != NULL) ? true : false;
				// offset = addr - 0xc2100000;
				return d_dictionary ;
			} else if(addr < 0xc2140000) {
				check_boundaries(addr, 0xc2140000, boundary2, boundary4);
				is_exists = false;
				return NULL;
			}
			if(d_font_20pix != NULL) {
				__LIKELY_IF(addr >= 0xc2180000) {
					check_boundaries(addr, 0xc2200000, boundary2, boundary4);
					is_exists = true;
					return d_font_20pix;
				} else {
					check_boundaries(addr, 0xc2180000, boundary2, boundary4);
					is_exists = false;
					return NULL;
				}
			}
			check_boundaries(addr, 0xc2200000, boundary2, boundary4);
			return NULL;
			break;
		case 2: // PCM
			if(addr < 0xc2201000) {
				check_boundaries(addr, 0xc2201000, boundary2, boundary4);
				is_exists = (d_pcm != NULL) ? true : false;
				return d_pcm;
			}
			check_boundaries(addr, 0xc2300000, boundary2, boundary4);
			return NULL;
			break;
		default:
			check_boundaries(addr, 0xc3000000, boundary2, boundary4);
			return NULL;
			// ToDo: External I/O
			break;
		}
	}
	// 0xc2xxxxxx - 0xfffbffff
	check_boundaries(addr, 0xfffc0000, boundary2, boundary4);
	break;
	case 0xff: // SYSROM
		__LIKELY_IF(is_read) {
			__LIKELY_IF(addr >= 0xfffc0000) {
				//check_boundaries(addr, 0x00000000, boundary2, boundary4);
				is_exists = (d_sysrom != NULL) ? true : false;
				boundary2 = (addr == 0xffffffff);
				boundary4 = (addr >  0xfffffffc);
				//offset = addr - 0xfffc0000;
				return d_sysrom;
			} else {
				check_boundaries(addr, 0xfffc0000, boundary2, boundary4);
				return NULL;
			}
		}
		// writing
		//check_boundaries(addr, 0x00000000, boundary2, boundary4);
		boundary2 = (addr == 0xffffffff);
		boundary4 = (addr >  0xfffffffc);
		break;
	default:
		if(addr < 0x80000000) {
			check_boundaries(addr, 0x80000000, boundary2, boundary4);
		} else if(addr < 0xc0000000) {
			check_boundaries(addr, 0xc0000000, boundary2, boundary4);
		} else if(addr > 0xfffffffc) {
			boundary2 = (addr == 0xffffffff);
			boundary4 = true;
		} else {
			check_boundaries(addr, 0xfffc0000, boundary2, boundary4);
		}
		break;
	}
	return NULL;
}

DEVICE* TOWNS_MEMORY::select_bank_lower_memory(const uint32_t addr, const bool is_dma, const bool is_read, bool& is_exists, uintptr_t& memptr, uint32_t& offset, int& waitval, bool& boundary2, bool& boundary4)
{
	uint8_t addr2 = (addr >> 16) & 0x0f;
	__LIKELY_IF(addr < 0x000c0000) {
		// 00000000 - 000bffff
		is_exists = true;
		offset = addr;
		memptr = (uintptr_t)ram_page0;
		check_boundaries(addr, 0x000c0000, boundary2, boundary4);
		return NULL;
	} else {
		__LIKELY_IF(!(dma_is_vram)) {
			__LIKELY_IF(addr < 0x000f8000) {
				check_boundaries(addr, 0x000f8000, boundary2, boundary4);
				is_exists = true;
				offset = addr - 0x000c0000;
				memptr = (uintptr_t)ram_pagec;
			} else {
				return select_bank_sysrom_f0000(addr, is_dma, is_read, is_exists, memptr, offset, waitval, boundary2, boundary4);
			}
			return NULL;
		}
		const uint8_t addr2 = (addr >> 16) & 0x0f;
		switch(addr2) {
		case 0x0c:
			return select_bank_vram_c0000(addr, is_dma, is_read, is_exists, memptr, offset, waitval, boundary2, boundary4);
			break;
		case 0x0d:
		case 0x0e:
			if((select_d0_dict) && (addr < 0x000da000)) {
				__LIKELY_IF(is_read) {
					check_boundaries(addr, 0x000da000, boundary2, boundary4);
					is_exists = (d_dictionary != NULL) ? true : false;
					return d_dictionary;
				}
				if(addr >= 0x000d8000) {
					check_boundaries(addr, 0x000da000, boundary2, boundary4);
					is_exists = (d_dictionary != NULL) ? true : false;
					return d_dictionary;
				}
				// addr < 0x000d8000 && !(is_read)
				check_boundaries(addr, 0x000d8000, boundary2, boundary4);
				is_exists = false;
				return NULL;
			}
			check_boundaries(addr, 0x000f0000, boundary2, boundary4);
			is_exists = false;
			return NULL;
			break;
		case 0x0f:
			return select_bank_sysrom_f0000(addr, is_dma, is_read, is_exists, memptr, offset, waitval, boundary2, boundary4);
			break;
		default: // FALLTHROUGH.DUMMY.
			check_boundaries(addr, 0x00100000, boundary2, boundary4);
			break;
		}
	}
	return NULL;
}

DEVICE* TOWNS_MEMORY::select_bank_vram_c0000(const uint32_t addr, const bool is_dma, const bool is_read, bool& is_exists, uintptr_t& memptr, uint32_t& offset, int& waitval, bool& boundary2, bool& boundary4)
{
	uint8_t addr3 = (addr >> 12) & 0x0f;
	switch(addr3) {
	default: // 0xc0000 - 0xc7fff
		__LIKELY_IF(d_planevram != NULL) {
			is_exists = true;
			// offset = addr - 0x000c0000;
			waitval = vram_wait_val;
		}
		check_boundaries(addr, 0x000c8000, boundary2, boundary4);
		return d_planevram;
		break;
	case 0x08:
	case 0x09:
		__LIKELY_IF(d_sprite != NULL) {
			is_exists = true;
			// offset = addr - 0x000c0000;
			waitval = vram_wait_val;
		}
		check_boundaries(addr, 0x000ca000, boundary2, boundary4);
		return d_sprite;
		break;
	case 0x0a:
	case 0x0b:
		check_boundaries(addr, 0x000cc000, boundary2, boundary4);
		if(ankcg_enabled) {
			if(is_read) {
				is_exists = (d_font != NULL) ? true : false;
				// offset = addr - 0x000c0000;
				return d_font;
			} else {
				is_exists = false;
				return NULL;
			}
		} else {
			__LIKELY_IF(d_sprite != NULL) {
				is_exists = true;
				// offset = addr - 0x000c0000;
				waitval = vram_wait_val;
			}
			return d_sprite;
		}
		break;
	case 0x0c: // RAM ... OK?
	case 0x0d:
	case 0x0e:
	case 0x0f:
		__UNLIKELY_IF(addr >= 0x000cff80) { // MMIO
			check_boundaries(addr, 0x000d0000, boundary2, boundary4);
			is_exists = true;
			// offset = addr - 0xcff80;
			return this;
		}
		// MEMORY (OK?)
		check_boundaries(addr, 0x000cff80, boundary2, boundary4);
		is_exists = true;
		offset = addr - 0x000c0000;
		memptr = (uintptr_t)ram_pagec;
		return NULL;
		break;
	}
	return NULL; // Fallthrough.
}

DEVICE* TOWNS_MEMORY::select_bank_sysrom_f0000(const uint32_t addr, const bool is_dma, const bool is_read, bool& is_exists, uintptr_t& memptr, uint32_t& offset, int& waitval, bool& boundary2, bool& boundary4)
{
	check_boundaries(addr, 0x00100000, boundary2, boundary4);
	if((select_d0_rom) && (addr >= 0x000f8000)) {
		__LIKELY_IF(is_read) {
			is_exists = (d_sysrom != NULL) ? true : false;
			return d_sysrom;
		}
	} else {
		__UNLIKELY_IF(select_d0_rom) {
			bool boundary2b, boundary4b;
			check_boundaries(addr, 0x000f8000, boundary2b, boundary4b);
			boundary2 |= boundary2b;
			boundary4 |= boundary4b;
		}
		is_exists = true;
		offset = addr - 0x000c0000;
		memptr = (uintptr_t)ram_pagec;
	}
	return NULL;
}

uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int* wait)
{
	int waitval;
	bool is_exists;
	uintptr_t memptr;
	uint32_t offset;
	bool boundary2;
	bool boundary4;

	DEVICE* dev = select_bank_memory_mpu(addr, false, true, is_exists, memptr, offset, waitval, boundary2, boundary4);
	__UNLIKELY_IF(!(is_exists)) {
		__LIKELY_IF(wait != NULL) {
			*wait = mem_wait_val;
		}
		return 0xff;
	}
	__LIKELY_IF(wait != NULL) {
		*wait = waitval;
	}
	if(dev != NULL) {
		return dev->read_memory_mapped_io8((offset != UINT32_MAX) ? offset : addr); // ToDo: Implement for offset.
	} else {
		__LIKELY_IF((memptr != UINTPTR_MAX) && (memptr != 0) && (offset != UINT32_MAX)) {			uint8_t* p = (uint8_t*)(memptr + offset);
			uint8_t dat = *p;
			return dat;
		}
	}
	return 0xff;
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int* wait)
{
	int waitval;
	bool is_exists;
	uintptr_t memptr;
	uint32_t offset;
	bool boundary2;
	bool boundary4;

	DEVICE* dev = select_bank_memory_mpu(addr, false, true, is_exists, memptr, offset, waitval, boundary2, boundary4);
	__UNLIKELY_IF(boundary2) {
		pair16_t dat;
		int wait1 = mem_wait_val;
		int wait2 = mem_wait_val;
		dat.b.l = read_data8w(addr    , &wait1);
		dat.b.h = read_data8w(addr + 1, &wait2);
		__LIKELY_IF(wait != NULL) {
			*wait = wait1 + wait2;
		}
		return dat.w;
	} else {
		// NOT beyond boundary.
		__UNLIKELY_IF(!(is_exists)) {
			__LIKELY_IF(wait != NULL) {
				// ToDo: Implement for 16bits data bus.
				//*wait = ((addr & 1) != 0) ? mem_wait_val * 2 : mem_wait_val;
				*wait = mem_wait_val;
			}
			return 0xffff;
		}
		__LIKELY_IF(wait != NULL) {
			// ToDo: Implement for 16bits data bus.
			//*wait = ((addr & 1) != 0) ? waitval * 2 : waitval;
			*wait = waitval;
		}
		if(dev != NULL) {
			return dev->read_memory_mapped_io16((offset != UINT32_MAX) ? offset : addr); // ToDo: Implement for offset.
		} else {
			__LIKELY_IF((memptr != UINTPTR_MAX) && (memptr != 0) && (offset != UINT32_MAX)) {
				uint8_t* p = (uint8_t*)(memptr + offset);
				pair16_t dat;
				#ifdef __BIG_ENDIAN__
				dat.read_2bytes_le_from(p);
				#else
				__LIKELY_IF(((memptr + offset) & 1) == 0) { // Physical memory is aligned.
					uint16_t* q = (uint16_t*)p;
					dat.w = *q;
				} else {
					dat.read_2bytes_le_from(p);
				}
				#endif
				return dat.w;
			}
		}
	}
	return 0xffff;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int* wait)
{
	int waitval;
	bool is_exists;
	uintptr_t memptr;
	uint32_t offset;
	bool boundary2;
	bool boundary4;

	DEVICE* dev = select_bank_memory_mpu(addr, false, true, is_exists, memptr, offset, waitval, boundary2, boundary4);
	__UNLIKELY_IF((boundary2) || (boundary4)) {
		pair32_t dat;
		int wait1 = mem_wait_val;
		int wait2 = mem_wait_val;
		int wait3 = 0;
		if(boundary2) {
			pair16_t w;
			dat.b.l  = read_data8w(addr    , &wait1);
			w.w      = read_data16w(addr + 1, &wait2);
			dat.b.h  = w.b.l;
			dat.b.h2 = w.b.h;
			dat.b.h3 = read_data8w(addr + 3 , &wait3);
		} else if(boundary4) {
			if((addr & 3) == 2) {
				dat.w.l = read_data16w(addr    , &wait1);
				dat.w.h = read_data16w(addr + 2, &wait2);
			} else { // ADDRESS = 3
				pair16_t w;
				dat.b.l  = read_data8w(addr    , &wait1);
				w.w      = read_data16w(addr + 1, &wait2);
				dat.b.h  = w.b.l;
				dat.b.h2 = w.b.h;
				dat.b.h3 = read_data8w(addr + 3 , &wait3);
			}
		} else { // Fallthrough
			dat.w.l = read_data16w(addr    , &wait1);
			dat.w.h = read_data16w(addr + 2, &wait2);
		}
		__LIKELY_IF(wait != NULL) {
			*wait = wait1 + wait2 + wait3;
		}
		return dat.d;
	} else {
		// NOT beyond boundary.
		__UNLIKELY_IF(!(is_exists)) {
			__LIKELY_IF(wait != NULL) {
				// ToDo: Implement for 16bits data bus.
				//*wait = ((addr & 3) != 0) ? mem_wait_val * 2 : mem_wait_val;
				*wait = mem_wait_val;
			}
			return 0xffffffff;
		}
		__LIKELY_IF(wait != NULL) {
			// ToDo: Implement for 16bits data bus.
			//*wait = ((addr & 3) != 0) ? waitval * 2 : waitval;
			*wait = waitval;
		}
		if(dev != NULL) {
			return dev->read_memory_mapped_io32((offset != UINT32_MAX) ? offset : addr); // ToDo: Implement for offset.
		} else {
			__LIKELY_IF((memptr != UINTPTR_MAX) && (memptr != 0) && (offset != UINT32_MAX)) {
				uint8_t* p = (uint8_t*)(memptr + offset);
				pair32_t dat;
				#ifdef __BIG_ENDIAN__
				dat.read_4bytes_le_from(p);
				#else
				__LIKELY_IF(((memptr + offset) & 3) == 0) { // Physical memory is aligned.
					uint32_t* q = (uint32_t*)p;
					dat.d = *q;
				} else {
					dat.read_4bytes_le_from(p);
				}
				#endif
				return dat.d;
			}
		}
	}
	return 0xffffffff;
}


void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	int waitval;
	bool is_exists;
	uintptr_t memptr;
	uint32_t offset;
	bool boundary2;
	bool boundary4;

	DEVICE* dev = select_bank_memory_mpu(addr, false, false, is_exists, memptr, offset, waitval, boundary2, boundary4);
	__UNLIKELY_IF(!(is_exists)) {
		__LIKELY_IF(wait != NULL) {
			*wait = mem_wait_val;
		}
		return;
	}
	__LIKELY_IF(wait != NULL) {
		*wait = waitval;
	}
	if(dev != NULL) {
		dev->write_memory_mapped_io8((offset != UINT32_MAX) ? offset : addr, data); // ToDo: Implement for offset.
		return;
	} else {
		__LIKELY_IF((memptr != UINTPTR_MAX) && (memptr != 0) && (offset != UINT32_MAX)) {
			uint8_t* p = (uint8_t*)(memptr + offset);
			*p = data;
			return;
		}
	}
	return;
}

void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	int waitval;
	bool is_exists;
	uintptr_t memptr;
	uint32_t offset;
	bool boundary2;
	bool boundary4;

	DEVICE* dev = select_bank_memory_mpu(addr, false, false, is_exists, memptr, offset, waitval, boundary2, boundary4);
	__UNLIKELY_IF(boundary2) {
		pair16_t dat;
		dat.w = data;
		int wait1 = mem_wait_val;
		int wait2 = mem_wait_val;
		write_data8w(addr    , dat.b.l, &wait1);
		write_data8w(addr + 1, dat.b.h, &wait2);
		__LIKELY_IF(wait != NULL) {
			*wait = wait1 + wait2;
		}
		return;
	}
	// NOT beyond boundary.
	__UNLIKELY_IF(!(is_exists)) {
		__LIKELY_IF(wait != NULL) {
			*wait = mem_wait_val;
		}
		return;
	}
	__LIKELY_IF(wait != NULL) {
		// ToDo: Implement for 16bits data bus.
		//*wait = ((addr & 1) != 0) ? mem_wait_val * 2 : mem_wait_val;
		*wait = waitval;
	}
	if(dev != NULL) {
		dev->write_memory_mapped_io16((offset != UINT32_MAX) ? offset : addr, data); // ToDo: Implement for offset.
		return;
	} else {
		__LIKELY_IF((memptr != UINTPTR_MAX) && (memptr != 0) && (offset != UINT32_MAX)) {
			uint8_t* p = (uint8_t*)(memptr + offset);
			pair16_t dat;
			dat.w = data;
			#ifdef __BIG_ENDIAN__
			dat.write_2bytes_le_to(p);
			#else
			__LIKELY_IF(((memptr + offset) & 1) == 0) { // Physical memory is aligned.
				uint16_t* q = (uint16_t*)p;
				*q = dat.w;
			} else {
				dat.write_2bytes_le_to(p);
			}
			#endif
			return;
		}
	}
	return;
}

void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	int waitval;
	bool is_exists;
	uintptr_t memptr;
	uint32_t offset;
	bool boundary2;
	bool boundary4;

	DEVICE* dev = select_bank_memory_mpu(addr, false, false, is_exists, memptr, offset, waitval, boundary2, boundary4);
	__UNLIKELY_IF((boundary2) || (boundary4)) {
		pair32_t dat;
		dat.d = data;
		int wait1 = mem_wait_val;
		int wait2 = mem_wait_val;
		int wait3 = 0;
		if(boundary2) {
			pair16_t w;
			w.b.l = dat.b.h;
			w.b.h = dat.b.h2;
			write_data8w (addr    , dat.b.l, &wait1);
			write_data16w(addr + 1, w.w, &wait2);
			write_data8w (addr + 3, dat.b.h3, &wait3);
		} else if(boundary4) {
			if((addr & 3) == 2) {
				write_data16w(addr    , dat.w.l, &wait1);
				write_data16w(addr + 2, dat.w.h, &wait2);
			} else { // ADDRESS = 3
				pair16_t w;
				w.b.l = dat.b.h;
				w.b.h = dat.b.h2;
				write_data8w (addr    , dat.b.l, &wait1);
				write_data16w(addr + 1, w.w, &wait2);
				write_data8w (addr + 3, dat.b.h3, &wait3);
			}
		} else { // Fallthrough
			write_data16w(addr    , dat.w.l, &wait1);
			write_data16w(addr + 2, dat.w.h, &wait2);
		}

		__LIKELY_IF(wait != NULL) {
			*wait = wait1 + wait2 + wait3;
		}
		return;
	}
	// NOT beyond boundary.
	__UNLIKELY_IF(!(is_exists)) {
		__LIKELY_IF(wait != NULL) {
			*wait = mem_wait_val;
		}
		return;
	}
	__LIKELY_IF(wait != NULL) {
		// ToDo: Implement for 16bits data bus.
		//*wait = ((addr & 3) != 0) ? mem_wait_val * 2 : mem_wait_val;
		*wait = waitval;
	}
	if(dev != NULL) {
		dev->write_memory_mapped_io32((offset != UINT32_MAX) ? offset : addr, data); // ToDo: Implement for offset.
		return;
	} else {
		__LIKELY_IF((memptr != UINTPTR_MAX) && (memptr != 0) && (offset != UINT32_MAX)) {
			uint8_t* p = (uint8_t*)(memptr + offset);
			pair32_t dat;
			dat.d = data;
			#ifdef __BIG_ENDIAN__
			dat.write_4bytes_le_to(p);
			#else
			__LIKELY_IF(((memptr + offset) & 3) == 0) { // Physical memory is aligned.
				uint32_t* q = (uint32_t*)p;
				*q = dat.d;
			} else {
				dat.write_4bytes_le_to(p);
			}
			#endif
			return;
		}
	}
	return;
}


uint32_t TOWNS_MEMORY::read_memory_mapped_io8(uint32_t addr)
{
	int wait = 0;
	return read_memory_mapped_io8w(addr, &wait);
}
uint32_t TOWNS_MEMORY::read_memory_mapped_io8w(uint32_t addr, int *wait)
{
	__UNLIKELY_IF(addr < 0xcff80) {
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
	__UNLIKELY_IF(addr < 0xcff80) {
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
		return 0xffff;
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
	__UNLIKELY_IF(addr < 0xcff80) {
		*wait = mem_wait_val;
		ram_pagec[addr & 0xffff] = data;
		return;
	}
	__LIKELY_IF(addr < 0xd0000) {
		*wait = 6; // Maybe 6.
		write_fmr_ports8(addr, data);
		return;
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
	__UNLIKELY_IF(addr < 0xcff80) {
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
		pair16_t v;
		v.w = data;
		*wait = 6 * 2; // Maybe 6.
		write_fmr_ports8(addr, v.b.l);
		write_fmr_ports8(addr + 1, v.b.h);
		return;
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
		//config_page00();
		set_wait_values();

		__LIKELY_IF(d_cpu != NULL) {
			d_cpu->set_address_mask(0xffffffff);
		}
		__LIKELY_IF(d_dmac != NULL) {
			uint8_t wrap_val = 0xff; // WRAP ON
			d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP, wrap_val, 0xff);
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

#define STATE_VERSION	6

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
	state_fio->StateValue(wait_register_older);
	state_fio->StateValue(wait_register_ram);
	state_fio->StateValue(wait_register_vram);

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
		//unset_memory_rw(0x00100000, 0x3fffffff);
		if(extra_ram == NULL) {
			extram_size = 0;
			return false;
		} else {
			state_fio->Fread(extra_ram, extram_size, 1);
			//set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
		}
		set_wait_values();
		//config_page00();
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
