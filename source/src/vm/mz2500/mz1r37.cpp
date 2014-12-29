/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ MZ-1R37 (640KB EMM) ]
*/

#include "mz1r37.h"
#include "../../fileio.h"

#define EMM_SIZE	(640 * 1024)

void MZ1R37::initialize()
{
	buffer = (uint8*)calloc(EMM_SIZE, sizeof(uint8));
	address = 0;
}

void MZ1R37::release()
{
	if(buffer != NULL) {
		free(buffer);
	}
}

void MZ1R37::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xac:
		address = ((addr & 0xff00) << 8) | (data << 8) | (address & 0x0000ff);
		break;
	case 0xad:
		address = (address & 0xffff00) | (addr >> 8);
		if(address < EMM_SIZE) {
			buffer[address] = data;
		}
		break;
	}
}

uint32 MZ1R37::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0xad:
		address = (address & 0xffff00) | (addr >> 8);
		if(address < EMM_SIZE) {
			return buffer[address];
		}
		break;
	}
	return 0xff;
}

#define STATE_VERSION	1

void MZ1R37::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(buffer, EMM_SIZE, 1);
	state_fio->FputUint32(address);
}

bool MZ1R37::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(buffer, EMM_SIZE, 1);
	address = state_fio->FgetUint32();
	return true;
}

