/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADC AD7820KR with FIFO]
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

#define SIG_AD7820_DATA_REG           1  // READ/SET A DATA
#define SIG_AD7820_RESET              2  // REEST DEVICE (not implemented at real machine)
#define SIG_AD7820_SET_SAMPLE_MODE    3  // NOT 0 = WR_RD_MODE, 0 = RD_MODE.
#define SIG_AD7820_CS                 4  // CS (Positive logic: differ from real hardware).
#define SIG_AD7820_WR_CONVERSION_MODE 5  // Automatic conversion mode: MUST SET SAMPLE_MODE = WR_RD_MODE (not 0).
#define SIG_AD7820_SAMPLE_RATE        6  // READ/SET SAMPLE RATE

// ToDo: Adjust sample rate.
class AD7820KR : public DEVICE {
	// ADC
	// Note: AD7820KR controls/outputs *INT and *OFL as NEGATIVE logic, but this outputs POSITIVE logic 20190307 K.O
	outputs_t outputs_intr;
	outputs_t outputs_overflow;
	outputs_t outputs_ready;
	
	uint8_t adc_data;
	uint8_t adc_msb;
	uint32_t prev_clock;
	
	bool cs_enabled;
	bool req_convert;
	bool wr_rd_mode;
	
	int this_bank;
	int this_sample_rate;
	int event_sample;
	
	void start_sample(double usec);
public:
	AD7820KR(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_intr);
		initialize_output_signals(&outputs_overflow);
		initialize_output_signals(&outputs_ready);
		this_sample_rate = 19200; // ToDo.
		this_bank = 0; // ToDo.
		wr_rd_mode = false;
		req_convert = false;
		adc_data = 0x00;
		adc_msb = 0x00;
		cs_enabled = false;
		prev_clock = 0;
		set_device_name(_T("A/D Converter AD7820KR"));
	
	}
	~AD7820KR()
	{
	}

	void initialize();
	void release();
	void reset();

	void event_callback(int event_id, int err);
	
	uint32_t __FASTCALL read_signal(int ch);
	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);

	bool process_state(FILEIO* state_fio, bool loading);

	// unique functions
	void set_sample_rate(int val)
	{
		this_sample_rate = val;
	}
	void set_sound_bank(int val)
	{
		this_bank = val;
	}
	void set_context_ready(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ready, device, id, mask);
	}
	void set_context_interrupt(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_intr, device, id, mask);
	}
	void set_context_overflow(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_overflow, device, id, mask);
	}
};

