/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2017.01.01 -

	[memory]
*/

#include "../../fileio.h"
#include "./towns_memory.h"
#include "./towns_vram.h"
#include "../i386.h"

namespace FMTOWNS {
	
void TOWNS_MEMORY::initialize()
{
	MEMORY::initialize();
	
	extra_nmi_mask = true;
	extra_nmi_val = false;
	
	vram_wait_val = 6;
	mem_wait_val = 3;

	set_memory_rw(0x00000000, 0x000bffff, ram_page0);
	set_memory_mapped_io_rw(0x000c0000, 0x000c7fff, d_vram); // PLANE ACCESSED VRAM(EMULATED)
	set_memory_mapped_io_rw(0x000c8000, 0x000c8fff, d_vram); // TEXT VRAM (EMULATED)
	set_memory_mapped_io_rw(0x000c9000, 0x000c9fff, d_vram); // VRAM RESERVED
	set_memory_mapped_io_rw(0x000ca000, 0x000cafff, d_vram); // ANKCG1 / IO / RAM
	set_memory_mapped_io_rw(0x000cb000, 0x000cbfff, d_cgrom);   // ANKCG1 / IO / RAM
	set_memory_mapped_io_rw(0x000cc000, 0x000cffff, d_vram); // MMIO / RAM
	set_memory_mapped_io_rw(0x000d0000, 0x000d7fff, d_dictionary); // DICTIONARY (BANKED)
	set_memory_mapped_io_rw(0x000d8000, 0x000d9fff, d_sram); // SRAM (LEARN/GAIJI)
	unset_memory_rw(0x000da000, 0x000effff); // RESERVED

	set_memory_rw(0x000f0000, 0x000f7fff, ram_pagef);
	set_memory_mapped_io_rw(0x000f8000, 0x000fffff, d_sysrom);    // SYSROM / RAM

	unset_memory_rw(0x00100000, 0x3fffffff);
	extra_ram_size = extra_ram_size & 0x3ff00000;
	if(extra_ram_size >= 0x00100000) {
		extra_ram = malloc(extra_ram_size);
		if(extra_ram != NULL) {
			set_memory_rw(0x00100000, (extra_ram_size + 0x00100000) - 1, extra_ram);
			memset(extra_ram, 0x00, extra_ram_size);
		}
	}		
	memset(ram_page0, 0x00, sizeof(ram_page0));
	memset(ram_pagef, 0x00, sizeof(ram_pagef));

	unset_memory_rw(0x40000000, 0x7fffffff);  // EXTRA SLOT
	set_memory_mapped_io_rw(0x80000000, 0x8007ffff, d_display); // VRAM
	unset_memory_rw(0x80080000, 0x800fffff); // RESERVED VRAM
	set_memory_mapped_io_rw(0x80100000, 0x8017ffff, d_display); // VRAM
	unset_memory_rw(0x80180000, 0x801fffff); // RESERVED VRAM
	unset_memory_rw(0x80200000, 0x80ffffff); // RESERVED VRAM
	set_memory_mapped_io_rw(0x81000000, 0x8101ffff, d_sprite); // SPRITE PATTERN

	unset_memory_rw(0xc0000000, 0xc1ffffff); // Reserved
	if(d_romcard[0] != NULL) {
		set_memory_mapped_io_rw(0xc0000000, 0xc0ffffff, d_romcard[0]); // DICTIONARY ROM
	}
#if 0
	if(d_romcard[1] != NULL) {
		set_memory_mapped_io_rw(0xc1000000, 0xc1ffffff, d_romcard[1]); // DICTIONARY ROM
	}
#endif
	set_memory_mapped_io_rw(0xc2000000, 0xc207ffff, d_msdos); // MSDOS
	set_memory_mapped_io_rw(0xc2080000, 0xc20fffff, d_dictionary); // DICTIONARY ROM
	set_memory_mapped_io_rw(0xc2100000, 0xc213ffff, d_font); // FONT ROM
	set_memory_mapped_io_rw(0xc2140000, 0xc2141fff, d_sram); // LEARN RAM
	unset_memory_rw(0xc2142000, 0xc21ffffff); // Reserved
	
	set_memory_mapped_io_rw(0xc2200000, 0xc2200fff, d_pcm);      // PCM RAM (ToDo:)

	unset_memory_rw(0xc2201000, 0xfffbffff); // Reserved
	set_memory_mapped_io_rw(0xfffc0000, 0xffffffff, d_sysrom);
	
	// load rom image
	// ToDo: More smart.
	vram_size = 0x80000; // OK?
	
	//dma_addr_reg = dma_wrap_reg = 0;
	dma_addr_mask = 0x00ffffff; // ToDo
	
	initialize_tables();
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
	protect = rst = 0;
	// ToDo
	dma_addr_reg = dma_wrap_reg = 0;
	dma_addr_mask = 0x00ffffff;
	d_cpu->set_address_mask(0xffffffff);
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
			//bool __cs = (d_serialrom->read_data8(SIG_SERIALROM_CS) == 0);
			bool __clk = (d_serialrom->read_data8(SIG_SERIALROM_CLK) != 0);
			bool __reset = (d_serialrom->read_data8(SIG_SERIALROM_RESET) != 0);
			bool __dat = (d_serialrom->read_data8(SIG_SERIALROM_DATA) != 0);
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
		val = (bankc0_vram) ? 0x7f : 0xff;
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
			d_serialrom->write_data8(SIG_SERIALROM_CS, ~data, 0x20);
			d_serialrom->write_data8(SIG_SERIALROM_CLK, data, 0x40);
			d_serialrom->write_data8(SIG_SERIALROM_RESET, data, 0x80);
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
		bankc0_vram = ((data & 0x80) != 0);
		break;
	case 0x05c0:
		extra_nmi_mask = ((data & 0x08) == 0);
		break;
	case 0x05ec:
		if(machine_id >= 0x0500) { // Towns2 CX : Is this hidden register after Towns 1F/2F/1H/2H?
			vram_wait_val = ((data & 0x01) != 0) ? 3 : 6;
			mem_wait_val = ((data & 0x01) != 0) ? 0 : 3;
			this->write_signal(SIG_FMTOWNS_SET_VRAMWAIT, vram_wait_val, 0xff);
			this->write_signal(SIG_FMTOWNS_SET_MEMWAIT, mem_wait_val, 0xff);
		}
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
		cvent_wait_1us = -1;
		if(machine_id >= 0x0300) { // After UX*/10F/20F/40H/80H
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		}
		break;
	default:
		break;
	}
	
}

