/*
	Common Source Code Project
	MSX Series (experimental)

	Author : umaiboux
	Date   : 2016.03.xx-

	[ PSG Stereo ]
*/

#include "psg_stereo.h"
#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#define USE_PSG_STEREO_REALLY
#define DECIBEL_MIN (-120)

PSG_STEREO::PSG_STEREO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
#if defined(USE_PSG_STEREO_REALLY)
//	d_psg[1] = new YM2203(parent_vm, parent_emu);
	d_psg[1] = new AY_3_891X(parent_vm, parent_emu);
//	d_psg[2] = new YM2203(parent_vm, parent_emu);
	d_psg[2] = new AY_3_891X(parent_vm, parent_emu);
#ifdef USE_DEBUGGER
	d_psg[1]->set_context_debugger(new DEBUGGER(vm, emu));
	d_psg[2]->set_context_debugger(new DEBUGGER(vm, emu));
#endif
	m_stereo = -1; // dummy
#endif
	set_device_name(_T("Stereo PSG"));
}

void PSG_STEREO::write_io8(uint32_t addr, uint32_t data)
{
#if defined(USE_PSG_STEREO_REALLY)
	switch(addr & 1) {
	case 0:
		ch = data & 0x0f;
		if (ch >= 14) break;
		return;
	default:
		if (ch >= 14) break;
		rreg[ch] = data;
		switch(ch) {
		case 0: case 1: case 8:
			/* ch A: Center(both) */
			sound(0, ch, data);
			break;
		case 2: case 3: case 9:
			/* ch B: Right */
			sound(1, ch, data);
			break;
		case 4: case 5: case 10:
			/* ch C: Left */
			sound(2, ch, data);
			break;
		case 6: case 11: case 12: case 13:
			/* All: */
			sound(0, ch, data);
			sound(1, ch, data);
			sound(2, ch, data);
			break;
		default:
			/* reg #7: modify? */
//			sound([012],  7, 0xXX);
			sound(0, ch, data );
			sound(1, ch, data );
			sound(2, ch, data );
			break;
		}
		return;
	}
#endif
	d_psg[0]->write_io8(addr, data);
}

uint32_t PSG_STEREO::read_io8(uint32_t addr)
{
#if defined(USE_PSG_STEREO_REALLY)
	if (ch < 14) return rreg[ch];
#endif
	return d_psg[0]->read_io8(addr);
}

void PSG_STEREO::initialize()
{
	/*d_psg[1]->initialize();*/
	/*d_psg[2]->initialize();*/
}

void PSG_STEREO::release()
{
	/*d_psg[1]->release();*/
	/*d_psg[2]->release();*/
}

void PSG_STEREO::reset()
{
	/*d_psg[1]->reset();*/
	/*d_psg[2]->reset();*/
	sound(0,  9, 0);
	sound(0, 10, 0);
	sound(1,  8, 0);
	sound(1, 10, 0);
	sound(2,  8, 0);
	sound(2,  9, 0);
//	sound([012],  7, 0xXX);
}

void PSG_STEREO::mix(int32_t* buffer, int cnt)
{
	d_psg[0]->mix(buffer, cnt);
#if defined(USE_PSG_STEREO_REALLY)
	d_psg[1]->mix(buffer, cnt);
	d_psg[2]->mix(buffer, cnt);
#endif
}

void PSG_STEREO::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
	d_psg[0]->initialize_sound(rate, clock, samples, decibel_fm, decibel_psg);
#if defined(USE_PSG_STEREO_REALLY)
	d_psg[1]->initialize_sound(rate, clock, samples, decibel_fm, decibel_psg);
	d_psg[2]->initialize_sound(rate, clock, samples, decibel_fm, decibel_psg);
	sound(0,  9, 0);
	sound(0, 10, 0);
	sound(1,  8, 0);
	sound(1, 10, 0);
	sound(2,  8, 0);
	sound(2,  9, 0);
//	sound([012],  7, 0xXX);
	m_decibel_l = m_decibel_r = decibel_psg;
	update_config();
#endif
}

void PSG_STEREO::sound(int dev, int reg, int val)
{
	d_psg[dev]->write_io8(0, reg);
	d_psg[dev]->write_io8(1, val);
}

void PSG_STEREO::set_volume(int ch, int decibel_l, int decibel_r)
{
#if defined(USE_PSG_STEREO_REALLY)
	m_decibel_l = decibel_l;
	m_decibel_r = decibel_r;
	if (0 == m_stereo) {
		d_psg[0]->set_volume(1, decibel_l, decibel_r);
		d_psg[1]->set_volume(1, decibel_l, decibel_r);
		d_psg[2]->set_volume(1, decibel_l, decibel_r);
	}
	else {
		d_psg[0]->set_volume(1, decibel_l, decibel_r);
		d_psg[1]->set_volume(1, DECIBEL_MIN, decibel_r);
		d_psg[2]->set_volume(1, decibel_l, DECIBEL_MIN);
	}
#else
	d_psg[0]->set_volume(1, decibel_l, decibel_r);
#endif
}

void PSG_STEREO::update_config()
{
	if (m_stereo != config.sound_type) {
		m_stereo = config.sound_type;
		set_volume(1, m_decibel_l, m_decibel_r);
	}
}
