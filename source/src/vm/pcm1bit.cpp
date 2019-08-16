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
	hpf_freq = 1;
	lpf_freq = 48000;
	use_hpf = false;
	use_lpf = false;
	register_frame_event(this);
	before_on = false;
	before_filter_l = (float)0.0f;
	before_filter_r = (float)0.0f;
}

void PCM1BIT::reset()
{
	prev_clock = get_current_clock();
	positive_clocks = negative_clocks = 0;
	before_on = false;
	before_filter_l = (float)0.0f;
	before_filter_r = (float)0.0f;
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
	int32_t p[cnt * 2];
	int32_t p_h[cnt * 2];
	int32_t p_l[cnt * 2];
	int32_t* pp = p;
	if(on && !mute && changed) {
		if(!(before_on)) {
			before_filter_l = (float)last_vol_l;
			before_filter_r = (float)last_vol_r;
			before_on = true;
		}
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
		last_vol_l = apply_volume(sample, volume_l);
		last_vol_r = apply_volume(sample, volume_r);
		for(int i = 0; i < cnt; i++) {
			p[nptr + 0] = last_vol_l; // L
			p[nptr + 1] = last_vol_r; // R
			nptr += 2;
		}
		if(use_lpf) {
			this->calc_low_pass_filter(p_l, pp, cnt, (use_hpf) ? false : true);
			pp = p_l;
		}
		if(use_hpf) {
			this->calc_high_pass_filter(p_h, pp, cnt, true);
			pp = p_h;
		}
	} else {
		// suppress petite noise when go to mute
		int nptr = 0;
		for(int i = 0; i < cnt; i++) {
			p[nptr + 0] = last_vol_l; // L
			p[nptr + 1] = last_vol_r; // R
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
		}
		before_on = false;
	}
	for(int i = 0; i < (cnt * 2); i++) {
		buffer[i] = buffer[i] + pp[i];
		if(buffer[i] >  32767) buffer[i] = 32767;
		if(buffer[i] < -32768) buffer[i] = -32768;
	}
	prev_clock = get_current_clock();
	positive_clocks = negative_clocks = 0;
}
void PCM1BIT::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void PCM1BIT::set_low_pass_filter_freq(int freq, double quality)
{
	if((freq <= 0) || (freq >= (sample_rate / 2))) {
		lpf_freq = sample_rate;
		use_lpf = false;
	} else {
		lpf_freq = freq;
		double isample = 1.0 / (double)sample_rate;
		double ifreq   = 1.0 / ((double)lpf_freq * 2.0 * M_PI);
		lpf_alpha = (float)(isample * quality / ((ifreq + isample)));
		if(lpf_alpha >= 1.0f) lpf_alpha = 1.0f;
		lpf_ialpha = 1.0f - lpf_alpha;
		before_filter_l = (float)last_vol_l;
		before_filter_r = (float)last_vol_r;
		use_lpf = true;
		//printf("LPF_ALPHA=%f\n", lpf_alpha);
	}
}

void PCM1BIT::calc_low_pass_filter(int32_t* dst, int32_t* src, int samples, int is_set_val)
{
	if(samples <= 0) return;
	if(dst == NULL) return;
	if(src == NULL) {
		memset(dst, 0x00, sizeof(int32_t) * samples * 2);
		return;
	}
	__DECL_ALIGNED(16) float tval[(samples + 1) * 2];
	__DECL_ALIGNED(16) float oval[(samples + 1) * 2];
	int __begin = 0;
	
	tval[0] = before_filter_l;
	tval[1] = before_filter_r;
	oval[0] = tval[0];
	oval[1] = tval[1];
	for(int i = 2; i < ((samples + 1) * 2); i++) {
		tval[i] = (float)(src[i - 2]);
	}
	
	for(int i = 2; i < ((samples + 1) * 2); i += 2) {
		oval[i + 0] = tval[i + 0] * lpf_alpha + oval[i - 2 + 0] * lpf_ialpha;
		oval[i + 1] = tval[i + 1] * lpf_alpha + oval[i - 2 + 1] * lpf_ialpha;
	}
	// copy
	for(int i = 2; i < ((samples + 1) * 2) ; i += 2) {
		dst[i - 2 + 0] = (int32_t)(oval[i + 0]);
		dst[i - 2 + 1] = (int32_t)(oval[i + 1]);
	}
	if(is_set_val) {
		before_filter_l = oval[samples * 2 + 0];
		before_filter_r = oval[samples * 2 + 1];
	} else {
		before_filter_l = oval[2 + 0];
		before_filter_r = oval[2 + 1];
	}		
}
	
