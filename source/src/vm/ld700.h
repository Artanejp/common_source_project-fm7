/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2014.02.12-

	[ Pioneer LD-700 ]
*/

#ifndef _LD700_H_
#define _LD700_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_LD700_REMOTE	0
#define SIG_LD700_MUTE_L	1
#define SIG_LD700_MUTE_R	2

#define MAX_TRACKS	1024
#define MAX_PAUSES	1024

class FIFO;

class LD700 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_exv;
	outputs_t outputs_ack;
	outputs_t outputs_sound;
	
	bool prev_remote_signal;
	uint32_t prev_remote_time;
	uint32_t command, num_bits;
	
	int phase, status;
	int seek_mode, seek_num;
	bool accepted;
	int cur_frame_raw;
	int wait_frame_raw;
	
	int num_tracks, track_frame_raw[MAX_TRACKS];
	int num_pauses, pause_frame_raw[MAX_PAUSES];
	
	bool prev_sound_signal;
	FIFO *sound_buffer_l, *sound_buffer_r, *signal_buffer;
	bool signal_buffer_ok;
	int sound_event_id;
	int16_t sound_sample_l, sound_sample_r;
	int volume_l, volume_r;
	bool sound_mute_l, sound_mute_r;
	
	int16_t *mix_buffer_l, *mix_buffer_r;
	int mix_buffer_ptr, mix_buffer_length;
	
	void set_status(int value);
	void set_ack(bool value);
	void set_cur_frame(int frame, bool relative);
	int get_cur_frame_raw();
	void set_cur_track(int track);
	
public:
	LD700(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_exv);
		initialize_output_signals(&outputs_ack);
		initialize_output_signals(&outputs_sound);
		volume_l = volume_r = 1024;
		sound_mute_l = sound_mute_r = true;
		set_device_name(_T("LD-700 Laser Disc Player"));
	}
	~LD700() {}
	
	// common functions
	void initialize();
	void release();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void event_frame();
	void event_callback(int event_id, int err);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	
	// unique functions
	void set_context_exv(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_exv, device, id, mask);
	}
	void set_context_ack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ack, device, id, mask);
	}
	void set_context_sound(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_sound, device, id, mask);
	}
	void open_disc(const _TCHAR* file_path);
	void close_disc();
	bool is_disc_inserted();
	void initialize_sound(int rate, int samples);
	void movie_sound_callback(uint8_t *buffer, long size);
};

#endif

