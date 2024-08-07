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

	unset_range_rw(0x00000000, 0xffffffff);

	reset_wait_values();

	__UNLIKELY_IF(extra_ram == NULL) {
		extra_ram = (uint8_t*)malloc(extram_size + 0x00100000);
		__LIKELY_IF(extra_ram != NULL) {
			set_region_memory_rw(0x00000000, (extram_size + 0x00100000) - 1, extra_ram, 0);
			memset(extra_ram, 0x00, extram_size + 0x00100000);
		}
	}


	initialized = true;

	// Lower 100000h

	config_page_c0_e0(true, false, true);
	config_page_f8(true, true);

	set_region_device_rw(0x80000000, 0x8007ffff, d_vram, NOT_NEED_TO_OFFSET);
	set_region_device_rw(0x80100000, 0x8017ffff, d_vram, NOT_NEED_TO_OFFSET);

	set_region_device_rw(0x81000000, 0x8101ffff, d_sprite, 0);
	set_region_device_rw(0xc0000000, 0xc0ffffff, d_iccard[0], 0);
	set_region_device_rw(0xc1000000, 0xc1ffffff, d_iccard[1], 0);
//	set_wait_rw(0x00000000, 0xffffffff,  vram_wait_val);

	set_region_device_r (0xc2000000, 0xc207ffff, d_msdos, 0);
	set_region_device_r (0xc2080000, 0xc20fffff, d_dictionary, NOT_NEED_TO_OFFSET);
	set_region_device_r (0xc2100000, 0xc213ffff, d_font, NOT_NEED_TO_OFFSET);
	// REAL IS C2140000h - C2141FFFh, but grain may be 8000h bytes.
	set_region_device_rw(0xc2140000, 0xc2147fff, d_cmos, 0);
	if(d_font_20pix != NULL) {
		set_region_device_r (0xc2180000, 0xc21fffff, d_font_20pix, 0);
	}
	// REAL IS C2200000h - C2200FFFh, but grain may be 8000h bytes.
	set_region_device_rw(0xc2200000, 0xc2200000 + memory_map_grain() - 1, d_pcm, 0);
	set_region_device_r (0xfffc0000, 0xffffffff, d_sysrom, 0);
	// Another devices are blank

	// load rom image
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
	register_vline_event(this);
	register_frame_event(this);
}

void TOWNS_MEMORY::reset_wait_values()
{
	set_mmio_wait_rw(0x00000000, 0x7fffffff, WAITVAL_RAM);
	set_dma_wait_rw (0x00000000, 0x7fffffff, WAITVAL_RAM);
	set_mmio_wait_rw(0x80000000, 0x803fffff, WAITVAL_VRAM); // Default Value
	set_dma_wait_rw (0x80000000, 0x803fffff, WAITVAL_VRAM); // Default Value
	set_mmio_wait_rw(0x80400000, 0xffffffff, WAITVAL_RAM);
	set_dma_wait_rw (0x80400000, 0xffffffff, WAITVAL_RAM);
}

void TOWNS_MEMORY::config_page_c0_e0(const bool vrambank, const bool dictbank, const bool force)
{
	if((dma_is_vram != vrambank) || (force)) {
		if(vrambank) { // VRAM AND around TEXT
			set_region_device_rw(0x000c0000, 0x000c7fff, d_planevram, NOT_NEED_TO_OFFSET);
			set_region_device_rw(0x000c8000, 0x000cffff, this, NOT_NEED_TO_OFFSET);

			set_mmio_wait_rw(0x000c0000, 0x000cffff, WAITVAL_VRAM); // Default Value
			set_dma_wait_rw (0x000c0000, 0x000cffff, WAITVAL_VRAM); // Default Value
			config_dictionary(vrambank, dictbank, force);
		} else {
			__LIKELY_IF(extra_ram != NULL) {
				set_region_memory_rw(0x000c0000, 0x000effff, extra_ram, 0x000c0000);
			} else {
				unset_range_rw(0x000c0000, 0x000effff);
			}
			set_mmio_wait_rw(0x000c0000, 0x000effff, WAITVAL_RAM); // Default Value
			set_dma_wait_rw (0x000c0000, 0x000effff, WAITVAL_RAM); // Default Value
		}
	}
	dma_is_vram = vrambank;
}

