/*
 * I/O termination dummy [dummydevice.cpp]
 * 
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jul 30, 2015 : Initial
 *
 */

#include "dummydevice.h"


DUMMYDEVICE::DUMMYDEVICE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	status = 0x00000000;
	clear_on_reset = true;
	clear_with_zero = true;
}

DUMMYDEVICE::~DUMMYDEVICE()
{
}

uint32_t DUMMYDEVICE::read_signal(int id)
{

	if((id >= SIG_DUMMYDEVICE_BIT0) && (id <= SIG_DUMMYDEVICE_BIT31)) {
		if((status & (1 << (id - SIG_DUMMYDEVICE_BIT0))) != 0) {
			return 0xffffffff;
		} else {
			return 0x00000000;
		}
	} else if(id == SIG_DUMMYDEVICE_READWRITE) {
		return status;
	} else if(id == SIG_DUMMYDEVICE_CLEAR_ON_RESET) {
		return (clear_on_reset) ? 0xffffffff : 0x00000000;
	} else if(id == SIG_DUMMYDEVICE_CLEAR_WITH_ZERO) {
		return (clear_with_zero) ? 0xffffffff : 0x00000000;
	}
	return 0;
}

void DUMMYDEVICE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if((id >= SIG_DUMMYDEVICE_BIT0) && (id <= SIG_DUMMYDEVICE_BIT31)) {
		bool flag = ((data & mask) != 0);
		if(flag) {
			status |= (1 << (id - SIG_DUMMYDEVICE_BIT0));
		} else {
			status &= ~(1 << (id - SIG_DUMMYDEVICE_BIT0));
		}			
	} else if(id == SIG_DUMMYDEVICE_READWRITE) {
		status = data & mask;
	} else if(id == SIG_DUMMYDEVICE_CLEAR_ON_RESET) {
		clear_on_reset = ((data & mask) != 0);
	} else if(id == SIG_DUMMYDEVICE_CLEAR_WITH_ZERO) {
		clear_with_zero = ((data & mask) != 0);
	}
	return;
}

void DUMMYDEVICE::reset()
{
	if(clear_on_reset) {
		if(clear_with_zero) {
			status = 0;
		} else {
			status = 0xffffffff;
		}
	}
}

#define STATE_VERSION 1
void DUMMYDEVICE::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	// Version 1
	{
		state_fio->FputUint32_BE(status);
		state_fio->FputBool(clear_on_reset);
		state_fio->FputBool(clear_with_zero);
	}
}

bool DUMMYDEVICE::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
	// Version 1
	if((version < 1) || (version > STATE_VERSION)) return false;
	{
		status = state_fio->FgetUint32_BE();
		clear_on_reset = state_fio->FgetBool();
		clear_with_zero = state_fio->FgetBool();
	}
	return true;
}
