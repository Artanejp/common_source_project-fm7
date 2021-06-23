/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.09.26 -
    History : 2020.09.26 Separate from joystick.cpp/joystick.h .
	[ Towns PAD ]

*/
#pragma once

#include "device.h"

#define SIG_JOYPAD_SELECT_BUS		1
#define SIG_JOYPAD_ENABLE			2
#define SIG_JOYPAD_QUERY			3

namespace FMTOWNS {

class JOYPAD : public DEVICE {
protected:
	outputs_t line_dat;
	outputs_t line_com;

	bool sel_line;
	bool type_6buttons;
	int pad_num;
	bool enabled;
	
	const uint32_t* rawdata;
	virtual void query_joystick();
public:
	JOYPAD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		sel_line = false;
		type_6buttons = false;
		enabled = true;
		pad_num = 0;

		initialize_output_signals(&line_dat);
		initialize_output_signals(&line_com);
		rawdata = NULL;
		//set_device name moved to initialize().
	}
	~JOYPAD() {}

	virtual void reset(void);
	virtual void initialize(void);

	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);

	virtual void update_config();
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// Unique functions
	void set_context_port_num(int num)
	{
		if((num >= 0) && (num <= 1)) {
			pad_num = num;
		}
	}
	void set_context_data(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&line_dat, dev, id, mask);
	}
	void set_context_com(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&line_com, dev, id, mask);
	}

};
}
