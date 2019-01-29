/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADC AD7820KR with FIFO]
*/

#include "../../common.h"
#include "ad7820kr.h"

void AD7820KR::initialize()
{
	// ADC
	adc_fifo->clear();
	adc_data = 0x00;
}

void AD7820KR::reset()
{
	// ToDo: IS CLEAR FIFO?
	initialize_sampler(in_rate, out_rate);
}

uint32_t AD7820KR::read_signal(int ch)
{
	if(ch == SIG_AD7820_POP_FIFO) {
		if(adc_fifo->empty()) {
			return 0x00;
		}
		return (uint32_t)(adc_fifo->read() & 0xff);
	} else if(ch == SIG_AD7820_PEEK_FIFO) {
		if(adc_fifo->empty()) {
			return 0x00;
		}
		return (uint32_t)(adc_fifo->read_not_remove() & 0xff);
	} else if(ch == SIG_AD7820_FIFO_IS_EMPTY) {
		return ((adc_fifo->empty()) ? 0x00 : 0xffffffff);
	}
	return 0x00;
}

void AD7820KR::push_fifo(uint32_t data)
{
	if(in_fifo->full()) {
		in_fifo->read(); // Dummy read OK?
	}
	in_fifo->write((int)(data & 0xff));
}

void AD7820KR::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch)
	{
	case SIG_AD7820_RESET:
		initialize_sampler(in_rate, out_rate);
		break;
	case SIG_AD7820_DO_SAMPLE:
		push_fifo(data);
		break;
	case SIG_AD7820_PERIOD_OUT:
		if(in_rate > out_rate) {
			out_mod = 0;
			out_count = 1;
			int local_count = in_rate / out_rate;
			int local_mod = in_rate % out_rate;
			if(in_fifo->empty()) {
				adc_data = 0x00;
			} else {
				for(uint32_t i = 0; i < local_count; i++) {
					if(in_fifo->empty()) break;
					adc_data = (uint8_t)(in_fifo->read() & 0xff);
				}
				in_mod = in_mod + local_mod;
				if(in_mod >= out_rate) {
					if(!(in_fifo->empty())) {
						adc_data = (uint8_t)(in_fifo->read() & 0xff);
					}
					in_mod -= out_rate;
				}
				write_signals(&output_data, adc_data);
			}
		} else if(in_rate == out_rate) {
			in_mod = 0;
			out_mod = 0;
			out_count = 1;
			if(!(in_fifo->empty())) {
				adc_data = (uint8_t)(in_fifo->read() & 0xff);
			} else {
				adc_data = 0x00;
			}
			write_signals(&output_data, adc_data);
		} else { // in_rate < out_rate
			int local_mod = out_rate % in_rate;
			int local_count = out_rate / in_rate;
			in_mod = 0;
			if(out_count == 0) {
				if(!(in_fifo->empty())) {
					adc_data = (uint8_t)(in_fifo->read() & 0xff);
				} else {
					adc_data = 0x00;
				}
				out_count = local_count;
			}
			if(local_mod != 0) {
				out_mod = out_mod + local_mod;
				if(out_mod >= in_rate) {
					out_mod -= in_rate;
					out_count++;
				}
			}
			write_signals(&output_data, adc_data);
			if(out_count > 0) out_count = out_count - 1;
		}
		break;
	}
}


void AD7802KR::initialize_sampler(uint32_t irate, uint32_t orate)
{
	in_rate = irate;
	out_rate = orate;
	if(in_rate > out_rate) {
		out_mod = 0;
		out_count = 1;
		in_mod = 0;
	} else if(in_rate == out_rate) {
		in_mod = 0;
		out_mod = 0;
		out_count = 1;
	} else { // in_rate < out_rate
		in_mod = 0;
		out_mod = 0;
		out_count = 0;
	}
	adc_data = 0x00;
	in_fifo->clear();
}

void AD7820KR::change_in_rate(int rate)
{
	initialize_sampler(rate, out_rate);
}

void AD7820KR::change_out_rate(int rate)
{
	initialize_sampler(in_rate, rate);
}

void AD7820KR::change_in_period(double usec)
{
	int rate = (int)(1.0e6 / usec);
	initialize_sampler(rate, out_rate);
}


void AD7820KR::change_out_period(double usec)
{
	int rate = (int)(1.0e6 / usec);
	initialize_sampler(in_rate, rate);
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
	if(!(in_fifo->process_state((void *)state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(adc_data);
	state_fio->StateValue(in_rate);
	state_fio->StateValue(out_rate);
	state_fio->StateValue(in_mod);
	state_fio->StateValue(out_count);
	state_fio->StateValue(out_mod);

	return true;
}