void TOWNS_MEMORY::config_dictionary(const bool vrambank, const bool dictbank, const bool force)
{
	__UNLIKELY_IF((vrambank != dma_is_vram) || (force)) {
		if(dictbank) {
			set_region_device_r(0x000d0000, 0x000d7fff, d_dictionary, NOT_NEED_TO_OFFSET);
			unset_range_w(0x000d0000, 0x000d7fff);
			// REAL IS 0000D8000h - 000D9FFFh, but grain may be 8000h bytes.
			set_region_device_rw(0x000d8000, 0x000dffff, d_cmos, 0);
			unset_range_rw(0x000e0000, 0x000effff);
		} else {
			__LIKELY_IF(extra_ram != NULL) {
				set_region_memory_rw(0x000d0000, 0x000effff, extra_ram, 0x000c0000);
			} else {
				unset_range_rw(0x000d0000, 0x000effff);
			}
		}
	}
	select_d0_dict = dictbank;
}

void TOWNS_MEMORY::config_page_f8(const bool sysrombank, const bool force)
{
	__UNLIKELY_IF((sysrombank != select_f8_rom) || (force)) {
		if(sysrombank) {
			set_region_device_r (0x000f8000, 0x000fffff, d_sysrom, 0x38000);
			__LIKELY_IF(extra_ram != NULL) {
				set_region_memory_w (0x000f8000, 0x000fffff, extra_ram, 0x000f8000);
			} else {
				unset_range_w(0x000f8000, 0x000fffff);
			}
		} else {
			__LIKELY_IF(extra_ram != NULL) {
				set_region_memory_rw(0x000f8000, 0x000fffff, extra_ram, 0x000f8000);
			} else {
				unset_range_rw(0x000f8000, 0x000fffff);
			}
		}
	}
	select_f8_rom = sysrombank;
}

void TOWNS_MEMORY::set_memory_devices_map_values(uint32_t start, uint32_t end, memory_device_map_t* dataptr, uint8_t* baseptr, DEVICE* device, uint32_t base_offset)
{
	uint64_t _start = (uint64_t)start;
	uint64_t _end =   (end == 0xffffffff) ? 0x100000000 : (uint64_t)(end + 1);
	__UNLIKELY_IF(dataptr == NULL) return;

	_start &= ~(memory_map_mask());
	_end &= ~(memory_map_mask());

	uint64_t mapptr = (uint32_t)(_start >> memory_map_shift());
	const uint64_t _incval = memory_map_grain();
	uint32_t realoffset = (base_offset == UINT32_MAX) ? 0 : base_offset;

	for(uint64_t addr = _start; addr < _end; addr += _incval) {
		__UNLIKELY_IF(mapptr >= memory_map_size()) break; // Safety
		if(baseptr == NULL) {
			dataptr[mapptr].mem_ptr = NULL;
			dataptr[mapptr].device_ptr = device;
		} else {
			dataptr[mapptr].mem_ptr = baseptr;
			dataptr[mapptr].device_ptr = NULL;
		}
		if(base_offset == UINT32_MAX) {
			dataptr[mapptr].base_offset = UINT32_MAX;
		} else {
			dataptr[mapptr].base_offset = realoffset;
		}
		realoffset += _incval;
		mapptr++;
	}
}

void TOWNS_MEMORY::set_memory_devices_map_wait(uint32_t start, uint32_t end, memory_device_map_t* dataptr, int wait)
{
	uint64_t _start = (uint64_t)start;
	uint64_t _end =   (end == 0xffffffff) ? 0x100000000 : (uint64_t)(end + 1);
	__UNLIKELY_IF(dataptr == NULL) return;

	_start &= ~(memory_map_mask());
	_end &= ~(memory_map_mask());

	uint64_t mapptr = (uint32_t)(_start >> memory_map_shift());
	const uint64_t _incval = memory_map_grain();

	for(uint64_t addr = _start; addr < _end; addr += _incval) {
		__UNLIKELY_IF(mapptr >= memory_map_size()) break; // Safety
		dataptr[mapptr].waitval = wait;
		mapptr++;
	}
}

void TOWNS_MEMORY::unset_memory_devices_map(uint32_t start, uint32_t end, memory_device_map_t* dataptr, int wait)
{
	uint64_t _start = (uint64_t)start;
	uint64_t _end =   (end == 0xffffffff) ? 0x100000000 : (uint64_t)(end + 1);
	__UNLIKELY_IF(dataptr == NULL) return;

	_start &= ~(memory_map_mask());
	_end &= ~(memory_map_mask());

	uint64_t mapptr = (uint32_t)(_start >> memory_map_shift());
	const uint64_t _incval = memory_map_grain();
	for(uint64_t addr = _start; addr < _end; addr += _incval) {
		__UNLIKELY_IF(mapptr >= memory_map_size()) break; // Safety
		dataptr[mapptr].mem_ptr = NULL;
		dataptr[mapptr].device_ptr = NULL;
		dataptr[mapptr].waitval = wait;
		dataptr[mapptr].base_offset = UINT32_MAX;
		mapptr++;
	}

}

