/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_PORT_C	0

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_pio;
	const uint8_t* key_stat;
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	
	// unique function
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

