/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom]
*/

#include "debugger.h"
#include "serialrom.h"

void SERIALROM::initialize()
{
	DEVICE::initialize();
	rom_addr = 0;
	reset_reg = false; // NOTE: Initial state is UNSTABLE.
	prev_reset = false;
	clk = false;
	cs = true;
	addr_mask = 0;
	bit_mask = 0;
	if((rom_size > 0) && (rom == NULL)) {
		allocate_memory(rom_size);
	}
	if((__USE_DEBUGGER) && (d_debugger != NULL)) {
//		d_mem_stored = d_mem;
		d_debugger->set_context_mem(this);
	}
}

void SERIALROM::release()
{
	if(rom != NULL) {
		free(rom);
		rom = NULL;
	}
}

void SERIALROM::pos2addr(uint32_t pos, uint32_t& nibble, uint8_t& bit)
{
	__UNLIKELY_IF(addr_mask == 0) {
		nibble = 0;
		bit = 0;
		return;
	}
	nibble = addr_mask - ((pos >> 3) & addr_mask);
	bit = pos & 7;
	return;
}

uint32_t SERIALROM::read_rom_bit(uint32_t pos)
{
	__UNLIKELY_IF(rom == NULL) {
		return 0;
	}
	uint32_t addr;
	uint8_t  bit;
	pos2addr(pos, addr, bit);

	uint32_t val = ((rom[addr] & (1 << (bit & 7))) != 0) ? 0xffffffff : 0x00000000;
	return val;
}


uint32_t SERIALROM::load_data(const uint8_t* data, uint32_t size)
{
	if((size == 0) || (rom_size == 0) || (data == NULL) || (rom == NULL)) {
		return 0;
	}
	if(size > rom_size) {
		size = rom_size;
	}
	memcpy(rom, data, size);
	return size;
}

bool SERIALROM::is_initialized(uint32_t size)
{
	if((rom != NULL) && (rom_size == size) && (size != 0)) {
		return true;
	}
	return false;
}

uint32_t SERIALROM::allocate_memory(uint32_t size)
{
	if(size == 0) {
		return 0;
	}
	uint32_t realsize = 1;
	for(int i = 0; i < (32 - 3); i++) {
		if(size <= realsize) {
			break;
		}
		realsize <<= 1;
	}
	if(realsize != 0) {
		if(rom != NULL) {
			free(rom);
			rom = NULL;
		}
		rom = (uint8_t*)malloc(realsize);
		if(rom != NULL) {
			rom_size = realsize;
			addr_mask = rom_size - 1;
			bit_mask = (addr_mask << 3) | 0x07;
		}
	}
	return rom_size;
}

void SERIALROM::check_and_reset_device()
{
	__UNLIKELY_IF((cs) && !(reset_reg) && (prev_reset)) {
		rom_addr = 0;
		prev_reset = false;
	}
}

void SERIALROM::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SERIALROM_CLK:
		if((cs) && !(reset_reg)) {
			if(!(clk)) {
				if((data & mask) != 0) { // RISE UP
					rom_addr = (rom_addr + 1) & bit_mask;
				}
			}
		}
		clk = ((data & mask) != 0) ? true : false;
		break;
	case SIG_SERIALROM_CS:
		cs = ((data & mask) != 0) ? true : false;
		//check_and_reset_device();
		break;
	case SIG_SERIALROM_RESET:
		prev_reset = reset_reg;
		reset_reg = ((data & mask) != 0) ? true : false;
		check_and_reset_device();
		break;
	}
}

uint32_t SERIALROM::read_signal(int ch)
{
	switch(ch) {
	case SIG_SERIALROM_CLK:
		return ((clk) ? 0xffffffff : 0x00000000);
		break;
	case SIG_SERIALROM_CS:
		return ((cs) ? 0xffffffff : 0x00000000);;
		break;
	case SIG_SERIALROM_RESET:
		return ((reset_reg) ? 0xffffffff : 0x00000000);
		break;
	case SIG_SERIALROM_DATA:
		return read_rom_bit(rom_addr);
		break;
	}
	return 0;
}

uint32_t SERIALROM::read_debug_data8(uint32_t addr)
{
	__UNLIKELY_IF((rom == NULL) || (addr_mask == 0) || (rom_size == 0)) {
		return 0;
	}
	return rom[addr & addr_mask];
}

void SERIALROM::write_debug_data8(uint32_t addr, uint32_t data)
{
	__UNLIKELY_IF((rom == NULL) || (addr_mask == 0) || (rom_size == 0)) {
		return;
	}
	rom[addr & addr_mask] = data;
}

bool SERIALROM::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	_TCHAR numseg[8] = {'\0'};
	int noff = 0;
	// ToDo: Implement
	return false;
}

bool SERIALROM::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{

	// Dump raw value
	uint32_t nibble;
	uint8_t bit;
	pos2addr(rom_addr, nibble, bit);

	my_tcscat_s(buffer, buffer_len,
				create_string(_T("\nSIZE=%d bytes\nCS=%s CLK=%s RESET REG=%s(PREV=%s)\nROM ADDR=%d (%08Xh) MASK=%08X\nROM BIT POSITION=%02d word + %01d bit\n\n"),
							  (rom == NULL) ? 0 : rom_size,
							  (cs) ? _T("ON ") : _T("OFF"),
							  (clk) ? _T("ON ") : _T("OFF"),
							  (reset_reg) ? _T("ON ") : _T("OFF"),
							  (prev_reset) ? _T("ON ") : _T("OFF"),
							  rom_addr,
							  rom_addr,
							  addr_mask,
							  nibble, bit)
		);
	return true;
}

#define STATE_VERSION	1

bool SERIALROM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(cs);
	state_fio->StateValue(clk);
	state_fio->StateValue(reset_reg);
	state_fio->StateValue(prev_reset);

	state_fio->StateValue(rom_size);
	state_fio->StateValue(rom_addr);
	if(loading) {
		if(rom != NULL) {
			free(rom);
			rom = NULL;
		}
		addr_mask = 0;
		bit_mask = 0;
		if(rom_size != 0) {
			rom = (uint8_t*)malloc(rom_size);
			if(rom != NULL) {
				addr_mask = rom_size - 1;
				bit_mask = (addr_mask << 3) | 0x07;
			}
		}
	}
	if((rom != NULL) && (rom_size != 0)) {
		state_fio->StateArray(rom, sizeof(rom), rom_size);
	}
	return true;
}
