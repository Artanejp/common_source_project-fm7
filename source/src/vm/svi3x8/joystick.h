/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/joystick.h

	modified by tanam
	Date   : 2018.12.09-

	[ joystick ]
*/

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_JOYSTICK_SEL	0

namespace SVI_3X8 {
class JOYSTICK : public DEVICE
{
private:
	DEVICE *d_psg;
	DEVICE *d_memory;
	const uint32_t *joy_stat;
	int select;

public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Joystick I/F"));
	}
	~JOYSTICK() {}

	// common functions
	void initialize() override;
	void event_frame() override;
//	void write_signal(int id, uint32_t data, uint32_t mask) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique function
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void set_context_memory(DEVICE* device)
	{
		d_memory = device;
	}
};
}
#endif
