/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[I/O around ADPCM]
*/
#pragma once
/*
  I/O:
  0x04e7 - 0x04e8 : ADC
  0x04f0 - 0x04f8 : DAC
  MEMORY:
  0xc2200000-c2200fff: banked ADPCM RAM.
*/

#include "../device.h"
#include "../../fifo.h"

#define	SIG_ADPCM_WRITE_INTERRUPT  1
#define SIG_ADPCM_OPX_INTR         2
#define SIG_ADPCM_PUSH_FIFO        3

namespace FMTOWNS {

class ADPCM : public DEVICE {

protected:
	DEVICE* d_rf5c68;
	
	outputs_t output_intr;

	FIFO* adc_fifo;
	bool intr_opx;
	bool dac_intr[8];
	bool dac_int_mask[8];

public:
	ADPCM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		adc_fifo = new FIFO(256); // OK?
		initialize_output_signals(output_intr);
		set_device_name(_T("FM-Towns ADPCM"));
	}
	~ADPCM() {}

	void initialize();
	
	uint32_t read_io8(uint32_t addr);
	void     write_io8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void     write_data8(uint32_t addr, uint32_t data);

	void write_signal(int ch, uint32_t data, uint32_t mask);

	bool process_state(FILEIO* state_fio, bool loading);

	void set_context_rf5c68(DEVICE* dev)
	{
		d_rf5c68 = dev;
	}
	void set_context_intr_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_intr, dev, id, mask);
	}

};


}
