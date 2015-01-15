/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class KEYBOARD : public DEVICE
{
private:
	uint8* key_stat;
	
#ifdef _MAP1010
	int kana_pressed;
#else
	uint8 status[16];
#endif
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	uint32 read_io8(uint32 addr);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif
