/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.22 -

	[ beep ]
*/

#ifndef _BEEP_H_
#define _BEEP_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_BEEP_ON	0
#define SIG_BEEP_MUTE	1

class BEEP : public DEVICE
{
private:
	int gen_rate;
	int gen_vol;
	bool signal;
	int count;
	int diff;
	
	bool on;
	bool mute;
	
public:
	BEEP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~BEEP() {}
	
	// common functions
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void mix(int32* buffer, int cnt);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void init(int rate, double frequency, int volume);
	void set_frequency(double frequency);
};

#endif