void TOWNS_MEMORY::set_mmio_memory_r(uint32_t start, uint32_t end, uint8_t* baseptr, uint32_t base_offset)
{
	set_memory_devices_map_values(start, end, &(membus_read_map[0]), baseptr, NULL, base_offset);
}

void TOWNS_MEMORY::set_mmio_memory_w(uint32_t start, uint32_t end, uint8_t* baseptr, uint32_t base_offset)
{
	set_memory_devices_map_values(start, end, &(membus_write_map[0]), baseptr, NULL, base_offset);
}

void  __FASTCALL TOWNS_MEMORY::set_mmio_device_r(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress)
{
	set_memory_devices_map_values(start, end, &(membus_read_map[0]), NULL, ptr, baseaddress);
}

void  __FASTCALL TOWNS_MEMORY::set_mmio_device_w(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress)
{
	set_memory_devices_map_values(start, end, &(membus_write_map[0]), NULL, ptr, baseaddress);
}

void  __FASTCALL TOWNS_MEMORY::set_mmio_wait_r(uint32_t start, uint32_t end, int wait)
{
	set_memory_devices_map_wait(start, end, &(membus_read_map[0]), wait);
}

void  __FASTCALL TOWNS_MEMORY::set_mmio_wait_w(uint32_t start, uint32_t end, int wait)
{
	set_memory_devices_map_wait(start, end, &(membus_write_map[0]), wait);
}

void TOWNS_MEMORY::unset_mmio_r(uint32_t start, uint32_t end, int wait)
{
	unset_memory_devices_map(start, end, &(membus_read_map[0]), wait);
}

void TOWNS_MEMORY::unset_mmio_w(uint32_t start, uint32_t end, int wait)
{
	unset_memory_devices_map(start, end, &(membus_write_map[0]), wait);
}

void TOWNS_MEMORY::unset_dma_r(uint32_t start, uint32_t end, int wait)
{
	unset_memory_devices_map(start, end, &(dma_read_map[0]), wait);
}

void TOWNS_MEMORY::unset_dma_w(uint32_t start, uint32_t end, int wait)
{
	unset_memory_devices_map(start, end, &(dma_write_map[0]), wait);
}

void TOWNS_MEMORY::set_dma_memory_r(uint32_t start, uint32_t end, uint8_t* baseptr, uint32_t base_offset)
{
	set_memory_devices_map_values(start, end, &(dma_read_map[0]), baseptr, NULL, base_offset);
}

void TOWNS_MEMORY::set_dma_memory_w(uint32_t start, uint32_t end, uint8_t* baseptr, uint32_t base_offset)
{
	set_memory_devices_map_values(start, end, &(dma_write_map[0]), baseptr, NULL, base_offset);
}

void  __FASTCALL TOWNS_MEMORY::set_dma_device_r(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress)
{
	set_memory_devices_map_values(start, end, &(dma_read_map[0]), NULL, ptr, baseaddress);
}

void  __FASTCALL TOWNS_MEMORY::set_dma_device_w(uint32_t start, uint32_t end, DEVICE* ptr, uint32_t baseaddress)
{
	set_memory_devices_map_values(start, end, &(dma_write_map[0]), NULL, ptr, baseaddress);
}

void  __FASTCALL TOWNS_MEMORY::set_dma_wait_r(uint32_t start, uint32_t end, int wait)
{
	set_memory_devices_map_wait(start, end, &(dma_read_map[0]), wait);
}

