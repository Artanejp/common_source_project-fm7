/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ Speaker ]
*/

#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_SPEAKER_SAMPLE	0

class SPEAKER : public DEVICE
{
private:
	bool realtime;
	int changed;
	int sample, prev_sample;
	uint32_t prev_clock, change_clock;
	int max_vol, last_vol_l, last_vol_r;
	int volume_l, volume_r;
	
	void update_realtime_render();
	
public:
	SPEAKER(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("Speaker"));
	}
	~SPEAKER() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void initialize_sound(int rate, int volume);
};

#endif

