/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ data recorder ]
*/

#ifndef _DREC_H_
#define _DREC_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_DATAREC_MIC		0
#define SIG_DATAREC_REMOTE	1
#define SIG_DATAREC_TRIG	2

class FILEIO;
class NOISE;

class DATAREC : public DEVICE
{
private:
	// output signals
	outputs_t outputs_ear;
	outputs_t outputs_remote;
	outputs_t outputs_rotate;
	outputs_t outputs_end;
	outputs_t outputs_top;
	outputs_t outputs_apss;
	
	// cmt noise
	NOISE* d_noise_play;
	NOISE* d_noise_stop;
	NOISE* d_noise_fast;
	
	// data recorder
	FILEIO* play_fio;
	FILEIO* rec_fio;
	
	bool play, rec, remote, trigger;
	_TCHAR rec_file_path[_MAX_PATH];
	int ff_rew;
	bool in_signal, out_signal;
	uint32_t prev_clock;
	int positive_clocks, negative_clocks;
	int signal_changed;
	int register_id;
	bool realtime;
	
	int sample_rate;
	double sample_usec;
	int buffer_ptr, buffer_length;
	uint8_t *buffer, *buffer_bak;
#ifdef DATAREC_SOUND
	int sound_buffer_length;
	int16_t *sound_buffer, sound_sample;
#endif
	bool is_wav, is_tap, is_t77;
	double ave_hi_freq;
	
	int apss_buffer_length;
	bool *apss_buffer;
	int apss_ptr, apss_count, apss_remain;
	bool apss_signals;
	
	int pcm_changed;
	uint32_t pcm_prev_clock;
	int pcm_positive_clocks, pcm_negative_clocks;
	int pcm_max_vol;
	int32_t pcm_last_vol_l, pcm_last_vol_r;
	int pcm_volume_l, pcm_volume_r;
#ifdef DATAREC_SOUND
	int32_t sound_last_vol_l, sound_last_vol_r;
	int sound_volume_l, sound_volume_r;
#endif
	_TCHAR message[1024];
	
	void update_event();
	void update_realtime_render();
	void close_file();
	
	int load_wav_image(int offset);
	void save_wav_image();
	int load_t77_image();
	int load_tap_image();
	int load_mzt_image();
	int load_p6_image(bool is_p6t);
	int load_bmjr_image();
	int load_cas_image();
	int load_m5_cas_image();
	int load_msx_cas_image();
	
public:
	DATAREC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_ear);
		initialize_output_signals(&outputs_remote);
		initialize_output_signals(&outputs_rotate);
		initialize_output_signals(&outputs_end);
		initialize_output_signals(&outputs_top);
		initialize_output_signals(&outputs_apss);
		d_noise_play = NULL;
		d_noise_stop = NULL;
		d_noise_fast = NULL;
#ifdef DATAREC_PCM_VOLUME
		pcm_max_vol = DATAREC_PCM_VOLUME;
#else
		pcm_max_vol = 8000;
#endif
		pcm_volume_l = pcm_volume_r = 1024;
#ifdef DATAREC_SOUND
		sound_volume_l = sound_volume_r = 1024;
#endif
		my_tcscpy_s(message, _T("Stop"));
		drive_num = 0;
		set_device_name(_T("Data Recorder"));
	}
	~DATAREC() {}
	
	// common functions
	void initialize();
	void reset();
	void release();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch)
	{
		return in_signal ? 1 : 0;
	}
	void event_frame();
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void initialize_sound(int rate, int volume)
	{
		pcm_max_vol = volume;
	}
	void set_context_ear(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ear, device, id, mask);
	}
	void set_context_remote(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_remote, device, id, mask);
	}
	void set_context_rotate(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rotate, device, id, mask);
	}
	void set_context_end(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_end, device, id, mask);
	}
	void set_context_top(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_top, device, id, mask);
	}
	void set_context_apss(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_apss, device, id, mask);
	}
	void set_context_noise_play(NOISE* device)
	{
		d_noise_play = device;
	}
	NOISE* get_context_noise_play()
	{
		return d_noise_play;
	}
	void set_context_noise_stop(NOISE* device)
	{
		d_noise_stop = device;
	}
	NOISE* get_context_noise_stop()
	{
		return d_noise_stop;
	}
	void set_context_noise_fast(NOISE* device)
	{
		d_noise_fast = device;
	}
	NOISE* get_context_noise_fast()
	{
		return d_noise_fast;
	}
	bool play_tape(const _TCHAR* file_path);
	bool rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (play || rec);
	}
	bool is_tape_playing()
	{
		return (remote && play);
	}
	bool is_tape_recording()
	{
		return (remote && rec);
	}
	int get_tape_position()
	{
		if(play && buffer_length > 0) {
			if(buffer_ptr >= buffer_length) {
				return 100;
			} else if(buffer_ptr <= 0) {
				return 0;
			} else {
				return (int)(((double)buffer_ptr / (double)buffer_length) * 100.0);
			}
		}
		return 0;
	}
	const _TCHAR* get_message()
	{
		return message;
	}
	bool get_remote()
	{
		return remote;
	}
	void set_remote(bool value);
	void set_ff_rew(int value);
	bool do_apss(int value);
	double get_ave_hi_freq();
	int drive_num;
};

#endif

