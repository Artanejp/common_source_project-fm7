/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.29-

	[ event manager ]
*/

#ifndef _EVENT_H_
#define _EVENT_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define MAX_CPU		8
#define MAX_SOUND	8
#define MAX_LINES	1024
#define MAX_EVENT	64
#define NO_EVENT	-1

class EVENT : public DEVICE
{
private:
	// event manager
	typedef struct {
		DEVICE* device;
		int cpu_clocks;
		int update_clocks;
		int accum_clocks;
	} cpu_t;
	cpu_t d_cpu[MAX_CPU];
	int dcount_cpu;
	
	int vclocks[MAX_LINES];
	int power;
	int event_remain;
	int cpu_remain, cpu_accum, cpu_done;
	uint64 event_clocks;
	
	typedef struct event_t {
		DEVICE* device;
		int event_id;
		uint64 expired_clock;
		uint32 loop_clock;
		bool active;
		int index;
		event_t *next;
		event_t *prev;
	} event_t;
	event_t event[MAX_EVENT];
	event_t *first_free_event;
	event_t *first_fire_event;
	
	DEVICE* frame_event[MAX_EVENT];
	DEVICE* vline_event[MAX_EVENT];
	int frame_event_count, vline_event_count;
	
	double frames_per_sec, next_frames_per_sec;
	int lines_per_frame, next_lines_per_frame;
	
	void update_event(int clock);
	void insert_event(event_t *event_handle);
	
	// sound manager
	DEVICE* d_sound[MAX_SOUND];
	int dcount_sound;
	
	uint16* sound_buffer;
	int32* sound_tmp;
	int buffer_ptr;
	int sound_rate;
	int sound_samples;
	int sound_tmp_samples;
	int accum_samples, update_samples;
	
	int dont_skip_frames;
	bool prev_skip, skip;
	bool sound_changed;
	
	void mix_sound(int samples);
	void update_sound();
	void* get_event(int index);
	
#ifdef _DEBUG_LOG
	bool initialize_done;
#endif
	
public:
	EVENT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		dcount_cpu = dcount_sound = 0;
		frame_event_count = vline_event_count = 0;
		
		// initialize event
		memset(event, 0, sizeof(event));
		for(int i = 0; i < MAX_EVENT; i++) {
			event[i].active = false;
			event[i].index = i;
			event[i].next = (i + 1 < MAX_EVENT) ? &event[i + 1] : NULL;
		}
		first_free_event = &event[0];
		first_fire_event = NULL;
		
		event_clocks = 0;
		
		// force update timing in the first frame
		frames_per_sec = 0.0;
		lines_per_frame = 0;
		next_frames_per_sec = FRAMES_PER_SEC;
		next_lines_per_frame = LINES_PER_FRAME;
		
#ifdef _DEBUG_LOG
		initialize_done = false;
#endif
	}
	~EVENT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// common event functions
	int event_manager_id()
	{
		return this_device_id;
	}
	void set_frames_per_sec(double new_frames_per_sec)
	{
		next_frames_per_sec = new_frames_per_sec;
	}
	void set_lines_per_frame(int new_lines_per_frame)
	{
		next_lines_per_frame = new_lines_per_frame;
	}
	void register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id);
	void register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id);
	void cancel_event(DEVICE* device, int register_id);
	void register_frame_event(DEVICE* device);
	void register_vline_event(DEVICE* device);
	uint32 current_clock();
	uint32 passed_clock(uint32 prev);
	double passed_usec(uint32 prev);
	uint32 get_cpu_pc(int index);
	void set_skip_frames(bool value);
	
	// unique functions
	double frame_rate()
	{
		return next_frames_per_sec;
	}
	void drive();
	
	void initialize_sound(int rate, int samples);
	uint16* create_sound(int* extra_frames);
	int sound_buffer_ptr();
	
	void set_context_cpu(DEVICE* device, int clocks)
	{
		int index = dcount_cpu++;
		d_cpu[index].device = device;
		d_cpu[index].cpu_clocks = clocks;
		d_cpu[index].accum_clocks = 0;
	}
	void set_context_cpu(DEVICE* device)
	{
		set_context_cpu(device, CPU_CLOCKS);
	}
	void set_context_sound(DEVICE* device)
	{
		d_sound[dcount_sound++] = device;
	}
	bool now_skip();
};

#endif

