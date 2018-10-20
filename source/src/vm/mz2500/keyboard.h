/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

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
	DEVICE *d_pio_i, *d_pio;	// i8255, z80pio
	
	// keyboard
	const uint8_t* key_stat;
	uint8_t keys[16];
	uint8_t column;
	void create_keystat();
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_pio_i(DEVICE* device)
	{
		d_pio_i = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

