/*
	MITEC MP-85 Emulator 'eMP-85'

	Author : Takeda.Toshiya
	Date   : 2021.01.19-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_SEL	0

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_kdc;
	uint8_t key_stat[256];
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	
	// unique functions
	void set_context_kdc(DEVICE* device)
	{
		d_kdc = device;
	}
	void key_down(int code)
	{
		key_stat[code] = 1;
	}
	void key_up(int code)
	{
		key_stat[code] = 0;
	}
};

#endif

