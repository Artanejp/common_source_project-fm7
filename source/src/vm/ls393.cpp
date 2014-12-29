/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ 74LS393 ]
*/

#include "ls393.h"
#include "../fileio.h"

void LS393::write_signal(int id, uint32 data, uint32 mask)
{
	bool signal = ((data & mask) != 0);
	if(prev_in && !signal) {
		int prev_count = count++;
		for(int i = 0; i < 8; i++) {
			if(outputs[i].count) {
				int bit = 1 << i;
				if((prev_count & bit) != (count & bit)) {
					uint32 val = (count & bit) ? 0xffffffff : 0;
					write_signals(&outputs[i], val);
				}
			}
		}
	}
	prev_in = signal;
}

#define STATE_VERSION	1

void LS393::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint32(count);
	state_fio->FputBool(prev_in);
}

bool LS393::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	count = state_fio->FgetUint32();
	prev_in = state_fio->FgetBool();
	return true;
}

