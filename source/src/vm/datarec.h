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
#include "../config.h"
#include "device.h"
#ifdef _USE_QT
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif
#endif

#define SIG_DATAREC_OUT		0
#define SIG_DATAREC_REMOTE	1
#define SIG_DATAREC_TRIG	2
#define SIG_DATAREC_MIX	        3
#define SIG_DATAREC_VOLUME      4

class FILEIO;

class DATAREC : public DEVICE
{
private:
	// output signals
	outputs_t outputs_out;
	outputs_t outputs_remote;
	outputs_t outputs_rotate;
	outputs_t outputs_end;
	outputs_t outputs_top;
	outputs_t outputs_apss;

protected:
	// data recorder
	FILEIO* play_fio;
	FILEIO* rec_fio;
	bool play, rec, remote, trigger;
#if defined(_USE_AGAR)
	_TCHAR rec_file_path[AG_PATHNAME_MAX];
#else
	_TCHAR rec_file_path[MAX_PATH];
#endif	
	int ff_rew;
	bool in_signal, out_signal;
	uint32 prev_clock;
	int positive_clocks, negative_clocks;
	int signal_changed;
	int register_id;
	
	int sample_rate;
	int buffer_ptr, buffer_length;
	uint8 *buffer, *buffer_bak;
#ifdef DATAREC_SOUND
	int sound_buffer_length;
	int16 *sound_buffer, sound_sample;
	bool mix_datarec_sound;
	int16 mix_datarec_volume;
#endif
	bool is_wav;
	
	int apss_buffer_length;
	bool *apss_buffer;
	int apss_ptr, apss_count, apss_remain;
	bool apss_signals;
	
	void update_event();
	void close_file();
	
	int load_cas_image();
	int load_wav_image(int offset);
	void save_wav_image();
	int load_m5_cas_image();
	int load_p6_image(bool is_p6t);
	int load_tap_image();
	int load_mzt_image();
#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)   
        unsigned int min(int x, unsigned int y) {
	   if((unsigned int)x < y) return (unsigned int)x;
	   return y;
	}
        unsigned int max(int x, unsigned int y) {
	   if((unsigned int)x > y) return (unsigned int)x;
	   return y;
	}
#endif   
public:
	DATAREC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_out);
		init_output_signals(&outputs_remote);
		init_output_signals(&outputs_rotate);
		init_output_signals(&outputs_end);
		init_output_signals(&outputs_top);
		init_output_signals(&outputs_apss);
#ifdef DATAREC_SOUND
		mix_datarec_sound = false;
		mix_datarec_volume = 0x1800;
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
	virtual void event_callback(int event_id, int err);
#ifdef DATAREC_SOUND
	void mix(int32* sndbuffer, int cnt);
#endif	
	void update_config(void);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_out(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_out, device, id, mask);
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
	virtual bool play_tape(_TCHAR* file_path);
	bool rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (play || rec);
	}
	void set_remote(bool value);
	void set_ff_rew(int value);
	bool do_apss(int value);
        virtual int get_tape_ptr(void) {
		if((buffer_length == 0) || (buffer == NULL)) return -1;
		return (100 * buffer_ptr) / buffer_length;
	};
   
//#ifdef DATAREC_SOUND
//	void initialize_sound(int rate, int samples);
//#endif
};

#endif

