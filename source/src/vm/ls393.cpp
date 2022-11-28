/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ 74LS393 ]
*/

#include "ls393.h"

void LS393::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool signal = ((data & mask) != 0);
	if(prev_in && !signal) {
		int prev_count = count++;
		for(int i = 0; i < 8; i++) {
			if(outputs[i].count) {
				int bit = 1 << i;
				if((prev_count & bit) != (count & bit)) {
					uint32_t val = (count & bit) ? 0xffffffff : 0;
					write_signals(&outputs[i], val);
				}
			}
		}
	}
	prev_in = signal;
}

#define STATE_VERSION	1

bool LS393::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(count);
	state_fio->StateValue(prev_in);
	return true;
}

