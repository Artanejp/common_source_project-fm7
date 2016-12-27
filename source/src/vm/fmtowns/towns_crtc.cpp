/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/

#include "towns_crtc.h"

#define EVENT_DISPLAY	0
#define EVENT_HSYNC_S	1
#define EVENT_HSYNC_E	2

void TOWNS_CRTC::initialize()
{
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));
	
	// register events
	register_frame_event(this);
	register_vline_event(this);
}

void TOWNS_CRTC::reset()
{
	// initialize
	display = false;
	vblank = vsync = hsync = true;
	
//	memset(regs, 0, sizeof(regs));
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
	
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	char_clock = 0;
	next_char_clock = TOWNS_CRTC_CHAR_CLOCK;
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
	horiz_freq = 0;
	next_horiz_freq = TOWNS_CRTC_HORIZ_FREQ;
#endif
}

void TOWNS_CRTC::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		if(ch < 32) {
			if(ch < 16 && regs[ch] != data) {
				timing_changed = true;
			}
			regs[ch] = data;
			regs_written[ch] = true;
		}
	} else {
		ch = data;
	}
}

uint32_t TOWNS_CRTC::read_io8(uint32_t addr)
{
	if(addr & 1) {
		return (12 <= ch && ch < 18) ? regs[ch] : 0xff;
	} else {
		return ch;
	}
}

void TOWNS_CRTC::event_pre_frame()
{
	if(timing_changed) {
		if(regs_written[0] && regs_written[1] && regs_written[2] && regs_written[3] && regs_written[4] && regs_written[5] && regs_written[6] && regs_written[7] && regs_written[9]) {
			int ch_height = (regs[9] & 0x1f) + 1;
			
			hz_total = regs[0] + 1;
			hz_disp = regs[1];
			hs_start = regs[2];
			hs_end = hs_start + (regs[3] & 0x0f);
			
			int new_vt_total = ((regs[4] & 0x7f) + 1) * ch_height + (regs[5] & 0x1f);
			vt_disp = (regs[6] & 0x7f) * ch_height;
			vs_start = ((regs[7] & 0x7f) + 1) * ch_height;
			vs_end = vs_start + ((regs[3] & 0xf0) ? (regs[3] >> 4) : 16);
			
			if(vt_total != new_vt_total) {
				vt_total = new_vt_total;
				set_lines_per_frame(vt_total);
			}
			timing_changed = false;
			disp_end_clock = 0;
#if defined(TOWNS_CRTC_CHAR_CLOCK)
			char_clock = 0;
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
			horiz_freq = 0;
#endif
		}
	}
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	if(char_clock != next_char_clock) {
		char_clock = next_char_clock;
		frames_per_sec = char_clock / (double)vt_total / (double)hz_total;
		if(regs[8] & 1) {
			frames_per_sec *= 2; // interlace mode
		}
		set_frames_per_sec(frames_per_sec);
	}
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
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

void TOWNS_CRTC::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
#if !defined(TOWNS_CRTC_CHAR_CLOCK) && !defined(TOWNS_CRTC_HORIZ_FREQ)
	frames_per_sec = new_frames_per_sec;
#endif
	
	// update event clocks
	disp_end_clock = 0;
}

void TOWNS_CRTC::event_frame()
{
	// update envet clocks after update_timing() is called
	if(disp_end_clock == 0 && vt_total != 0) {
		disp_end_clock = (int)((double)cpu_clocks * (double)hz_disp / frames_per_sec / (double)vt_total / (double)hz_total);
		hs_start_clock = (int)((double)cpu_clocks * (double)hs_start / frames_per_sec / (double)vt_total / (double)hz_total);
		hs_end_clock = (int)((double)cpu_clocks * (double)hs_end / frames_per_sec / (double)vt_total / (double)hz_total);
	}
}

void TOWNS_CRTC::event_vline(int v, int clock)
{
	// if vt_disp == 0, raise vblank for one line
	bool new_vblank = ((v < vt_disp) || (v == 0 && vt_disp == 0));
	
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

void TOWNS_CRTC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_DISPLAY) {
		set_display(false);
	} else if(event_id == EVENT_HSYNC_S) {
		set_hsync(true);
	} else if(event_id == EVENT_HSYNC_E) {
		set_hsync(false);
	}
}

void TOWNS_CRTC::set_display(bool val)
{
	if(display != val) {
		write_signals(&outputs_disp, val ? 0xffffffff : 0);
		display = val;
	}
}

void TOWNS_CRTC::set_vblank(bool val)
{
	if(vblank != val) {
		write_signals(&outputs_vblank, val ? 0xffffffff : 0);
		vblank = val;
	}
}

#define STATE_VERSION	1

void TOWNS_CRTC::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	state_fio->Fwrite(regs, sizeof(regs), 1);
	state_fio->Fwrite(regs_written, sizeof(regs_written), 1);
	state_fio->FputInt32(ch);
	state_fio->FputBool(timing_changed);
	state_fio->FputInt32(cpu_clocks);
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	state_fio->FputDouble(char_clock);
	state_fio->FputDouble(next_char_clock);
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
	state_fio->FputDouble(horiz_freq);
	state_fio->FputDouble(next_horiz_freq);
#endif
	state_fio->FputDouble(frames_per_sec);
	state_fio->FputInt32(hz_total);
	state_fio->FputInt32(hz_disp);
	state_fio->FputInt32(hs_start);
	state_fio->FputInt32(hs_end);
	state_fio->FputInt32(vt_total);
	state_fio->FputInt32(vt_disp);
	state_fio->FputInt32(vs_start);
	state_fio->FputInt32(vs_end);
	state_fio->FputInt32(disp_end_clock);
	state_fio->FputInt32(hs_start_clock);
	state_fio->FputInt32(hs_end_clock);
	state_fio->FputBool(display);
	state_fio->FputBool(vblank);
	state_fio->FputBool(vsync);
	state_fio->FputBool(hsync);
}

bool TOWNS_CRTC::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(regs, sizeof(regs), 1);
	state_fio->Fread(regs_written, sizeof(regs_written), 1);
	ch = state_fio->FgetInt32();
	timing_changed = state_fio->FgetBool();
	cpu_clocks = state_fio->FgetInt32();
#if defined(TOWNS_CRTC_CHAR_CLOCK)
	char_clock = state_fio->FgetDouble();
	next_char_clock = state_fio->FgetDouble();
#elif defined(TOWNS_CRTC_HORIZ_FREQ)
	horiz_freq = state_fio->FgetDouble();
	next_horiz_freq = state_fio->FgetDouble();
#endif
	frames_per_sec = state_fio->FgetDouble();
	hz_total = state_fio->FgetInt32();
	hz_disp = state_fio->FgetInt32();
	hs_start = state_fio->FgetInt32();
	hs_end = state_fio->FgetInt32();
	vt_total = state_fio->FgetInt32();
	vt_disp = state_fio->FgetInt32();
	vs_start = state_fio->FgetInt32();
	vs_end = state_fio->FgetInt32();
	disp_end_clock = state_fio->FgetInt32();
	hs_start_clock = state_fio->FgetInt32();
	hs_end_clock = state_fio->FgetInt32();
	display = state_fio->FgetBool();
	vblank = state_fio->FgetBool();
	vsync = state_fio->FgetBool();
	hsync = state_fio->FgetBool();

	return true;
}
