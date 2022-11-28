/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.08-

	[ memory bus ]
*/

#include "membus.h"

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
	memset(ram, 0x00, sizeof(ram));
	memset(boot, 0xff, sizeof(boot));
	memset(basic, 0xff, sizeof(basic));
	
	if(!read_bios(_T("IPL.ROM"), boot, sizeof(boot))) {
		read_bios(_T("BOOT.ROM"), boot, sizeof(boot));
	}
	read_bios(_T("BASIC.ROM"), basic, sizeof(basic));
	
	set_memory_rw(0x0000, 0xffff, ram);
}

void MEMBUS::reset()
{
	MEMORY::reset();
	
	basic_addr.d = 0;
	ram_selected = false;
	update_bank();
}

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0c:
		basic_addr.b.l = data;
		break;
	case 0x0d:
		basic_addr.b.h = data;
		break;
	case 0x0e:
		// error code ???
		break;
	case 0x80:
		ram_selected = ((data & 0x80) != 0);
		update_bank();
		break;
	}
}

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0c:
		return basic[basic_addr.w.l];
	}
	return 0xff;
}

void MEMBUS::write_dma_data8(uint32_t addr, uint32_t data)
{
	ram[addr & 0xffff] = data;
}

uint32_t MEMBUS::read_dma_data8(uint32_t addr)
{
	return ram[addr & 0xffff];
}

void MEMBUS::update_bank()
{
	set_memory_r(0x0000, 0x07ff, ram_selected ? ram : boot);
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
	state_fio->StateValue(basic_addr.d);
	state_fio->StateValue(ram_selected);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

