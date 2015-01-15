/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

	[ psg ]
*/

#include <math.h>
#include "psg.h"
#include "../../fileio.h"

#define PSG_CLOCK
#define PSG_VOLUME	8192

void PSG::reset()
{
	memset(ch, 0, sizeof(ch));
}

void PSG::write_io8(uint32 addr, uint32 data)
{
	ch[addr & 3].period = 0x3f - (data & 0x3f);
}

void PSG::init(int rate)
{
	diff = (int)(1.3 * CPU_CLOCKS / rate);
}

void PSG::mix(int32* buffer, int cnt)
{
	// create sound buffer
	for(int i = 0; i < cnt; i++) {
		int vol = 0;
		for(int j = 0; j < 3; j++) {
			if(!ch[j].period) {
				continue;
			}
			ch[j].count -= diff;
			if(ch[j].count < 0) {
				ch[j].count += ch[j].period << 8;
				ch[j].signal = !ch[j].signal;
			}
			vol += ch[j].signal ? PSG_VOLUME : -PSG_VOLUME;
		}
		*buffer++ += vol; // L
		*buffer++ += vol; // R
	}
}

#define STATE_VERSION	1

void PSG::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 3; i++) {
		state_fio->FputInt32(ch[i].period);
	}
}

bool PSG::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 3; i++) {
		ch[i].period = state_fio->FgetInt32();
	}
	return true;
}

