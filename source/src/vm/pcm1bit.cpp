/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.02.09 -

	[ 1bit PCM ]
*/

#include "pcm1bit.h"

void PCM1BIT::initialize()
{
	signal = false;
	on = true;
	mute = false;
	changed = 0;
	last_vol = 0;
	
	register_frame_event(this);
}

void PCM1BIT::reset()
{
	prev_clock = current_clock();
	positive_clocks = negative_clocks = 0;
}

void PCM1BIT::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PCM1BIT_SIGNAL) {
		bool next = ((data & mask) != 0);
		if(signal != next) {
			if(signal) {
				positive_clocks += passed_clock(prev_clock);
			} else {
				negative_clocks += passed_clock(prev_clock);
			}
			prev_clock = current_clock();
			// mute if signal is not changed in 2 frames
			changed = 2;
			signal = next;
		}
	} else if(id == SIG_PCM1BIT_ON) {
		on = ((data & mask) != 0);
	} else if(id == SIG_PCM1BIT_MUTE) {
		mute = ((data & mask) != 0);
	}
}

void PCM1BIT::event_frame()
{
	if(changed) {
		changed--;
	}
}

void PCM1BIT::mix(int32* buffer, int cnt)
{
	if(on && !mute && changed) {
		if(signal) {
			positive_clocks += passed_clock(prev_clock);
		} else {
			negative_clocks += passed_clock(prev_clock);
		}
		int clocks = positive_clocks + negative_clocks;
		last_vol = clocks ? (max_vol * positive_clocks - max_vol * negative_clocks) / clocks : signal ? max_vol : -max_vol;
		
		for(int i = 0; i < cnt; i++) {
			*buffer++ += last_vol; // L
			*buffer++ += last_vol; // R
		}
	} else if(last_vol > 0) {
		// suppress petite noise when go to mute
		for(int i = 0; i < cnt && last_vol != 0; i++, last_vol--) {
			*buffer++ += last_vol; // L
			*buffer++ += last_vol; // R
		}
	} else if(last_vol < 0) {
		// suppress petite noise when go to mute
		for(int i = 0; i < cnt && last_vol != 0; i++, last_vol++) {
			*buffer++ += last_vol; // L
			*buffer++ += last_vol; // R
		}
	}
	prev_clock = current_clock();
	positive_clocks = negative_clocks = 0;
}

void PCM1BIT::init(int rate, int volume)
{
	max_vol = volume;
}

#define STATE_VERSION	2

void PCM1BIT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(signal);
	state_fio->FputBool(on);
	state_fio->FputBool(mute);
	state_fio->FputInt32(changed);
	state_fio->FputUint32(prev_clock);
	state_fio->FputInt32(positive_clocks);
	state_fio->FputInt32(negative_clocks);
}

bool PCM1BIT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	signal = state_fio->FgetBool();
	on = state_fio->FgetBool();
	mute = state_fio->FgetBool();
	changed = state_fio->FgetInt32();
	prev_clock = state_fio->FgetUint32();
	positive_clocks = state_fio->FgetInt32();
	negative_clocks = state_fio->FgetInt32();
	
	// post process
	last_vol = 0;
	return true;
}

