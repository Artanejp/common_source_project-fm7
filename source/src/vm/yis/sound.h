/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.05.07-

	[ sound i/f ]
*/

#ifndef _SOUND_H_
#define _SOUND_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class BEEP;

class SOUND : public DEVICE
{
private:
	BEEP* d_beep;
	
public:
	SOUND(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Sound I/F"));
	}
	~SOUND() {}
	
	// common function
	void write_io8(uint32_t addr, uint32_t data);
	
	// unique function
	void set_context_beep(BEEP* device)
	{
		d_beep = device;
	}
};

#endif

