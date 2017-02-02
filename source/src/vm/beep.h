/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.22 -

	[ beep ]
*/

#ifndef _BEEP_H_
#define _BEEP_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_BEEP_ON	0
#define SIG_BEEP_MUTE	1

class VM;
class EMU;
class BEEP : public DEVICE
{
private:
	int gen_rate;
	int gen_vol;
	int volume_l, volume_r;
	
	bool signal;
	int count;
	int diff;
	
	bool on;
	bool mute;
	
public:
	BEEP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("Beep Generator"));
	}
	~BEEP() {}
	
	// common functions
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	// unique function
	void initialize_sound(int rate, double frequency, int volume);
	void set_frequency(double frequency);
};

#endif

