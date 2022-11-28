/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ CZ-8RB (ROM BASIC board) ]
*/

#include "cz8rb.h"

void CZ8RB::initialize()
{
	memset(data_buffer, 0, sizeof(data_buffer));
	data_addr = 0;

	// load ROM image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("CZ8RB.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(data_buffer, sizeof(data_buffer), 1);
		fio->Fclose();
	}
	delete fio;
}

void CZ8RB::reset()
{
	data_addr = 0;
}

void CZ8RB::write_io8(uint32_t addr, uint32_t data)
{
	// The order of the ports is the reverse of EMM

	switch(addr & 3) {
	case 0:
		data_addr = (data_addr & 0x00ffff) | (data << 16);
		break;
	case 1:
		data_addr = (data_addr & 0xff00ff) | (data << 8);
		break;
	case 2:
		data_addr = (data_addr & 0xffff00) | data;
		break;
	case 3:
		// You can't write to any ROMs. :-)
		break;
	}
}

uint32_t CZ8RB::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	
	switch(addr & 3) {
	case 3:
		if(data_addr < CZ8RB_BUFFER_SIZE) {
			data = data_buffer[data_addr];
		}
		// no auto address increment
		return data;
	}
	return 0xff;
}

#define STATE_VERSION	1

bool CZ8RB::process_state(FILEIO* state_fio, bool loading)
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