void  __FASTCALL TOWNS_MEMORY::set_dma_wait_w(uint32_t start, uint32_t end, int wait)
{
	set_memory_devices_map_wait(start, end, &(dma_write_map[0]), wait);
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

	//<! @note From technical book Page 115, Figure I-3-34,
	//<! 「シャットダウンリセット、ソフトウェアリセットともにCPUとNDPにのみリセットがかかる」
	reset_happened = false;
	software_reset = false;
	poff_status = false;

	nmi_vector_protect = false;
	ankcg_enabled = false;
	nmi_mask = false;
	reset_wait_values();
	config_page_c0_e0(true, false, true); // VRAM, DICT, FORCE
	config_page_f8(true,  true); // SYSROM, FORCE

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

/*!
 @note This uses for caching memory table.
*/
void TOWNS_MEMORY::event_pre_frame()
{
	event_vline(0, 0); // Force reload cache.
}
void TOWNS_MEMORY::event_vline(int v, int clk)
{
	__LIKELY_IF((v & 3) != 0) {
		return;
	}
	// Prefetch tables.
	memory_device_map_t *mr_p;
	memory_device_map_t *mw_p;
	memory_device_map_t *dr_p;
	memory_device_map_t *dw_p;

	// 0x00000000 - EXTRA_RAM END
	size_t mainmem_tbl_bytes = (((size_t)extram_size + 0x00100000) >> memory_map_shift()) * sizeof(memory_device_map_t);
	mr_p = &(membus_read_map[0]);
	mw_p = &(membus_write_map[0]);
	dr_p = &(dma_read_map[0]);
	dr_p = &(dma_write_map[0]);
	
	make_prefetch_volatile(mr_p, mainmem_tbl_bytes);
	make_prefetch_volatile(mw_p, mainmem_tbl_bytes);
	make_prefetch_volatile(dr_p, mainmem_tbl_bytes);	
	make_prefetch_volatile(dw_p, mainmem_tbl_bytes);

	uint32_t vram_begin = 0x80000000 >> memory_map_shift();
	size_t vram_tbl_bytes = ((0x81020000 - 0x80000000) >> memory_map_shift()) * sizeof(memory_device_map_t);
	mr_p = &(membus_read_map[vram_begin]);
	mw_p = &(membus_write_map[vram_begin]);
	dr_p = &(dma_read_map[vram_begin]);
	dr_p = &(dma_write_map[vram_begin]);
	
	make_prefetch_volatile(mr_p, vram_tbl_bytes);
	make_prefetch_volatile(mw_p, vram_tbl_bytes);
	make_prefetch_volatile(dr_p, vram_tbl_bytes);	
	make_prefetch_volatile(dw_p, vram_tbl_bytes);

	// ToDo: High resolution VRAMs.

	// MISC DEVICES
	uint32_t dev_begin = 0xc2000000 >> memory_map_shift();
	size_t   dev_tbl_bytes = ((0xc2200000 - 0xc2000000) >> memory_map_shift()) * sizeof(memory_device_map_t);
	mr_p = &(membus_read_map[dev_begin]);
	mw_p = &(membus_write_map[dev_begin]);
	dr_p = &(dma_read_map[dev_begin]);
	dr_p = &(dma_write_map[dev_begin]);
	
	make_prefetch(mr_p, dev_tbl_bytes);
	make_prefetch(mw_p, dev_tbl_bytes);
	make_prefetch(dr_p, dev_tbl_bytes);	
	make_prefetch(dw_p, dev_tbl_bytes);

	// SYSTEM ROM
	uint32_t sysrom_begin = 0xfffc0000 >> memory_map_shift();
	size_t   sysrom_tbl_bytes = (0x00040000 >> memory_map_shift()) * sizeof(memory_device_map_t);
	mr_p = &(membus_read_map[sysrom_begin]);
	mw_p = &(membus_write_map[sysrom_begin]);
	dr_p = &(dma_read_map[sysrom_begin]);
	dr_p = &(dma_write_map[sysrom_begin]);
	
	make_prefetch_volatile(mr_p, sysrom_tbl_bytes);
	make_prefetch_volatile(mw_p, sysrom_tbl_bytes);
	make_prefetch_volatile(dr_p, sysrom_tbl_bytes);	
	make_prefetch_volatile(dw_p, sysrom_tbl_bytes);

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
	__UNLIKELY_IF((addr < 0xcff80) || (addr > 0xcffbb)) {
		return val;
	}
	__LIKELY_IF(addr < 0xcff88) {
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_memory_mapped_io8(addr & 0xffff);
		}
		return val;
	}
	if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
		switch(addr) {
		case 0xcff88:
			__LIKELY_IF(d_crtc != NULL) {
				val = d_crtc->read_signal(SIG_TOWNS_CRTC_MMIO_CFF82H);
			}
			return val;
			break;
		case 0xcff99:
			return (ankcg_enabled) ? 0x01 : 0x00;
			break;
		case 0xcff9c:
		case 0xcff9d:
		case 0xcff9e:
			__LIKELY_IF(d_font != NULL) {
				val = d_font->read_io8(addr & 0xffff);
			}
			return val;
			break;
		default:
			break;
		}
	}
	switch(addr) {
	case 0xcff94:
	case 0xcff95:
	case 0xcff96:
	case 0xcff97:
		__LIKELY_IF(d_font != NULL) {
			val = d_font->read_io8(addr & 0xffff);
		}
		break;
	case 0xcff98:
		__LIKELY_IF(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 1, 1);
		}
		break;
	case 0xcff99:
		__LIKELY_IF(d_planevram != NULL) {
			val = d_planevram->read_memory_mapped_io8(addr);
		}
		break;
	default:
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
//		val = (dma_is_vram) ? 0x7f : 0xff;
		val = (dma_is_vram) ? 0x00 : 0x80;
		break;
	case 0x0480:
		val  =  (select_d0_dict) ? 0x01 : 0x00;
		val |=  ((select_f8_rom) ? 0x00 : 0x02);
