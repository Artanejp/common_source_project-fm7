/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ joystick ]
*/

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define DEVICE_JOYSTICK_1X03	0	// SHARP MZ-1X03
#define DEVICE_JOYSTICK_JOY700	1	// TSUKUMO JOY-700
#define DEVICE_JOYSTICK_AM7J	2	// AM7J ATARI Joystick adaptor

#define EVENT_1X03_X1		0
#define EVENT_1X03_Y1		1
#define EVENT_1X03_X2		2
#define EVENT_1X03_Y2		3

class JOYSTICK : public DEVICE
{
private:
	uint32_t val_1x03;
	const uint32_t* joy_stat;

	uint64_t pulse_width_1x03(uint32_t js, uint32_t mmin, uint32_t mmax);
    	uint32_t read_AM7J(int jnum);

public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Joystick I/F"));
	}
	~JOYSTICK() {}
	
	// common functions
	void initialize();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif
