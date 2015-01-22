/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.29-

	[ event manager ]
*/

#include "event.h"
#include "../fileio.h"

#ifndef EVENT_CONTINUOUS_SOUND
//#ifdef PCM1BIT_HIGH_QUALITY
#define EVENT_CONTINUOUS_SOUND
//#endif
#endif

void EVENT::initialize()
{
	// load config
	if(!(0 <= config.cpu_power && config.cpu_power <= 4)) {
		config.cpu_power = 0;
	}
	power = config.cpu_power;
	
	// initialize sound buffer
	sound_buffer = NULL;
	sound_tmp = NULL;
	
	dont_skip_frames = 0;
	prev_skip = next_skip = false;
	sound_changed = false;
}

void EVENT::initialize_sound(int rate, int samples)
{
	// initialize sound
	sound_rate = rate;
	sound_samples = samples;
#ifdef EVENT_CONTINUOUS_SOUND
	sound_tmp_samples = samples * 2;
#else
	sound_tmp_samples = samples;
#endif
	sound_buffer = (uint16*)malloc(sound_samples * sizeof(uint16) * 2);
	memset(sound_buffer, 0, sound_samples * sizeof(uint16) * 2);
	sound_tmp = (int32*)malloc(sound_tmp_samples * sizeof(int32) * 2);
	memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32) * 2);
	buffer_ptr = accum_samples = 0;
}

void EVENT::release()
{
	// release sound
	if(sound_buffer) {
		free(sound_buffer);
	}
	if(sound_tmp) {
		free(sound_tmp);
	}
}

void EVENT::reset()
{
	// clear events except loop event
	for(int i = 0; i < MAX_EVENT; i++) {
		if(event[i].active && event[i].loop_clock == 0) {
			cancel_event(NULL, i);
		}
	}
	
	event_remain = 0;
	cpu_remain = cpu_accum = cpu_done = 0;
	
	// reset sound
	if(sound_buffer) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16) * 2);
	}
	if(sound_tmp) {
		memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32) * 2);
	}
//	buffer_ptr = 0;
	
#ifdef _DEBUG_LOG
	initialize_done = true;
#endif
}

void EVENT::drive()
{
	// raise pre frame events to update timing settings
	for(int i = 0; i < frame_event_count; i++) {
		frame_event[i]->event_pre_frame();
	}
	
	// generate clocks per line
	if(frames_per_sec != next_frames_per_sec || lines_per_frame != next_lines_per_frame) {
		frames_per_sec = next_frames_per_sec;
		lines_per_frame = next_lines_per_frame;
		
		int sum = (int)((double)d_cpu[0].cpu_clocks / frames_per_sec + 0.5);
		int remain = sum;
		
		for(int i = 0; i < lines_per_frame; i++) {
			vclocks[i] = (int)(sum / lines_per_frame);
			remain -= vclocks[i];
		}
		for(int i = 0; i < remain; i++) {
			int index = (int)((double)lines_per_frame * (double)i / (double)remain);
			vclocks[index]++;
		}
		for(int i = 1; i < dcount_cpu; i++) {
			d_cpu[i].update_clocks = (int)(1024.0 * (double)d_cpu[i].cpu_clocks / (double)d_cpu[0].cpu_clocks + 0.5);
		}
		for(DEVICE* device = vm->first_device; device; device = device->next_device) {
			if(device->event_manager_id() == this_device_id) {
				device->update_timing(d_cpu[0].cpu_clocks, frames_per_sec, lines_per_frame);
			}
		}
		update_samples = (int)(1024.0 * (double)sound_rate / frames_per_sec / (double)lines_per_frame + 0.5);
	}
	
	// run virtual machine for 1 frame period
	for(int i = 0; i < frame_event_count; i++) {
		frame_event[i]->event_frame();
	}
	for(int v = 0; v < lines_per_frame; v++) {
		// run virtual machine per line
		for(int i = 0; i < vline_event_count; i++) {
			vline_event[i]->event_vline(v, vclocks[v]);
		}
		
		if(event_remain < 0) {
			if(-event_remain > vclocks[v]) {
				update_event(vclocks[v]);
			} else {
				update_event(-event_remain);
			}
		}
		event_remain += vclocks[v];
		cpu_remain += vclocks[v] << power;
		
		while(event_remain > 0) {
			int event_done = event_remain;
			if(cpu_remain > 0) {
				// run one opecode on primary cpu
				int cpu_done_tmp;
				if(dcount_cpu == 1) {
					cpu_done_tmp = d_cpu[0].device->run(-1);
				} else {
					// sync to sub cpus
					if(cpu_done == 0) {
						cpu_done = d_cpu[0].device->run(-1);
					}
					cpu_done_tmp = (cpu_done < 4) ? cpu_done : 4;
					cpu_done -= cpu_done_tmp;
					
					for(int i = 1; i < dcount_cpu; i++) {
						// run sub cpus
						d_cpu[i].accum_clocks += d_cpu[i].update_clocks * cpu_done_tmp;
						int sub_clock = d_cpu[i].accum_clocks >> 10;
						if(sub_clock) {
							d_cpu[i].accum_clocks -= sub_clock << 10;
							d_cpu[i].device->run(sub_clock);
						}
					}
				}
				cpu_remain -= cpu_done_tmp;
				cpu_accum += cpu_done_tmp;
				event_done = cpu_accum >> power;
				cpu_accum -= event_done << power;
			}
			if(event_done > 0) {
				if(event_done > event_remain) {
					update_event(event_remain);
				} else {
					update_event(event_done);
				}
				event_remain -= event_done;
			}
		}
		update_sound();
	}
}

