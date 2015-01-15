/*
	CASIO PV-1000 Emulator 'ePV-1000'

	Author : Takeda.Toshiya
	Date   : 2006.11.16 -

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
	uint8* key;
	uint32* joy;
	uint8 column;
	uint8 status;
	
public:
	JOYSTICK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~JOYSTICK() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif
