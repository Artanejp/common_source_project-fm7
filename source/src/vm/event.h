/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.29-

	[ event manager ]
*/

#ifndef _EVENT_H_
#define _EVENT_H_

//#include "vm_template.h"
//#include "../emu_template.h"
#include "device.h"

#include <typeinfo>

#define MAX_DEVICE	64
#define MAX_CPU		8
#define MAX_SOUND	32
#define MAX_LINES	1024
#define MAX_EVENT	64
#define NO_EVENT	-1
#if !defined(MAX_SOUND_IN_BUFFERS)
	#define MAX_SOUND_IN_BUFFERS 8
#else
	#if (MAX_SOUND_IN_BUFFERS <= 0)
	#define MAX_SOUND_IN_BUFFERS 8
	#endif
#endif


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

	uint32_t cpu_update_clocks[MAX_CPU][6];
	int dcount_cpu;

	bool event_half;
	int frame_clocks;
	int vline_clocks[MAX_LINES];
	int power;
	int event_remain, event_extra;
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
	// For State
	DEVICE* frame_event[MAX_EVENT];
	DEVICE* vline_event[MAX_EVENT];
	int frame_event_count, vline_event_count;
	
	double frames_per_sec, next_frames_per_sec;
	int lines_per_frame, next_lines_per_frame;
	uint32_t vline_start_clock;
	int cur_vline;
	
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

	int16_t* sound_in_tmp_buffer[MAX_SOUND_IN_BUFFERS]; // This is buffer from recording devices.
	int sound_in_rate[MAX_SOUND_IN_BUFFERS];
	int sound_in_samples[MAX_SOUND_IN_BUFFERS];
	int sound_in_channels[MAX_SOUND_IN_BUFFERS];
	int sound_in_writeptr[MAX_SOUND_IN_BUFFERS];
	int sound_in_readptr[MAX_SOUND_IN_BUFFERS];
	int sound_in_write_size[MAX_SOUND_IN_BUFFERS];
	int sound_in_read_size[MAX_SOUND_IN_BUFFERS];
	int sound_in_read_mod[MAX_SOUND_IN_BUFFERS];
	
	int dont_skip_frames;
	bool prev_skip, next_skip;
	bool sound_changed;
	
	int mix_counter;
	int mix_limit;
	bool dev_need_mix[MAX_DEVICE];
	int need_mix;
	
	void mix_sound(int samples);
	void* get_event(int index);

#ifdef _DEBUG_LOG
	bool initialize_done;
#endif
public:
	EVENT(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
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
		// reset before other device may call set_realtime_render()
		memset(dev_need_mix, 0, sizeof(dev_need_mix));
		need_mix = 0;
		
		for(int i = 0; i < MAX_SOUND_IN_BUFFERS; i++) {
			sound_in_tmp_buffer[i] = NULL;
			sound_in_rate[i] = 0;
			sound_in_samples[i] = 0;
			sound_in_channels[i] = 0;
			sound_in_readptr[i] = 0;
			sound_in_writeptr[i] = 0;
			sound_in_read_size[i] = 0;
			sound_in_write_size[i] = 0;
			sound_in_read_mod[i] = 0;
		}
#ifdef _DEBUG_LOG
		initialize_done = false;
#endif
		set_device_name(_T("Event Manager"));
	}
	~EVENT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_callback(int event_id, int err);
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// common event functions
	int get_event_manager_id()
	{
		return this_device_id;
	}
	uint32_t get_event_clocks()
	{
		return d_cpu[0].cpu_clocks;
	}
	bool is_primary_cpu(DEVICE* device)
	{
		return (d_cpu[0].device == device);
	}
	uint32_t get_cpu_clocks(DEVICE* device)
	{
		for(int index = 0; index < dcount_cpu; index++) {
			if(d_cpu[index].device == device) {
				return d_cpu[index].cpu_clocks;
			}
		}
		return CPU_CLOCKS;
	}
	void set_frames_per_sec(double new_frames_per_sec)
	{
		next_frames_per_sec = new_frames_per_sec;
	}
	void set_lines_per_frame(int new_lines_per_frame)
	{
		if(new_lines_per_frame < MAX_LINES) {
			next_lines_per_frame = new_lines_per_frame;
		}
	}
	int get_lines_per_frame()
	{
		return next_lines_per_frame;
	}
	void update_extra_event(int clock);
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
	uint32_t get_passed_clock_since_vline();
	double get_passed_usec_since_vline();
	int get_cur_vline()
	{
		return cur_vline;
	}
	int get_cur_vline_clocks()
	{
		return vline_clocks[cur_vline];
	}
	uint32_t get_cpu_pc(int index);
	void request_skip_frames();
	void touch_sound();
	void set_realtime_render(DEVICE* device, bool flag);
	uint64_t get_current_clock_uint64();
	double get_current_usec();
	uint32_t get_cpu_clock(int index);
	// unique functions
	double get_frame_rate()
	{
		return next_frames_per_sec;
	}
	void drive();
	
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
	// Sound input functions
	void clear_sound_in_source(int bank);
	int add_sound_in_source(int rate, int samples, int channels);
	int release_sound_in_source(int bank);
	
	bool is_sound_in_source_exists(int bank);
	int increment_sound_in_passed_data(int bank, double passed_usec);
	int get_sound_in_buffers_count();
	int get_sound_in_samples(int bank);
	int get_sound_in_rate(int bank);
	int get_sound_in_channels(int bank);
	int16_t* get_sound_in_buf_ptr(int bank);
	int write_sound_in_buffer(int bank, int32_t* src, int samples);
	// Add sampled values to sample buffer;value may be -32768 to +32767.
	int get_sound_in_latest_data(int bank, int32_t* dst, int expect_channels);
	int get_sound_in_data(int bank, int32_t* dst, int expect_samples, int expect_rate, int expect_channels);
	int rechannel_sound_in_data(int32_t*dst, int16_t* src, int dst_channels, int src_channels, int samples);
	
	int set_context_cpu(DEVICE* device, uint32_t clocks = CPU_CLOCKS)
	{
		assert(dcount_cpu < MAX_CPU);
		int index = dcount_cpu++;
		d_cpu[index].device = (DEVICE *)device;
		d_cpu[index].cpu_clocks = clocks;
		d_cpu[index].accum_clocks = 0;
		for(int k = 0; k < 6; k++) cpu_update_clocks[index][k] = d_cpu[index].update_clocks * k;
		return index;
	}
	bool remove_context_cpu(DEVICE* device, int num)
	{
		if(num <= 0) return false; // Number one must not be removed.
		if(num >= MAX_CPU) return false;
		if(num >= dcount_cpu) return false;
		if(dcount_cpu <= 1) return false;
		// Note: This function is dangerous.
		if(d_cpu[num].device != device) return false;
		if(d_cpu[num].device == NULL) return false;
		if(dcount_cpu == 2) {
			d_cpu[1].device = (DEVICE *)NULL;
			d_cpu[1].cpu_clocks = 0;
			d_cpu[1].accum_clocks = 0;
			dcount_cpu = 1;
			for(int k = 0; k < 6; k++)	cpu_update_clocks[1][k] = d_cpu[1].update_clocks * k;
		} else {
			for(int i = num; i < (dcount_cpu - 1); i++) {
				d_cpu[i].device = d_cpu[i + 1].device;
				d_cpu[i].cpu_clocks = d_cpu[i + 1].cpu_clocks;
				d_cpu[i].accum_clocks = d_cpu[i + 1].accum_clocks;
			}
			int n = dcount_cpu - 1;
			d_cpu[n].device = (DEVICE *)NULL;
			d_cpu[n].cpu_clocks = 0;
			d_cpu[n].accum_clocks = 0;
			for(int i = 1; i < dcount_cpu; i++) {
				for(int k = 0; k < 6; k++)	cpu_update_clocks[i][k] = d_cpu[i].update_clocks * k;
			}
			dcount_cpu = dcount_cpu - 1;
		}
		return true;
	}
	void set_secondary_cpu_clock(DEVICE* device, uint32_t clocks)
	{
		// XXX: primary cpu clock should not be changed
		for(int index = 1; index < dcount_cpu; index++) {
			if(d_cpu[index].device == device) {
				d_cpu[index].accum_clocks = 0;
				d_cpu[index].cpu_clocks = clocks;
				d_cpu[index].update_clocks = (int)(1024.0 * (double)d_cpu[index].cpu_clocks / (double)d_cpu[0].cpu_clocks + 0.5)
;
				for(int k = 0; k < 6; k++) cpu_update_clocks[index][k] = d_cpu[index].update_clocks * k;
				break;
			}
		}
	}
	void set_context_sound(DEVICE* device)
	{
		assert(dcount_sound < MAX_SOUND);
		d_sound[dcount_sound++] = device;
	}
	bool is_frame_skippable();
};

/*
 * Faster runncing cpu.
 * Expect to optimize switch(...) - case to jump table.
 * You should include real header of CPU DEVICE begin of this file.
 * -- 20180317 K.O.
 */



#endif

