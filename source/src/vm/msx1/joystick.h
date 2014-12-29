/*
	ASCII MSX1 Emulator 'yaMSX1'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya

	[ joystick ]
*/

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_JOYSTICK_SEL	0

class JOYSTICK : public DEVICE
{
private:
	DEVICE *d_psg;
	uint32 *joy_stat;
	int select;
	
public:
	JOYSTICK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~JOYSTICK() {}
	
	// common functions
	void initialize();
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
};
#endif
