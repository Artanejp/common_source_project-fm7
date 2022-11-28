// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 *   streaming ADPCM driver
 *   by Aaron Giles
 *
 *   Library to transcode from an ADPCM source to raw PCM.
 *   Written by Buffoni Mirko in 08/06/97
 *   References: various sources and documents.
 *
 *   HJB 08/31/98
 *   modified to use an automatically selected oversampling factor
 *   for the current sample rate
 *
 *   01/06/99
 *    separate MSM5205 emulator form adpcm.c and some fix
 *
 *   07/29/12
 *    added basic support for the MSM6585
 */

#include <math.h>
#include "msm5205.h"

#define EVENT_TIMER	0

/*

    MSM 5205 ADPCM chip:

    Data is streamed from a CPU by means of a clock generated on the chip.

    A reset signal is set high or low to determine whether playback (and interrupts) are occurring.

  MSM6585: is an upgraded MSM5205 voice synth IC.
   Improvements:
    More precise internal DA converter
    Built in low-pass filter
    Expanded sampling frequency

   Differences between MSM6585 & MSM5205:

                              MSM6586          MSM5205
    Master clock frequency    640kHz           384kHz
    Sampling frequency        4k/8k/16k/32kHz  4k/6k/8kHz
    ADPCM bit length          4-bit            3-bit/4-bit
    DA converter              12-bit           10-bit
    Low-pass filter           -40dB/oct        N/A
    Overflow prevent circuit  Included         N/A

    Timer callback at VCLK low edge on MSM5205 (at rising edge on MSM6585)

   TODO:
   - lowpass filter for MSM6585

 */

void MSM5205::initialize()
{
//	m_mod_clock = clock();
//	m_vclk_cb.resolve();

	/* compute the difference tables */
	compute_tables();

	/* stream system initialize */
//	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock());
//	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(msm5205_device::vclk_callback), this));
	m_timer = -1;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void MSM5205::reset()
{
	/* initialize work */
	m_data    = 0;
	m_vclk    = 0;
	m_reset   = 0;
	m_signal  = 0;
	m_step    = 0;

	/* initialize clock */
	change_clock_w(m_mod_clock);

	/* timer and bitwidth set */
	playmode_w(m_select);
}


/*
 * ADPCM lookup table
 */

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/*
 *   Compute the difference table
 */

void MSM5205::compute_tables()
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = (int)floor (16.0 * pow (11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			m_diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}
}

/* timer callback at VCLK low edge on MSM5205 (at rising edge on MSM6585) */
void MSM5205::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TIMER)
	{
		int val;
		int new_signal;

		/* callback user handler and latch next data */
//		if (!m_vclk_cb.isnull())
//			m_vclk_cb(1);
		write_signals(&m_vclk_cb, 0xffffffff);

		/* reset check at last hiedge of VCLK */
		if (m_reset)
		{
			new_signal = 0;
			m_step = 0;
		}
		else
		{
			/* update signal */
			/* !! MSM5205 has internal 12bit decoding, signal width is 0 to 8191 !! */
			val = m_data;
			new_signal = m_signal + m_diff_lookup[m_step * 16 + (val & 15)];

			if (new_signal > 2047) new_signal = 2047;
			else if (new_signal < -2048) new_signal = -2048;

			m_step += index_shift[val & 7];

			if (m_step > 48) m_step = 48;
			else if (m_step < 0) m_step = 0;
		}

		/* update when signal changed */
		if( m_signal != new_signal)
		{
//			m_stream->update();
			m_signal = new_signal;
		}
	}
}



/*
 *    Handle an update of the vclk status of a chip (1 is reset ON, 0 is reset OFF)
 *    This function can use selector = MSM5205_SEX only
 */
void MSM5205::vclk_w(int vclk)
{
	if (m_prescaler != 0)
	{
//		logerror("error: msm5205_vclk_w() called with chip = '%s', but VCLK selected master mode\n", this->device().tag());
	}
	else
	{
		if (m_vclk != vclk)
		{
			m_vclk = vclk;
			if (!vclk)
//				vclk_callback(this, 0);
				event_callback(EVENT_TIMER, 0);
		}
	}
}

/*
 *    Handle an update of the reset status of a chip (1 is reset ON, 0 is reset OFF)
 */

void MSM5205::reset_w(int reset)
{
	touch_sound();
	m_reset = reset;
	set_realtime_render(this, (m_reset == 0));
}

/*
 *    Handle an update of the data to the chip
 */

void MSM5205::data_w(int data)
{
	touch_sound();
	if (m_bitwidth == 4) {
		m_data = data & 0x0f;
	} else {
		m_data = (data & 0x07) << 1; /* unknown */
	}
}

/*
 *    Handle a change of the selector
 */

void MSM5205::playmode_w(int select)
{
	static const int prescaler_table[2][4] =
	{
		{ 96, 48, 64,  0},
		{160, 40, 80, 20}
	};
	int prescaler = prescaler_table[(select >> 3) & 1][select & 3];
	int bitwidth = (select & 4) ? 4 : 3;

	if (m_prescaler != prescaler)
	{
//		m_stream->update();

		touch_sound();
		m_prescaler = prescaler;

		/* timer set */
		if (prescaler)
		{
//			attotime period = attotime::from_hz(m_mod_clock) * prescaler;
//			m_timer->adjust(period, 0, period);
			double period = 1000000.0 / m_mod_clock * prescaler;
			if (m_timer != -1) {
				cancel_event(this, m_timer);
			}
			register_event(this, EVENT_TIMER, period, true, &m_timer);
		}
		else
		{
//			m_timer->adjust(attotime::never);
			if (m_timer != -1) {
				cancel_event(this, m_timer);
				m_timer = -1;
			}
		}
	}

	if (m_bitwidth != bitwidth)
	{
		touch_sound();
//		m_stream->update();
		m_bitwidth = bitwidth;
	}
}


void MSM5205::set_volume(int volume)
{
	touch_sound();
	volume_m = (int)(1024.0 * (max(0, min(100, volume)) / 100.0));
}

void MSM5205::change_clock_w(int32_t clock)
{
	m_mod_clock = clock;

	if (m_prescaler != 0) {
		touch_sound();
		double period = 1000000.0 / m_mod_clock * m_prescaler;
		if(m_timer != -1) {
			cancel_event(this, m_timer);
		}
		register_event(this, EVENT_TIMER, period, true, &m_timer);
	} else {
		touch_sound();
		if(m_timer != -1) {
			cancel_event(this, m_timer);
			m_timer = -1;
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void MSM5205::mix(int32_t* buffer, int cnt)
{
	/* if this voice is active */
	if(m_signal)
	{
		int32_t val = apply_volume(m_signal * 16, volume_m);
		int32_t val_l = apply_volume(val, volume_l);
		int32_t val_r = apply_volume(val, volume_r);
		
		for(int i = 0; i < cnt; i++)
		{
			*buffer++ += val_l; // L
			*buffer++ += val_r; // R
		}
	}
}

void MSM5205::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

#define STATE_VERSION	1

bool MSM5205::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(m_mod_clock);
	state_fio->StateValue(m_timer);
	state_fio->StateValue(m_data);
	state_fio->StateValue(m_vclk);
	state_fio->StateValue(m_reset);
	state_fio->StateValue(m_prescaler);
	state_fio->StateValue(m_bitwidth);
	state_fio->StateValue(m_signal);
	state_fio->StateValue(m_step);
	state_fio->StateValue(m_select);
	state_fio->StateValue(volume_m);
	return true;
}

