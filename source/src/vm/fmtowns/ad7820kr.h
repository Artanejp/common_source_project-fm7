/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADC AD7820KR aith FIFO]
*/
#pragma once

/*
  ADC: 
    Reading data via read_signal().
    Setting data (from external device) via write_signal().
*/
#include "../device.h"
#include "../../common.h"
#include "../../fifo.h"

#define SIG_AD7820_RESET         1
#define SIG_AD7820_DO_SAMPLE     2  // SET A DATA (from any internal device)
#define SIG_AD7820_PERIOD_OUT    3  // TICK A CLOCK
#define SIG_AD7820_POP_FIFO      4
#define SIG_AD7820_PEEK_FIFO     5

// ToDo: Adjust sample rate.
class AD7820KR : public DEVICE {
	// ADC
	outputs_t output_data;
	FIFO* in_fifo;
	uint32_t adc_data;

	int in_rate;
	int out_rate;
	
	int in_mod;
	int out_count;
	int out_mod;
	
	void push_fifo(uint32_t data);

public:
	AD7820KR(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		adc_fifo = new FIFO(96000 * 2); // DEPTH OK?
		initialize_output_signals(&output_data);
		set_device_name(_T("AD7820KR A/D CONVERTER"));
	}
	~AD7820KR() {}

	void initialize();
	void reset();

	uint32_t read_signal(int ch);

	// Note: To sample:
	// 1. Set input rate (maybe host AUDIO_INPUT) and output rate (maybe emulated devices's rate).
	// 2. Get nSamples from HOST.
	// 3. Convert (adjust) sampled datas width to this device (maybe signed int or float -> unsigned 8bits).
	// 4. Push a data to this device's FIFO via write_signal(SIG_AD7820_DO_SAMPLE, converted_data) or set_data().
	// 5. Repeat 4. nSamples times.
	// 6. Go to 2.
	// Note2: To get sampled data (from emulated devices):
	// 1. Use set_context_output_data() to set target device(s).
	// 2. Transfer data within target's write_signal().
	// 3. Trigger to this device by AD7820KR::write_signal(SIG_AD7820_PERIOD_OUT,...).
	// See, vm/fmtowns/adpcm.[cpp|h] .
	void write_signal(int ch, uint32_t data, uint32_t mask);

	void set_data(uint8_t data)
	{
		push_fifo(data);
	}
	void set_data(int8_t data)
	{
		uint8_t _n;
		if(data < 0) {
			_n = (uint8_t)(-data);
			_n = 128 - _n;
		} else {
			_n = (uint8_t)data;
			_n = _n + 128;
		}
		push_fifo(_n);
	}
	void set_data(int32_t data)
	{
		uint8_t _n;
		data >>= 24;
		_n = (uint8_t)(data + 128);
		push_fifo(_n);
	}
	void set_data(uint32_t data)
	{
		uint8_t _n;
		data >>= 24;
		_n = (uint8_t)data;
		push_fifo(_n);
	}
	void set_data(int16_t data)
	{
		uint8_t _n;
		data >>= 8;
		_n = (uint8_t)(data + 128);
		push_fifo(_n);
	}
	void set_data(uint16_t data)
	{
		uint8_t _n;
		data >>= 8;
		_n = (uint8_t)data;
		push_fifo(_n);
	}
	// Note: These functions are to set sample rate both in/out.
	void initialize_sampler(uint32_t irate, uint32_t orate);
	void change_in_rate(int rate);
	void change_in_period(double us);
	void change_out_rate(int rate);
	void change_out_period(double us);
	
	bool process_state(FILEIO* state_fio, bool loading);

	void set_context_output_data(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&output_data, device, id, mask);
	}

};

