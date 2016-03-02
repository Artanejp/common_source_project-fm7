/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.22 -

	[ beep ]
*/

#include "beep.h"

void BEEP::reset()
{
	signal = true;
	count = 0;
	on = mute = false;
}

void BEEP::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_BEEP_ON) {
		on = ((data & mask) != 0);
	} else if(id == SIG_BEEP_MUTE) {
		mute = ((data & mask) != 0);
	}
}

void BEEP::mix(int32_t* buffer, int cnt)
{
	if(on && !mute) {
		for(int i = 0; i < cnt; i++) {
			int sample = (count < 1024) ? (gen_vol * (count - 512)) / 512 : gen_vol;
			int vol_l = apply_volume(sample, volume_l);
			int vol_r = apply_volume(sample, volume_r);
			*buffer++ += signal ? vol_l : -vol_l; // L
			*buffer++ += signal ? vol_r : -vol_r; // R
			if((count -= 1024) < 0) {
				count += diff;
				signal = !signal;
			}
		}
	}
}

void BEEP::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void BEEP::initialize_sound(int rate, double frequency, int volume)
{
	gen_rate = rate;
	gen_vol = volume;
	set_frequency(frequency);
}

void BEEP::set_frequency(double frequency)
{
	diff = (int)(1024.0 * gen_rate / frequency / 2.0 + 0.5);
}

#define STATE_VERSION	1

void BEEP::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(signal);
	state_fio->FputInt32(count);
	state_fio->FputBool(on);
	state_fio->FputBool(mute);
}

bool BEEP::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	signal = state_fio->FgetBool();
	count = state_fio->FgetInt32();
	on = state_fio->FgetBool();
	mute = state_fio->FgetBool();
	return true;
}

