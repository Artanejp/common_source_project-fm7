/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[I/O around ADPCM]
*/
#pragma once
/*
  I/O:
  0x04d5          : OPN2/ADPCM MUTE
  0x04e7 - 0x04e8 : ADC
  0x04e9 - 0x04ec : ADPCM CONTROL 
  0x04e8 - 0x04f8 : DAC

  MEMORY:
  0xc2200000-c2200fff: banked ADPCM RAM.
*/

#include "../device.h"

#define	SIG_ADPCM_WRITE_INTERRUPT  1
#define SIG_ADPCM_OPX_INTR         2
#define SIG_ADPCM_ADC_INTR         3

class FIFO;
namespace FMTOWNS {

class ADPCM : public DEVICE {

protected:
	DEVICE* d_rf5c68;
	DEVICE* d_opn2;
	DEVICE* d_pic;
	DEVICE* d_adc;
	
	outputs_t outputs_intr;
	outputs_t outputs_led_control;
	outputs_t outputs_allmute;

	FIFO* adc_fifo;
	bool intr_opx;
	bool dac_intr[8];
	bool dac_intr_mask[8];

	bool opn2_mute;
	bool adpcm_mute;

	int event_adc_clock;
	int event_adpcm_clock;
	void initialize_adc_clock(int freq);
public:
	ADPCM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		adc_fifo = NULL;
		initialize_output_signals(&outputs_intr);
		initialize_output_signals(&outputs_led_control);
		initialize_output_signals(&outputs_allmute);
		d_rf5c68 = NULL;
		d_opn2 = NULL;
		d_pic = NULL;
		d_adc = NULL;

		for(int i = 0; i < 8; i++) {
			dac_intr[i] = false;
			dac_intr_mask[i] = true;
		}
		intr_opx = false;
		adpcm_mute = false;
		opn2_mute = false;
		event_adc_clock = -1;
		event_adpcm_clock = -1;
		set_device_name(_T("FM-Towns ADPCM"));
	}
	~ADPCM() {}

	void initialize();
	void release();
	void reset();
	void event_callback(int id, int err);
	
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void     __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_data8(uint32_t addr);
	void     __FASTCALL write_data8(uint32_t addr, uint32_t data);

	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int ch);

	bool process_state(FILEIO* state_fio, bool loading);

	void set_context_pic(DEVICE* dev)
	{
		d_pic = dev;
	}
	void set_context_adc(DEVICE* dev)
	{
		d_adc = dev;
	}
	void set_context_rf5c68(DEVICE* dev)
	{
		d_rf5c68 = dev;
	}
	void set_context_opn2(DEVICE* dev)
	{
		d_opn2 = dev;
	}
	void set_context_intr_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_intr, dev, id, mask);
	}
	void set_context_led(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_led_control, dev, id, mask);
	}
	void set_context_all_mute(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_allmute, dev, id, mask);
	}

};


}
