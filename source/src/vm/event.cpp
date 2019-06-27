/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.11.29-

	[ event manager ]
*/

/*
 * BELOW INCLUDES ARE for run_cpu().
 * ToDo: Minimum include.
 */
#include "vm.h"

#if defined(USE_CPU_HD6301)
#include "hd6301.h"
#endif

#if defined(USE_CPU_HUC6280)
#include "huc6280.h"
#endif

#if defined(USE_CPU_I86) || defined(USE_CPU_I186) || defined(USE_CPU_I88)
#include "i86.h"
#endif

#if defined(USE_CPU_V30)
#include "v30.h"
#endif

#if defined(USE_CPU_I286)
#include "i286.h"
#endif

#if defined(USE_CPU_I386) || defined(USE_CPU_I486) || defined(USE_CPU_PENTIUM)
#include "i386.h"
#endif

#if defined(USE_CPU_I8080)
#include "i8080.h"
#endif

#if defined(USE_CPU_M6502) || defined(USE_CPU_N2A03)
#include "m6502.h"
#endif

#if defined(USE_CPU_MB8861)
#include "mb8861.h"
#endif

#if defined(USE_CPU_MC6800)
#include "mc6800.h"
#endif

#if defined(USE_CPU_MC6801)
#include "mc6801.h"
#endif

#if defined(USE_CPU_MC6809)
#include "mc6809.h"
#endif

#if defined(USE_CPU_MCS48)
#include "mcs48.h"
#endif

#if defined(USE_CPU_TMS9995)
#include "tms9995.h"
#endif

#if defined(USE_CPU_UPD7801)
#include "upd7801.h"
#endif

#if defined(USE_CPU_Z80)
#include "z80.h"
#endif

#include "event.h"

#define EVENT_MIX	0
#define EVENT_VLINE	1

void EVENT::initialize()
{
	DEVICE::initialize();
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
	
	// temporary
	frame_clocks = (int)((double)d_cpu[0].cpu_clocks / (double)FRAMES_PER_SEC + 0.5);
	vline_start_clock = 0;
	cur_vline = 0;
	vline_clocks[0] = (int)((double)d_cpu[0].cpu_clocks / (double)FRAMES_PER_SEC / (double)LINES_PER_FRAME + 0.5);
}

void EVENT::initialize_sound(int rate, int samples)
{
	// initialize sound
	sound_samples = samples;
	sound_tmp_samples = samples * 2;
	sound_buffer = (uint16_t*)malloc(sound_samples * sizeof(uint16_t) * 2);
	memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
	sound_tmp = (int32_t*)malloc(sound_tmp_samples * sizeof(int32_t) * 2);
	memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
	buffer_ptr = 0;
	mix_counter = 1;
	mix_limit = (int)((double)(emu->get_sound_rate() / 2000.0)); // per 0.5ms.
	// ToDo: Lock Mutex
	for(int i = 0; i < MAX_SOUND_IN_BUFFERS; i++) {
		if(sound_in_rate[i] != rate) {
			release_sound_in_source(i);
			sound_in_samples[i] = samples;
		}
		if((sound_in_samples[i] > 0) && (sound_in_channels[i] > 0)) {
			if(sound_in_tmp_buffer[i] == NULL) {
				sound_in_tmp_buffer[i] = (int16_t*)malloc(sound_in_samples[i] * sound_in_channels[i] * sizeof(int16_t));
				if(sound_in_tmp_buffer[i] != NULL) {
					clear_sound_in_source(i);
				}
			}
		}
	}
	// ToDo: UnLock Mutex
	// register event
	this->register_event(this, EVENT_MIX, 1000000.0 / rate, true, NULL);
}

void EVENT::clear_sound_in_source(int bank)
{
	if(bank < 0) return;
	if(bank >= MAX_SOUND_IN_BUFFERS) return;

	if(sound_in_tmp_buffer[bank] == NULL) return;
	if(sound_in_samples[bank] <= 0) return;
	if(sound_in_channels[bank] <= 0) return;
	memset(sound_in_tmp_buffer[bank], 0x00, sound_in_samples[bank] * sound_in_channels[bank] * sizeof(int16_t));
}

int EVENT::add_sound_in_source(int rate, int samples, int channels)
{
	int banknum;
	for(banknum = 0; banknum < MAX_SOUND_IN_BUFFERS; banknum++) {
		if(sound_in_samples[banknum] == 0) break;
	}
	if(banknum < MAX_SOUND_IN_BUFFERS) {
	// ToDo: Lock Mutex
		if(sound_in_tmp_buffer[banknum] != NULL) free(sound_in_tmp_buffer[banknum]);
		sound_in_tmp_buffer[banknum] = NULL;
		if((rate > 0) && (samples > 0)) {
			sound_in_rate[banknum] = rate;
			sound_in_samples[banknum] = samples;
			sound_in_channels[banknum] = channels;
			sound_in_tmp_buffer[banknum] = (int16_t*)malloc(samples * sizeof(int16_t) * channels);
		}
	// ToDo: UnLock Mutex
		sound_in_writeptr[banknum] = 0;
		sound_in_readptr[banknum] = 0;
		sound_in_write_size[banknum] = 0;
		sound_in_read_size[banknum] = 0;
		sound_in_read_mod[banknum] = 0;
		return banknum;
	}
	return -1; // error
}