//		val |= 0xfc;
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
			val = extram_size >> 20;
			switch(nid >> 8) {
			case 0x00:
			case 0x01: // Towns 1/2 : Not Supported.
				val = 0xff;
				break;
			case 0x03: // Towns II UX
			case 0x06: // Towns II U6
				val = val & 0x0f;
				if(val >= 9) val = 9;
				break;
			case 0x02: // Towns 1F/2F/1H/2H.
			case 0x04: // Towns 10F/20F/40H/80H.
				val = val & 0x07;
				break;
			case 0x05: // Towns II CX
				val = val & 0x0f;
				break;
			case 0x08: // Towns II HG : OK?
				val = val & 0x0f;
				break;
			case 0x07: // Towns II HR
			case 0x09: // Towns II UR
				val = val & 0x1f;
				break;
			default:   // After MA/MX/ME/MF, Fresh
				val = val & 0x7f;
				break;
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
//	uint32_t val = 0x00;  // MAY NOT FILL to "1" for unused bit 20200129 K.O
	__LIKELY_IF((addr & 0xffff) >= 0xff80) {
		return read_fmr_ports8((addr & 0xffff) | 0x000c0000);
	}
	return read_sys_ports8(addr);
}

void TOWNS_MEMORY::write_fmr_ports8(uint32_t addr, uint32_t data)
{
	__UNLIKELY_IF((addr < 0xcff80) || (addr > 0xcffbb)) {
		return;
	}

	__LIKELY_IF(addr < 0xcff88) {
		__LIKELY_IF(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
		return;
	}

	if((machine_id >= 0x0600) && !(is_compatible)) { // After UG
		switch(addr) {
		case 0xcff9e:
			__LIKELY_IF(d_font != NULL) {
				d_font->write_io8(addr & 0xffff, data);
			}
			return;
		default:
			break;
		}
	}
	switch(addr) {
	case 0xcff94:
	case 0xcff95:
		__LIKELY_IF(d_font != NULL) {
			d_font->write_io8(addr & 0xffff, data);
		}
		break;
	case 0xcff96:
	case 0xcff97:
		break;
	case 0xcff98:
		__LIKELY_IF(d_timer != NULL) {
			d_timer->write_signal(SIG_TIMER_BEEP_ON, 0, 1);
		}
		break;
	case 0xcff99:
		{
			bool _b = ankcg_enabled;
			ankcg_enabled = ((data & 1) != 0) ? true : false;
		}
		break;
	case 0xcffa0:
		__LIKELY_IF(d_planevram != NULL) {
			d_planevram->write_io8(addr & 0xffff, data);
		}
		break;
	default:
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
		} else {
			poff_status = false;
		}

		if((software_reset) || (poff_status)){
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
			//<! @note From technical book Page 115, Figure I-3-34,
			//<! 「シャットダウンリセット、ソフトウェアリセットともにCPUとNDPにのみリセットがかかる」
			#if 1
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->reset();
			}
			#else
			//vm->reset();
			#endif
		}
		// Towns SEEMS to not set addreess mask (a.k.a A20 mask). 20200131 K.O
		break;
	case 0x0022:
		if((data & 0x40) != 0) {
			poff_status = true;
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->set_shutdown_flag(1);
			}
			// Todo: Implement true power off.
			emu->notify_power_off();
//			emu->power_off();
			//<! @note From technical book Page 115, Figure I-3-34,
			//<! 「シャットダウンリセット、ソフトウェアリセットともにCPUとNDPにのみリセットがかかる」
			#if 1
			__LIKELY_IF(d_cpu != NULL) {
				d_cpu->reset();
			}
			#else
			//vm->reset();
			#endif
		}
		// Power register
		break;
	case 0x0024:
		//if((d_dmac != NULL) && (machine_id >= 0x0b00)) { // After MA/MX/ME
		//	d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP, data, 0xff);
		//}
		break;
	case 0x0404: // System Status Reg.
		{
			bool vram_new = ((data & 0x80) == 0) ? true : false;
			config_page_c0_e0(vram_new, select_d0_dict, false);
		}
		break;
	case 0x0480:
		{
			bool is_dict, is_sysrom;
			bool dict_bak = select_d0_dict;
			bool sysrom_bak =
			is_dict = ((data & 0x01) != 0) ? true : false;
			is_sysrom = ((data & 0x02) == 0) ? true : false;
			if(dict_bak != is_dict) {
				config_page_c0_e0(dma_is_vram, is_dict, true);
			}
			config_page_f8(is_sysrom, false);
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
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 3;
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
		write_fmr_ports8((addr & 0xffff) | 0x000c0000, data);
		return;
	}
	write_sys_ports8(addr, data);
}


