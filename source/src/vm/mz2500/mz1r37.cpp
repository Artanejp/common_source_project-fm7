/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ MZ-1R37 (640KB EMM) ]
*/

#include "mz1r37.h"

#define EMM_SIZE	(640 * 1024)

void MZ1R37::initialize()
{
	buffer = (uint8_t*)calloc(EMM_SIZE, sizeof(uint8_t));
	address = 0;
}

void MZ1R37::release()
{
	if(buffer != NULL) {
		free(buffer);
	}
}

void MZ1R37::write_io8(uint32_t addr, uint32_t data)
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

uint32_t MZ1R37::read_io8(uint32_t addr)
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

#include "../../statesub.h"

void MZ1R37::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_INT32(tmp_buffer_size);
	DECL_STATE_ENTRY_VARARRAY_VAR(buffer, tmp_buffer_size);
	DECL_STATE_ENTRY_UINT32(address);
	
	leave_decl_state();
}

void MZ1R37::save_state(FILEIO* state_fio)
{
	tmp_buffer_size = EMM_SIZE;
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(buffer, EMM_SIZE, 1);
//	state_fio->FputUint32(address);
}

bool MZ1R37::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(buffer, EMM_SIZE, 1);
//	address = state_fio->FgetUint32();
	return true;
}