uint32_t TOWNS_MEMORY::read_mmio(uint32_t addr, int *wait, bool *hit)
{
	if(hit != NULL) *hit = false;
	if(wait != NULL) *wait = 0; // OK?
	if(addr >= 0x000d0000) return 0xffffffff;
	if(addr <  0x000cff80) return 0xffffffff;
	uint32_t val = 0xff;
	bool found = false;
	switch(addr & 0x7f) {
	case 0x00:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_CURSOR);
			found = true;
		}
		break;
	case 0x01:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_RAMSELECT);
			found = true;
		}
		break;
	case 0x02:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_DISPMODE);
			found = true;
		}
		break;
	case 0x03:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_FMR_PAGESEL);
			found = true;
		}
		break;
	case 0x04:
		val = 0x7f; // Reserve.FIRQ
		found = true;
		break;
	case 0x06:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_IO_SYNC_STATUS);
			found = true;
		}
		break;
	//case 0x14:
	//case 0x15:
	case 0x16:
	case 0x17:
		if(d_vram != NULL) {
			val = d_vram->read_io8(FMTOWNS_VRAM_KANJICG + (addr & 3));
			found = true;
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			d_beep->write_signal(SIG_BEEP_ON, 1, 1);
			found = true;
		}
		break;
	case 0x19:
		val = val & ((ankcg_enabled) ? 0x00 : 0x01);
		found = true;
		break;
	case 0x20:
		val = 0xff;
		val = val & 0x7f;
		found = true;
		break;
	default:
		break;
	}
	if(hit != NULL) *hit = found;
	return (uint32_t)val;
}

