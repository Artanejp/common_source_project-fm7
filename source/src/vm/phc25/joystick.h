/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.06-

	[ joystick ]
*/

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class JOYSTICK : public DEVICE
{
private:
	DEVICE *d_psg;
	const uint32_t *joy_stat;
	
public:
	JOYSTICK(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Joystick I/F"));
	}
	~JOYSTICK() {}
	
	// common functions
	void initialize();
	void event_frame();
	
	// unique function
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
};

#endif
