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

#define CLEAR_COLOR RGBA_COLOR(0,0,0,0)

#if defined(_RGB888)
#define _USE_ALPHA_CHANNEL
#endif

void TOWNS_CRTC::initialize()
{
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));

#ifdef _USE_ALPHA_CHANNEL
	for(int i = 0; i < 32768; i++) {
		uint8_t g = (i / (32 * 32)) & 0x1f;
		uint8_t r = (i / 32) & 0x1f;
		uint8_t b = i & 0x1f;
		table_32768c[i] = RGBA_COLOR(r << 3, g << 3, b << 3, 0xff);
	}
	for(int i = 32768; i < 65536; i++) {
		table_32768c[i] = _CLEAR_COLOR;
	}
#endif
	line_count[0] = line_count[1] = 0;
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_rendered[0][i] = false;
		line_changed[1][i] = true;
		line_rendered[1][i] = false;
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

// I/Os
// Palette.
void TOWNS_CRTC::calc_apalette16(int layer, int index)
{
	if(index < 0) return;
	if(index > 15) return;
	apalette_16_rgb[layer][index] =
		((uint16_t)(apalette_b & 0x0f)) |
		((uint16_t)(apalette_r & 0x0f) << 4) |
		((uint16_t)(apalette_g & 0x0f) << 8);
	if(index == 0) {
		apalette_16_pixel[layer][index] = _CLEAR_COLOR; // ??
	} else {
		apalette_16_pixel[layer][index] = RGBA_COLOR((apalette_r & 0x0f) << 4, (apalette_g & 0x0f) << 4, (apalette_b & 0x0f) << 4, 0xff);
	}
}

void TOWNS_CRTC::calc_apalette256(int index)
{
	if(index < 0) return;
	if(index > 255) return;
	apalette_256_rgb[layer][index] =
		((uint32_t)apalette_b) |
		((uint32_t)apalette_r << 8) |
		((uint32_t)apalette_g << 16);
	if(index == 0) {
		apalette_256_pixel[index] = _CLEAR_COLOR; // ??
	} else {
		apalette_256_pixel[index] = RGBA_COLOR(apalette_r, apalette_g, apalette_b, 0xff);
	}
}