void EVENT::update_event(int clock)
{
	uint64 event_clocks_tmp = event_clocks + clock;
	
	while(first_fire_event != NULL && first_fire_event->expired_clock <= event_clocks_tmp) {
		event_t *event_handle = first_fire_event;
		uint64 expired_clock = event_handle->expired_clock;
		
		first_fire_event = event_handle->next;
		if(first_fire_event != NULL) {
			first_fire_event->prev = NULL;
		}
		if(event_handle->loop_clock != 0) {
			event_handle->expired_clock += event_handle->loop_clock;
			insert_event(event_handle);
		} else {
			event_handle->active = false;
			event_handle->next = first_free_event;
			first_free_event = event_handle;
		}
		event_clocks = expired_clock;
		event_handle->device->event_callback(event_handle->event_id, 0);
	}
	event_clocks = event_clocks_tmp;
}

uint32 EVENT::current_clock()
{
	return (uint32)(event_clocks & 0xffffffff);
}

uint32 EVENT::passed_clock(uint32 prev)
{
	uint32 current = current_clock();
	return (current > prev) ? current - prev : current + (0xffffffff - prev) + 1;
}

double EVENT::passed_usec(uint32 prev)
{
	return 1000000.0 * passed_clock(prev) / d_cpu[0].cpu_clocks;
}

uint32 EVENT::get_cpu_pc(int index)
{
	return d_cpu[index].device->get_pc();
}

void EVENT::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
{
	int clock = (int)((double)d_cpu[0].cpu_clocks / 1000000.0 * usec + 0.5);
	register_event_by_clock(device, event_id, clock, loop, register_id);
}

void EVENT::register_event_by_clock(DEVICE* device, int event_id, int clock, bool loop, int* register_id)
{
#ifdef _DEBUG_LOG
	if(!initialize_done && !loop) {
		emu->out_debug_log(_T("EVENT: non-loop event is registered before initialize is done\n"));
	}
#endif
	
	// register event
	if(first_free_event == NULL) {
#ifdef _DEBUG_LOG
		emu->out_debug_log(_T("EVENT: too many events !!!\n"));
#endif
		if(register_id != NULL) {
			*register_id = -1;
		}
		return;
	}
	event_t *event_handle = first_free_event;
	first_free_event = first_free_event->next;
	
	if(register_id != NULL) {
		*register_id = event_handle->index;
	}
	event_handle->active = true;
	event_handle->device = device;
	event_handle->event_id = event_id;
	event_handle->expired_clock = event_clocks + clock;
	event_handle->loop_clock = loop ? clock : 0;
	
	insert_event(event_handle);
}

void EVENT::insert_event(event_t *event_handle)
{
	if(first_fire_event == NULL) {
		first_fire_event = event_handle;
		event_handle->prev = event_handle->next = NULL;
	} else {
		for(event_t *insert_pos = first_fire_event; insert_pos != NULL; insert_pos = insert_pos->next) {
			if(insert_pos->expired_clock > event_handle->expired_clock) {
				if(insert_pos->prev != NULL) {
					// insert
					insert_pos->prev->next = event_handle;
					event_handle->prev = insert_pos->prev;
					event_handle->next = insert_pos;
					insert_pos->prev = event_handle;
					break;
				} else {
					// add to head
					first_fire_event = event_handle;
					event_handle->prev = NULL;
					event_handle->next = insert_pos;
					insert_pos->prev = event_handle;
					break;
				}
			} else if(insert_pos->next == NULL) {
				// add to tail
				insert_pos->next = event_handle;
				event_handle->prev = insert_pos;
				event_handle->next = NULL;
				break;
			}
		}
	}
}

