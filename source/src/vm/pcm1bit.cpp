/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.02.09 -

	[ 1bit PCM ]
*/

#include "pcm1bit.h"

void PCM1BIT::initialize()
{
	DEVICE::initialize();
	signal = false;
	on = true;
	mute = false;
	realtime = false;
	changed = 0;
	last_vol_l = last_vol_r = 0;
	
	register_frame_event(this);
	
	sample_old = 0;
}

void PCM1BIT::reset()
{
	prev_clock = get_current_clock();
	positive_clocks = negative_clocks = 0;
	lpf_mod_val = 0;
	lpf_skip_val = lpf_skip_factor;
	
	int clocks = positive_clocks + negative_clocks;
	int sample = clocks ? (max_vol * positive_clocks - max_vol * negative_clocks) / clocks : signal ? max_vol : -max_vol;
	last_vol_l = apply_volume(sample, volume_l);
	last_vol_r = apply_volume(sample, volume_r);
	sample_old = sample;
}

void PCM1BIT::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_PCM1BIT_SIGNAL) {
		bool next = ((data & mask) != 0);
		if(signal != next) {
			touch_sound();
			if(signal) {
				positive_clocks += get_passed_clock(prev_clock);
			} else {
				negative_clocks += get_passed_clock(prev_clock);
			}
			prev_clock = get_current_clock();
			// mute if signal is not changed in 2 frames
			changed = 2;
			signal = next;
		}
	} else if(id == SIG_PCM1BIT_ON) {
		touch_sound();
		on = ((data & mask) != 0);
		set_realtime_render(this, on & !mute);
	} else if(id == SIG_PCM1BIT_MUTE) {
		touch_sound();
		mute = ((data & mask) != 0);
		set_realtime_render(this, on & !mute);
	}
}

void PCM1BIT::event_frame()
{
	if(changed) {
		changed--;
	}
}

void PCM1BIT::mix(int32_t* buffer, int cnt)
{
	int32_t* p;
	p = buffer;
	if(on && !mute && changed) {
		if(signal) {
			positive_clocks += get_passed_clock(prev_clock);
		} else {
			negative_clocks += get_passed_clock(prev_clock);
		}
		int clocks = positive_clocks + negative_clocks;
		int sample = clocks ? (max_vol * positive_clocks - max_vol * negative_clocks) / clocks : signal ? max_vol : -max_vol;
		
		int nptr = 0;
		int inc_l = 0;
		int inc_r = 0;
		int sval = 0;;
		if(use_lpf) {
			if(lpf_skip_val < 0) {
				lpf_skip_val = lpf_skip_factor;
				sample_old = sample;
				sval = sample_old;
			} else {
				sval = (sample_old * 3 + 1 * ((sample_old * lpf_skip_val) + (sample) * (lpf_skip_factor - lpf_skip_val)) / lpf_skip_factor + sample * 28) / 32;
				//sval = (sample_old * 3 + sample * 29) / 32;
			}
			last_vol_l = apply_volume(sval, volume_l);
			last_vol_r = apply_volume(sval, volume_r);
		} else {
			sval = 0;
			last_vol_l = apply_volume(sample, volume_l);
			last_vol_r = apply_volume(sample, volume_r);
			sample_old = sample;
		}
		for(int i = 0; i < cnt; i++) {
			p[nptr + 0] += last_vol_l; // L
			p[nptr + 1] += last_vol_r; // R
			nptr += 2;
			if(use_lpf) {
				lpf_skip_val--;
				if(lpf_mod_factor != 0) {
					lpf_mod_val = lpf_mod_val + lpf_mod_factor;
					if(lpf_mod_val >= lpf_src_freq) {
						lpf_mod_val = lpf_mod_val - lpf_src_freq;
						lpf_skip_val++;
					}	
				}
			}
		}
	} else {
		// suppress petite noise when go to mute
		//sample_old = 0;
		int nptr = 0;
		for(int i = 0; i < cnt; i++) {
			p[nptr + 0] += last_vol_l; // L
			p[nptr + 1] += last_vol_r; // R
			nptr += 2;
			
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
			if(use_lpf) {
				lpf_skip_val--;
				if(lpf_mod_factor != 0) {
					lpf_mod_val = lpf_mod_val + lpf_mod_factor;
					if(lpf_mod_val >= lpf_src_freq) {
						lpf_mod_val = lpf_mod_val - lpf_src_freq;
						lpf_skip_val++;
					}
				}
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

void PCM1BIT::initialize_sound(int rate, int volume, int lpf_freq)
{
	sample_rate = rate;
	if((lpf_freq < (rate / 2)) && (lpf_freq > 0)) {
		use_lpf = true;
		lpf_src_freq = lpf_freq;
		lpf_skip_factor = rate / lpf_freq;
		lpf_mod_factor = rate % lpf_freq;
		lpf_mod_val = 0;
		lpf_skip_val = lpf_skip_factor;
	} else {
		use_lpf = false;
		lpf_src_freq = rate;
		lpf_skip_factor = 0;
		lpf_mod_factor = 0;
		lpf_mod_val = 0;
		lpf_skip_val = lpf_skip_factor;
	}
	max_vol = volume;
}

#define STATE_VERSION	4

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

	//state_fio->StateValue(use_lpf);
	//state_fio->StateValue(sample_rate);
	//state_fio->StateValue(lpf_src_freq);
	//state_fio->StateValue(lpf_skip_factor);
	//state_fio->StateValue(lpf_skip_val);
	//state_fio->StateValue(lpf_mod_factor);
	//state_fio->StateValue(lpf_mod_val);
	state_fio->StateValue(sample_old);
	
 	// post process
	if(loading) {
		last_vol_l = last_vol_r = 0;
		set_realtime_render(this, on & !mute);
		//touch_sound();
	}
 	return true;
}