void TOWNS_MEMORY::write_mmio(uint32_t addr, uint32_t data, int *wait, bool *hit)
{
	if(hit != NULL) *hit = false;
	if(wait != NULL) *wait = 0; // OK?
	if(addr >= 0x000d0000) return;
	if(addr <  0x000cff80) return;
	bool found = false;
	switch(addr & 0x7f) {
	case 0x00:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_CURSOR, data);
			found = true;
		}
		break;
	case 0x01:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_RAMSELECT, data);
			found = true;
		}
		break;
	case 0x02:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_DISPMODE, data);
			found = true;
		}
		break;
	case 0x03:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_IO_FMR_PAGESEL, data);
			found = true;
		}
		break;
	case 0x04:
		found = true;
		break;
	case 0x06:
		found = true;
		break;
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
		if(d_vram != NULL) {
			d_vram->write_io8(FMTOWNS_VRAM_KANJICG + (addr & 3), data);
			found = true;
		}
		break;
	case 0x18:
		if(d_beep != NULL) {
			d_beep->write_signal(SIG_BEEP_ON, 0, 1);
			found = true;
		}
		break;
	case 0x19:
	    ankcg_enabled = ((data & 1) == 0);
		found = true;
		break;
	case 0x20:
		found = true;
		break;
	default:
		break;
	}
	if(hit != NULL) *hit = found;
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
	} else if(ch == SIG_FMTOWNS_SET_MEMWAIT) {
		mem_wait_val = (int)data;
		d_sysrom->write_signal(SIG_FMTOWNS_SET_MEMWAIT, data, mask);
		d_dictionary->write_signal(SIG_FMTOWNS_SET_MEMWAIT, data, mask);
		d_msdos->write_signal(SIG_FMTOWNS_SET_MEMWAIT, data, mask);
		d_fonts->write_signal(SIG_FMTOWNS_SET_MEMWAIT, data, mask);
	} else if(ch == SIG_FMTOWNS_SET_VRAMWAIT) {
		vram_wait_val = (int)data;
		d_vram->write_signal(SIG_FMTOWNS_SET_MEMWAIT, data, mask);
	}
}

uint32_t TOWNS_MEMORY::read_signal(int ch)
{
	if(ch == SIG_FMTOWNS_MACHINE_ID) {
		uint16_t d = (machine_id & 0xfff8) | ((uint16_t)(cpu_id & 0x07));
		return (uint32_t)d;
	} else if(ch == SIG_FMTOWNS_SET_MEMWAIT) {
		return (uint32_t)mem_wait_val;
	} else if(ch == SIG_FMTOWNS_SET_VRAMWAIT) {
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
	state_fio->StateValue(bankc0_vram);
	state_fio->StateValue(ankcg_enabled);
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);

	state_fio->StateValue(dma_addr_mask);
	//state_fio->StateValue(dma_addr_reg);
	//state_fio->StateValue(dma_wrap_reg);
	
	state_fio->StateArray(ram_page0, sizeof(ram_page0), 1);
	state_fio->StateArray(ram_pagef, sizeof(ram_pagef), 1);
	if(loading) {
		uint32_t length_tmp = state_fio->FgetUint32_LE();
		if(extra_ram != NULL) {
			free(extra_ram);
			extra_ram = NULL;
		}
		length_tmp = length_tmp & 0x3ff00000;
		extra_ram_size = length_tmp;
		if(length_tmp > 0) {
			extra_ram = (uint8_t*)malloc(length_tmp);
		}
		unset_memory_rw(0x00100000, 0x3fffffff);
		if(extra_ram == NULL) {
			extra_ram_size = 0;
			return false;
		} else {
			state_fio->Fread(extra_ram, extra_ram_size, 1);
			set_memory_rw(0x00100000, (extra_ram_size + 0x00100000) - 1, extra_ram);
		}
		
	} else {
		if(extra_ram == NULL) {
			state_fio->FputUint32_LE(0);
		} else {
			state_fio->FputUint32_LE(extra_ram_size & 0x3ff00000);
			state_fio->Fwrite(extra_ram, extra_ram_size, 1);
		}
	}
			
	state_fio->StateValue(vram_wait_val);
	state_fio->StateValue(mem_wait_val);
	state_fio->StateValue(vram_size);

	// ToDo: Do save ROMs?

	if(loading) {
		initialize_tables();
	}
	return true;
}

}
