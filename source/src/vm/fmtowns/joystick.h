/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns PAD ]

*/

#pragma once
#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace FMTOWNS {
class JOYSTICK : public DEVICE
{
private:
	bool emulate_mouse[2];
	uint32_t joydata[2];
	
	const uint32_t *rawdata;
	const int32_t *mouse_state;
	int dx, dy;
	int lx, ly;
	uint32_t mouse_button;
	int mouse_phase;
	bool mouse_strobe;
	uint8_t mouse_data;
	
	int mouse_timeout_event;
	int mouse_sampling_event;
	int mouse_type;
	uint8_t mask;
	
	void set_emulate_mouse();
	virtual void update_strobe(bool flag);
	uint32_t update_mouse();

public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		mouse_timeout_event = -1;
		mouse_sampling_event = -1;
		set_device_name(_T("FM-Towns PAD and MOUSE (JIS)"));
	}
	~JOYSTICK() {}
	
	// common functions
	void initialize(void);
	void event_frame(void);
	void release();
	void reset();
	
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void update_config();
	
	bool process_state(FILEIO* state_fio, bool loading);
	
};
}