uint32_t TOWNS_MEMORY::read_data8w(uint32_t addr, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	return read_8bit_data(membus_read_map, mapptr, addr, offset, false, wait);
}

uint32_t TOWNS_MEMORY::read_data16w(uint32_t addr, int* wait)
{
	uint16_t val;
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	__UNLIKELY_IF(check_device_boundary(membus_read_map, mapptr, offset, 2)) {
		val = read_beyond_boundary_data16(membus_read_map, addr, offset, mapptr, false, wait);
	} else {
		val = read_16bit_data(membus_read_map, mapptr, addr, offset, false, wait);
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_data32w(uint32_t addr, int* wait)
{
	uint32_t val;
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	__UNLIKELY_IF(check_device_boundary(membus_read_map, mapptr, offset, 4)) {
		val =  read_beyond_boundary_data32(membus_read_map, addr, offset, mapptr,false,  wait);
	} else {
		val = read_32bit_data(membus_read_map, mapptr, addr, offset, false, wait);

	}
	return val;
}

uint32_t TOWNS_MEMORY::read_dma_data8w(uint32_t addr, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	uint8_t val;
	int waitval;
	val = read_8bit_data(dma_read_map, mapptr, addr, offset, true, &waitval);
	//val = read_8bit_data(dma_read_map, mapptr, addr, offset, false, &waitval);
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard wait value for DMA.
	}
	return val;
}

uint32_t TOWNS_MEMORY::read_dma_data16w(uint32_t addr, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	uint16_t val;
	int waitval;
	__UNLIKELY_IF(check_device_boundary(dma_read_map, mapptr, offset, 2)) {
		val = read_beyond_boundary_data16(dma_read_map, addr, offset, mapptr, true, &waitval);
	} else {
		val = read_16bit_data(dma_read_map, mapptr, addr, offset, true, &waitval);
		//val = read_16bit_data(dma_read_map, mapptr, addr, offset, false, &waitval);
	}
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard wait value for DMA.
	}
	return val;

}

uint32_t TOWNS_MEMORY::read_dma_data32w(uint32_t addr, int* wait)
{
	uint32_t val;
	int waitval;
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	__UNLIKELY_IF(check_device_boundary(dma_read_map, mapptr, offset, 4)) {
		val =  read_beyond_boundary_data32(dma_read_map, addr, offset, mapptr, true, &waitval);
	} else {
		val = read_32bit_data(dma_read_map, mapptr, addr, offset, true, &waitval);
		//val = read_32bit_data(dma_read_map, mapptr, addr, offset, false, &waitval);
	}
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard wait value for DMA.
	}
	return val;
}


void TOWNS_MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	write_8bit_data(membus_write_map, mapptr, addr, offset, false, data, wait);
}


void TOWNS_MEMORY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	__UNLIKELY_IF(check_device_boundary(membus_write_map, mapptr, offset, 2)) {
		write_beyond_boundary_data16(membus_write_map, addr, offset, mapptr, false, data, wait);
	} else {
		write_16bit_data(membus_write_map, mapptr, addr, offset, false, data, wait);
	}
}


void TOWNS_MEMORY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	__UNLIKELY_IF(check_device_boundary(membus_write_map, mapptr, offset, 4)) {
		write_beyond_boundary_data32(membus_write_map, addr, offset, mapptr, false, data, wait);
	} else {
		write_32bit_data(membus_write_map, mapptr, addr, offset, false, data, wait);
	}
}

