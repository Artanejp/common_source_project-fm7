/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ joystick ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class JOYSTICK : public DEVICE
{
private:
	DEVICE* d_pio;
	const uint32_t* joy_stat;
	
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
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif
