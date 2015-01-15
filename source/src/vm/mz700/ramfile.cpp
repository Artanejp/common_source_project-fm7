/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2010.09.02 -

	[ ram file ]
*/

#include "ramfile.h"
#include "../../fileio.h"

#define DATA_SIZE	0x10000
#define ADDR_MASK	(DATA_SIZE - 1)

void RAMFILE::initialize()
{
	// init memory
	data_buffer = (uint8 *)malloc(DATA_SIZE);
	memset(data_buffer, 0, DATA_SIZE);
}

void RAMFILE::release()
{
	// release memory
	free(data_buffer);
}

void RAMFILE::reset()
{
	data_addr = 0;
}

void RAMFILE::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xea:
		data_buffer[(data_addr++) & ADDR_MASK] = data;
		break;
	case 0xeb:
		data_addr = (addr & 0xff00) | data;
		break;
	}
}

uint32 RAMFILE::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xea:
		return data_buffer[(data_addr++) & ADDR_MASK];
	}
	return 0xff;
}

#define STATE_VERSION	1

void RAMFILE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(data_buffer, DATA_SIZE, 1);
	state_fio->FputUint32(data_addr);
}

bool RAMFILE::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(data_buffer, DATA_SIZE, 1);
	data_addr = state_fio->FgetUint32();
	return true;
}

