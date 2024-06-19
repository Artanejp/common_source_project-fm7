/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ memory bus ]
*/

#include "../z80.h"
#include "membus.h"

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	memset(ram, 0x00, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	
	read_bios(_T("BOOT.ROM"), rom, sizeof(rom));
	
	dma_bank = false;
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
	rom_selected = true;
	page = false;
	page_after_jump = after_jump = false;
	page_exchange = false;
	update_bank();
}

uint32_t MEMBUS::fetch_op(uint32_t addr, int *wait)
{
	uint32_t val = MEMORY::read_data8(addr);
	
	if(after_jump) {
		if(page != page_after_jump) {
			page = page_after_jump;
			update_bank();
		}
		after_jump = false;
	} else if(val == 0xc3) {
		after_jump = true;
	}
	return val;
}

uint32_t MEMBUS::read_data8w(uint32_t addr, int *wait)
{
	if(page_exchange) {
		page_exchange = false;
		d_cpu->set_intr_enb(true);
		return ram[(page ? 0 : 0x10000) | (addr & 0xffff)];
	}
	return MEMORY::read_data8w(addr, wait);
}

void MEMBUS::write_data8w(uint32_t addr, uint32_t data, int *wait)
{
	if(page_exchange) {
		page_exchange = false;
		d_cpu->set_intr_enb(true);
		ram[(page ? 0 : 0x10000) | (addr & 0xffff)] = data;
		return;
	}
	MEMORY::write_data8w(addr, data, wait);
}

uint32_t MEMBUS::read_dma_data8w(uint32_t addr, int* wait)
{
	*wait = 0;
	return ram[(dma_bank ? 0x10000 : 0) | (addr & 0xffff)];
}

void MEMBUS::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	ram[(dma_bank ? 0x10000 : 0) | (addr & 0xffff)] = data;
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xd0:
		return (~config.dipswitch) & 0xff;
	case 0xd1:
		return page ? 1 : 0;
	}
	return 0xff;
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xb6:
	case 0xc6:
	case 0xce:
		dma_bank = ((data & 1) != 0);
		break;
	case 0xcf:
		if(rom_selected) {
			rom_selected = false;
			update_bank();
		}
		break;
	case 0xd0:
		page_after_jump = false;
		break;
	case 0xd1:
		page_after_jump = true;
		break;
	case 0xd2:
		page_exchange = true;
		d_cpu->set_intr_enb(false);
		break;
	}
}

void MEMBUS::update_bank()
{
	if(page) {
		set_memory_rw(0x0000, 0xffff, ram + 0x10000);
	} else {
		set_memory_rw(0x0000, 0xffff, ram);
	}
	if(rom_selected) {
		set_memory_r(0x0000, sizeof(rom) - 1, rom);
		unset_memory_w(0x0000, sizeof(rom) - 1);
	}
}

#define STATE_VERSION	1

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!MEMORY::process_state(state_fio, loading)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(rom_selected);
	state_fio->StateValue(page);
	state_fio->StateValue(page_after_jump);
	state_fio->StateValue(after_jump);
	state_fio->StateValue(page_exchange);
	state_fio->StateValue(dma_bank);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

