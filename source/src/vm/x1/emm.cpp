/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ emm ]
*/

#include "emm.h"

void EMM::initialize()
{
	memset(data_buffer, 0, sizeof(data_buffer));
}

void EMM::reset()
{
	data_addr = 0;
}

void EMM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 3) {
	case 0:
		data_addr = (data_addr & 0xffff00) | data;
		break;
	case 1:
		data_addr = (data_addr & 0xff00ff) | (data << 8);
		break;
	case 2:
		data_addr = (data_addr & 0x00ffff) | (data << 16);
		break;
	case 3:
		if(data_addr < EMM_BUFFER_SIZE) {
			data_buffer[data_addr] = data;
		}
		data_addr = (data_addr + 1) & 0xffffff;
		break;
	}
}

uint32_t EMM::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	
	switch(addr & 3) {
	case 3:
		if(data_addr < EMM_BUFFER_SIZE) {
			data = data_buffer[data_addr];
		}
		data_addr = (data_addr + 1) & 0xffffff;
		return data;
	}
	return 0xff;
}

#define STATE_VERSION	1

bool EMM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(data_buffer, sizeof(data_buffer), 1);
	state_fio->StateValue(data_addr);
	return true;
}
