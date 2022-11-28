/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.13-

	[ memory mapper ]
*/

#include "mapper.h"
#include "../memory.h"

void MAPPER::initialize()
{
	memset(ram, 0, sizeof(ram));
}

void MAPPER::reset()
{
	mapper_reg = 0x00;
	for(int i = 0; i < 15; i++) {
		bank_reg[i] = i;
		cur_bank[i] = -1;
		update_bank(i);
	}
	bank_reg[15] = 0x01;
}

void MAPPER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xf04f:
		if(mapper_reg != data) {
			mapper_reg = data;
			for(int i = 0; i < 15; i++) {
				update_bank(i);
			}
		}
		break;
	case 0xf050:
	case 0xf051:
	case 0xf052:
	case 0xf053:
	case 0xf054:
	case 0xf055:
	case 0xf056:
	case 0xf057:
	case 0xf058:
	case 0xf059:
	case 0xf05a:
	case 0xf05b:
	case 0xf05c:
	case 0xf05d:
	case 0xf05e:
		if(bank_reg[addr & 0x0f] != data) {
			bank_reg[addr & 0x0f] = data;
			update_bank(addr & 0x0f);
		}
		break;
	case 0xf05f:
		bank_reg[0x0f] = data;
		break;
	}
}

uint32_t MAPPER::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0xf04f:
		return mapper_reg;
	case 0xf050:
	case 0xf051:
	case 0xf052:
	case 0xf053:
	case 0xf054:
	case 0xf055:
	case 0xf056:
	case 0xf057:
	case 0xf058:
	case 0xf059:
	case 0xf05a:
	case 0xf05b:
	case 0xf05c:
	case 0xf05d:
	case 0xf05e:
	case 0xf05f:
		return bank_reg[addr & 0x0f];
	}
	return 0xff;
}

void MAPPER::update_bank(int num)
{
	uint8_t bank = (mapper_reg & 1) ? (bank_reg[num] & 0x1f) : num;
	
	if(cur_bank[num] != bank) {
		cur_bank[num] = bank;
		d_memory->set_memory_rw(0x1000 * num, 0x1000 * num + 0x0fff, ram + 0x1000 * bank);
	}
}

#define STATE_VERSION	1

bool MAPPER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(mapper_reg);
	state_fio->StateArray(bank_reg, sizeof(bank_reg), 1);
//	state_fio->StateArray(cur_bank, sizeof(cur_bank), 1);
	
	// post process
	if(loading) {
		for(int i = 0; i < 15; i++) {
			cur_bank[i] = -1;
			update_bank(i);
		}
	}
	return true;
}

