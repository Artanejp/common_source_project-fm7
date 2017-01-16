/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/

#include "towns_crtc.h"

enum {
	EVENT_CRTC_VSTART = 0,
	EVENT_CRTC_VST1   = 2,
	EVENT_CRTC_VST2   = 4,
	EVENT_CRTC_VDS    = 6,
	EVENT_CRTC_VDE    = 8,
	EVENT_CRTC_HSTART = 10,
	EVENT_CRTC_HEND   = 12,
	EVENT_CRTC_HSW    = 14,
};


void TOWNS_CRTC::initialize()
{
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));

	line_count[0] = line_count[1] = 0;
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_changed[1][i] = true;
	}
	event_id_hsync = -1;
	event_id_hsw = -1;
	event_id_vsync = -1;
	event_id_vstart = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vblank = -1;
	
	// register events
	//register_frame_event(this);
	//register_vline_event(this);
	
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

	line_count[0] = line_count[1] = 0;
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_rendered[0][i] = false;
		line_changed[1][i] = true;
		line_rendered[1][i] = false;
	}
	if(event_id_hsync  >= 0) cancel_event(this, event_id_hsync);
	if(event_id_hsw    >= 0) cancel_event(this, event_id_hsw);
	if(event_id_vsync  >= 0) cancel_event(this, event_id_vsync);
	if(event_id_vstart >= 0) cancel_event(this, event_id_vstart);
	if(event_id_vst1   >= 0) cancel_event(this, event_id_vst1);
	if(event_id_vst2   >= 0) cancel_event(this, event_id_vst2);
	if(event_id_vblank >= 0) cancel_event(this, event_id_vblank);
	
	event_id_hsw = -1;
	event_id_vsync = -1;
	event_id_vstart = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vblank = -1;

	// Register vstart
	register_event(this, EVENT_CRTC_VSTART, vstart_us, false, &event_id_vstart);
}
// CRTC register #29
void TOWNS_CRTC::set_crtc_clock(uint16_t val)
{
	scsel = (val & 0x0c) >> 2;
	clksel = val & 0x03;
	const double clocks[] = {
		28.6363e6, 24.5454e6, 25.175e6, 21.0525e6
	};
	if(crtc_clock[clksel] != crtc_clock) {
		crtc_clock = crtc_clock[clksel];
		force_recalc_crtc_param(-1);
	}
}

void TOWNS_CRTC::set_crtc_hsw1(void)
{
	int val = (int)(regs[ch + TOWNS_CRTC_REG_HSW1] & 0x0fff);
	horiz_width_posi_us = horiz_us * (double)val;
	// Update
}
void TOWNS_CRTC::set_crtc_hsw2(void)
{
	int val = (int)(regs[TOWNS_CRTC_REG_HSW2] & 0x0fff);
	horiz_width_nega_us = horiz_us * (double)val;
	// Update
}

void TOWNS_CRTC::set_crtc_hst(void)
{
	int val = (int)(regs[TOWNS_CRTC_REG_HST] & 0x07ff) | 1;
	next_horiz_us = (double)(val + 1) * 1.0 / crtc_clock;
	// Update
}

void TOWNS_CRTC::set_crtc_vst1(void)
{
	int val = (int)regs[TOWNS_CRTC_REG_VST1];
	vert_sync_pre_us = ((double)val * horiz_us) / 2.0;
}

void TOWNS_CRTC::set_crtc_vst2(void)
{
	int val = (int)regs[TOWNS_CRTC_REG_VST2];
	vert_sync_end_us = ((double)val * horiz_us) / 2.0;
}