void PCM1BIT::set_high_pass_filter_freq(int freq, double quality)
{
	if((freq < 0) || (freq >= (sample_rate / 2))) {
		hpf_freq = 1;
		use_hpf = false;
	} else {
		hpf_freq = freq;
		double isample = 1.0 / (double)sample_rate;
		double ifreq   = 1.0 / ((double)hpf_freq * 2.0 * M_PI);
		//hpf_alpha = (float)(ifreq / ((ifreq + isample) * quality));
		hpf_alpha = (float)(isample * quality / ((ifreq + isample)));
		if(hpf_alpha >= 1.0f) hpf_alpha = 1.0f;
		hpf_ialpha = 1.0f - hpf_alpha;
		before_filter_l = (float)last_vol_l;
		before_filter_r = (float)last_vol_r;
		//printf("HPF_ALPHA=%f\n", hpf_alpha);
		use_hpf = true;
	}
}

void PCM1BIT::calc_high_pass_filter(int32_t* dst, int32_t* src, int samples, int is_set_val)
{
	if(samples <= 0) return;
	if(dst == NULL) return;
	if(src == NULL) {
		memset(dst, 0x00, sizeof(int32_t) * samples * 2);
		return;
	}
	__DECL_ALIGNED(16) float tval[(samples + 1) * 2];
	__DECL_ALIGNED(16) float oval[(samples + 1) * 2];
	int __begin = 0;

	tval[0] = before_filter_l;
	tval[1] = before_filter_r;
	oval[0] = tval[0];
	oval[1] = tval[1];
	for(int i = 2; i < ((samples + 1) * 2); i++) {
		tval[i] = (float)(src[i - 2]);
	}
	for(int i = 2; i < ((samples + 1) * 2); i++) {
		oval[i + 0] = tval[i + 0] * hpf_alpha + oval[i - 2 + 0] * hpf_alpha;
		oval[i + 1] = tval[i + 1] * hpf_alpha + oval[i - 2 + 1] * hpf_alpha;
		oval[i + 0] = tval[i + 0] - oval[i + 0];
		oval[i + 1] = tval[i + 1] - oval[i + 1];
	}		
	// copy
	for(int i = 2; i < ((samples + 1) * 2) ; i += 2) {
		dst[i - 2 + 0] = (int32_t)(oval[i + 0]);
		dst[i - 2 + 1] = (int32_t)(oval[i + 1]);
	}
	if(is_set_val) {
		before_filter_l = oval[samples * 2 + 0];
		before_filter_r = oval[samples * 2 + 1];
	} else {
		before_filter_l = oval[2 + 0];
		before_filter_r = oval[2 + 1];
	}
}
	
void PCM1BIT::initialize_sound(int rate, int volume)
{
	sample_rate = rate;
	max_vol = volume;
	if(use_hpf) {
		set_high_pass_filter_freq(hpf_freq);
	}
	if(use_lpf) {
		set_low_pass_filter_freq(lpf_freq);
	}
}

bool PCM1BIT::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len - 1, _T("OUTPUT=%s SIGNAL=%s POSITIVE CLOCK=%d NEGATIVE CLOCK=%d\nLAST VOLUME(L)=%d LAST_VOLUME(R)=%d\Low pass filter=%s FREQ=%d\nHigh pass filter=%s FREQ=%d\n"),
				  (on) ? ((mute) ? _T("ON(MUTE) ") : _T("ON       ")) : ((mute) ? _T("OFF(MUTE)") : _T("OFF      ")),
				  (signal) ? _T("ON") : _T("OFF"),
				  positive_clocks, negative_clocks,
				  last_vol_l, last_vol_r,
				  (use_lpf) ? _T("ON ") : _T("OFF"), lpf_freq,
				  (use_hpf) ? _T("ON ") : _T("OFF"), hpf_freq
		);
	return true;
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

	state_fio->StateValue(use_hpf);
	state_fio->StateValue(use_lpf);
	state_fio->StateValue(hpf_freq);
	state_fio->StateValue(lpf_freq);
	state_fio->StateValue(before_on);
	
 	// post process
	if(loading) {
		last_vol_l = last_vol_r = 0;
		set_realtime_render(this, on & !mute);
		//touch_sound();
		if(use_hpf) {
			set_high_pass_filter_freq(hpf_freq);
		}
		if(use_lpf) {
			set_low_pass_filter_freq(lpf_freq);
		}
	}
 	return true;
}
