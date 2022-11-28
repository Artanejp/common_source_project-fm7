/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/keyboard.h

	modified by tanam
	Date   : 2018.12.09-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_COLUMN	0

class KEYBOARD : public DEVICE
{
private:
//	DEVICE *d_cpu, *d_pio;
	DEVICE *d_pio;
	
	const uint8_t* key_stat;
	uint8_t column;
//	bool break_pressed;
	
	void update_keyboard();
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void event_frame();
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
//	void set_context_cpu(DEVICE* device)
//	{
//		d_cpu = device;
//	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};
#endif
