/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ HD46505 ]
*/

#include "hd46505.h"

#define EVENT_DISPLAY	0
#define EVENT_HSYNC_S	1
#define EVENT_HSYNC_E	2

void HD46505::initialize()
{
	// register events
	register_frame_event(this);
	register_vline_event(this);
}

void HD46505::reset()
{
	// initialize
	display = false;
	vblank = vsync = hsync = true;
	
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));
	ch = 0;
	
	// initial settings for 1st frame
#ifdef CHARS_PER_LINE
	hz_total = (CHARS_PER_LINE > 54) ? CHARS_PER_LINE : 54;
#else
	hz_total = 54;
#endif
	hz_disp = (hz_total > 80) ? 80 : 40;
	hs_start = hz_disp + 4;
	hs_end = hs_start + 4;
	
	vt_total = LINES_PER_FRAME;
	vt_disp = (SCREEN_HEIGHT > LINES_PER_FRAME) ? (SCREEN_HEIGHT >> 1) : SCREEN_HEIGHT;
	vs_start = vt_disp + 16;
	vs_end = vs_start + 16;
	
	timing_changed = false;
	disp_end_clock = 0;
	
#if defined(HD46505_CHAR_CLOCK)
	char_clock = 0;
	next_char_clock = HD46505_CHAR_CLOCK;
#elif defined(HD46505_HORIZ_FREQ)
	horiz_freq = 0;
	next_horiz_freq = HD46505_HORIZ_FREQ;
#endif
}

void HD46505::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		if(ch < 18) {
			if(ch < 10 && regs[ch] != data) {
				timing_changed = true;
			}
			if(ch == 5 && !regs_written[5]) {
				reg5_bottom = data;
			}
			if(ch == 9 && !regs_written[9]) {
				reg9_bottom = data;
			}
			regs[ch] = data;
			regs_written[ch] = true;
		}
	} else {
		ch = data;
	}
}

uint32_t HD46505::read_io8(uint32_t addr)
{
	if(addr & 1) {
		return (12 <= ch && ch < 18) ? regs[ch] : 0xff;
	} else {
		return ch;
	}
}

void HD46505::event_pre_frame()
{
	if(timing_changed) {
		if(regs_written[0] && regs_written[1] && regs_written[2] && regs_written[3] && regs_written[4] && regs_written[5] && regs_written[6] && regs_written[7] && regs_written[9]) {
			int ch_height = (regs[9] & 0x1f) + 1;
			int ch_height_bottom = (reg9_bottom & 0x1f) + 1;
			
			hz_total = regs[0] + 1;
			hz_disp = regs[1];
			hs_start = regs[2];
			hs_end = hs_start + (regs[3] & 0x0f);
			
//			vt_total = ((regs[4] & 0x7f) + 1) * ch_height + (regs[5] & 0x1f);
			vt_total = (regs[4] & 0x7f) * ch_height + ch_height_bottom + (reg5_bottom & 0x1f);
			vt_disp = (regs[6] & 0x7f) * ch_height;
			vs_start = ((regs[7] & 0x7f) + 1) * ch_height;
			vs_end = vs_start + ((regs[3] & 0xf0) ? (regs[3] >> 4) : 16);
			
			set_lines_per_frame(vt_total);
			
			timing_changed = false;
			disp_end_clock = 0;
#if defined(HD46505_CHAR_CLOCK)
			char_clock = 0;
#elif defined(HD46505_HORIZ_FREQ)
			horiz_freq = 0;
#endif
		}
	}
#if defined(HD46505_CHAR_CLOCK)
	if(char_clock != next_char_clock) {
		char_clock = next_char_clock;
		frames_per_sec = char_clock / (double)vt_total / (double)hz_total;
		if(regs[8] & 1) {
			frames_per_sec *= 2; // interlace mode
		}
		set_frames_per_sec(frames_per_sec);
	}
#elif defined(HD46505_HORIZ_FREQ)
	if(horiz_freq != next_horiz_freq) {
		horiz_freq = next_horiz_freq;
		frames_per_sec = horiz_freq / (double)vt_total;
		if(regs[8] & 1) {
			frames_per_sec *= 2; // interlace mode
		}
		set_frames_per_sec(frames_per_sec);
	}
#endif
}

void HD46505::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
#if !defined(HD46505_CHAR_CLOCK) && !defined(HD46505_HORIZ_FREQ)
	frames_per_sec = new_frames_per_sec;
#endif
	
	// update event clocks
	disp_end_clock = 0;
}

