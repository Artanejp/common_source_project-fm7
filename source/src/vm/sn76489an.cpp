/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ SN76489AN ]
*/

#include "sn76489an.h"

#ifdef HAS_SN76489
// SN76489
#define NOISE_FB	0x4000
#define NOISE_DST_TAP	1
#define NOISE_SRC_TAP	2
#else
// SN76489A, SN76496
#define NOISE_FB	0x10000
#define NOISE_DST_TAP	4
#define NOISE_SRC_TAP	8
#endif
#define NOISE_MODE	((regs[6] & 4) ? 1 : 0)

void SN76489AN::initialize()
{
	mute = false;
	cs = we = true;
}

void SN76489AN::reset()
{
	touch_sound();
	for(int i = 0; i < 4; i++) {
		ch[i].count = 0;
		ch[i].period = 1;
		ch[i].volume = 0;
		ch[i].signal = false;
	}
	for(int i = 0; i < 8; i += 2) {
		regs[i + 0] = 0;
		regs[i + 1] = 0x0f;	// volume = 0
	}
	noise_gen = NOISE_FB;
	ch[3].signal = false;
}

void SN76489AN::write_io8(uint32_t addr, uint32_t data)
{
	if(data & 0x80) {
		index = (data >> 4) & 7;
		int c = index >> 1;
		
		switch(index & 7) {
		case 0: case 2: case 4:
			// tone : frequency
			touch_sound();
			regs[index] = (regs[index] & 0x3f0) | (data & 0x0f);
			ch[c].period = regs[index] ? regs[index] : 0x400;
//			ch[c].count = 0;
			break;
		case 1: case 3: case 5: case 7:
			// tone / noise : volume
			touch_sound();
			regs[index] = data & 0x0f;
			ch[c].volume = volume_table[data & 0x0f];
			break;
		case 6:
			// noise : frequency, mode
			touch_sound();
			regs[6] = data;
			data &= 3;
			ch[3].period = (data == 3) ? (ch[2].period << 1) : (1 << (data + 5));
//			ch[3].count = 0;
			noise_gen = NOISE_FB;
			ch[3].signal = false;
			break;
		}
	} else {
		int c = index >> 1;
		
		switch(index & 0x07) {
		case 0: case 2: case 4:
			// tone : frequency
			touch_sound();
			regs[index] = (regs[index] & 0x0f) | (((uint16_t)data << 4) & 0x3f0);
			ch[c].period = regs[index] ? regs[index] : 0x400;
//			ch[c].count = 0;
			// update noise shift frequency
			if(index == 4 && (regs[6] & 3) == 3) {
				ch[3].period = ch[2].period << 1;
			}
			break;
		}
	}
}

void SN76489AN::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SN76489AN_MUTE) {
		mute = ((data & mask) != 0);
	} else if(id == SIG_SN76489AN_DATA) {
		val = data & mask;
	} else if(id == SIG_SN76489AN_CS) {
		bool next = ((data & mask) != 0);
		if(cs != next) {
			if(!(cs = next) && !we) {
				write_io8(0, val);
			}
		}
	} else if(id == SIG_SN76489AN_CS) {
		bool next = ((data & mask) != 0);
		if(cs != next) {
			cs = next;
			if(!cs && !we) {
				write_io8(0, val);
			}
		}
	} else if(id == SIG_SN76489AN_WE) {
		bool next = ((data & mask) != 0);
		if(we != next) {
			we = next;
			if(!cs && !we) {
				write_io8(0, val);
			}
		}
	}
}

void SN76489AN::mix(int32_t* buffer, int cnt)
{
	if(mute) {
		return;
	}
	for(int i = 0; i < cnt; i++) {
		int32_t vol_l = 0, vol_r = 0;
		for(int j = 0; j < 4; j++) {
			if(!ch[j].volume) {
				continue;
			}
			bool prev_signal = ch[j].signal;
			int prev_count = ch[j].count;
			ch[j].count -= diff;
			if(ch[j].count < 0) {
				ch[j].count += ch[j].period << 8;
				if(j == 3) {
					if(((noise_gen & NOISE_DST_TAP) ? 1 : 0) ^ (((noise_gen & NOISE_SRC_TAP) ? 1 : 0) * NOISE_MODE)) {
						noise_gen >>= 1;
						noise_gen |= NOISE_FB;
					} else {
						noise_gen >>= 1;
					}
					ch[3].signal = ((noise_gen & 1) != 0);
				} else {
					ch[j].signal = !ch[j].signal;
				}
			}
			int32_t sample = (prev_signal != ch[j].signal && prev_count < diff) ? (ch[j].volume * (2 * prev_count - diff)) / diff : ch[j].volume;
			int32_t vol_tmp_l = apply_volume(sample, volume_l);
			int32_t vol_tmp_r = apply_volume(sample, volume_r);
			
			vol_l += prev_signal ? vol_tmp_l : -vol_tmp_l;
			vol_r += prev_signal ? vol_tmp_r : -vol_tmp_r;
		}
		*buffer++ += vol_l; // L
		*buffer++ += vol_r; // R
	}
}

void SN76489AN::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void SN76489AN::initialize_sound(int rate, int clock, int volume)
{
	// create gain
	double vol = volume;
	for(int i = 0; i < 15; i++) {
		volume_table[i] = (int)vol;
		vol /= 1.258925412;
	}
	volume_table[15] = 0;
	diff = (int)(16.0 * (double)clock / (double)rate + 0.5);
}

#define STATE_VERSION	2

bool SN76489AN::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateValue(index);
	for(int i = 0; i < array_length(ch); i++) {
		state_fio->StateValue(ch[i].count);
		state_fio->StateValue(ch[i].period);
		state_fio->StateValue(ch[i].volume);
		state_fio->StateValue(ch[i].signal);
	}
	state_fio->StateValue(noise_gen);
	state_fio->StateValue(mute);
	state_fio->StateValue(cs);
	state_fio->StateValue(we);
	state_fio->StateValue(val);
	return true;
}

