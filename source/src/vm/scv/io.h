/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class IO : public DEVICE
{
private:
	DEVICE* d_mem;
	DEVICE* d_sound;
	
	uint8* key;
	uint32* joy;
	uint8 pa, pb, pc, si, so;
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_sound(DEVICE* device)
	{
		d_sound = device;
	}
};

#endif
