/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.01.28 -
    History : 2020.01.28 Initial from FM7.
	[ Towns Joystick ports]

*/

#pragma once
#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_JOYPORT_LINE_UP			1
#define SIG_JOYPORT_LINE_DOWN		2
#define SIG_JOYPORT_LINE_LEFT		3
#define SIG_JOYPORT_LINE_RIGHT		4
#define SIG_JOYPORT_LINE_A			5
#define SIG_JOYPORT_LINE_B			6
#define SIG_JOYPORT_CH0				0
#define SIG_JOYPORT_CH1				4096
#define SIG_JOYPORT_TYPE_NULL		0
#define SIG_JOYPORT_TYPE_2BUTTONS	256
#define SIG_JOYPORT_TYPE_6BUTTONS	512
#define SIG_JOYPORT_TYPE_ANALOG		768 /* ToDo: For CYBER STICK */
#define SIG_JOYPORT_TYPE_MOUSE		1024
#define SIG_JOYPORT_TYPE_TWIN_2B_0	1280 /* ToDo: For RIBBLE RABBLE */
#define SIG_JOYPORT_TYPE_TWIN_2B_1	1536 /* ToDo: For RIBBLE RABBLE */
#define SIG_JOYPORT_CONNECT			65536 /* ToDo: RESET */

namespace FMTOWNS {
class JOYSTICK : public DEVICE
{
private:
	outputs_t outputs_mask;
	outputs_t outputs_enable[2];
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
	uint32_t connected_type[2];
	
	void set_emulate_mouse();
	virtual void update_strobe(bool flag);
	uint32_t update_mouse();

public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		mouse_timeout_event = -1;
		mouse_sampling_event = -1;
		initialize_output_signals(&outputs_mask);
		initialize_output_signals(&outputs_enable[0]);
		initialize_output_signals(&outputs_enable[1]);
		connected_type[0] = SIG_JOYPORT_CH0 | SIG_JOYPORT_TYPE_NULL;
		connected_type[1] = SIG_JOYPORT_CH1 | SIG_JOYPORT_TYPE_NULL;
		set_device_name(_T("FM-Towns PAD Port and MOUSE (JIS)"));
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

	// unique functions
	void set_context_enable0(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_enable[0], dev, id, mask);
	}
	void set_context_enable1(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_enable[1], dev, id, mask);
	}
	void set_context_mask(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_mask, dev, id, mask);
	}

			
};
}


