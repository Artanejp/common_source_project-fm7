/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ SN76489AN ]
*/

#include "sn76489an.h"
#include "../fileio.h"

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

void SN76489AN::write_io8(uint32 addr, uint32 data)
{
	if(data & 0x80) {
		index = (data >> 4) & 7;
		int c = index >> 1;
		
		switch(index & 7) {
		case 0: case 2: case 4:
			// tone : frequency
			regs[index] = (regs[index] & 0x3f0) | (data & 0x0f);
			ch[c].period = regs[index] ? regs[index] : 0x400;
//			ch[c].count = 0;
			break;
		case 1: case 3: case 5: case 7:
			// tone / noise : volume
			regs[index] = data & 0x0f;
			ch[c].volume = volume_table[data & 0x0f];
			break;
		case 6:
			// noise : frequency, mode
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
			regs[index] = (regs[index] & 0x0f) | (((uint16)data << 4) & 0x3f0);
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

void SN76489AN::write_signal(int id, uint32 data, uint32 mask)
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

void SN76489AN::mix(int32* buffer, int cnt)
{
	if(mute) {
		return;
	}
	for(int i = 0; i < cnt; i++) {
		int32 vol = 0;
		for(int j = 0; j < 4; j++) {
			if(!ch[j].volume) {
				continue;
			}
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
			vol += ch[j].signal ? ch[j].volume : -ch[j].volume;
		}
		*buffer++ += vol; // L
		*buffer++ += vol; // R
	}
}

void SN76489AN::init(int rate, int clock, int volume)
{
	// create gain
	double vol = volume;
	for(int i = 0; i < 15; i++) {
		volume_table[i] = (int)vol;
		vol /= 1.258925412;
	}
	volume_table[15] = 0;
	diff = 16 * clock / rate;
}

#define STATE_VERSION	1

void SN76489AN::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(regs, sizeof(regs), 1);
	state_fio->FputInt32(index);
	state_fio->Fwrite(ch, sizeof(ch), 1);
	state_fio->FputUint32(noise_gen);
	state_fio->FputBool(mute);
	state_fio->FputBool(cs);
	state_fio->FputBool(we);
	state_fio->FputUint8(val);
}

bool SN76489AN::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(regs, sizeof(regs), 1);
	index = state_fio->FgetInt32();
	state_fio->Fwrite(ch, sizeof(ch), 1);
	noise_gen = state_fio->FgetUint32();
	mute = state_fio->FgetBool();
	cs = state_fio->FgetBool();
	we = state_fio->FgetBool();
	val = state_fio->FgetUint8();
	return true;
}

