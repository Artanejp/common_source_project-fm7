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
	realtime = false;
	changed = 0;
	last_vol_l = last_vol_r = 0;
	
	register_frame_event(this);
}

void PCM1BIT::reset()
{
	prev_clock = get_current_clock();
	positive_clocks = negative_clocks = 0;
}

void PCM1BIT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_PCM1BIT_SIGNAL) {
		bool next = ((data & mask) != 0);
		if(signal != next) {
			if(signal) {
				positive_clocks += get_passed_clock(prev_clock);
			} else {
				negative_clocks += get_passed_clock(prev_clock);
			}
			prev_clock = get_current_clock();
			// mute if signal is not changed in 2 frames
			changed = 2;
			update_realtime_render();
			signal = next;
		}
	} else if(id == SIG_PCM1BIT_ON) {
		touch_sound();
		on = ((data & mask) != 0);
		update_realtime_render();
	} else if(id == SIG_PCM1BIT_MUTE) {
		touch_sound();
		mute = ((data & mask) != 0);
		update_realtime_render();
	}
}

void PCM1BIT::event_frame()
{
	if(changed > 0 && --changed == 0) {
		update_realtime_render();
	}
}

void PCM1BIT::update_realtime_render()
{
	bool value = (on && !mute && changed != 0);
	
	if(realtime != value) {
		set_realtime_render(this, value);
		realtime = value;
	}
}

void PCM1BIT::mix(int32_t* buffer, int cnt)
{
	if(on && !mute && changed) {
		if(signal) {
			positive_clocks += get_passed_clock(prev_clock);
		} else {
			negative_clocks += get_passed_clock(prev_clock);
		}
		int clocks = positive_clocks + negative_clocks;
		int sample = clocks ? (max_vol * positive_clocks - max_vol * negative_clocks) / clocks : signal ? max_vol : -max_vol;
		
		last_vol_l = apply_volume(sample, volume_l);
		last_vol_r = apply_volume(sample, volume_r);
		
		for(int i = 0; i < cnt; i++) {
			*buffer++ += last_vol_l; // L
			*buffer++ += last_vol_r; // R
		}
	} else {
		// suppress petite noise when go to mute
		for(int i = 0; i < cnt; i++) {
			*buffer++ += last_vol_l; // L
			*buffer++ += last_vol_r; // R
			
			if(last_vol_l > 0) {
				last_vol_l--;
			} else if(last_vol_l < 0) {
				last_vol_l++;
			}
			if(last_vol_r > 0) {
				last_vol_r--;
			} else if(last_vol_r < 0) {
				last_vol_r++;
			}
		}
	}
	prev_clock = get_current_clock();
	positive_clocks = negative_clocks = 0;
}

void PCM1BIT::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void PCM1BIT::initialize_sound(int rate, int volume)
{
	max_vol = volume;
}

#define STATE_VERSION	3

bool PCM1BIT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(signal);
	state_fio->StateValue(on);
	state_fio->StateValue(mute);
	state_fio->StateValue(realtime);
	state_fio->StateValue(changed);
	state_fio->StateValue(prev_clock);
	state_fio->StateValue(positive_clocks);
	state_fio->StateValue(negative_clocks);
	
	// post process
	if(loading) {
		last_vol_l = last_vol_r = 0;
	}
	return true;
}

