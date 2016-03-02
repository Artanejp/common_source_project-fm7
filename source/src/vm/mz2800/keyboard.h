/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

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
	DEVICE* d_pio0;	// i8255
	DEVICE* d_pio1;	// z80pio
	
	// keyboard
	const uint8_t* key_stat;
	uint8_t keys[16];
	uint8_t column;
	void create_keystat();
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_pio0(DEVICE* device)
	{
		d_pio0 = device;
	}
	void set_context_pio1(DEVICE* device)
	{
		d_pio1 = device;
	}
};

#endif

