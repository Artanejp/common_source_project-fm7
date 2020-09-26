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

namespace FMTOWNS {

class JOYPAD : public DEVICE {
protected:
	outputs_t line_a[2];
	outputs_t line_b[2];
	outputs_t line_up[2];
	outputs_t line_down[2];
	outputs_t line_left[2];
	outputs_t line_right[2];

	bool sel_6buttons;
	bool type_6buttons;
	int pad_num;
	bool enabled;
	bool enabled_bak;

	uint32_t a_status;
	uint32_t b_status;
	
	uint32_t c_status;
	uint32_t x_status;
	uint32_t y_status;
	uint32_t z_status;
	
	uint32_t up_status;
	uint32_t down_status;
	uint32_t left_status;
	uint32_t right_status;
	
	uint32_t run_status;
	uint32_t sel_status;


	uint32_t a_status_bak;
	uint32_t b_status_bak;
	
	uint32_t c_status_bak;
	uint32_t x_status_bak;
	uint32_t y_status_bak;
	uint32_t z_status_bak;
	
	uint32_t up_status_bak;
	uint32_t down_status_bak;
	uint32_t left_status_bak;
	uint32_t right_status_bak;
	
	uint32_t run_status_bak;
	uint32_t sel_status_bak;
	
	const uint32_t* rawdata;
	virtual void backup_status();

public:
	JOYPAD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		sel_6buttons = false;
		type_6buttons = false;
		enabled = true;
		enabled_bak = !enabled;
		pad_num = 0;
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&line_a[i]);
			initialize_output_signals(&line_b[i]);
			initialize_output_signals(&line_up[i]);
			initialize_output_signals(&line_down[i]);
			initialize_output_signals(&line_left[i]);
			initialize_output_signals(&line_right[i]);
		}
		rawdata = NULL;
		//set_device name moved to initialize().
	}
	~JOYPAD() {}

	virtual void reset(void);
	virtual void initialize(void);
	virtual void event_pre_frame(void);
	virtual void event_frame(void);

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
	void set_context_a_button(DEVICE* dev, int id, uint32_t mask)
	{
		id = id & 0xffff00ff;
		register_output_signal(&line_a[0], dev, id | SIG_JOYPORT_TYPE_2BUTTONS, mask);
		register_output_signal(&line_a[1], dev, id | SIG_JOYPORT_TYPE_6BUTTONS, mask);
	}
	void set_context_b_button(DEVICE* dev, int id, uint32_t mask)
	{
		id = id & 0xffff00ff;
		register_output_signal(&line_b[0], dev, id | SIG_JOYPORT_TYPE_2BUTTONS, mask);
		register_output_signal(&line_b[1], dev, id | SIG_JOYPORT_TYPE_6BUTTONS, mask);
	}
	void set_context_up(DEVICE* dev, int id, uint32_t mask)
	{
		id = id & 0xffff00ff;
		register_output_signal(&line_up[0], dev, id | SIG_JOYPORT_TYPE_2BUTTONS, mask);
		register_output_signal(&line_up[1], dev, id | SIG_JOYPORT_TYPE_6BUTTONS, mask);
	}
	void set_context_down(DEVICE* dev, int id, uint32_t mask)
	{
		id = id & 0xffff00ff;
		register_output_signal(&line_down[0], dev, id | SIG_JOYPORT_TYPE_2BUTTONS, mask);
		register_output_signal(&line_down[1], dev, id | SIG_JOYPORT_TYPE_6BUTTONS, mask);
	}
	void set_context_left(DEVICE* dev, int id, uint32_t mask)
	{
		id = id & 0xffff00ff;
		register_output_signal(&line_left[0], dev, id | SIG_JOYPORT_TYPE_2BUTTONS, mask);
		register_output_signal(&line_left[1], dev, id | SIG_JOYPORT_TYPE_6BUTTONS, mask);
	}
	void set_context_right(DEVICE* dev, int id, uint32_t mask)
	{
		id = id & 0xffff00ff;
		register_output_signal(&line_right[0], dev, id | SIG_JOYPORT_TYPE_2BUTTONS, mask);
		register_output_signal(&line_right[1], dev, id | SIG_JOYPORT_TYPE_6BUTTONS, mask);
	}

};
}
