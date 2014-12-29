/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.09-

	[ speaker ]
*/

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SPEAKER_PIO	0

class SPEAKER : public DEVICE
{
private:
	DEVICE *d_pcm, *d_psg;
	
public:
	SPEAKER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SPEAKER() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
};

#endif

