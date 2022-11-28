/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.03.08-

	[ noise player ]
*/

#ifndef _NOISE_H_
#define _NOISE_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class NOISE : public DEVICE
{
private:
	int16_t *buffer_l;
	int16_t *buffer_r;
	int samples;
	int sample_rate;
	int register_id;
	int ptr;
	int sample_l, sample_r;
	int volume_l, volume_r;
	bool loop;
	bool mute;
	
	void get_sample();
	
public:
	NOISE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		buffer_l = buffer_r = NULL;
		samples = 0;
		volume_l = volume_r = 1024;
		loop = false;
		mute = false;
		set_device_name(_T("Noise Player"));
	}
	~NOISE() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	bool load_wav_file(const _TCHAR *file_name);
	void play();
	void stop();
	void set_loop(bool value)
	{
		loop = value;
	}
	void set_mute(bool value)
	{
		mute = value;
	}
};

#endif

