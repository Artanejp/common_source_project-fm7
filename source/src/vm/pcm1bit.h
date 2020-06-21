/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.02.09 -

	[ 1bit PCM ]
*/

#ifndef _PCM1BIT_H_
#define _PCM1BIT_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_PCM1BIT_SIGNAL	0
#define SIG_PCM1BIT_ON		1
#define SIG_PCM1BIT_MUTE	2

class VM;
class EMU;
class PCM1BIT : public DEVICE
{
private:
	bool signal, on, mute;
	int changed;
	uint32_t prev_clock;
	int positive_clocks, negative_clocks;
	int max_vol, last_vol_l, last_vol_r;
	int volume_l, volume_r;
	bool realtime;

	bool before_on;
	bool use_lpf;
	bool use_hpf;
	int32_t lpf_freq;
	int32_t hpf_freq;
	float before_filter_l;
	float before_filter_r;
	float hpf_alpha;
	float hpf_ialpha;
	float lpf_alpha;
	float lpf_ialpha;
	
	int sample_rate;
	void calc_low_pass_filter(int32_t* dst, int32_t* src, int samples, int is_set_val);
	void calc_high_pass_filter(int32_t* dst, int32_t* src, int samples, int is_set_val);
	
public:
	PCM1BIT(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("1BIT PCM SOUND"));
	}
	~PCM1BIT() {}
	
	// common functions
	void initialize();
	void reset();
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void set_high_pass_filter_freq(int freq, double quality = 1.0);
	void set_low_pass_filter_freq(int freq, double quality = 1.0);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	bool is_debugger_available()
	{
		return true;
	}
	
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void initialize_sound(int rate, int volume);
	
};

#endif

