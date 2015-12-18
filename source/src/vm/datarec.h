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
#define SIG_DATAREC_MIX	        3
#define SIG_DATAREC_VOLUME      4
#define SIG_DATAREC_LVOLUME     5
#define SIG_DATAREC_RVOLUME     6

class FILEIO;

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

//protected:
	// data recorder
	FILEIO* play_fio;
	FILEIO* rec_fio;
	bool play, rec, remote, trigger;
	_TCHAR rec_file_path[_MAX_PATH];
	int ff_rew;
	bool in_signal, out_signal;
	uint32 prev_clock;
	int positive_clocks, negative_clocks;
	int signal_changed;
	int register_id;
	
	int sample_rate;
	double sample_usec;
	int buffer_ptr, buffer_length;
	uint8 *buffer, *buffer_bak;
#ifdef DATAREC_SOUND
	int sound_buffer_length;
	int16 *sound_buffer, sound_sample;
#endif
	int32 vol_l, vol_r;
	bool is_wav, is_tap;
	
	int apss_buffer_length;
	bool *apss_buffer;
	int apss_ptr, apss_count, apss_remain;
	bool apss_signals;
	
	int pcm_changed;
	uint32 pcm_prev_clock;
	int pcm_positive_clocks, pcm_negative_clocks;
	int pcm_max_vol, pcm_last_vol;
	
	void update_event();
	void close_file();
	
	int load_cas_image();
	int load_wav_image(int offset);
	void save_wav_image();
	int load_m5_cas_image();
	int load_msx_cas_image();
	int load_p6_image(bool is_p6t);
	int load_tap_image();
	int load_t77_image();
	int load_mzt_image();
	
public:
	DATAREC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_ear);
		init_output_signals(&outputs_remote);
		init_output_signals(&outputs_rotate);
		init_output_signals(&outputs_end);
		init_output_signals(&outputs_top);
		init_output_signals(&outputs_apss);
#ifdef DATAREC_PCM_VOLUME
		pcm_max_vol = DATAREC_PCM_VOLUME;
#else
		pcm_max_vol = 8000;
#endif
	}
	~DATAREC() {}
	
	// common functions
	void initialize();
	void reset();
	void release();
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int ch)
	{
		return in_signal ? 1 : 0;
	}
	void event_frame();
	void event_callback(int event_id, int err);
	void mix(int32* sndbuffer, int cnt);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name(void)
	{
		return "CMT_DATA_RECORDER";
	}

	// unique functions
	void init_pcm(int rate, int volume)
	{
		pcm_max_vol = volume;
	}
	
	// unique functions
	void set_context_ear(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_ear, device, id, mask);
	}
	void set_context_remote(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_remote, device, id, mask);
	}
	void set_context_rotate(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_rotate, device, id, mask);
	}
	void set_context_end(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_end, device, id, mask);
	}
	void set_context_top(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_top, device, id, mask);
	}
	void set_context_apss(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_apss, device, id, mask);
	}
	bool play_tape(const _TCHAR* file_path);
	bool rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (play || rec);
	}
	bool tape_playing()
	{
		return (remote && play);
	}
	bool tape_recording()
	{
		return (remote && rec);
	}
	int tape_position()
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
	void set_remote(bool value);
	void set_ff_rew(int value);
	bool do_apss(int value);
};

#endif

