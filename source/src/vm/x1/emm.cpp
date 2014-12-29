/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ emm ]
*/

#include "emm.h"
#include "../../fileio.h"

void EMM::initialize()
{
	memset(data_buffer, 0, sizeof(data_buffer));
}

void EMM::reset()
{
	data_addr = 0;
}

void EMM::write_io8(uint32 addr, uint32 data)
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

uint32 EMM::read_io8(uint32 addr)
{
	uint32 data = 0xff;
	
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

void EMM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(data_buffer, sizeof(data_buffer), 1);
	state_fio->FputUint32(data_addr);
}

bool EMM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(data_buffer, sizeof(data_buffer), 1);
	data_addr = state_fio->FgetUint32();
	return true;
}