int EVENT::release_sound_in_source(int bank)
{
	if(bank < 0) return -1;
	if(bank >= MAX_SOUND_IN_BUFFERS) return -1;
	// ToDo: Lock Mutex
	if(sound_in_tmp_buffer[bank] != NULL) {
		free(sound_in_tmp_buffer[bank]);
		sound_in_tmp_buffer[bank] = NULL;
	}
	sound_in_readptr[bank] = 0;
	sound_in_writeptr[bank] = 0;
	sound_in_read_size[bank] = 0;
	sound_in_write_size[bank] = 0;
	clear_sound_in_source(bank);
	// ToDo: UnLock Mutex
	return bank;
}

void EVENT::release()
{
	// release sound
	for(int i = 0; i < MAX_SOUND_IN_BUFFERS; i++) {
		release_sound_in_source(i);
	}
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
	
	event_remain = event_extra = 0;
	cpu_remain = cpu_accum = cpu_done = 0;
	
	// reset sound
	if(sound_buffer) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
	}
	if(sound_tmp) {
		memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
	}
//	buffer_ptr = 0;
	
#ifdef _DEBUG_LOG
	initialize_done = true;
#endif
}

#define RUN_CPU_N(num, _class, arg)								\
	return static_cast<_class *>(d_cpu[num].device)->run(arg)

#define RUN_CPU_GENERIC(num, arg)			\
	return d_cpu[num].device->run(arg)


int EVENT::run_cpu(uint32_t num, int cycles)
{
	if(num < MAX_CPU) {
		uint32_t dom_num = cpu_type[num];
		switch(dom_num) {
#if defined(USE_CPU_HD6301)
		case EVENT_CPUTYPE_HD6301:
			RUN_CPU_N(num, HD6301, cycles);
			break;
#endif
#if defined(USE_CPU_HUC6280)
		case EVENT_CPUTYPE_HUC6280:
			RUN_CPU_N(num, HUC6280, cycles);
			break;
#endif
#if defined(USE_CPU_V30)
		case EVENT_CPUTYPE_V30:
			RUN_CPU_N(num, V30, cycles);
			break;
#endif
#if defined(USE_CPU_I286)
		case EVENT_CPUTYPE_I286:
	#if defined(_JX)
			RUN_CPU_N(num, JX::I286, cycles);
    #else
			RUN_CPU_N(num, I80286, cycles);
	#endif
			break;
#endif
#if defined(USE_CPU_I86) || defined(USE_CPU_I186) || defined(USE_CPU_I88)
		case EVENT_CPUTYPE_I86:
			RUN_CPU_N(num, I8086, cycles);
			break;
#endif
#if defined(USE_CPU_I386)
		case EVENT_CPUTYPE_I386:
			RUN_CPU_N(num, I386, cycles);
			break;
#endif
#if defined(USE_CPU_I8080)
		case EVENT_CPUTYPE_I8080:
			RUN_CPU_N(num, I8080, cycles);
			break;
#endif
#if defined(USE_CPU_M6502)
		case EVENT_CPUTYPE_M6502:
			RUN_CPU_N(num, M6502, cycles);
			break;
#endif
#if defined(USE_CPU_N2A03)
		case EVENT_CPUTYPE_N2A03:
			RUN_CPU_N(num, N2A03, cycles);
			break;
#endif
#if defined(USE_CPU_MB8861)
		case EVENT_CPUTYPE_MB8861:
			RUN_CPU_N(num, MB8861, cycles);
			break;
#endif
#if defined(USE_CPU_MC6800)
		case EVENT_CPUTYPE_MC6800:
			RUN_CPU_N(num, MC6800, cycles);
			break;
#endif
#if defined(USE_CPU_MC6801)
		case EVENT_CPUTYPE_MC6801:
			RUN_CPU_N(num, MC6801, cycles);
			break;
#endif
#if defined(USE_CPU_MC6809)
		case EVENT_CPUTYPE_MC6809:
			RUN_CPU_N(num, MC6809, cycles);
			break;
#endif
#if defined(USE_CPU_MCS48)
		case EVENT_CPUTYPE_MCS48:
			RUN_CPU_N(num, MCS48, cycles);
			break;
#endif
#if defined(USE_CPU_TMS9995)
		case EVENT_CPUTYPE_TMS9995:
			RUN_CPU_N(num, TMS9995, cycles);
			break;
#endif
#if defined(USE_CPU_UPD7801)
		case EVENT_CPUTYPE_UPD7801:
			RUN_CPU_N(num, UPD7801, cycles);
			break;
#endif
#if defined(USE_CPU_Z80)
		case EVENT_CPUTYPE_Z80:
			RUN_CPU_N(num, Z80, cycles);
			break;
#endif
		case EVENT_CPUTYPE_GENERIC:
		default:
			RUN_CPU_GENERIC(num, cycles);
			break;
		}
	}
	if(cycles <= 0) {
		return 1;
	} else {
		return cycles;
	}
}

