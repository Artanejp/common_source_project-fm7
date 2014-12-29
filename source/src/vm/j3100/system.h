/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ system ]
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SYSTEM_TC2O

class SYSTEM : public DEVICE
{
private:
	DEVICE *d_pcm, *d_pit;
	
	uint8 status;
	
public:
	SYSTEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SYSTEM() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
};

#endif