void EVENT::cancel_event(DEVICE* device, int register_id)
{
	// cancel registered event
	if(0 <= register_id && register_id < MAX_EVENT) {
		event_t *event_handle = &event[register_id];
		if(device != NULL && device != event_handle->device) {
			emu->out_debug_log("EVENT: event cannot be canceled by non ownew device (id=%d) !!!\n", device->this_device_id);
			return;
		}
		if(event_handle->active) {
			if(event_handle->prev != NULL) {
				event_handle->prev->next = event_handle->next;
			} else {
				first_fire_event = event_handle->next;
			}
			if(event_handle->next != NULL) {
				event_handle->next->prev = event_handle->prev;
			}
			event_handle->active = false;
			event_handle->next = first_free_event;
			first_free_event = event_handle;
		}
	}
}

void EVENT::register_frame_event(DEVICE* dev)
{
	if(frame_event_count < MAX_EVENT) {
		frame_event[frame_event_count++] = dev;
	} else {
#ifdef _DEBUG_LOG
		emu->out_debug_log(_T("EVENT: too many frame events !!!\n"));
#endif
	}
}

void EVENT::register_vline_event(DEVICE* dev)
{
	if(vline_event_count < MAX_EVENT) {
		vline_event[vline_event_count++] = dev;
	} else {
#ifdef _DEBUG_LOG
		emu->out_debug_log(_T("EVENT: too many vline events !!!\n"));
#endif
	}
}

void EVENT::mix_sound(int samples)
{
	if(samples > 0) {
		int32* buffer = sound_tmp + buffer_ptr * 2;
		memset(buffer, 0, samples * sizeof(int32) * 2);
		for(int i = 0; i < dcount_sound; i++) {
			d_sound[i]->mix(buffer, samples);
		}
		if(!sound_changed) {
			for(int i = 0; i < samples * 2; i += 2) {
				if(buffer[i] != sound_tmp[0] || buffer[i + 1] != sound_tmp[1]) {
					sound_changed = true;
					break;
				}
			}
		}
		buffer_ptr += samples;
	} else {
		// notify to sound devices
		for(int i = 0; i < dcount_sound; i++) {
			d_sound[i]->mix(sound_tmp + buffer_ptr * 2, 0);
		}
	}
}

void EVENT::update_sound()
{
	accum_samples += update_samples;
	int samples = accum_samples >> 10;
	accum_samples -= samples << 10;
	
	// mix sound
	if(prev_skip && dont_skip_frames == 0 && !sound_changed) {
		buffer_ptr = 0;
	}
	if(samples > sound_tmp_samples - buffer_ptr) {
		samples = sound_tmp_samples - buffer_ptr;
	}
	mix_sound(samples);
}

uint16* EVENT::create_sound(int* extra_frames)
{
	if(prev_skip && dont_skip_frames == 0 && !sound_changed) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16) * 2);
		*extra_frames = 0;
		return sound_buffer;
	}
	int frames = 0;
	
#ifdef EVENT_CONTINUOUS_SOUND
	// drive extra frames to fill the sound buffer
	while(sound_samples > buffer_ptr) {
		drive();
		frames++;
	}
#else
	// fill sound buffer
	int samples = sound_samples - buffer_ptr;
	mix_sound(samples);
#endif
#ifdef LOW_PASS_FILTER
	// low-pass filter
	for(int i = 0; i < sound_samples - 1; i++) {
		sound_tmp[i * 2    ] = (sound_tmp[i * 2    ] + sound_tmp[i * 2 + 2]) / 2; // L
		sound_tmp[i * 2 + 1] = (sound_tmp[i * 2 + 1] + sound_tmp[i * 2 + 3]) / 2; // R
	}
#endif
	// copy to buffer
	for(int i = 0; i < sound_samples * 2; i++) {
		int dat = sound_tmp[i];
		uint16 highlow = (uint16)(dat & 0x0000ffff);
		
		if((dat > 0) && (highlow >= 0x8000)) {
			sound_buffer[i] = 0x7fff;
			continue;
		}
		if((dat < 0) && (highlow < 0x8000)) {
			sound_buffer[i] = 0x8000;
			continue;
		}
		sound_buffer[i] = highlow;
	}
	if(buffer_ptr > sound_samples) {
		buffer_ptr -= sound_samples;
		memcpy(sound_tmp, sound_tmp + sound_samples * 2, buffer_ptr * sizeof(int32) * 2);
	} else {
		buffer_ptr = 0;
	}
	*extra_frames = frames;
	return sound_buffer;
}