//#define USE_SUPRESS_VTABLE
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
		
		frame_clocks = (int)((double)d_cpu[0].cpu_clocks / frames_per_sec + 0.5);
		int remain = frame_clocks;
		
		for(int i = 0; i < lines_per_frame; i++) {
			assert(i < MAX_LINES);
			vline_clocks[i] = (int)(frame_clocks / lines_per_frame);
			remain -= vline_clocks[i];
		}
		for(int i = 0; i < remain; i++) {
			int index = (int)((double)lines_per_frame * (double)i / (double)remain);
			assert(index < MAX_LINES);
			vline_clocks[index]++;
		}
		for(int i = 1; i < dcount_cpu; i++) {
			d_cpu[i].update_clocks = (int)(1024.0 * (double)d_cpu[i].cpu_clocks / (double)d_cpu[0].cpu_clocks + 0.5);
		}
		for(DEVICE* device = vm->first_device; device; device = device->next_device) {
			if(device->get_event_manager_id() == this_device_id) {
				device->update_timing(d_cpu[0].cpu_clocks, frames_per_sec, lines_per_frame);
			}
		}
	}
	
	// run virtual machine for 1 frame period
	for(int i = 0; i < frame_event_count; i++) {
		frame_event[i]->event_frame();
	}
	
	cur_vline = 0;
	vline_start_clock = get_current_clock();
	
	for(int i = 0; i < vline_event_count; i++) {
		vline_event[i]->event_vline(cur_vline, vline_clocks[cur_vline]);
	}
	this->register_event_by_clock(this, EVENT_VLINE, vline_clocks[cur_vline], false, NULL);
	
	if(event_remain < 0) {
		if(-event_remain > vline_clocks[cur_vline]) {
			update_event(vline_clocks[cur_vline]);
		} else {
			update_event(-event_remain);
		}
	}
	event_remain += frame_clocks;
	cpu_remain += frame_clocks << power;
	
	while(event_remain > 0) {
		int event_done = event_remain;
		if(cpu_remain > 0) {
			event_extra = 0;
			int cpu_done_tmp;
			if(dcount_cpu == 1) {
				// run one opecode on primary cpu
				cpu_done_tmp = d_cpu[0].device->run(-1);
			} else {
				// sync to sub cpus
				if(cpu_done == 0) {
					// run one opecode on primary cpu
#if !defined(USE_SUPRESS_VTABLE)
					cpu_done = d_cpu[0].device->run(-1);
#else
					cpu_done = run_cpu(0, -1);
#endif
				}
				
				// sub cpu runs continuously and no events will be fired while the given clocks,
				// so I need to give small enough clocks...
				cpu_done_tmp = (event_extra > 0 || cpu_done < 4) ? cpu_done : 4;
				cpu_done -= cpu_done_tmp;
				
				for(int i = 1; i < dcount_cpu; i++) {
					// run sub cpus
					d_cpu[i].accum_clocks += d_cpu[i].update_clocks * cpu_done_tmp;
					int sub_clock = d_cpu[i].accum_clocks >> 10;
					if(sub_clock) {
						d_cpu[i].accum_clocks -= sub_clock << 10;
#if !defined(USE_SUPRESS_VTABLE)
						d_cpu[i].device->run(sub_clock);
#else
						run_cpu(i, sub_clock);
#endif
					}
				}
			}
			cpu_remain -= cpu_done_tmp;
			cpu_accum += cpu_done_tmp;
			event_done = cpu_accum >> power;
			cpu_accum -= event_done << power;
			event_done -= event_extra;
		}
		if(event_done > 0) {
			if(event_remain > 0) {
				if(event_done > event_remain) {
					update_event(event_remain);
				} else {
					update_event(event_done);
				}
			}
			event_remain -= event_done;
		}
	}
}

void EVENT::update_extra_event(int clock)
{
	// this is called from primary cpu while running one opecode
	int event_done = clock >> power;
	
	if(event_done > 0) {
		if(event_remain > 0) {
			if(event_done > event_remain) {
				update_event(event_remain);
			} else {
				update_event(event_done);
			}
		}
		event_remain -= event_done;
		event_extra += event_done;
	}
}

