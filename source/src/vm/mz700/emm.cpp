/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2010.09.02 -

	[ emm ]
*/

#include "emm.h"

#define DATA_SIZE	0x1000000
#define ADDR_MASK	(DATA_SIZE - 1)

void EMM::initialize()
{
	// init memory
	data_buffer = (uint8_t *)malloc(DATA_SIZE);
	memset(data_buffer, 0xff, DATA_SIZE);
	
	// load emm image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("EMM.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(data_buffer, DATA_SIZE, 1);
		fio->Fclose();
	}
	delete fio;
}

void EMM::release()
{
	// release memory
	free(data_buffer);
}

void EMM::reset()
{
	data_addr = 0;
}

void EMM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x00:
		data_addr = (data_addr & 0xffff00) | data;
		break;
	case 0x01:
		data_addr = (data_addr & 0xff00ff) | (data << 8);
		break;
	case 0x02:
		data_addr = (data_addr & 0x00ffff) | (data << 16);
		break;
	case 0x03:
		data_buffer[(data_addr++) & ADDR_MASK] = data;
		break;
	}
}

uint32_t EMM::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x00:
		return data_addr & 0xff;
	case 0x01:
		return (data_addr >> 8) & 0xff;
	case 0x02:
		return (data_addr >> 16) & 0xff;
	case 0x03:
		return data_buffer[(data_addr++) & ADDR_MASK];
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
	state_fio->StateArray(data_buffer, DATA_SIZE, 1);
	state_fio->StateValue(data_addr);
	return true;
}