int EVENT::sound_buffer_ptr()
{
	return buffer_ptr;
}

void EVENT::request_skip_frames()
{
	next_skip = true;
}

bool EVENT::now_skip()
{
	bool value = next_skip;
	
	if(sound_changed || (prev_skip && !next_skip)) {
		dont_skip_frames = (int)frames_per_sec;
	}
	if(dont_skip_frames > 0) {
		value = false;
		dont_skip_frames--;
	}
	prev_skip = next_skip;
	next_skip = false;
	sound_changed = false;
	
	return value;
}

void EVENT::update_config()
{
	if(power != config.cpu_power) {
		power = config.cpu_power;
		cpu_accum = 0;
	}
}

#define STATE_VERSION	1

void EVENT::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(dcount_cpu);
	for(int i = 0; i < dcount_cpu; i++) {
		state_fio->FputInt32(d_cpu[i].cpu_clocks);
		state_fio->FputInt32(d_cpu[i].update_clocks);
		state_fio->FputInt32(d_cpu[i].accum_clocks);
	}
	state_fio->Fwrite(vclocks, sizeof(vclocks), 1);
	state_fio->FputInt32(event_remain);
	state_fio->FputInt32(cpu_remain);
	state_fio->FputInt32(cpu_accum);
	state_fio->FputInt32(cpu_done);
	state_fio->FputUint64(event_clocks);
	for(int i = 0; i < MAX_EVENT; i++) {
		state_fio->FputInt32(event[i].device != NULL ? event[i].device->this_device_id : -1);
		state_fio->FputInt32(event[i].event_id);
		state_fio->FputUint64(event[i].expired_clock);
		state_fio->FputUint32(event[i].loop_clock);
		state_fio->FputBool(event[i].active);
		state_fio->FputInt32(event[i].next != NULL ? event[i].next->index : -1);
		state_fio->FputInt32(event[i].prev != NULL ? event[i].prev->index : -1);
	}
	state_fio->FputInt32(first_free_event != NULL ? first_free_event->index : -1);
	state_fio->FputInt32(first_fire_event != NULL ? first_fire_event->index : -1);
	state_fio->FputDouble(frames_per_sec);
	state_fio->FputDouble(next_frames_per_sec);
	state_fio->FputInt32(lines_per_frame);
	state_fio->FputInt32(next_lines_per_frame);
}

bool EVENT::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(state_fio->FgetInt32() != dcount_cpu) {
		return false;
	}
	for(int i = 0; i < dcount_cpu; i++) {
		d_cpu[i].cpu_clocks = state_fio->FgetInt32();
		d_cpu[i].update_clocks = state_fio->FgetInt32();
		d_cpu[i].accum_clocks = state_fio->FgetInt32();
	}
	state_fio->Fread(vclocks, sizeof(vclocks), 1);
	event_remain = state_fio->FgetInt32();
	cpu_remain = state_fio->FgetInt32();
	cpu_accum = state_fio->FgetInt32();
	cpu_done = state_fio->FgetInt32();
	event_clocks = state_fio->FgetUint64();
	for(int i = 0; i < MAX_EVENT; i++) {
		event[i].device = vm->get_device(state_fio->FgetInt32());
		event[i].event_id = state_fio->FgetInt32();
		event[i].expired_clock = state_fio->FgetUint64();
		event[i].loop_clock = state_fio->FgetUint32();
		event[i].active = state_fio->FgetBool();
		event[i].next = (event_t *)get_event(state_fio->FgetInt32());
		event[i].prev = (event_t *)get_event(state_fio->FgetInt32());
	}
	first_free_event = (event_t *)get_event(state_fio->FgetInt32());
	first_fire_event = (event_t *)get_event(state_fio->FgetInt32());
	frames_per_sec = state_fio->FgetDouble();
	next_frames_per_sec = state_fio->FgetDouble();
	lines_per_frame = state_fio->FgetInt32();
	next_lines_per_frame = state_fio->FgetInt32();
	
	// post process
	if(sound_buffer) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16) * 2);
	}
	if(sound_tmp) {
		memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32) * 2);
	}
	buffer_ptr = accum_samples = 0;
	update_samples = (int)(1024.0 * (double)sound_rate / frames_per_sec / (double)lines_per_frame + 0.5);
	return true;
}

void* EVENT::get_event(int index)
{
	if(index >= 0 && index < MAX_EVENT) {
		return &event[index];
	}
	return NULL;
}