void TOWNS_CRTC::set_crtc_vst2(void)
{
	int val = (int)regs[TOWNS_CRTC_REG_VST];
	next_vert_us = ((double)val * horiz_us) / 2.0;
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

void TOWNS_CRTC::event_callback(int event_id, int err)
{
	/*
	 * Related CRTC registers:
	 * HST, HSW1, HSW2 : HSYNC
	 * VST, VST1, VST2 : VSYNC
	 * (EET: for interlace : still not implement)
	 * VDS0, VDE0, HDS0, HDE0 : Display timing for Layer0
	 * VDS1, VDE1, HDS1, HDE1 : Display timing for Layer1
	 * FA0, HAJ0, LO0, FO0  : ToDo (For calculating address)
	 * FA1, HAJ1, LO1, FO1  : ToDo (For calculating address)
	 * ZOOM (#27) : ToDo
	 */
	int eid2 = (event_id / 2) * 2;
	if(eid2 == EVENT_CRTC_VSTART) {
		line_count[0] = line_count[1] = 0;
		if((horiz_us != next_horiz_us) || (vert_us != next_vert_us)) {
			horiz_us = next_horiz_us;
			vert_us = next_vert_us;
			force_recalc_crtc_param(-1);
			if(event_id_vsync >= 0) cancel_event(this, event_id_vsync);
			if(event_id_hsw >= 0) cancel_event(this, event_id_hsw);
			
			register_event(this, EVENT_CRTC_HSTART + 0, vert_sync_pre_us, false, &event_id_hsw); // HSW = HSYNC
			register_event(this, EVENT_CRTC_VSTART + 0, vert_us, true, &event_id_vsync); // VST
		}
		frame_in[0] = frame_in[1] = false;
		line_count[0] = line_count[1] = 0;
		line_count_mod[0] = line_count_mod[1] = 0;
		hsync = false;
		hdisp = false;
		vdisp = true;
		vblank = true;
		vsync = false;
		major_line_count = -1;
		register_event(this, EVENT_CRTC_VST1 + 0, vert_sync_pre_us, false, &event_id_vstn); // VST1
		register_event(this, EVENT_CRTC_VDS + 0, vert_start_us[0], false, &event_id_vstart[0]); // VDS0 : Layer1
		register_event(this, EVENT_CRTC_VDS + 1, vert_start_us[1], false, &event_id_vstart[1]); // VDS1 : Layer2
	} else if(eid2 == EVENT_CRTC_VST1) {
		vsync = true;
		if(vert_sync_end_us > 0.0) {
			register_event(this, EVENT_CRTC_VST2 + 0, vert_sync_end_us, false, &event_id_vstn); // VST2 - VST1.
		} else {
			vsync = false;
		}
	} else if (eid2 == EVENT_CRTC_VST2) {
		vsync = false;
		vblank = false;
		event_id_vstn = -1;
	} else if(eid2 == EVENT_CRTC_VDS) { // Display start
		int layer = event_id & 1;
		frame_in[layer] = true;
		// DO ofset line?
		if(event_id_vend[layer] >= 0) cancel_event(this, event_id_vend[layer]);
		register_event(this, EVENT_CRTC_VDE + layer, vert_start_us[0], false, &event_id_vend[layer]); // VDEx
		event_id_vstart[layer] = -1;
	} else if(eid2 == EVENT_CRTC_VDE) { // Display end
		int layer = event_id & 1;
		frame_in[layer] = false;
		if(event_id_vstart[layer] >= 0) cancel_event(this, event_id_vstart[layer]);
		event_id_vstart[layer] = -1;
		event_id_vend[layer] = -1;
		// DO ofset line?
	} else if(eid2 == EVENT_CRTC_HSTART) {
		// Do render
		hsync = true;
		hdisp = false;
		major_line_count++;
		if(vsync) { // in VSYNC. Not Display.
			if(event_id_hswn >= 0) cancel_event(this, event_id_hswn);
			register_event(this, EVENT_CRTC_HSW + 0, hsw2_us, false, &event_id_hswn); // Indicate HSYNC
		} else { // Not in VSYNC. May be displaied.
			for(layer = 0; layer < 2; layer++) {
				if(frame_in[layer] && (major_line_count >= 0) && (major_line_count < TOWNS_CRTC_MAX_LINES)) {
					if((vstart_lines[layer] <= major_line_count) && (vend_lines[layer] => major_line_count)) {
						// Proccessing zooming by ZOOM register(reg #27).
						{
							// If zoom is not supported by hardware, if first line : render.
							// Not first: copy (put here?)
						} // else {
						if(line_changed[layer][line_count[layer]]) {
							{
								// Do rendering.
								line_changed[layer][line_count[layer]] = false;
								line_rendered[layer][line_count[layer]] = true;
							}
						}
					}
					{
						// If zoom by hardware
						line_count_mod[layer] += line_count_factor[layer]; // line_count_factor[n] = 2048 / VZOOM_FACTOR[01]
						if(line_count_mod[layer] >= 2048) {
							line_count[layer]++;
							line_count_mod[layer] -= 2048;
						}
					}
				}
			}
			if(event_id_hswn >= 0) cancel_event(this, event_id_hswn);
			register_event(this, EVENT_CRTC_HSW + 0, hsw1_us, false, &event_id_hswn); // Indicate HSYNC
		}
	} else if(eid2 == EVENT_CRTC_HSW) {
		if(!vsync) {
			hdisp = true;
		}
		hsync = false;
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