void TOWNS_CRTC::set_apalette_r(int layer, uint8_t val)
{
	apalette_r = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_CRTC::set_apalette_g(int layer, uint8_t val)
{
	apalette_g = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_CRTC::set_apalette_b(int layer, uint8_t val)
{
	apalette_b = val;
	if(apalette_code < 16) {
		calc_apalette16(layer, (int)apalette_code);
	}
	// if layer == 0 ?
	calc_apalette256((int)apalette_code % 256);
}

void TOWNS_CRTC::set_apalette_num(int layer, uint8_t val)
{
	apalette_code = ((int)val) % 256;
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
	/*
	if(event_id == EVENT_DISPLAY) {
		set_display(false);
	} else if(event_id == EVENT_HSYNC_S) {
		set_hsync(true);
	} else if(event_id == EVENT_HSYNC_E) {
		set_hsync(false);
	}
	*/
	int eid2 = (event_id / 2) * 2;
	if(eid2 == EVENT_ID_VSTART) {
		line_count[0] = line_count[1] = 0;
		if((horiz_us != next_horiz_us) || (vert_us != next_vert_us)) {
			horiz_us = next_horiz_us;
			vert_us = next_vert_us;
			force_recalc_crtc_param(-1);
			//register_event(this, EVENT_CRTC_VSTART + 0, vert_us, true, &event_id_vstart); // OK?
		}
		frame_in = true;
		hsync = true;
		hdisp = false;
		vdisp = true;
		vblank = false;
		vsync = false;
		
		register_event(this, EVENT_CRTC_VSYNC  + 0, vert_sync_end_us, false, &event_id_vsync);
		register_event(this, EVENT_CRTC_HSTART + 0, vert_sync_pre_us, false, &event_id_hsw[0]);
		register_event(this, EVENT_CRTC_HSTART + 1, vert_sync_pre_us, false, &event_id_hsw[1]);
	} else if(eid2 == EVENT_ID_HSW) {
		// Do render
		hsync = false;
		int i = event_id & 1;
		if(!vsync) {
			hdisp = true;
			if((vstart_lines[i] <= line_count[i]) && ((vend_lines[i] + vstart_lines[i]) > line_count[i])) {
				if(line_changed[i][line_count[i]]) {
					// Renderer main
				}
			}
			line_changed[i][line_count[i]] = false;
			line_rendered[i][line_count[i]] = true;
			line_count[i]++;
			register_event(this, EVENT_CRTC_HSTART + i, horiz_us[i], false, &event_id_hsw[i]);
		} else {
			hsync = false;
			hdisp = false;
			register_event(this, EVENT_CRTC_HSTART + i, horiz_us[i], false, &event_id_hsw[i]);
		}
	} else if(eid2 == EVENT_ID_HSTART) {
		int i = event_id & 1;
		hsync = true;
		if(!vsync) {
			frame_in = true;
			register_event(this, EVENT_CRTC_HSW + i, horiz_width_posi_us[i], false, &event_id_hsw[i]);
		} else {
			register_event(this, EVENT_CRTC_HSW + i, horiz_width_nega_us[i], false, &event_id_hsw[i]);
			hdisp = false;
		}				
	} else if(eid2 == EVENT_ID_VSYNC) {
		int i = event_id & 1;

		frame_in = false;
		vsync = true;
		vdisp = false;
		vblank = true;
		register_event(this, EVENT_CRTC_VSTART + 0, vert_us, false, &event_id_vstart); // OK?
		// If display has not supported BLENDING, put blending ?.
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


// Renderers
void TOWNS_CRTC::render_line_16(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint32_t src;
	uint32_t src1, src2;
	uint8_t  srcdat1[4], srcdat2[1];
	scrntype_t *pdst = framebuffer;
	uint32_t *pp = (uint32_t *)vramptr;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	for(ip = 0; ip < nwords; ip++) {
		src = pp[ip];
		src1 = (src & 0xf0f0f0f0) >> 4;
		src2 = src & 0x0f0f0f0f;
		
		srcdat1[0] = (uint8_t)(src1 >> 24);
		srcdat1[1] = (uint8_t)(src1 >> 16);
		srcdat1[2] = (uint8_t)(src1 >> 8);
		srcdat1[3] = (uint8_t)(src1 >> 0);
		
		srcdat2[0] = (uint8_t)(src2 >> 24);
		srcdat2[1] = (uint8_t)(src2 >> 16);
		srcdat2[2] = (uint8_t)(src2 >> 8);
		srcdat2[3] = (uint8_t)(src2 >> 0);
		/*
		srcdat[0] = (uint8_t)((src & 0xf0000000) >> 28);
		srcdat[1] = (uint8_t)((src & 0x0f000000) >> 24);
		srcdat[2] = (uint8_t)((src & 0x00f00000) >> 20);
		srcdat[3] = (uint8_t)((src & 0x000f0000) >> 16);
		srcdat[4] = (uint8_t)((src & 0x0000f000) >> 12);
		srcdat[5] = (uint8_t)((src & 0x00000f00) >> 8);
		srcdat[6] = (uint8_t)((src & 0x000000f0) >> 4);
		srcdat[7] = (uint8_t)(src & 0x0f);
		for(int i = 0; i < 8; i++) {
			pdst[i] = apalette_16_pixel[layer][srcdat[i]];
		}
		pdst += 8;
		*/
		for(int i = 0; i < 4; i++) {
			pdst[0] = apalette_16_pixel[layer][srcdat1[i]];
			pdst[1] = apalette_16_pixel[layer][srcdat2[i]];
			pdst += 2;
		}
	}
	int mod_words = words - (nwords * 8);
	if(mod_words > 0) {
		uint8_t *ppp = (uint8_t *)(&pp[ip]);
		uint8_t hi, lo;
		for(int i = 0; i < mod_words / 2; i++) {
			uint8_t d = ppp[i];
			hi = (d & 0xf0) >> 4;
			lo = d & 0x0f;
			*pdst++ = apalette_16_pixel[layer][hi];
			*pdst++ = apalette_16_pixel[layer][lo];
		}
		if((mod_words & 1) != 0) {
			hi = (ppp[mod_words / 2] & 0xf0) >> 4;
			*pdst++ = apalette_16_pixel[layer][hi];
		}
	}
}

void TOWNS_CRTC::render_line_256(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 4;
	int ip;
	uint32_t src;
	uint8_t  srcdat[4];
	scrntype_t *pdst = framebuffer;
	uint32_t *pp = (uint32_t *)vramptr;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	for(ip = 0; ip < nwords; ip++) {
		src = pp[ip];
		srcdat[0] = (uint8_t)((src & 0xff000000) >> 24);
		srcdat[1] = (uint8_t)((src & 0x00ff0000) >> 16);
		srcdat[2] = (uint8_t)((src & 0x0000ff00) >> 8);
		srcdat[3] = (uint8_t) (src & 0x000000ff);
		for(int i = 0; i < 4; i++) {
			pdst[i] = apalette_256_pixel[srcdat[i]];
		}
		pdst += 4;
	}
	int mod_words = words - (nwords * 4);
	if(mod_words > 0) {
		uint8_t src8;
		uint8_t *p8 = (uint8_t *)(&pp[ip]);
		for(int i = 0; i < mod_words; i++) {
			src8 = p8[i];
			pdst[i] = apalette_256_pixel[src8];
		}
	}
}

// To be used table??
void TOWNS_CRTC::render_line_32768(int layer, scrntype_t *framebuffer, uint8_t *vramptr, uint32_t words)
{
	uint32_t wordptr = 0;
	int nwords = (int)words / 8;
	int ip;
	uint16_t src16;
	uint32_t  srcdat[4];
	scrntype_t *pdst = framebuffer;
	uint32_t *pp = (uint32_t *)vramptr;
	uint16_t *cachep = (uint16_t *)srcdat;
	int i = 0;
	
	if(framebuffer == NULL) return;
	if(vramptr == NULL) return;
	
	for(ip = 0; ip < nwords; ip++) {
		for(int i = 0; i < 4; i++) {
			srcdat[i] = pp[i];
		}
		pp = pp + 4;
		scrntype_t dcache[8];
		for(int i = 0; i < 8; i++) {
			dcache[i] = _CLEAR_COLOR;
		}
		for(int i = 0; i < 8; i++) {
			uint16_t v = cachep[i];
#ifdef _USE_ALPHA_CHANNEL
#ifdef __BIG_ENDIAN__
			pair_t n;
			n.d = 0;
			n.b.l = v & 0xff;
			n.b.h = (v & 0xff00) >> 8;
			dcache[i] = table_32768c[n.sw.l];
#else
			dcache[i] = table_32768c[v];
#endif
#else
			if((v & 0x8000) == 0) {
				dcache[i] = RGBA_COLOR((v & 0x03e0) >> 2, (v & 0x7c00) >> 7, (v & 0x001f) << 3, 0xff); // RGB555 -> PIXEL
			}
#endif
		}
		for(int i = 0; i < 8; i++) {
			pdst[i] = dcache[i];
		}
		pdst += 8;
	}
	int mod_words = words - nwords * 8;
	if(mod_words > 0) {
		uint16_t *vp = (uint16_t *)pp;
		scrntype_t dc;
		for(int i = 0; i < mod_words; i++) {
			src16 = vp[i];
#ifdef _USE_ALPHA_CHANNEL
#ifdef __BIG_ENDIAN__
			pair_t n;
			n.d = 0;
			n.b.l = src16 & 0xff;
			n.b.h = (src16 & 0xff00) >> 8;
			dc = table_32768c[n.sw.l];
#else
			dc = table_32768c[src16];
#endif
#else
			dc = _CLEAR_COLOR;
			if((src16 & 0x8000) == 0) {
				dc = RGBA_COLOR((src16 & 0x03e0) >> 2, (src16 & 0x7c00) >> 7, (src16 & 0x001f) << 3, 0xff); // RGB555 -> PIXEL
			}
#endif
			pdst[i] = dc;
		}
	}
}
#undef _CLEAR_COLOR

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
