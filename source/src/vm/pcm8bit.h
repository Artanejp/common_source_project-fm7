/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ 8bit PCM ]
*/

#ifndef _PCM8BIT_H_
#define _PCM8BIT_H_

#include "./vm_template.h"
#include "../emu_template.h"
#include "./device.h"

#define SIG_PCM8BIT_SAMPLE	0
#define SIG_PCM8BIT_ON		1
#define SIG_PCM8BIT_MUTE	2

class PCM8BIT : public DEVICE
{
protected:
	bool on, mute, realtime;
	int changed;
	int sample, prev_sample;
	uint32_t prev_clock, change_clock;
	int max_vol, last_vol_l, last_vol_r;
	int volume_l, volume_r;
	
	void update_realtime_render();
	
public:
	PCM8BIT(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("8-Bit PCM Sound"));
	}
	~PCM8BIT() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void event_frame() override;
	void __FASTCALL mix(int32_t* buffer, int cnt) override;
	void set_volume(int ch, int decibel_l, int decibel_r) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique function
	void initialize_sound(int rate, int volume);
};

#endif