void TOWNS_MEMORY::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	int waitval;
	write_8bit_data(dma_write_map, mapptr, addr, offset, true, data, &waitval);
	//write_8bit_data(dma_write_map, mapptr, addr, offset, false, data, &waitval);
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard wait value for DMA.
	}
}

void TOWNS_MEMORY::write_dma_data16w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	int waitval;
	__UNLIKELY_IF(check_device_boundary(dma_write_map, mapptr, offset, 2)) {
		write_beyond_boundary_data16(dma_write_map, addr, offset, mapptr, true, data, &waitval);
	} else {
		write_16bit_data(dma_write_map, mapptr, addr, offset, true, data, &waitval);
		//write_16bit_data(dma_write_map, mapptr, addr, offset, false, data, &waitval);
	}
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard wait value for DMA.
	}
}


void TOWNS_MEMORY::write_dma_data32w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t mapptr = (uint32_t)(((uint64_t)addr) >> memory_map_shift());
	uint32_t offset = addr & memory_map_mask();
	int waitval;
	__UNLIKELY_IF(check_device_boundary(dma_write_map, mapptr, offset, 4)) {
		write_beyond_boundary_data32(dma_write_map, addr, offset, mapptr, true, data, &waitval);
	} else {
		write_32bit_data(dma_write_map, mapptr, addr, offset, true, data, &waitval);
		//write_32bit_data(dma_write_map, mapptr, addr, offset, false, data, &waitval);
	}
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // Discard wait value for DMA.
	}
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	// This should be for VRAM MODE, with ROMs (000C8000h - 000CFFFFh)
	int dummywait;
	__LIKELY_IF(wait != NULL) {
		*wait = vram_wait_val; // ToDo
	}
	switch(addr & 0xfffff000) {
	case 0x000c8000: // TEXT VRAM(ANK)
		__LIKELY_IF(d_sprite != NULL) {
			return d_sprite->read_memory_mapped_io8w(addr - 0xc8000, &dummywait);
		}
		break;
	case 0x000c9000: // TEXT VRAM(RESERVED)
		__LIKELY_IF(!(ankcg_enabled) && (d_sprite != NULL)) {
			return d_sprite->read_memory_mapped_io8w(addr - 0xc8000,  &dummywait);
		}
		break;
	case 0x000ca000: // FONT / TEXT VRAM(KANJI)
	case 0x000cb000: // FONT / TEXT VRAM(KANJI)
		if(ankcg_enabled) {
			__LIKELY_IF(d_font != NULL) {
				return d_font->read_memory_mapped_io8(addr);
			}
		} else {
			__LIKELY_IF(d_sprite != NULL) {
				return d_sprite->read_memory_mapped_io8w(addr - 0xc8000,  &dummywait);
			}
		}
		break;
	case 0x000cf000: // FONT / TEXT VRAM(KANJI)
		__UNLIKELY_IF(addr >= 0x000cff80) {
			return read_fmr_ports8(addr);
		} else __LIKELY_IF(extra_ram != NULL) {
				return extra_ram[addr];
		}
		break;
	default:
		__LIKELY_IF(extra_ram != NULL) {
			return extra_ram[addr];
		}
		break;
	}
	return 0xff;
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io16w(uint32_t addr, int* wait)
{
	pair16_t w;
	int wait1, wait2;
	w.b.l = read_memory_mapped_io8w(addr    , &wait1);
	w.b.h = read_memory_mapped_io8w(addr + 1, &wait2);
	__LIKELY_IF(wait != NULL) {
		*wait = wait1 + wait2;
	}
	return w.w;
}

uint32_t TOWNS_MEMORY::read_memory_mapped_io32w(uint32_t addr, int* wait)
{
	pair32_t d;
	int wait1, wait2, wait3, wait4;
	d.b.l  = read_memory_mapped_io8w(addr    , &wait1);
	d.b.h  = read_memory_mapped_io8w(addr + 1, &wait2);
	d.b.h2 = read_memory_mapped_io8w(addr + 2, &wait3);
	d.b.h3 = read_memory_mapped_io8w(addr + 3, &wait4);
	__LIKELY_IF(wait != NULL) {
		*wait = wait1 + wait2 + wait3 + wait4;
	}
	return d.d;
}


