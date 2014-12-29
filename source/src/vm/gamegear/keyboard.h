/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

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
	DEVICE *d_cpu, *d_pio;
	
	uint8* key_stat;
	uint32* joy_stat;
	uint8 column;
	bool break_pressed;
	bool start_pressed;
	void update_keyboard();
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	bool is_start();
	bool sk1100;
};
#endif
