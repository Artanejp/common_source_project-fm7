/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ 8bit PCM ]
*/

#include "pcm8bit.h"

void PCM8BIT::initialize()
{
	sample = prev_sample = 0;
	on = true;
	mute = false;
	realtime = false;
	changed = 0;
	change_clock = 0;//get_current_clock();
	last_vol_l = last_vol_r = 0;
	
	register_frame_event(this);
}

void PCM8BIT::reset()
{
	prev_clock = get_current_clock();
}

void PCM8BIT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_PCM8BIT_SAMPLE || id == SIG_PRINTER_DATA) {
		// this device may be connected to printer port
		int next = data & mask;
		if(sample != next) {
			// mute if signal is not changed in 2 frames
			changed = 2;
			update_realtime_render();
			
			sample = next;
			change_clock = get_current_clock();
		}
	} else if(id == SIG_PCM8BIT_ON) {
		touch_sound();
		on = ((data & mask) != 0);
		update_realtime_render();
	} else if(id == SIG_PCM8BIT_MUTE) {
		touch_sound();
		mute = ((data & mask) != 0);
		update_realtime_render();
	}
}

void PCM8BIT::event_frame()
{
	if(changed > 0 && --changed == 0) {
		update_realtime_render();
	}
}

void PCM8BIT::update_realtime_render()
{
	bool value = (on && !mute && changed != 0);
	
	if(realtime != value) {
		set_realtime_render(this, value);
		realtime = value;
	}
}

void PCM8BIT::mix(int32_t* buffer, int cnt)
{
	if(on && !mute && changed) {
		uint32_t cur_clock = get_current_clock();
		int cur_sample;
		
		if(change_clock > prev_clock) {
			cur_sample  = prev_sample * (change_clock - prev_clock) + sample * (cur_clock - change_clock);
			cur_sample /= cur_clock - prev_clock;
		} else {
			cur_sample = sample;
		}
		prev_sample = sample;
		prev_clock = cur_clock;
		
		int volume = max_vol * cur_sample / 256;
		
		last_vol_l = apply_volume(volume, volume_l);
		last_vol_r = apply_volume(volume, volume_r);
		
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
}

void PCM8BIT::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void PCM8BIT::initialize_sound(int rate, int volume)
{
	max_vol = volume;
}

#define STATE_VERSION	2

bool PCM8BIT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(on);
	state_fio->StateValue(mute);
	state_fio->StateValue(realtime);
	state_fio->StateValue(changed);
	state_fio->StateValue(sample);
	state_fio->StateValue(prev_sample);
	state_fio->StateValue(prev_clock);
	state_fio->StateValue(change_clock);
	
	// post process
	if(loading) {
		last_vol_l = last_vol_r = 0;
	}
	return true;
}