void TOWNS_MEMORY::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = vram_wait_val; // ToDo
	}
	int dummywait;
	__UNLIKELY_IF((addr < 0x000c8000) || (addr >= 0x000d0000)) {
		return;
	}
	switch(addr & 0xfffff000) {
	case 0x000c8000: //
	case 0x000c9000: //
	case 0x000ca000:
		__LIKELY_IF(d_sprite != NULL) {
			d_sprite->write_memory_mapped_io8w(addr - 0xc8000, data,  &dummywait);
			d_sprite->write_signal(SIG_TOWNS_SPRITE_TVRAM_ENABLED, 0xffffffff, 0xffffffff);
		}
		break;
	case 0x000cf000:
		__LIKELY_IF(addr < 0xcff80) {
			__LIKELY_IF(extra_ram != NULL) {
				extra_ram[addr] = data;
			}
			return;
		} else { // I/O
			write_fmr_ports8(addr, data);
			return;
		}
		break;
	default: // 000CB000h - 000CEFFFh
		__LIKELY_IF(extra_ram != NULL) {
			extra_ram[addr] = data;
		}
		break;
	}
	return;
}

void TOWNS_MEMORY::write_memory_mapped_io16w(uint32_t addr, uint32_t data, int* wait)
{
	pair16_t w;
	int wait1, wait2;
	w.w = data;
	write_memory_mapped_io8w(addr    , w.b.l, &wait1);
	write_memory_mapped_io8w(addr + 1, w.b.h, &wait2);
	__LIKELY_IF(wait != NULL) {
		*wait = wait1 + wait2;
	}
}

void TOWNS_MEMORY::write_memory_mapped_io32w(uint32_t addr, uint32_t data, int* wait)
{
	pair32_t d;
	int wait1, wait2, wait3, wait4;
	d.d = data;
	write_memory_mapped_io8w(addr    , d.b.l,  &wait1);
	write_memory_mapped_io8w(addr + 1, d.b.h,  &wait2);
	write_memory_mapped_io8w(addr + 1, d.b.h2, &wait3);
	write_memory_mapped_io8w(addr + 1, d.b.h3, &wait4);
	__LIKELY_IF(wait != NULL) {
		*wait = wait1 + wait2 + wait3 + wait4;
	}
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
		#if 1
		config_page_c0_e0(true, false, true);
		config_page_f8(true, true);

		__LIKELY_IF(d_cpu != NULL) {
			d_cpu->set_address_mask(0xffffffff);
		}
		__LIKELY_IF(d_dmac != NULL) {
			uint8_t wrap_val = 0xff; // WRAP ON
			d_dmac->write_signal(SIG_TOWNS_DMAC_WRAP, wrap_val, 0xff);
		}
		#endif
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

#define STATE_VERSION	8

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


	state_fio->StateValue(select_f8_rom);
	state_fio->StateValue(select_d0_dict);
	state_fio->StateValue(ankcg_enabled);

	state_fio->StateValue(vram_wait_val);
	state_fio->StateValue(mem_wait_val);
	state_fio->StateValue(vram_size);
	state_fio->StateValue(cpu_clock_val);

	if(loading) {
		update_machine_features(); // Update MISC3, MISC4 by MACHINE ID.

		uint32_t length_tmp = state_fio->FgetUint32_LE();
		unset_range_rw(0x00100000, 0x3fffffff);
		if(extra_ram != NULL) {
			free(extra_ram);
			extra_ram = NULL;
		}
		length_tmp = length_tmp & 0x3ff00000;
		extram_size = length_tmp;
		extra_ram = (uint8_t*)malloc(length_tmp + 0x00100000);
		__LIKELY_IF(extra_ram != NULL) {
			set_region_memory_rw(0x00000000, (extram_size + 0x00100000) - 1, extra_ram, 0);
			memset(extra_ram, 0x00, extram_size + 0x00100000);
		}

		if(extra_ram == NULL) {
			extram_size = 0;
			return false;
		} else {
			state_fio->Fread(extra_ram, extram_size + 0x00100000, 1);
			//set_memory_rw(0x00100000, (extram_size + 0x00100000) - 1, extra_ram);
		}
		config_page_c0_e0(dma_is_vram, select_d0_dict, true);
		config_page_f8(select_f8_rom, true);
		set_wait_values();
		//config_page00();
	} else {
		// At saving
		if(extra_ram == NULL) {
			state_fio->FputUint32_LE(0);
		} else {
			state_fio->FputUint32_LE(extram_size & 0x3ff00000);
			state_fio->Fwrite(extra_ram, extram_size + 0x00100000, 1);
		}
	}

	// ToDo: Do save ROMs?
	return true;
}

}
