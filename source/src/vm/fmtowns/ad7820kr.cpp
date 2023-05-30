/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADC AD7820KR]
*/

#include "../../common.h"
#include "ad7820kr.h"

#define EVENT_SAMPLE 1

void AD7820KR::initialize()
{
	// ADC
	write_signals(&outputs_intr, 0x00);
	write_signals(&outputs_overflow, 0x00);
	write_signals(&outputs_ready, 0xffffffff);
	event_sample = -1;
}

void AD7820KR::release()
{
}


void AD7820KR::reset()
{
	// Q: OK?
	//if(event_sample >= 0) {
	///	cancel_event(this, event_sample);
	//}
	//event_sample = -1;
	prev_clock = get_current_clock();
}

uint32_t AD7820KR::read_signal(int ch)
{
	if(ch == SIG_AD7820_DATA_REG) {
		if(cs_enabled) {
			if(!(wr_rd_mode) && (req_convert)) {
				if(event_sample < 0) {
					start_sample(2.50);
					req_convert = false;
				}
			}
			return (uint32_t)adc_data;
		} else {
			return 0xff;
		}
	} else if(ch == SIG_AD7820_SAMPLE_RATE) {
		return (uint32_t)this_sample_rate;
	}
	return 0;
}

void AD7820KR::event_callback(int event_id, int err)
{
	if(event_id == EVENT_SAMPLE) {
		event_sample = -1;
		uint32_t passed_usec = get_passed_usec(prev_clock);
		prev_clock = get_current_clock();
		int in_rate = get_sound_in_rate(this_bank);
		int in_samples = get_sound_in_samples(this_bank);
		double max_usec = (1.0e6 / (double)in_rate) * (double)in_samples;
		req_convert = false;
		write_signals(&outputs_ready, 0xffffffff);  // SAMPLING END
		if(passed_usec >= 0.0f) {
			if(passed_usec >= max_usec) {
				passed_usec = max_usec;
			}
			bool overflow = false;
			if(!(passed_usec < (1.0e6 / (double)this_sample_rate))) {
				int32_t buffer[2];
				increment_sound_in_passed_data(this_bank, passed_usec);
				int gotsize = get_sound_in_data(this_bank, buffer, 1, this_sample_rate, 1);
				if(gotsize > 0) {
					int32_t _n = buffer[gotsize - 1];
					if(_n >= 16383) { // OVERFLOW
						overflow = true;
						_n = 16383;
					} else if(_n <= -16384) {
						overflow = true;
						_n = -16384;
					}
					_n = _n + 16384;
					_n >>= 7;
					//_n = _n / 128;
					//_n = _n + 128;
					if(wr_rd_mode) {
						adc_data = ((adc_msb & 0xf0) | (_n & 0x0f));
					} else {
						adc_data = _n & 0xff;
					}
					adc_msb = _n & 0xf0;
					write_signals(&outputs_overflow, (overflow) ? 0xffffffff : 0x00000000); // Write OVERFLOW.
				}
			}
		}
		write_signals(&outputs_intr, 0xffffffff); // Write INT.
	}
}

void AD7820KR::start_sample(double usec)
{
	write_signals(&outputs_ready, 0x00000000);    // IN SAMPLING
	write_signals(&outputs_intr, 0x00000000);     // CLEAR INTRRUPT
	write_signals(&outputs_overflow, 0x00000000); // CLEAR OVERFLOW
	if(usec > 0.0) {
		register_event(this, EVENT_SAMPLE, usec, false, &event_sample);
	}
}

void AD7820KR::write_signal(int ch, uint32_t data, uint32_t mask)
{
	bool f;
	switch(ch)
	{
	case SIG_AD7820_RESET:
		adc_data = 0x00; // Is reset
		adc_msb = 0x00;
		//wr_rd_mode = false; // HOLD
		//req_convert = false; // HOLD
		if(event_sample >= 0) {
			cancel_event(this, event_sample);
			event_sample = -1;
		}
		write_signals(&outputs_intr, 0x00);
		write_signals(&outputs_overflow, 0x00);
		write_signals(&outputs_ready, 0xffffffff);
		break;
	case SIG_AD7820_SET_SAMPLE_MODE:
		wr_rd_mode = ((data & mask) != 0) ? true : false;
		break;
	case SIG_AD7820_DATA_REG:
		adc_msb = data & 0xf0;
		if(wr_rd_mode) {
			if(event_sample < 0) {
				start_sample(1.36);
			}
		}
		break;
	case SIG_AD7820_CS:
		f = ((data & mask) != 0) ? true : false;
		if((f) && (cs_enabled != f)) {
			req_convert = true;
			if(event_sample >= 0) { // Discard before conversion.
				cancel_event(this, event_sample);
				event_sample = -1;
			}
		} /*else if(!(f)) { // CONVERSION ABORT
			if(event_sample >= 0) {
				cancel_event(this, event_sample);
				event_sample = -1;
			}
		}*/
		cs_enabled = f;
		break;
	case SIG_AD7820_WR_CONVERSION_MODE:
		if((wr_rd_mode) && (cs_enabled)) {
			// ToDo: Implement wr-rd-sampling sequence.
			if(event_sample < 0) {
				start_sample(1.36);
			}
		}
		break;
	case SIG_AD7820_SAMPLE_RATE:
		this_sample_rate = data;
		break;
	}
}


#define STATE_VERSION	1

bool AD7820KR::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(adc_data);
	state_fio->StateValue(adc_msb);
	state_fio->StateValue(prev_clock);
	state_fio->StateValue(cs_enabled);
	state_fio->StateValue(wr_rd_mode);
	state_fio->StateValue(req_convert);
	state_fio->StateValue(this_sample_rate);
	state_fio->StateValue(this_bank);
	state_fio->StateValue(event_sample);

	return true;
}
