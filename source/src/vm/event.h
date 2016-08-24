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
		uint32_t cpu_clocks;
		uint32_t update_clocks;
		uint32_t accum_clocks;
	} cpu_t;
	cpu_t d_cpu[MAX_CPU];
	int dcount_cpu;
	
	int vclocks[MAX_LINES];
	int power;
	int event_remain;
	int cpu_remain, cpu_accum, cpu_done;
	uint64_t event_clocks;
	
	typedef struct event_t {
		DEVICE* device;
		int event_id;
		uint64_t expired_clock;
		uint64_t loop_clock;
		uint64_t accum_clocks;
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
	
	uint16_t* sound_buffer;
	int32_t* sound_tmp;
	int buffer_ptr;
	int sound_samples;
	int sound_tmp_samples;
	
	int dont_skip_frames;
	bool prev_skip, next_skip;
	bool sound_changed;
	
	void mix_sound(int samples);
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
		set_device_name(_T("EVENT"));
	}
	~EVENT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_callback(int event_id, int err);
	void update_config();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("Event Manager");
	}
	
	// common event functions
	int get_event_manager_id()
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
	void register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id);
	void cancel_event(DEVICE* device, int register_id);
	void register_frame_event(DEVICE* device);
	void register_vline_event(DEVICE* device);
	uint32_t get_event_remaining_clock(int register_id);
	double get_event_remaining_usec(int register_id);
	uint32_t get_current_clock();
	uint32_t get_passed_clock(uint32_t prev);
	double get_passed_usec(uint32_t prev);
	uint32_t get_cpu_pc(int index);
	void request_skip_frames();
	
	// unique functions
	double get_frame_rate()
	{
		return next_frames_per_sec;
	}
	void drive();
	
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
	
	void set_context_cpu(DEVICE* device, uint32_t clocks)
	{
		int index = dcount_cpu++;
		d_cpu[index].device = device;
		d_cpu[index].cpu_clocks = clocks;
		d_cpu[index].accum_clocks = 0;
	}
	void set_secondary_cpu_clock(DEVICE* device, uint32_t clocks)
	{
		// XXX: primary cpu clock should not be changed
		for(int index = 1; index < dcount_cpu; index++) {
			if(d_cpu[index].device == device) {
				d_cpu[index].accum_clocks = 0;
				d_cpu[index].cpu_clocks = clocks;
				d_cpu[index].update_clocks = (int)(1024.0 * (double)d_cpu[index].cpu_clocks / (double)d_cpu[0].cpu_clocks + 0.5);
				break;
			}
		}
	}
	void set_context_cpu(DEVICE* device)
	{
		set_context_cpu(device, CPU_CLOCKS);
	}
	void set_context_sound(DEVICE* device)
	{
		d_sound[dcount_sound++] = device;
	}
	bool is_frame_skippable();
};

#endif