void HD46505::event_frame()
{
	// update envet clocks after update_timing() is called
	if(disp_end_clock == 0 && vt_total != 0) {
		disp_end_clock = (int)((double)cpu_clocks * (double)hz_disp / frames_per_sec / (double)vt_total / (double)hz_total);
		hs_start_clock = (int)((double)cpu_clocks * (double)hs_start / frames_per_sec / (double)vt_total / (double)hz_total);
		hs_end_clock = (int)((double)cpu_clocks * (double)hs_end / frames_per_sec / (double)vt_total / (double)hz_total);
	}
}

void HD46505::event_vline(int v, int clock)
{
	bool new_vblank;
	
	if((regs[8] & 0x30) != 0x30) {
		// if vt_disp == 0, raise vblank for one line
		new_vblank = ((v < vt_disp) || (v == 0 && vt_disp == 0));
	} else {
		new_vblank = false;
	}
	
	// virtical smooth scroll
	if(v == vt_disp) {
		reg5_bottom = regs[5];
		reg9_bottom = regs[9];
	}
	
	// display
	if(outputs_disp.count) {
		set_display(new_vblank);
		if(new_vblank && hz_disp < hz_total) {
			register_event_by_clock(this, EVENT_DISPLAY, disp_end_clock, false, NULL);
		}
	}
	
	// vblank
	set_vblank(new_vblank);	// active low
	
	// vsync
	set_vsync(vs_start <= v && v < vs_end);
	
	// hsync
	if(outputs_hsync.count && hs_start < hs_end && hs_end < hz_total) {
		set_hsync(false);
		register_event_by_clock(this, EVENT_HSYNC_S, hs_start_clock, false, NULL);
		register_event_by_clock(this, EVENT_HSYNC_E, hs_end_clock, false, NULL);
	}
}

void HD46505::event_callback(int event_id, int err)
{
	if(event_id == EVENT_DISPLAY) {
		set_display(false);
	} else if(event_id == EVENT_HSYNC_S) {
		set_hsync(true);
	} else if(event_id == EVENT_HSYNC_E) {
		set_hsync(false);
	}
}

void HD46505::set_display(bool val)
{
	if(display != val) {
		write_signals(&outputs_disp, val ? 0xffffffff : 0);
		display = val;
	}
}

void HD46505::set_vblank(bool val)
{
	if(vblank != val) {
		write_signals(&outputs_vblank, val ? 0xffffffff : 0);
		vblank = val;
	}
}

void HD46505::set_vsync(bool val)
{
	if(vsync != val) {
		write_signals(&outputs_vsync, val ? 0xffffffff : 0);
		vsync = val;
	}
}

void HD46505::set_hsync(bool val)
{
	if(hsync != val) {
		write_signals(&outputs_hsync, val ? 0xffffffff : 0);
		hsync = val;
	}
}

#define STATE_VERSION	4

bool HD46505::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateArray(regs_written, sizeof(regs_written), 1);
	state_fio->StateValue(reg5_bottom);
	state_fio->StateValue(reg9_bottom);
	state_fio->StateValue(ch);
	state_fio->StateValue(timing_changed);
	state_fio->StateValue(cpu_clocks);
#if defined(HD46505_CHAR_CLOCK)
	state_fio->StateValue(char_clock);
	state_fio->StateValue(next_char_clock);
#elif defined(HD46505_HORIZ_FREQ)
	state_fio->StateValue(horiz_freq);
	state_fio->StateValue(next_horiz_freq);
#endif
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(hz_total);
	state_fio->StateValue(hz_disp);
	state_fio->StateValue(hs_start);
	state_fio->StateValue(hs_end);
	state_fio->StateValue(vt_total);
	state_fio->StateValue(vt_disp);
	state_fio->StateValue(vs_start);
	state_fio->StateValue(vs_end);
	state_fio->StateValue(disp_end_clock);
	state_fio->StateValue(hs_start_clock);
	state_fio->StateValue(hs_end_clock);
	state_fio->StateValue(display);
	state_fio->StateValue(vblank);
	state_fio->StateValue(vsync);
	state_fio->StateValue(hsync);
	
	// post process
	if(loading) {
		if(regs_written[0] && regs_written[1] && regs_written[2] && regs_written[3] && regs_written[4] && regs_written[5] && regs_written[6] && regs_written[7] && regs_written[9]) {
			// force update timing
			timing_changed = true;
		}
	}
	return true;
}

