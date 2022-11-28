/*
 * I/O termination dummy [dummydevice.cpp]
 * 
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jul 30, 2015 : Initial
 *
 */

//#include "emu.h"
#include "dummydevice.h"

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

#define STATE_VERSION 2

bool DUMMYDEVICE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// Version 1
	{
		state_fio->StateValue(status);
		state_fio->StateValue(clear_on_reset);
		state_fio->StateValue(clear_with_zero);
	}
	return true;
}