void EVENT::update_event(int clock)
{
	uint64_t event_clocks_tmp = event_clocks + clock;
	
	while(first_fire_event != NULL && first_fire_event->expired_clock <= event_clocks_tmp) {
		event_t *event_handle = first_fire_event;
		uint64_t expired_clock = event_handle->expired_clock;
		
		first_fire_event = event_handle->next;
		if(first_fire_event != NULL) {
			first_fire_event->prev = NULL;
		}
		if(event_handle->loop_clock != 0) {
			event_handle->accum_clocks += event_handle->loop_clock;
			uint64_t clock_tmp = event_handle->accum_clocks >> 10;
			event_handle->accum_clocks -= clock_tmp << 10;
			event_handle->expired_clock += clock_tmp;
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

uint32_t EVENT::get_current_clock()
{
	return (uint32_t)(event_clocks & 0xffffffff);
}

uint64_t EVENT::get_current_clock_uint64()
{
	return event_clocks;
}

uint32_t EVENT::get_cpu_clock(int index)
{
	if((index < 0) || (index >= MAX_CPU)) return 0;
	return d_cpu[index].cpu_clocks;
}

uint32_t EVENT::get_passed_clock(uint32_t prev)
{
	uint32_t current = get_current_clock();
	return (current > prev) ? current - prev : current + (0xffffffff - prev) + 1;
}

double EVENT::get_passed_usec(uint32_t prev)
{
	return 1000000.0 * get_passed_clock(prev) / d_cpu[0].cpu_clocks;
}

uint32_t EVENT::get_passed_clock_since_vline()
{
	return get_passed_clock(vline_start_clock);
}

double EVENT::get_passed_usec_since_vline()
{
	return get_passed_usec(vline_start_clock);
}

uint32_t EVENT::get_cpu_pc(int index)
{
	return d_cpu[index].device->get_pc();
}

void EVENT::register_event(DEVICE* device, int event_id, double usec, bool loop, int* register_id)
{
#ifdef _DEBUG_LOG
	if(!initialize_done && !loop) {
		this->out_debug_log(_T("EVENT: non-loop event is registered before initialize is done\n"));
	}
#endif
	
	// register event
	if(first_free_event == NULL) {
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("EVENT: too many events !!!\n"));
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
	uint64_t clock;
	if(loop) {
		event_handle->loop_clock = (uint64_t)(1024.0 * (double)d_cpu[0].cpu_clocks / 1000000.0 * usec + 0.5);
		event_handle->accum_clocks = event_handle->loop_clock;
		clock = event_handle->accum_clocks >> 10;
		event_handle->accum_clocks -= clock << 10;
	} else {
		clock = (uint64_t)((double)d_cpu[0].cpu_clocks / 1000000.0 * usec + 0.5);
		event_handle->loop_clock = 0;
		event_handle->accum_clocks = 0;
	}
	event_handle->expired_clock = event_clocks + clock;
	
	insert_event(event_handle);
}

void EVENT::register_event_by_clock(DEVICE* device, int event_id, uint64_t clock, bool loop, int* register_id)
{
#ifdef _DEBUG_LOG
	if(!initialize_done && !loop) {
		this->out_debug_log(_T("EVENT: device (name=%s, id=%d) registeres non-loop event before initialize is done\n"), device->this_device_name, device->this_device_id);
	}
#endif
	
	// register event
	if(first_free_event == NULL) {
#ifdef _DEBUG_LOG
		this->out_debug_log(_T("EVENT: too many events !!!\n"));
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
	event_handle->loop_clock = loop ? (clock << 10) : 0;
	event_handle->accum_clocks = 0;
	
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
			this->out_debug_log(_T("EVENT: device (name=%s, id=%d) tries to cancel event %d that is not its own (owned by (name=%s id=%d))!!!\n"), device->this_device_name, device->this_device_id,
								register_id,
								event_handle->device->this_device_name,
								event_handle->device->this_device_id);			
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

void EVENT::register_frame_event(DEVICE* device)
{
	if(frame_event_count < MAX_EVENT) {
		for(int i = 0; i < frame_event_count; i++) {
			if(frame_event[i] == device) {
#ifdef _DEBUG_LOG
				this->out_debug_log(_T("EVENT: device (name=%s, id=%d) has already registered frame event !!!\n"), device->this_device_name, device->this_device_id);
#endif
				return;
			}
		}
		frame_event[frame_event_count++] = device;
#ifdef _DEBUG_LOG
	} else {
		this->out_debug_log(_T("EVENT: too many frame events !!!\n"));
#endif
	}
}

void EVENT::register_vline_event(DEVICE* device)
{
	if(vline_event_count < MAX_EVENT) {
		for(int i = 0; i < vline_event_count; i++) {
			if(vline_event[i] == device) {
#ifdef _DEBUG_LOG
				this->out_debug_log(_T("EVENT: device (name=%s, id=%d) has already registered vline event !!!\n"), device->this_device_name, device->this_device_id);
#endif
				return;
			}
		}
		vline_event[vline_event_count++] = device;
#ifdef _DEBUG_LOG
	} else {
		this->out_debug_log(_T("EVENT: too many vline events !!!\n"));
#endif
	}
}

uint32_t EVENT::get_event_remaining_clock(int register_id)
{
	if(0 <= register_id && register_id < MAX_EVENT) {
		event_t *event_handle = &event[register_id];
		if(event_handle->active && event->expired_clock > event_clocks) {
			return (uint32_t)(event->expired_clock - event_clocks);
		}
	}
	return 0;
}

double EVENT::get_event_remaining_usec(int register_id)
{
	return 1000000.0 * get_event_remaining_clock(register_id) / d_cpu[0].cpu_clocks;
}

void EVENT::touch_sound()
{
	if(!(config.sound_strict_rendering || (need_mix > 0))) {
		int samples = mix_counter;
		if(samples >= (sound_tmp_samples - buffer_ptr)) {
			samples = sound_tmp_samples - buffer_ptr;
		}
		if(samples > 0) {
			mix_sound(samples);
			mix_counter -= samples;
		}
	}
}

void EVENT::set_realtime_render(DEVICE* device, bool flag)
{
	assert(device != NULL && device->this_device_id < MAX_DEVICE);
	if(dev_need_mix[device->this_device_id] != flag) {
		if(flag) {
			need_mix++;
		} else {
			assert(need_mix > 0);
			need_mix--;
			if(need_mix < 0) need_mix = 0;
		}
		dev_need_mix[device->this_device_id] = flag;
	}
}

void EVENT::event_callback(int event_id, int err)
{
	if(event_id == EVENT_MIX) {
		// mix sound
		if(prev_skip && dont_skip_frames == 0 && !sound_changed) {
			buffer_ptr = 0;
		}
		int remain = sound_tmp_samples - buffer_ptr;
		
		if(remain > 0) {
			int samples = mix_counter;
			
			if(config.sound_strict_rendering || (need_mix > 0)) {
				if(samples < 1) {
					samples = 1;
				}
			}
			if(samples >= remain) {
				samples = remain;
			}
			if(config.sound_strict_rendering || (need_mix > 0)) {
				if(samples > 0) {
					mix_sound(samples);
				}
				mix_counter = 1;
			} else {
				if(samples > 0 && mix_counter >= mix_limit) {
					mix_sound(samples);
					mix_counter -= samples;
				}
				mix_counter++;
			}
		}
	} else if(event_id == EVENT_VLINE) {
		if(cur_vline + 1 < lines_per_frame) {
			cur_vline++;
			vline_start_clock = get_current_clock();
			
			for(int i = 0; i < vline_event_count; i++) {
				vline_event[i]->event_vline(cur_vline, vline_clocks[cur_vline]);
			}
			
			// do not register if next vline is the first vline of next frame
			if(cur_vline + 1 < lines_per_frame) {
				this->register_event_by_clock(this, EVENT_VLINE, vline_clocks[cur_vline], false, NULL);
			}
		}
	}
}

void EVENT::mix_sound(int samples)
{
	if(samples > 0) {
		int32_t* buffer = sound_tmp + buffer_ptr * 2;
		memset(buffer, 0, samples * sizeof(int32_t) * 2);
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

uint16_t* EVENT::create_sound(int* extra_frames)
{
	if(prev_skip && dont_skip_frames == 0 && !sound_changed) {
		memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
		*extra_frames = 0;
		return sound_buffer;
	}
	int frames = 0;
	
	// drive extra frames to fill the sound buffer
	while(sound_samples > buffer_ptr) {
		drive();
		frames++;
	}
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
		uint16_t highlow = (uint16_t)(dat & 0x0000ffff);
		
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
		memcpy(sound_tmp, sound_tmp + sound_samples * 2, buffer_ptr * sizeof(int32_t) * 2);
	} else {
		buffer_ptr = 0;
	}
	*extra_frames = frames;
	return sound_buffer;
}

int EVENT::get_sound_buffer_ptr()
{
	return buffer_ptr;
}

bool EVENT::is_sound_in_source_exists(int bank)
{
	bool f = true;
	if(bank < 0) return false;
	if(bank >= MAX_SOUND_IN_BUFFERS) return false;
	
	// ToDo: Lock Mutex
	if(sound_in_tmp_buffer[bank] == NULL) f = false;
	// ToDo: UnLock Mutex
	if(sound_in_samples[bank] <= 0) f = false;
	if(sound_in_rate[bank] <= 0) f = false;
	if(sound_in_channels[bank] <= 0) f = false;
	return f;
}

int EVENT::get_sound_in_buffers_count()
{
	int _n = 0;
	for(int i = 0; i < MAX_SOUND_IN_BUFFERS; i++) {
		if(sound_in_samples[i] == 0) break;
		_n++;
	}
	return _n;
}

int EVENT::get_sound_in_samples(int bank)
{
	if(bank < 0) return 0;
	if(bank >= MAX_SOUND_IN_BUFFERS) return 0;
	return sound_in_samples[bank];
}

int EVENT::get_sound_in_rate(int bank)
{
	if(bank < 0) return 0;
	if(bank >= MAX_SOUND_IN_BUFFERS) return 0;
	return sound_in_rate[bank];
}

int EVENT::get_sound_in_channels(int bank)
{
	if(bank < 0) return 0;
	if(bank >= MAX_SOUND_IN_BUFFERS) return 0;
	return sound_in_channels[bank];
}

int16_t* EVENT::get_sound_in_buf_ptr(int bank)
{
	if(bank < 0) return NULL;
	if(bank >= MAX_SOUND_IN_BUFFERS) return NULL;
	return &(sound_in_tmp_buffer[bank][sound_in_writeptr[bank]]);
}

int EVENT::write_sound_in_buffer(int bank, int32_t* src, int samples)
{
	if(bank < 0) return 0;
	if(bank >= MAX_SOUND_IN_BUFFERS) return 0;
	if(samples <= 0) return 0;
	if(sound_in_tmp_buffer[bank] == NULL) return 0;
	if(samples >= sound_in_samples[bank]) samples = sound_in_samples[bank];

	int n_samples = samples;
	int16_t* dst = &(sound_in_tmp_buffer[bank][sound_in_writeptr[bank] * sound_in_channels[bank]]);
	// ToDo: Lock Mutex
	if((sound_in_writeptr[bank] + samples) >= sound_in_samples[bank]) {
		if((sound_in_samples[bank] - sound_in_writeptr[bank] - 1) > 0) {
			memcpy(dst, src, (sound_in_samples[bank] - sound_in_writeptr[bank] - 1) * sound_in_channels[bank] * sizeof(int16_t));
			src =  src + (sound_in_samples[bank] - sound_in_writeptr[bank] - 1) * sound_in_channels[bank];
			sound_in_writeptr[bank] = 0;
			n_samples = n_samples - (sound_in_samples[bank] - sound_in_writeptr[bank] - 1);
		} else {
			sound_in_writeptr[bank] = 0;
		}
		dst = &(sound_in_tmp_buffer[bank][sound_in_writeptr[bank] * sound_in_channels[bank]]);
	}
	memcpy(dst, src, n_samples * sound_in_channels[bank] * sizeof(int16_t));
	// ToDo: UnLock Mutex
	sound_in_writeptr[bank] = sound_in_writeptr[bank] + n_samples;
	if(sound_in_writeptr[bank] >= sound_in_samples[bank]) sound_in_writeptr[bank] = 0;
	sound_in_write_size[bank] += samples;
	if(sound_in_write_size[bank] >= sound_in_samples[bank]) sound_in_write_size[bank] = sound_in_samples[bank];
	return samples;
}

// Add sampled values to sample buffer;value may be -32768 to +32767.
int EVENT::rechannel_sound_in_data(int32_t*dst, int16_t* src, int dst_channels, int src_channels, int samples)
{
	if(dst == NULL) return 0;
	if(src == NULL) return 0;
	if(dst_channels <= 0) return 0;
	if(src_channels <= 0) return 0;

	int cvt_bytes = 0;
	memset(dst, 0x00, sizeof(int32_t) * samples * dst_channels);
	if(dst_channels == src_channels) {
		for(int i = 0; i < (src_channels * samples) ; i++) {
			dst[i] = (int32_t)(src[i]);
		}
	} else if(dst_channels < src_channels) {
		for(int x = 0; x < samples; x++) {
			int mp = 0;
			int div_ch = src_channels / dst_channels;
			int mod_ch = src_channels % dst_channels;
			for(int i = 0; i < dst_channels; i++) {
				for(int j = 0; j < div_ch; j++) {
					dst[i] = dst[i] + (int32_t)(src[mp]);
					mp++;
				}
			}
			if(mod_ch != 0) {
				for(int j = 0; j < mod_ch; j++) {
					dst[dst_channels - 1] = dst[dst_channels - 1] + (int32_t)(src[mp]);
					mp++;
				}
				for(int i = 0; i < (dst_channels - 1); i++) {
					dst[i] = dst[i] / div_ch;
				}
				dst[dst_channels - 1] = dst[dst_channels - 1] / (div_ch + mod_ch);
			} else {
				for(int i = 0; i < dst_channels; i++) {
					dst[i] = dst[i] / div_ch;
				}
			}
			src = src + src_channels;
			dst = dst + dst_channels;
		}
	} else if(dst_channels > src_channels) {
		for(int x = 0; x < samples; x++) {
			int mp = 0;
			int div_ch = dst_channels / src_channels;
			int _n = div_ch;
			for(int i = 0; i < dst_channels; i++) {
				dst[i] = (int32_t)(src[mp]);
				_n--;
				if(_n <= 0) {
					_n = div_ch;
					mp++;
					if(mp >= src_channels) {
						mp = src_channels - 1;
					}
				}
			}
			src = src + src_channels;
			dst = dst + dst_channels;
		}
	}
	return samples;
}

int EVENT::increment_sound_in_passed_data(int bank, double passed_usec)
{

	if(bank < 0) return 0;
	if(bank >= MAX_SOUND_IN_BUFFERS) return 0;
	if(sound_in_rate[bank] <= 0) return 0;
	if(sound_in_samples[bank] <= 0) return 0;
	if(passed_usec <= 0.0) return 0;
	
	double freq = 1.0e6 / sound_in_rate[bank];
	int inc_ptr = (int)(nearbyint(passed_usec / freq));
	int readptr = sound_in_readptr[bank];
	int _ni = inc_ptr;
	
	if(_ni >= sound_in_samples[bank]) {
		_ni = _ni % sound_in_samples[bank];
	}
	readptr += _ni;
	if(readptr < 0) readptr = 0;
	readptr = readptr % sound_in_samples[bank];

	sound_in_readptr[bank] = readptr;
	sound_in_write_size[bank] = sound_in_write_size[bank] - _ni;
	if(sound_in_write_size[bank] <= 0) {
		sound_in_write_size[bank] = 1;
		sound_in_readptr[bank] = sound_in_readptr[bank] - 1;
		if(sound_in_readptr[bank] < 0) sound_in_readptr[bank] = sound_in_samples[bank] - 1;
	}
	return inc_ptr;
}
	
int EVENT::get_sound_in_latest_data(int bank, int32_t* dst, int expect_channels)
{
	int gave_samples = 0;
	int sound_div = 1;
	int sound_mod = 0;

	if(bank < 0) return 0;
	if(bank >= MAX_SOUND_IN_BUFFERS) return 0;
	if(sound_in_channels[bank] <= 0) return 0;
	if(sound_in_rate[bank] <= 0) return 0;
	if(sound_in_samples[bank] <= 0) return 0;
	if(expect_channels <= 0) return 0;
	if(dst == NULL) return 0;

	int readptr = sound_in_writeptr[bank] - 1;
	if(readptr < 0) {
		readptr = sound_in_samples[bank] - 1;
	}
	if(readptr >= sound_in_samples[bank]) {
		readptr = 0;
	}
	int16_t tmpbuf[sound_in_channels[bank] + 1];
	int16_t* p = sound_in_tmp_buffer[bank];
	if(p == NULL) return 0;
	p =&(p[readptr * sound_in_channels[bank]]);
	
	for(int i = 0; i < sound_in_channels[bank]; i++) {
		tmpbuf[i] = p[i];
	}

	readptr++;
	if(readptr >= sound_in_samples[bank]) {
		readptr = 0;
	}
	sound_in_readptr[bank] = readptr;
	sound_in_write_size[bank] = 0;
	return rechannel_sound_in_data(dst, tmpbuf, expect_channels, sound_in_channels[bank], 1); 
}

int EVENT::get_sound_in_data(int bank, int32_t* dst, int expect_samples, int expect_rate, int expect_channels)
{
	if(bank < 0) return -1;
	if(bank >= MAX_SOUND_IN_BUFFERS) return -1;
	if(sound_in_tmp_buffer[bank] == NULL) return -1;
	if(dst == NULL) return -1;

	int16_t* src = sound_in_tmp_buffer[bank];
	int readptr = sound_in_readptr[bank];
	if(readptr < 0) readptr = 0;
	if(readptr >= sound_in_samples[bank]) readptr = 0;
	
	int gave_samples = 0;
	// ToDo: Lock Mutex
	int in_count;
	in_count = (int)(nearbyint((double)(sound_in_rate[bank]) / (double)expect_rate) * (double)expect_samples);
	if(in_count >= sound_in_samples[bank]) in_count = sound_in_samples[bank];
	if(in_count >= sound_in_write_size[bank]) in_count = sound_in_write_size[bank];
	if(in_count <= 0) return 0;

	int16_t tmpbuf_in[(in_count + 1) * sound_in_channels[bank]];
	int32_t tmpbuf[(in_count + 1) * expect_channels] = {0};

	int mp = 0;
	for(int i = 0; i < in_count; i++) {
		int tmpr = readptr * sound_in_channels[bank];
		for(int j = 0; j < sound_in_channels[bank]; j++) {
			tmpbuf_in[mp] = src[tmpr + j];
			mp++;
		}
		readptr++;
		if(readptr >= sound_in_samples[bank]) readptr = 0;
	}
	sound_in_readptr[bank] = readptr;
	sound_in_write_size[bank] -= in_count;
	if(sound_in_write_size[bank] <= 0) sound_in_write_size[bank] = 0;
	
	gave_samples = rechannel_sound_in_data(tmpbuf, tmpbuf_in, expect_channels, sound_in_channels[bank], in_count);
	
	// ToDo: UnLock Mutex
	// Got to TMP Buffer
	if(expect_rate == sound_in_rate[bank]) {
		int32_t* p = tmpbuf;
		int32_t* q = dst;
		
		for(int i = 0; i < (gave_samples * expect_channels); i++) {
			q[i] = p[i];
		}
		return gave_samples;
	} else if(expect_rate > sound_in_rate[bank]) {
		int32_t* p = tmpbuf;
		int32_t* q = dst;
		int32_t tval;
		int s_div = expect_rate / sound_in_rate[bank];
		int s_mod = expect_rate % sound_in_rate[bank];
		int mod_count = 0;
		// ToDo: Interpollate
		int n_samples = (int)((double)gave_samples * ((double)expect_rate / (double)sound_in_rate[bank]));
		int32_t tmpdata[expect_channels];
		for(int i = 0; i < n_samples; i++) {
			for(int ch = 0; ch < expect_channels; ch++) {
				tmpdata[ch] = p[ch];
			}
			for(int n = 0; n < s_div; n++) {
				for(int ch = 0; ch < expect_channels; ch++) {
					dst[ch] = tmpdata[ch];
				}
				dst = dst + expect_channels;
			}
			mod_count += s_mod;
			if(mod_count >= sound_in_rate[bank]) {
				mod_count = mod_count - sound_in_rate[bank];
				for(int ch = 0; ch < expect_channels; ch++) {
					q[ch] = tmpdata[ch];
				}
				q = q + expect_channels;
			}
			p = p + expect_channels;
		}
		return n_samples;
	} else { // expect_rate < sound_in_rate[bank]
		// ToDo: Interpollate
		int32_t* p = tmpbuf;
		int32_t* q = dst;
		int32_t tval;
		int s_div = sound_in_rate[bank] / expect_rate;
		int s_mod = sound_in_rate[bank] % expect_rate;
		int mod_count = 0;
		int div_count = s_div;
		int s_count = 0;
		// ToDo: Interpollate
		int n_samples = (int)((double)gave_samples * ((double)expect_rate / (double)sound_in_rate[bank]));
		int32_t tmpdata[expect_channels] = {0};
		for(int i = 0; i < gave_samples; i++) {
			for(int ch = 0; ch < expect_channels; ch++) {
				tmpdata[ch] += p[ch];
			}
			mod_count += s_mod;
			if(mod_count >= expect_rate) {
				mod_count = mod_count - expect_rate;
				div_count++;
			}
			div_count--;
			s_count++;
			if(div_count <= 0) {
				div_count = s_div;
				for(int ch = 0; ch < expect_channels; ch++) {
					q[ch] = tmpdata[ch] / s_count;
				}
				s_count = 0;
				q = q + expect_channels;
			}
			p = p + expect_channels;
		}
		return n_samples;
	}
	return 0;
}
void EVENT::request_skip_frames()
{
	next_skip = true;
}

bool EVENT::is_frame_skippable()
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

#define STATE_VERSION	4

bool EVENT::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(dcount_cpu)) {
 		return false;
 	}
 	for(int i = 0; i < dcount_cpu; i++) {
		state_fio->StateValue(d_cpu[i].cpu_clocks);
		state_fio->StateValue(d_cpu[i].update_clocks);
		state_fio->StateValue(d_cpu[i].accum_clocks);
	}
	state_fio->StateValue(frame_clocks);
	state_fio->StateArray(vline_clocks, sizeof(vline_clocks), 1);
	state_fio->StateValue(event_remain);
	state_fio->StateValue(event_extra);
	state_fio->StateValue(cpu_remain);
	state_fio->StateValue(cpu_accum);
	state_fio->StateValue(cpu_done);
	state_fio->StateValue(event_clocks);
 	for(int i = 0; i < MAX_EVENT; i++) {
		if(loading) {
			event[i].device = vm->get_device(state_fio->FgetInt32_LE());
		} else {
			state_fio->FputInt32_LE(event[i].device != NULL ? event[i].device->this_device_id : -1);
		}
		state_fio->StateValue(event[i].event_id);
		state_fio->StateValue(event[i].expired_clock);
		state_fio->StateValue(event[i].loop_clock);
		state_fio->StateValue(event[i].accum_clocks);
		state_fio->StateValue(event[i].active);
		if(loading) {
			event[i].next = (event_t *)get_event(state_fio->FgetInt32_LE());
			event[i].prev = (event_t *)get_event(state_fio->FgetInt32_LE());
		} else {
			state_fio->FputInt32_LE(event[i].next != NULL ? event[i].next->index : -1);
			state_fio->FputInt32_LE(event[i].prev != NULL ? event[i].prev->index : -1);
		}
	}
	if(loading) {
		first_free_event = (event_t *)get_event(state_fio->FgetInt32_LE());
		first_fire_event = (event_t *)get_event(state_fio->FgetInt32_LE());
	} else {
		state_fio->FputInt32_LE(first_free_event != NULL ? first_free_event->index : -1);
		state_fio->FputInt32_LE(first_fire_event != NULL ? first_fire_event->index : -1);
	}
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(next_frames_per_sec);
	state_fio->StateValue(lines_per_frame);
	state_fio->StateValue(next_lines_per_frame);
	state_fio->StateArray(dev_need_mix, sizeof(dev_need_mix), 1);
	state_fio->StateValue(need_mix);
 	
 	// post process
	if(loading) {
		if(sound_buffer) {
			memset(sound_buffer, 0, sound_samples * sizeof(uint16_t) * 2);
		}
		if(sound_tmp) {
			memset(sound_tmp, 0, sound_tmp_samples * sizeof(int32_t) * 2);
		}
		buffer_ptr = 0;
		mix_counter = 1;
		mix_limit = (int)((double)(emu->get_sound_rate() / 2000.0));  // per 0.5ms.
 	}
 	return true;
}

void* EVENT::get_event(int index)
{
	if(index >= 0 && index < MAX_EVENT) {
		return &event[index];
	}
	return NULL;
}
