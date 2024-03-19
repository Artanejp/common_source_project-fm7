/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/
#include "../vm.h"
#include "../../common.h"
#include "../../types/simd.h"

#include "crtc.h"
#include "vram.h"
#include "sprite.h"
#include "fontroms.h"
#include "../debugger.h"

namespace FMTOWNS {
enum {
	EVENT_HSYNC_OFF = 1,
	EVENT_HDS0 = 2,
	EVENT_HDS1 = 3,
	EVENT_HDE0 = 4,
	EVENT_HDE1 = 5,
};


void TOWNS_CRTC::initialize()
{
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));
	set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	set_lines_per_frame(SCREEN_HEIGHT); // Its dummy.

	line_count[0] = line_count[1] = 0;
	event_hsync = -1;
	for(int i = 0; i < 2; i++) {
		event_hdisp[i] = -1;
	}
	for(int i = 0; i < 4; i++) {
		for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
			memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
		}
	}
	// register events

	register_frame_event(this);
	register_vline_event(this);
	video_out_regs[FMTOWNS::VOUTREG_CTRL] = 0x15;
	video_out_regs[FMTOWNS::VOUTREG_PRIO] = 0x00;
	video_out_regs[FMTOWNS::VOUTREG_2] = 0x00;
	video_out_regs[FMTOWNS::VOUTREG_3] = 0x00;
	for(int i = 0; i <= display_linebuf_mask; i++) {
		is_single_layer[i] = false;
	}
}

void TOWNS_CRTC::release()
{
}

void TOWNS_CRTC::reset()
{
	// initialize
	display_enabled = true;
	display_enabled_pre = display_enabled;
	vsync = hsync = false;
	hstart_position = 0;
	write_signals(&outputs_int_vsync, 0);
	
	fo1_offset_value = 0;
	// 20230717 K.O
	// From Tsugaru,
	const uint16_t default_registers_value[32] =
	{
		0x0040,0x0320,0x0000,0x0000,0x035F,0x0000,0x0010,0x0000,0x036F,0x009C,0x031C,0x009C,0x031C,0x0040,0x0360,0x0040,
		0x0360,0x0000,0x009C,0x0000,0x0050,0x0000,0x009C,0x0000,0x0050,0x004A,0x0001,0x0000,0x003F,0x0003,0x0000,0x0150,
	};
	for(int i = 0; i < 32; i++) {
		regs[i] = default_registers_value[i];
	}
	//memset(regs, 0, sizeof(regs));
	crtc_ch = 0;
	sprite_offset = 0x00000;
	// initial settings for 1st frame
	req_recalc = false;

	crtc_clock = 1.0e6 / 28.6363e6;
	update_horiz_khz();
	frame_us = 1.0e6 / FRAMES_PER_SEC;

	interlace_field = false;
	is_compatible = true;

	line_count[0] = line_count[1] = 0;
	vert_line_count = -1;
	display_linebuf = 0;
	render_linebuf = 0;

	r50_planemask = 0x0f;
	r50_pagesel = 0;
	crtout_reg = 0x0f;

	//int dummy_mode0, dummy_mode1;
	video_out_regs[FMTOWNS::VOUTREG_CTRL] = 0x15;
	video_out_regs[FMTOWNS::VOUTREG_PRIO] = 0x00;
	video_out_regs[FMTOWNS::VOUTREG_2] = 0x00;
	video_out_regs[FMTOWNS::VOUTREG_3] = 0x00;
	reset_paletts();
	for(int i = 0; i < 2; i++) {
		timing_changed[i] = true;
		address_changed[i] = true;
		mode_changed[i] = true;

		impose_mode[i] = false; // OK?
		carry_enable[i] = false; //OK?
	}
	for(int i = 0; i < 2; i++) {
		zoom_factor_vert[i] = 2;
		zoom_factor_horiz[i] = 1;
		zoom_raw_vert[i] = 2;
		zoom_raw_horiz[i] = 1;
		zoom_count_vert[i] = 1;
	}

	for(int i = 0; i < 2; i++) {
		frame_offset[i] = 0;
		line_offset[i] = 80;
		for(int t = 0; t < 4; t++) {
			is_interlaced[t][i] = false;
		}
	}
	
	for(int i = 0; i < 2; i++) {
		vstart_addr[i] = 0;
		head_address[i] = 0;
		voffset_val[i] = 0;
	}
	cancel_event_by_id(event_hsync);
	for(int i = 0; i < 2; i++) {
		cancel_event_by_id(event_hdisp[i]);
		update_vstart(i);	
		update_line_offset(i);
	}
	// Register vstart

	begin_of_display();
	
	for(int i = 0; i <= display_linebuf_mask; i++) {
		hst[i] = pixels_per_line;
		vst[i] = max_lines;
		is_single_layer[i] = is_single_layer[0];
		for(int l = 0; l < 2; l++) {
			voffset[i][l] = voffset_val[l];
		}
		is_interlaced[i][0] = layer_is_interlaced(0);
		is_interlaced[i][1] = layer_is_interlaced(1);
		update_control_registers(i);
	}

	for(int layer = 0; layer < 2; layer++) {
		horiz_start_us[layer] = horiz_start_us_next[layer];
		horiz_end_us[layer] = horiz_end_us_next[layer];
	}
	for(int i = 0; i < 2; i++) {
		frame_offset_bak[i] = frame_offset[i];
	}

}

void TOWNS_CRTC::reset_paletts()
{
	for(int i = 0; i < 8; i++) {
		dpalette_regs[i] = i;
	}

	for(int i = 0; i < 16; i++) {
		uint16_t r;
		uint16_t g;
		uint16_t b;

		if((i & 8) != 0) {
			r = ((i & 2) != 0) ? 0xf0 : 0;
			g = ((i & 4) != 0) ? 0xf0 : 0;
			b = ((i & 1) != 0) ? 0xf0 : 0;
		} else {
			r = ((i & 2) != 0) ? 0x70 : 0;
			g = ((i & 4) != 0) ? 0x70 : 0;
			b = ((i & 1) != 0) ? 0x70 : 0;
		}
		for(int l = 0; l < 2; l++) {
			apalette_16_rgb[l][i][TOWNS_CRTC_PALETTE_R] = r;
			apalette_16_rgb[l][i][TOWNS_CRTC_PALETTE_G] = g;
			apalette_16_rgb[l][i][TOWNS_CRTC_PALETTE_B] = b;
			apalette_16_pixel[l][i] = RGBA_COLOR(r, g, b, 0xff);
		}
	}
	for(int i = 0; i < 256; i++) {
		#if 0
		uint8_t r = (i & 0x38) << 2;
		uint8_t g = i & 0xc0;
		uint8_t b = (i & 0x07) << 5;
		r |= 0x1f;
		b |= 0x1f;
		g |= 0x3f;
		#else
		uint8_t b = 0xff;
		uint8_t r = 0xff;
		uint8_t g = 0xff;
		#endif
		apalette_256_rgb[i][TOWNS_CRTC_PALETTE_B] = b;
		apalette_256_rgb[i][TOWNS_CRTC_PALETTE_R] = r;
		apalette_256_rgb[i][TOWNS_CRTC_PALETTE_G] = g;
		apalette_256_pixel[i] = RGBA_COLOR(r, g, b, 0xff);
	}
	dpalette_changed = true;
	apalette_code = 0;
}

void TOWNS_CRTC::reset_vsync()
{
	write_signals(&outputs_int_vsync, 0);
}
void TOWNS_CRTC::set_vsync(bool val)
{
	bool vsync_bak = vsync;
	vsync = val;
	if(vsync_bak != val) {
		write_signals(&outputs_int_vsync, (val) ? 0x00000000 : 0xffffffff);
		if(!(val)) {
			sprite_offset = get_sprite_offset();
			// Start sprite tranferring when VSYNC has asserted.
			// This is temporally working, not finally.
			// - 2024314 K.O
			__LIKELY_IF(d_sprite != NULL) {
				//! Note:
				//! - Below is from Tsugaru, commit 1a442831 .
				//! - I wonder sprite offset effects every display mode at page1.
				//! - -- 20230715 K.O
				d_sprite->write_signal(SIG_TOWNS_SPRITE_VSYNC, 0xffffffff, 0xffffffff);
			}
		}
	}
}
void TOWNS_CRTC::restart_display()
{
	// ToDo
	display_enabled_pre = true;
}

void TOWNS_CRTC::stop_display()
{
	// ToDo
	display_enabled_pre = false;
}

void TOWNS_CRTC::notify_mode_changed(int layer, uint8_t mode)
{
	mode_changed[layer] = true;
	display_mode[layer] = mode;
}

// I/Os
// Palette.
void TOWNS_CRTC::calc_apalette16(int layer, int index)
{
	index = index & 0x0f;
	uint32_t r = apalette_16_rgb[layer][index][TOWNS_CRTC_PALETTE_R];
	uint32_t g = apalette_16_rgb[layer][index][TOWNS_CRTC_PALETTE_G];
	uint32_t b = apalette_16_rgb[layer][index][TOWNS_CRTC_PALETTE_B];

	r = (r == 0) ? 0x00 : (r | 0x0f);
	g = (g == 0) ? 0x00 : (g | 0x0f);
	b = (b == 0) ? 0x00 : (b | 0x0f);

	apalette_16_pixel[layer][index] = RGBA_COLOR(r, g, b, 0xff);
}

void TOWNS_CRTC::calc_apalette256(int index)
{
	index = index & 255;
	uint32_t r = apalette_256_rgb[index][TOWNS_CRTC_PALETTE_R];
	uint32_t g = apalette_256_rgb[index][TOWNS_CRTC_PALETTE_G];
	uint32_t b = apalette_256_rgb[index][TOWNS_CRTC_PALETTE_B];
//	__UNLIKELY_IF(index == 0) {
//		apalette_256_pixel[index] = RGBA_COLOR(0, 0, 0, 0); // ??
//	} else {
		apalette_256_pixel[index] = RGBA_COLOR(r, g, b, 0xff);
//	}
}

void TOWNS_CRTC::set_apalette(uint8_t ch, uint8_t val, bool recalc)
{
	if(ch == TOWNS_CRTC_PALETTE_INDEX) {
		apalette_code = val;
		return;
	} else {
		ch &= 3;
		switch(video_out_regs[FMTOWNS::VOUTREG_PRIO] & 0x30) {
		case 0x00:
			apalette_16_rgb[0][apalette_code & 0x0f][ch] = val & 0xf0;
			if(recalc) {
				calc_apalette16(0, apalette_code);
			}
			break;
		case 0x20:
			apalette_16_rgb[1][apalette_code & 0x0f][ch] = val & 0xf0;
			if(recalc) {
				calc_apalette16(1, apalette_code);
			}
			break;
		default:
			apalette_256_rgb[apalette_code][ch] = val;
			if(recalc) {
				calc_apalette256(apalette_code);
			}
			break;
		}
	}
}

uint8_t TOWNS_CRTC::get_apalette_r()
{
	uint8_t val = 0x00;
	switch(video_out_regs[FMTOWNS::VOUTREG_PRIO] & 0x30) {
	case 0x00:
		val = apalette_16_rgb[0][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_R];
		break;
	case 0x20:
		val = apalette_16_rgb[1][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_R];
		break;
	default:
		val = apalette_256_rgb[apalette_code & 0xff][TOWNS_CRTC_PALETTE_R];
		break;
	}
	return val;
}

uint8_t TOWNS_CRTC::get_apalette_g()
{
	uint8_t val = 0x00;
	switch(video_out_regs[FMTOWNS::VOUTREG_PRIO]  & 0x30) {
	case 0x00:
		val = apalette_16_rgb[0][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_G];
		break;
	case 0x02:
		val = apalette_16_rgb[1][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_G];
		break;
	default:
		val = apalette_256_rgb[apalette_code & 0xff][TOWNS_CRTC_PALETTE_G];
		break;
	}
	return val;
}

uint8_t TOWNS_CRTC::get_apalette_b()
{
	uint8_t val = 0x00;
	switch(video_out_regs[FMTOWNS::VOUTREG_PRIO] & 0x30) {
	case 0x00:
		val = apalette_16_rgb[0][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_B];
		break;
	case 0x02:
		val = apalette_16_rgb[1][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_B];
		break;
	default:
		val = apalette_256_rgb[apalette_code & 0xff][TOWNS_CRTC_PALETTE_B];
		break;
	}
	return val;
}



// CRTC register #29
void TOWNS_CRTC::set_crtc_clock(uint16_t val, bool force)
{
	scsel = (val & 0x0c) >> 2;
	clksel = val & 0x03;
	double clock_bak = crtc_clock;
	static const double clocks[] = {
		28.6363e6, 24.5454e6, 25.175e6, 21.0525e6
	};
	crtc_clock = 1.0e6 / (clocks[clksel] / (double)(scsel + 1));
	update_horiz_khz();
	if((crtc_clock != clock_bak) || (force)) {
		force_recalc_crtc_param();
	}
}
void TOWNS_CRTC::copy_regs_v()
{
	for(int layer = 0; layer < 2; layer++) {
		update_regs_v(layer);
	}
}
	
void TOWNS_CRTC::update_regs_v(const int layer)
{
	vds[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_VDS0] & 0x07ff;
	vde[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_VDE0] & 0x07ff;
		
	frame_offset[layer] = regs[(layer * 4) + TOWNS_CRTC_REG_FO0]  & 0xffff;
		
	int voffset_tmp = (int)(vds[layer]);
	int vend_tmp =   (int)(vde[layer]);
	int vheight_tmp = max(0, (int)(vde[layer]) - (int)(vds[layer]));
	// From Tsugaru
	switch(clksel) {
	case 0x00: // 28.6363MHz
		voffset_tmp = (voffset_tmp - 0x002a) >> 1;
		break;
	case 0x01:
		voffset_tmp = (voffset_tmp - 0x002a) >> 1;
		//vheight_tmp >>= 1;
		break;
	case 0x02:
		voffset_tmp =  (voffset_tmp - 0x0046) >> 1;
		//vheight_tmp >>= 1;
		break;
	case 0x03:
		if(hst_reg != 0x029d) {
			voffset_tmp =  (voffset_tmp - 0x0040) >> 1;
		} else {
			voffset_tmp =  (voffset_tmp - 0x0046) >> 1;
		}
		//vheight_tmp >>= 1;
		break;
	default:
		voffset_tmp = 0;
		break;
	}
	__UNLIKELY_IF(horiz_khz < 16) {
		vheight_tmp <<= 1;
		voffset_tmp <<= 1;
	}
//	__LIKELY_IF(frame_offset[layer] == 0) {
//		vheight_tmp = vheight_tmp / 2;
//	}
	voffset_val[layer] = voffset_tmp;
	vheight_val[layer] = vheight_tmp;
}


void TOWNS_CRTC::copy_regs_h()
{
	for(int layer = 0; layer < 2; layer++) {
		update_regs_h(layer);
	}
}

void TOWNS_CRTC::update_regs_h(const int layer)
{
	hds[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_HDS0] & 0x07ff;
	hde[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_HDE0] & 0x07ff;
	haj[layer] = regs[(layer * 4) + TOWNS_CRTC_REG_HAJ0] & 0x07ff;

	int hstart_tmp = (int)(min(haj[layer], hds[layer]));
	int hwidth_tmp = max(0, (int)(hde[layer]) - (int)(hds[layer]));
	int hoff_tmp = 0;
	int hbitshift_tmp = 0;
	#if 0
	hoff_tmp = max(0, (int)(haj[layer]) - (int)(hds[layer]));
	hbitshift_tmp = max(0, (int)(hds[layer]) - (int)(haj[layer]));
	switch(clksel & 3) {
	case 0x00:
		hoff_tmp >>= 1;
		break;
	case 0x01:
		__LIKELY_IF(hst_reg != 0x031f) {
			hoff_tmp >>= 1;
		}
		break;
	default:
		break;
	}
	#else
	hoff_tmp = (int)(max(haj[layer], hds[layer]));		
	// Width
	hbitshift_tmp = max(0, (int)(hds[layer]) - (int)(haj[layer]));

	switch(clksel) {
	case 0x00: // 28.6363MHz
		hoff_tmp = (hoff_tmp - 0x0129) >> 1;
		break;
	case 0x01:
		__UNLIKELY_IF(hst_reg == 0x031f) {
			hoff_tmp -= 0x008a;
		} else {
			hoff_tmp = (hoff_tmp - 0x00e7) >> 1;
		}
		break;
	case 0x02:
		hoff_tmp -= 0x008a;
		break;
	case 0x03:
		__LIKELY_IF(hst_reg != 0x029d) {
			hoff_tmp -= 0x009c;
		} else {
			hoff_tmp -= 0x008a;
		}
		break;
	default:
		hoff_tmp = 0;
		break;
	}
	#endif
	hstart_val[layer] = hstart_tmp;
	hwidth_val[layer]  = hwidth_tmp;
	hbitshift_val[layer]  = hbitshift_tmp;
	hoffset_val[layer]  = hoff_tmp;
}

void TOWNS_CRTC::recalc_offset_by_clock(const uint32_t magx, int& hoffset_p, int64_t& hbitshift_p)
{
	uint32_t magxx = magx;
	__UNLIKELY_IF(magx == 0) {
		magxx = 1;
	}
	__UNLIKELY_IF(horiz_khz < 16) {
		hbitshift_p = ((hbitshift_p << 16) / (magxx * 2)) >> 16;
//		hoffset_p = ((hoffset_p << 16) / (magxx * 2)) >> 16;
	} else {
		hbitshift_p = ((hbitshift_p << 16) / magxx) >> 16;
//		hoffset_p = ((hoffset_p << 16) / magxx) >> 16;
	}
}
		
void TOWNS_CRTC::recalc_width_by_clock(const uint32_t magx, int64_t& width)
{
	__UNLIKELY_IF(horiz_khz < 16) {
		width = width / 2;
	} else {
		// From Tsugaru: 
		// VING games use this settings.  Apparently zoom-x needs to be interpreted as 4+(pageZoom&15).
		// Chase HQ        HST=029DH  ZOOM=1111H  Zoom2X=5
		// Viewpoint       HST=029DH  ZOOM=1111H  Zoom2X=5
		// Pu Li Ru La     HST=029DH  ZOOM=1111H  Zoom2X=5
		// Splatter House  HST=029DH  ZOOM=1111H  Zoom2X=5
		// Operation Wolf  N/A
		// New Zealand Story  N/A
		// Alshark Opening HST=029DH  ZOOM=0000H  Zoom2X=2
		// Freeware Collection 8 Oh!FM TOWNS Cover Picture Collection  HST=029DH  ZOOM=0000H  Zoom2X=2
		__UNLIKELY_IF((clksel == 3) && (hst_reg == 0x029d)) {
			__LIKELY_IF(magx >= 5) {
				width = (width * magx) >> 2; // magx / 4
				__UNLIKELY_IF(width > 640) {
					width = 640;
				}
			}
		}
	}
}


void TOWNS_CRTC::calc_width(const bool is_single, int64_t& hwidth_p)
{
	if(is_single) {
		hwidth_p = hwidth_val[0];
		uint32_t magx = zoom_factor_horiz[0];
		recalc_width_by_clock(magx, hwidth_p);
	} else {
		int64_t hwidth_tmp[2] = {0};
		for(int l = 0; l < 2; l++) {
			uint32_t magx = zoom_factor_horiz[l];
			hwidth_tmp[l] = hwidth_val[l];
			recalc_width_by_clock(magx, hwidth_tmp[l]);
		}
		hwidth_p  = max((int)hwidth_tmp[0], (int)hwidth_tmp[1]);
	}
}
		

void TOWNS_CRTC::calc_pixels_lines()
{
	int _width[2];
	int _height[2];

	int64_t pixels_per_line_next;
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];
	calc_width(is_single_tmp, pixels_per_line_next);
	pixels_per_line = pixels_per_line_next;
	
	// Omit around FO0.
	if(is_single_tmp) {
		// Single layer
		max_lines = vheight_val[0];
	} else {
		max_lines = max(vheight_val[0], vheight_val[1]);
	}

	// ToDo: High resolution MODE.

	__UNLIKELY_IF(pixels_per_line < 0) pixels_per_line = 0;
	__UNLIKELY_IF(pixels_per_line >= TOWNS_CRTC_MAX_PIXELS) pixels_per_line = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(max_lines < 0) max_lines = 0;
	__UNLIKELY_IF(max_lines >= TOWNS_CRTC_MAX_LINES) max_lines = TOWNS_CRTC_MAX_LINES;
}

void TOWNS_CRTC::recalc_hdisp_from_crtc_params(int layer, double& start_us, double& end_us)
{
	layer = layer & 1;
	update_regs_h(layer);
	start_us = ((double)(haj[layer] >> 1)) * crtc_clock;
	end_us   = ((double)(hde[layer] >> 1)) * crtc_clock;   // HDEx
	__UNLIKELY_IF(start_us > horiz_us) {
		start_us = horiz_us;
	}
	__UNLIKELY_IF(end_us > horiz_us) {
		end_us = horiz_us;
	}
	__UNLIKELY_IF(start_us > end_us) {
		start_us = end_us;
	}
}

void TOWNS_CRTC::force_recalc_crtc_param(void)
{
	horiz_width_posi_us_next = crtc_clock * ((double)hsw1); // HSW1
	horiz_width_nega_us_next = crtc_clock * ((double)hsw2); // HSW2
	horiz_us_next = crtc_clock * ((double)((hst_reg >> 1) + 1)); // HST
	for(int layer = 0; layer < 2; layer++) {
		recalc_hdisp_from_crtc_params(layer, horiz_start_us_next[layer], horiz_end_us_next[layer]);
	}

	req_recalc = false;
}


void TOWNS_CRTC::write_io8(uint32_t addr, uint32_t data)
{

//	out_debug_log(_T("WRITE8  ADDR=%04x DATA=%04x"), addr, data);
	switch(addr) {
	case 0x0440:
		crtc_ch = data & 0x1f;
		break;
	case 0x0442:
		update_crtc_reg(crtc_ch, (regs[crtc_ch] & 0xff00) | (data & 0xff));
		break;
	case 0x0443:
		update_crtc_reg(crtc_ch, (regs[crtc_ch] & 0x00ff) | ((data & 0xff) << 8));
		break;
	case 0x0448:
		voutreg_num = data & 0x03;
		break;
	case 0x044a:
		set_io_044a(data);
		break;
	case 0x044c:
		break;
	case 0x05ca: // clear interrupt
		reset_vsync();
		break;
	case 0xfd90:
		set_apalette(TOWNS_CRTC_PALETTE_INDEX, data, false);
		break;
	case 0xfd91:
		break;
	case 0xfd92:
		set_apalette(TOWNS_CRTC_PALETTE_B, data, true);
		break;
	case 0xfd93:
		break;
	case 0xfd94:
		set_apalette(TOWNS_CRTC_PALETTE_R, data, true);
		break;
	case 0xfd95:
		break;
	case 0xfd96:
		set_apalette(TOWNS_CRTC_PALETTE_G, data, true);
		break;
	case 0xfd97:
		break;
	case 0xfd98:
	case 0xfd99:
	case 0xfd9a:
	case 0xfd9b:
	case 0xfd9c:
	case 0xfd9d:
	case 0xfd9e:
	case 0xfd9f:
		{
			pair32_t n;
			n.d = data;
			if(addr == 0xfd9f) {
				dpalette_regs[7] = n.b.l & 0x0f;
			} else {
				dpalette_regs[addr & 7] = n.b.l & 0x0f;
				dpalette_regs[(addr + 1) & 7] = n.b.h & 0x0f;
			}
			dpalette_changed = true;
		}
		break;
	case 0xfda0:
		crtout_reg = data & 0x0f;
		make_crtout_from_fda0h(crtout_reg);
		break;
	}
}

void TOWNS_CRTC::write_io16(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE16 ADDR=%04x DATA=%04x"), addr, data);
	addr = addr & 0xffff;
	switch(addr) {
		// ToDo: FM Towns MX's I/O 0474h - 0477h. 20240219 K.O
	case 0x0442:
		update_crtc_reg(crtc_ch, data);
		break;
	case 0x0443: // From Tsugaru.
		break;
	case 0x044a:
		set_io_044a(data);
		break;
	case 0x044b: // From Tsugaru.
		break;
	default:
		DEVICE::write_io16(addr, data);
		break;
	}
	return;
}


void TOWNS_CRTC::write_io32(uint32_t addr, uint32_t data)
{
	// From Tsugaru (at least commit cdb071fc "* BRKON INT xx CSEIP=xxxx:xxxxxxxx/EIP=xxxxxxxx" :
	// Analog-Palette Registers allow DWORD Access.
	// Towns MENU V2.1 writes to palette like:
	// 0110:000015C4 66BA94FD                  MOV     DX,FD94H
	// 0110:000015C8 EF                        OUT     DX,EAX
	// 0110:000015C9 8AC4                      MOV     AL,AH
	// 0110:000015CB B292                      MOV     DL,92H
	// 0110:000015CD EE                        OUT     DX,AL
	addr = addr & 0xffff;
	switch(addr) {
		// ToDo: FM Towns MX's I/O 0474h - 0477h. 20240219 K.O
	case 0xfd90:
		set_apalette(TOWNS_CRTC_PALETTE_INDEX, data & 0xff, false);
		set_apalette(TOWNS_CRTC_PALETTE_B, (data >> 16) & 0xff, true);
		break;
	case 0xfd92:
		set_apalette(TOWNS_CRTC_PALETTE_B, data & 0xff, false);
		set_apalette(TOWNS_CRTC_PALETTE_R, (data >> 16) & 0xff, true);
		break;
	case 0xfd94:
		set_apalette(TOWNS_CRTC_PALETTE_R, data & 0xff, false);
		set_apalette(TOWNS_CRTC_PALETTE_G, (data >> 16) & 0xff, true);
		break;
	case 0xfd96:
		set_apalette(TOWNS_CRTC_PALETTE_G, data  & 0xff, true);
		break;
	default:
		DEVICE::write_io32(addr, data);
		break;
	}
	return;
}

void TOWNS_CRTC::update_crtc_reg(uint8_t ch, uint32_t data)
{
	ch = ch & 0x1f;
	uint16_t reg_bak = regs[ch];
	regs[ch] = (uint16_t)data;
	switch(ch) {
	case TOWNS_CRTC_REG_HSW1:
	case TOWNS_CRTC_REG_HSW2:
	case TOWNS_CRTC_REG_HST:
	case TOWNS_CRTC_REG_VST1:
	case TOWNS_CRTC_REG_VST2:
	case TOWNS_CRTC_REG_EET:
	case TOWNS_CRTC_REG_VST:
		break;
	case TOWNS_CRTC_REG_HDS0:
	case TOWNS_CRTC_REG_HDE0:
	case TOWNS_CRTC_REG_HDS1:
	case TOWNS_CRTC_REG_HDE1:
	case TOWNS_CRTC_REG_VDS0:
	case TOWNS_CRTC_REG_VDE0:
	case TOWNS_CRTC_REG_VDS1:
	case TOWNS_CRTC_REG_VDE1:
	break;
	case TOWNS_CRTC_REG_FA0:
		#if 0
		out_debug_log(_T("FA0 changed to 0x%04x; V=%d VST1=%d VST2=%d\n    VSYNC=%s HSYNC=%s HDISP0=%s FRAME IN=%s"),
					  data,
					  get_cur_vline(),
					  vst1,
					  vst2,
					  (vsync) ? _T("ON ") : _T("OFF"),
					  (hsync) ? _T("ON ") : _T("OFF"),
					  (hdisp[0]) ? _T("ON ") : _T("OFF"),
					  (frame_in[0]) ? _T("ON ") : _T("OFF")
			);
		out_debug_log(_T("    HDS0=%04x HAJ0=%04x HDE0=%04x"),
					  regs[FMTOWNS::TOWNS_CRTC_REG_HDS0],
					  regs[FMTOWNS::TOWNS_CRTC_REG_HAJ0],
					  regs[FMTOWNS::TOWNS_CRTC_REG_HDE0]);
		update_vstart(0);
		#endif
		break;
	case TOWNS_CRTC_REG_FA1:
		#if 0
		out_debug_log(_T("FA1 changed to 0x%04x; V=%d VST1=%d VST2=%d\n    VSYNC=%s HSYNC=%s HDISP1=%s FRAME IN=%s"),
					  data,
					  get_cur_vline(),
					  vst1,
					  vst2,
					  (vsync) ? _T("ON ") : _T("OFF"),
					  (hsync) ? _T("ON ") : _T("OFF"),
					  (hdisp[1]) ? _T("ON ") : _T("OFF"),
					  (frame_in[1]) ? _T("ON ") : _T("OFF")
			);
		out_debug_log(_T("    HDS1=%04x HAJ1=%04x HDE1=%04x"),
					  regs[FMTOWNS::TOWNS_CRTC_REG_HDS1],
					  regs[FMTOWNS::TOWNS_CRTC_REG_HAJ1],
					  regs[FMTOWNS::TOWNS_CRTC_REG_HDE1]);
		update_vstart(1);
		#endif
		break;
	case TOWNS_CRTC_REG_HAJ0:
	case TOWNS_CRTC_REG_FO0:
		break;
	case TOWNS_CRTC_REG_LO0:
		//update_line_offset(0);
		break;
	case TOWNS_CRTC_REG_HAJ1:
	case TOWNS_CRTC_REG_FO1:
		break;
	case TOWNS_CRTC_REG_LO1:
		//update_line_offset(1);
		break;
	break;
	case TOWNS_CRTC_REG_EHAJ:
		// ToDo
		break;
	case TOWNS_CRTC_REG_EVAJ:
		// ToDo
		break;
	case TOWNS_CRTC_REG_ZOOM:
		break;
	case TOWNS_CRTC_REG_DISPMODE: // CR0
		break;
	case TOWNS_CRTC_REG_CLK: // CR1
		set_crtc_clock((uint16_t)data, false);
		break;
	case TOWNS_CRTC_REG_DUMMY: // RESERVED(REG#30)
		// ToDo
		break;
	case TOWNS_CRTC_REG_CTRL: // CR2
		// ToDo: External Trigger.
		break;
	default:
		// ToDo
		break;
	}
}


uint16_t TOWNS_CRTC::read_reg30()
{
	//uint16_t data = 0x00f0;
	uint16_t data = 0x0000;
	#if 0 /* Why is this... Ported from Tsugaru (；´Д｀) */
	data |= (!(vsync)          ?  0x8000 : 0);
	data |= (!(vsync)          ?  0x4000 : 0);
	data |= (!(hsync)          ?  0x2000 : 0);
	data |= (!(hsync)          ?  0x1000 : 0);
	data |= ((false)           ?  0x0800 : 0);
	#else
	data |= ((frame_in[1])     ?  0x8000 : 0);
	data |= ((frame_in[0])     ?  0x4000 : 0);
	data |= ((hdisp[1])        ?  0x2000 : 0);
	data |= ((hdisp[0])        ?  0x1000 : 0);
	data |= ((interlace_field) ?  0x0800 : 0);
	#endif
	data |= ((vsync)           ?  0x0400 : 0);
	data |= ((hsync)           ?  0x0200 : 0);
	//data |= ((video_in)     ? 0x0100 : 0);
	//data |= ((half_tone)    ? 0x0008 : 0);
	//data |= ((sync_enable)  ? 0x0004 : 0);
	//data |= ((vcard_enable) ? 0x0002 : 0);
	//data |= ((sub_carry)    ? 0x0001 : 0);
	data = (data & 0xff00 ) | (regs[TOWNS_CRTC_REG_DUMMY] & 0x00ff);

	return data;
}


uint32_t TOWNS_CRTC::read_io8(uint32_t addr)
{
//	out_debug_log(_T("READ8 ADDR=%04x"), addr);
	switch(addr) {
	case 0x0440:
		return (uint32_t)crtc_ch;
		break;
	case 0x0442:
		{
			pair16_t d;
			if(crtc_ch == TOWNS_CRTC_REG_DUMMY) {
				d.w = read_reg30();
			} else {
				d.w = regs[crtc_ch];
			}
			return (uint32_t)(d.b.l);
		}
		break;
	case 0x0443:
		{
			pair16_t d;
			if(crtc_ch == TOWNS_CRTC_REG_DUMMY) {
				d.w = read_reg30();
			} else {
				d.w = regs[crtc_ch];
			}
			return (uint32_t)(d.b.h);
		}
		break;
	case 0x0448:
		return voutreg_num;
		break;
	case 0x044a:
		return get_io_044a();
		break;
	case 0x044c:
		{
//			uint16_t d = 0x7c;
			uint16_t d = 0x00;
			d = d | ((dpalette_changed) ? 0x80 : 0x00);
			__LIKELY_IF(d_sprite != nullptr) {
				d = d | ((d_sprite->read_signal(SIG_TOWNS_SPRITE_BUSY) != 0) ? 0x02 : 0x00);
				d = d | ((d_sprite->read_signal(SIG_TOWNS_SPRITE_DISP_PAGE1) != 0) ? 0x01 : 0x00);
			}
			dpalette_changed = false;
			return d;
		}
		break;
	case 0xfd90:
		return apalette_code;
		break;
	case 0xfd92:
		return get_apalette_b();
		break;
	case 0xfd94:
		return get_apalette_r();
		break;
	case 0xfd96:
		return get_apalette_g();
		break;
	case 0xfd98:
	case 0xfd99:
	case 0xfd9a:
	case 0xfd9b:
	case 0xfd9c:
	case 0xfd9d:
	case 0xfd9e:
	case 0xfd9f:
		return dpalette_regs[addr & 7];
		break;
	case 0xfda0:
		{
			uint8_t d = 0x00;
			d = d | ((vsync) ? 0x01 : 0x00);
			d = d | ((hsync) ? 0x02 : 0x00);
			return d;
		}
		break;
	case 0xfda2:
		if((machine_id >= 0x0700) && !(is_compatible)) { // After UG
			return (crtout_reg & 0x0f);
		} else {
			return 0x00;
		}
		break;
	}
	return 0xff;
}
uint32_t TOWNS_CRTC::read_io16(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x0442:
		#if 1 /* Why is this... Ported from Tsugaru (；´Д｀) */
		return regs[crtc_ch];
		#else
		if(crtc_ch == TOWNS_CRTC_REG_DUMMY) {
			return (uint32_t)read_reg30();
		} else {
			return regs[crtc_ch];
		}
		#endif
		break;
	default:
		break;
	}
	return DEVICE::read_io16(addr);
}

bool TOWNS_CRTC::render_32768(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = mask;
	__UNLIKELY_IF(pwidth <= 0) return false;
	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	__DECL_ALIGNED(16) int magx_tmp[8];
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		magx_tmp[i] = (magx + (i & 1)) / 2;
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2+ 16 * magx);
	rendered_pixels = pwidth;
	if((pwidth * magx) > width) {
		__UNLIKELY_IF(magx < 1) {
			pwidth = width;
		} else {
			pwidth = width / magx;
			if((width % magx) > 1) {
				pwidth++;
			}
		}
	} else {
		__LIKELY_IF(magx > 0) {
			if((pwidth % magx) > 1) {
				pwidth = pwidth / magx + 1;
			} else {
				pwidth = pwidth / magx;
			}
		}
	}
	magx = magx / 2;
	__UNLIKELY_IF(magx < 1) return false;
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 0) return false;
	if(y == 128) {
		//out_debug_log("RENDER_32768 Y=%d LAYER=%d PWIDTH=%d WIDTH=%d DST=%08X MASK=%08X ALPHA=%d", y, layer, pwidth, width, dst, mask, do_alpha);
	}
	__DECL_ALIGNED(16) uint16_t pbuf[8];
	__DECL_ALIGNED(16) uint16_t rbuf[8];
	__DECL_ALIGNED(16) uint16_t gbuf[8];
	__DECL_ALIGNED(16) uint16_t bbuf[8];
	csp_vector8<scrntype_t> sbuf;
	csp_vector8<scrntype_t> abuf;
	__DECL_ALIGNED(32) uint8_t a2buf[8];
	pair16_t ptmp16;
	int rwidth = pwidth & 7;

	int k = 0;

	size_t width_tmp1 = width;
	size_t width_tmp2 = width;
	for(int x = 0; x < (pwidth >> 3); x++) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			ptmp16.read_2bytes_le_from(p);
			pbuf[i] = ptmp16.w;
			p += 2;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] = pbuf[i];
			gbuf[i] = pbuf[i];
			bbuf[i] = pbuf[i];
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] = rbuf[i] >> 5;
			gbuf[i] = gbuf[i] >> 10;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] = rbuf[i] & 0x1f;
			gbuf[i] = gbuf[i] & 0x1f;
			bbuf[i] = bbuf[i] & 0x1f;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] <<= 3;
			gbuf[i] <<= 3;
			bbuf[i] <<= 3;
		}
		__UNLIKELY_IF(do_alpha) {
			if(is_transparent) {
		__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					a2buf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
				}
			} else {
		__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					a2buf[i] = 255;
				}
			}
			for(size_t i = 0; i < 8; i++) {
				sbuf.set(i, RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], a2buf[i]));
			}
		} else {
			if(is_transparent) {
				for(size_t i = 0; i < 8; i++) {
					abuf.set(i, (pbuf[i] & 0x8000) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255));
				}
			} else {
				abuf.fill(RGBA_COLOR(255, 255, 255, 255));
			}
			for(size_t i = 0; i < 8; i++) {
				sbuf.set(i, (pbuf[i] & 0x8000) ? RGBA_COLOR(0, 0, 0, 255) : RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], 255));
			}
		}
		__LIKELY_IF((((magx << 3) + k) <= width) && !(odd_mag)) {
			q = scaling_store(q, &sbuf, magx, 1, width_tmp1);
			r2 = scaling_store(r2, &abuf, magx, 1, width_tmp2);
			k += (magx << 3);
			__UNLIKELY_IF((width_tmp1 <= 0) || (width_tmp2 <= 0)) break;
		} else {
			int kbak = k;
			for(int i = 0; i < 8; i++) {
				scrntype_t dd = sbuf.at(i);
				for(int j = 0; j < magx_tmp[i]; j++) {
					*q++ = dd;
					kbak++;
					if(kbak >= width) break;
				}
			}
			__LIKELY_IF(r2 != nullptr) {
				for(int i = 0; i < 8; i++) {
					scrntype_t dm = abuf.at(i);
					for(int j = 0; j < magx_tmp[i]; j++) {
						*r2++ = dm;
						k++;
						if(k >= width) break;
					}
				}
			}
			k = kbak;
			__UNLIKELY_IF(k >= width) return true;
		}
	}
	__LIKELY_IF(k >= width) return true;

	__UNLIKELY_IF((pwidth & 7) != 0) {

__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			pbuf[i] = 0x8000;
		}

		for(int i = 0; i < rwidth; i++) {
			ptmp16.read_2bytes_le_from(p);
			pbuf[i] = ptmp16.w;
			p += 2;
		}

__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] = pbuf[i];
			gbuf[i] = pbuf[i];
			bbuf[i] = pbuf[i];
		}

__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] = (rbuf[i] >> 5) & 0x1f;
			gbuf[i] = (gbuf[i] >> 10) & 0x1f;
			bbuf[i] = bbuf[i] & 0x1f;
		}

__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			rbuf[i] <<= 3;
			gbuf[i] <<= 3;
			bbuf[i] <<= 3;
		}
		__UNLIKELY_IF(do_alpha) {
			if(is_transparent) {
		__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					a2buf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
				}
			} else {
		__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					a2buf[i] = 255;
				}
			}
			for(size_t i = 0; i < 8; i++) {
				sbuf.set(i, RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], a2buf[i]));
			}
		} else {
			if(is_transparent) {
				for(size_t i = 0; i < 8; i++) {
					abuf.set(i, (pbuf[i] & 0x8000) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255));
				}
			} else {
				abuf.fill(RGBA_COLOR(255, 255, 255, 255));
			}
		__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				sbuf.set(i, RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], 255));
			}
		}
		if((magx < 2) || !(odd_mag)) {
			q = scaling_store(q, &sbuf, magx, 1, width_tmp1);
			r2 = scaling_store(r2, &abuf, magx, 1, width_tmp2);
			k += (magx << 3);
			__UNLIKELY_IF((width_tmp1 <= 0) || (width_tmp2 <= 0)) return true;
		} else {
			for(int i = 0; i < rwidth; i++) {
				scrntype_t dd = sbuf.at(i);
				scrntype_t dm = abuf.at(i);
				for(int j = 0; j < magx_tmp[i]; j++) {
					*q++ = dd;
					__LIKELY_IF(r2 != nullptr) {
						*r2++ = dm;
					}
					k++;
					__UNLIKELY_IF(k >= width) break;
				}
				__UNLIKELY_IF(k >= width) return true;
			}
		}
	}
	return true;
}

bool TOWNS_CRTC::render_256(int trans, scrntype_t* dst, int y, int& rendered_pixels)
{
	// 256 colors
	__UNLIKELY_IF(dst == nullptr) return false;
	int magx = linebuffers[trans][y].mag[0];
	int pwidth = linebuffers[trans][y].pixels[0];
	uint8_t *p = linebuffers[trans][y].pixels_layer[0];

	scrntype_t* q = dst;
	__UNLIKELY_IF(pwidth <= 0) return false;
	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	__DECL_ALIGNED(16) int magx_tmp[16];
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		magx_tmp[i] = (magx + (i & 1)) / 2;
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2 + 16 * magx);

	rendered_pixels = pwidth;
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) > 1) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) > 1) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}
	magx = magx / 2;
	__UNLIKELY_IF(magx < 1) return false;
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 0) return false;

	__DECL_ALIGNED(32)  scrntype_t apal256[256];
	my_memcpy(apal256, &(linebuffers[trans][y].palettes[0].pixels[0]), sizeof(scrntype_t) * 256);


//	out_debug_log(_T("Y=%d MAGX=%d WIDTH=%d pWIDTH=%d"), y, magx, width, pwidth);
	__UNLIKELY_IF(pwidth < 1) pwidth = 1;
	int xx = 0;
	int k = 0;
	csp_vector8<uint8_t> __pbuf[2];
	csp_vector8<scrntype_t> __sbuf[2];
	size_t width_tmp1 = width;
	for(int x = 0; x < (pwidth >> 4); x++) {
__DECL_VECTORIZED_LOOP
		for(int ii = 0; ii < 2; ii++) {
			__pbuf[ii].load(&p[ii << 3]);
		}
__DECL_VECTORIZED_LOOP
		for(int ii = 0; ii < 2; ii++) {
			__sbuf[ii].lookup(__pbuf[ii], apal256);
		}
		p += 16;
		int kbak = k;
		if((((magx << 4) + k) <= width) && !(odd_mag)) {
			__LIKELY_IF(width_tmp1 >= 16) {
				q = scaling_store(q, __sbuf, magx, 2, width_tmp1);
				k += (magx * 16);
			}
			__UNLIKELY_IF(width_tmp1 < 16) break;
		} else {
			for(size_t ii = 0; ii < 2; ii++) {
				for(size_t i = 0; i < 8; i++) {
					scrntype_t tmp = __sbuf[ii].at(i);
					for(int j = 0; j < magx_tmp[(ii << 3) + i]; j++) {
						*q++ = tmp;
						k++;
						__UNLIKELY_IF(k >= width) break;
					}
					__UNLIKELY_IF(k >= width) break;
				}
				__UNLIKELY_IF(k >= width) break;
			}
		}
	}
	__LIKELY_IF(k >= width) return true;
	size_t w = pwidth & 0x0f;
	__UNLIKELY_IF(w != 0) {
		__pbuf[0].clear();
		__sbuf[0].clear();
		__pbuf[0].load_limited(p, w);
		__sbuf[0].lookup(__pbuf[0], apal256, w);

		if((((magx * w) + k) <= width) && !(odd_mag)) {
			switch(magx) {
			case 1:
				__sbuf[0].store_limited(q, w);
				k += w;
				q += w;
				break;
			case 2:
				__sbuf[0].store2_limited(q, w);
				k += (w << 1);
				q += (w << 1);
				break;
			case 4:
				__sbuf[0].store4_limited(q, w);
				k += (w << 2);
				q += (w << 2);
				break;
			default:
				__sbuf[0].store_n_limited(q, magx, w);
				q += (magx * w);
				k += (magx * w);
				break;
			}
		} else {
			__DECL_ALIGNED(32) scrntype_t sbuf[16];
			__sbuf[0].store_aligned(sbuf);
			for(int i = 0; i < w; i++) {
				for(int j = 0; j < magx_tmp[i]; j++) {
					q[j] = sbuf[i];
					k++;
					__UNLIKELY_IF(k >= width) break;
				}
				__UNLIKELY_IF(k >= width) break;
				q += magx;
			}
		}
	}
	return true;
}

bool TOWNS_CRTC::render_16(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	__DECL_ALIGNED(32) static const scrntype_t maskdata_transparent[16] = {
		RGBA_COLOR(0, 0, 0, 0),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),

		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),

		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),

		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255),
		RGBA_COLOR(255, 255, 255, 255)
	};	
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = (do_alpha) ? NULL : mask;
	__UNLIKELY_IF(pwidth <= 0) return false;

	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	__DECL_ALIGNED(16) int magx_tmp[16];
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		magx_tmp[i] = (magx + (i & 1)) / 2;
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2 + 16 * magx);
	rendered_pixels = pwidth;

	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) > 1) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) > 1) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}
	magx = magx / 2;
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 1) return false;
	__UNLIKELY_IF(magx < 1) return false;

	csp_vector8<uint8_t> pbuf;
	csp_vector8<uint8_t> hlbuf[2];
	csp_vector8<uint8_t> mbuf;
	csp_vector8<scrntype_t> sbuf[2];
	csp_vector8<scrntype_t> abuf[2];
	__DECL_ALIGNED(32) scrntype_t palbuf[16];
	
	uint8_t pmask = linebuffers[trans][y].r50_planemask[layer] & 0x0f;
	mbuf.fill(pmask);

	scrntype_t *pal = &(linebuffers[trans][y].palettes[layer].pixels[0]);
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		palbuf[i] = pal[i];
	}
	// Clear palbuf[0]?
	if((do_alpha) && (is_transparent)) {
		palbuf[0] &= RGBA_COLOR(255, 255, 255, 0); // OK?
	} else if(!is_transparent) {
		abuf[0].fill(RGBA_COLOR(255, 255, 255, 255));
		abuf[1].fill(RGBA_COLOR(255, 255, 255, 255));
	}
	int k = 0;

	size_t width_tmp1 = pwidth << 1;
	size_t width_tmp2 = pwidth << 1;

	csp_vector8<uint8_t> bytes_mask;
	for(int i = 0; i < 8; i += 2) {
		bytes_mask.set(i + 0, 0x0f);
		bytes_mask.set(i + 1, 0xf0);
	}
		
	for(int x = 0; x < (pwidth >> 3); x++) {
		hlbuf[0].load2(&(p[0]));
		hlbuf[1].load2(&(p[4]));
		p += 8;

		hlbuf[0] &= bytes_mask;
		hlbuf[1] &= bytes_mask;

		for(int j = 0; j < 2; j++) {
			for(size_t i = 1; i < 8; i += 2) {
				hlbuf[j].rshift(i, 4);
			}
		}
		hlbuf[0] &= mbuf;
		hlbuf[1] &= mbuf;

		sbuf[0].lookup(hlbuf[0], palbuf);
		sbuf[1].lookup(hlbuf[1], palbuf);
		
		if(!(do_alpha) && (is_transparent)) {
			abuf[0].lookup(hlbuf[0], (scrntype_t*)maskdata_transparent);
			abuf[1].lookup(hlbuf[1], (scrntype_t*)maskdata_transparent);
		}			
		int kbak = k;
		__LIKELY_IF((((magx << 4) + k) <= width) && !(odd_mag)) {
			__LIKELY_IF((width_tmp1 >= 16)) {
				q = scaling_store(q, sbuf, magx, 2, width_tmp1);
				__LIKELY_IF(r2 != nullptr) {
					r2 = scaling_store(r2, abuf, magx, 2, width_tmp2);
				}
				k += (16 * magx);
			}
			__UNLIKELY_IF((width_tmp1 < 16) || (width_tmp2 < 16)) break;
		} else {
			int i1 = 0;
			auto kbak = k;
			for(size_t i0 = 0; i0 < 2; i0++) {
				__LIKELY_IF(q != NULL) {
					for(size_t i = 0; i < 8; i++) {
						size_t dd = sbuf[i0].at(i);
						for(int j = 0; j < magx_tmp[i]; j++) {
							*q++ = dd;
							k++;
							width_tmp1--;
							__UNLIKELY_IF(k >= width) break;
						}
						__UNLIKELY_IF(k >= width) break;
					}
				}
				__LIKELY_IF(r2 != NULL) {
					for(size_t i = 0; i < 8; i++) {
						size_t dm = abuf[i0].at(i);
						for(int j = 0; j < magx_tmp[i]; j++) {
							*r2++ = dm;
							kbak++;
							width_tmp2--;
							__UNLIKELY_IF(kbak >= width) break;
						}
						__UNLIKELY_IF(kbak >= width) break;
					}
				}
				__UNLIKELY_IF((width_tmp1 < 8) || (width_tmp2 < 8)) break;
			}
		}

		__UNLIKELY_IF(k >= width) return true;
	}
	__LIKELY_IF(k >= width) return true;
	uint8_t tmpp;
	uint8_t tmph;
	uint8_t tmpl;
	scrntype_t ah, al;
	int rwidth = pwidth & 7;
	scrntype_t stmp[2];

	__UNLIKELY_IF(rwidth > 0) {
		for(int x = 0; x < rwidth; x++) {
			tmpp = *p++;
			tmph = tmpp >> 4;
			tmpl = tmpp & 0x0f;
			__UNLIKELY_IF(do_alpha) {
				if(is_transparent) {
					stmp[0] = (tmph == 0) ? RGBA_COLOR(0, 0, 0, 0) : palbuf[tmph];
					stmp[1] = (tmpl == 0) ? RGBA_COLOR(0, 0, 0, 0) : palbuf[tmpl];
				} else {
					stmp[0] = palbuf[tmph];
					stmp[1] = palbuf[tmpl];
				}

				if((magx == 1) && !(odd_mag)) {
					for(int ii = 0; ii < 2; ii++) {
						*q++ = stmp[ii];
						k++;
						__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					}
				} else {
					for(int ii = 0; ii < 2; ii++) {
						for(int xx = 0; xx < magx_tmp[(x << 1) + ii]; xx++) {
							*q++ = stmp[ii];
							k++;
							__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
						}
						__UNLIKELY_IF(k >= width) break;
					}
				}
			} else {
				stmp[0] = palbuf[tmph];
				stmp[1] = palbuf[tmpl];
				if(is_transparent) {
					ah = (tmph == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
					al = (tmpl == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
				} else {
					ah = RGBA_COLOR(255, 255, 255,255);
					al = RGBA_COLOR(255, 255, 255,255);
				}
				if((magx == 1) && !(odd_mag)) {
					for(int ii = 0; ii < 2; ii++) {
						*q++ = stmp[ii];
					}
					__LIKELY_IF(r2 != nullptr) {
						*r2++ = ah;
						*r2++ = al;
					}
					k += 2;
					__UNLIKELY_IF(k >= (TOWNS_CRTC_MAX_PIXELS - 1)) break;
				} else {
					for(int ii = 0; ii < 2; ii++) {
						scrntype_t tmp = stmp[ii];
						for(int j = 0; j < magx_tmp[(x << 1) + ii]; j++) {
							*q++ = tmp;
						}
					}
					__LIKELY_IF(r2 != nullptr) {
						for(int j = 0; j < magx_tmp[(x << 1) + 0]; j++) {
							*r2++ = ah;
						}
						for(int j = 0; j < magx_tmp[(x << 1) + 1]; j++) {
							*r2++ = al;
						}
					}
					k += 2;
					__UNLIKELY_IF(k >= (TOWNS_CRTC_MAX_PIXELS - 2)) break;
				}
			}
		}
	}
	return true;
}

inline void TOWNS_CRTC::transfer_pixels(scrntype_t* dst, scrntype_t* src, int w)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (w <= 0)) return;
//	for(int i = 0; i < w; i++) {
//		dst[i] = src[i];
//	}
	my_memcpy(dst, src, w * sizeof(scrntype_t));
}


// This function does alpha-blending.
// If CSP support hardware-accelalations, will support.
// (i.e: Hardware Alpha blending, Hardware rendaring...)
void TOWNS_CRTC::mix_screen(int y, int width, bool do_mix0, bool do_mix1, int bitshift0, int bitshift1, int words0, int words1, bool is_hloop0, bool is_hloop1)
{
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(width <= 0) return;

	scrntype_t *pp = osd->get_vm_screen_buffer(y);
	if(y == 128) {
		//out_debug_log(_T("MIX_SCREEN Y=%d WIDTH=%d DST=%08X"), y, width, pp);
	}
	
	__LIKELY_IF(pp != nullptr) {
		int left0 = words0;
		int left1 = words1;
		__UNLIKELY_IF(words0 <= 0) {
			do_mix0 = false;
		}
		__UNLIKELY_IF(words1 <= 0) {
			do_mix1 = false;
		}
		if((do_mix1) || (do_mix0)) {
			// Clear BG
			csp_vector8<scrntype_t> bgclean((scrntype_t)RGBA_COLOR(0, 0, 0, 255));
			for(int xx = 0; xx < width; xx += 8) {
				bgclean.store(&(pp[xx]));
			}
			if((width & 7) != 0) {
				bgclean.store_limited(&(pp[width & ~(7)]), (const size_t)(width & 7));
			}
		}
		size_t begin_x0, end_x0, begin_x1, end_x1;
		size_t offset00, offset10;
		begin_x0 = bitshift0;
		end_x0 = bitshift0 + words0;
		begin_x1 = bitshift1;
		if(is_hloop0) {
			offset00 = words0 - bitshift0;
		} else {
		    offset00 = 0;
		}
		if(is_hloop1) {
			offset10 = words1 - bitshift1;
		} else {
		    offset10 = 0;
		}
		size_t xptr0 = 0;
		size_t xptr1 = 0;
		// Store BG
		scrntype_t pix1;
		scrntype_t pix0;
		scrntype_t mask_front;
		for(size_t xx = 0; xx < width; xx++) {
			bool is_got0 = false;
			bool is_got1 = false;
			pix1   = RGBA_COLOR(0, 0, 0, 255);
			pix0   = RGBA_COLOR(0, 0, 0, 255);
			mask_front = RGBA_COLOR(255, 255, 255, 255);
			if(do_mix0) {
				if(is_hloop0) {
					if(xx < bitshift0) { // Lower
						pix0 = lbuffer0[offset00];
						mask_front = abuffer0[offset00];
						offset00++;
						is_got0 = true;
					} else if((xx >= bitshift0) && (xx < (bitshift0 + words0))) {
						pix0 = lbuffer0[xptr0];
						mask_front = abuffer0[xptr0];
						xptr0++;
						is_got0 = true;
					}
				} else {
					__LIKELY_IF((xx >= bitshift0) && (xx < (bitshift0 + words0))) {
						pix0 = lbuffer0[xptr0];
						mask_front = abuffer0[xptr0];
						xptr0++;
						is_got0 = true;
					}
				}
			}
			if(do_mix1) {
				if(is_hloop1) {
					if(xx < bitshift1) { // Lower
						pix1 = lbuffer1[offset10];
						offset10++;
						is_got1 = true;
					} else if((xx >= bitshift1) && (xx < (bitshift1 + words1))) {
						pix1 = lbuffer1[xptr1];
						xptr1++;
						is_got1 = true;
					}
				} else {
					__LIKELY_IF((xx >= bitshift1) && (xx < (bitshift1 + words1))) {
						pix1 = lbuffer1[xptr1];
						xptr1++;
						is_got1 = true;
					}
				}
			}
			__LIKELY_IF((is_got0) && (is_got1)) {
				scrntype_t mask_back = ~(mask_front);
				pix1 &= mask_back;
				pix0 &= mask_front;
				pix0 |= pix1;
				pp[xx] = pix0;
			} else if(is_got0) {
				pp[xx] = pix0;
			} else if(is_got1) {
				pp[xx] = pix1;
			}
		}
	}
}

void TOWNS_CRTC::draw_screen()
{
	int trans = display_linebuf.load() & display_linebuf_mask;
/*	display_remain--;
	if(display_remain.load() >= 0) {
		display_linebuf = (display_linebuf.load() + 1) & display_linebuf_mask;
	} else {
		display_remain = 0;
	}
*/
	bool do_alpha = false; // ToDo: Hardware alpha rendaring.
	__UNLIKELY_IF(d_vram == nullptr) {
		return;
	}
	int lines = vst[trans];
	int width = hst[trans];

	//bool is_single_tmp = is_single_layer[trans];
	bool is_single_tmp = is_single_mode_for_standard(control_cache[trans]);
	// Will remove.
	__UNLIKELY_IF(lines <= 0) lines = 1;
	__UNLIKELY_IF(width <= 16) width = 16;

	__UNLIKELY_IF(lines > TOWNS_CRTC_MAX_LINES) lines = TOWNS_CRTC_MAX_LINES;
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	osd->set_vm_screen_size(width, lines, 1024, lines, 1024, 768);
	//out_debug_log(_T("%s RENDER WIDTH=%d HEIGHT=%d"), __FUNCTION__, width, lines);
	osd->set_vm_screen_lines(lines / 2);

	int yskip[2] = {0};
	int ycount[2] = {0};

	int disp_y = 0;
	if(voffset[trans][1] > voffset[trans][0]) {
		yskip[1] = voffset[trans][1] - voffset[trans][0];
	} else if(voffset[trans][0] > voffset[trans][1]) {
		yskip[0] = voffset[trans][0] - voffset[trans][1];
	}
	for(int y = 0; y < lines; y++) {
		int real_mode[2] = { DISPMODE_NONE };
		int real_y[2] = {0};
		int prio[2] = {0, 1};
		bool do_render[2] = {false};
		// Clear buffers per every lines.

		// ToDo: Enable to raster scrolling. 20240104 K.O
		for(int l = 0; l < 2; l++) {
			int __y = ycount[l];
			int tmp_num = linebuffers[trans][__y].num[l];
			prio[l] = tmp_num & 1;
			
			do_render[l] = (linebuffers[trans][__y].crtout[l] != 0) ? true : false;
			__LIKELY_IF(do_render[l]) {
				real_y[l] = __y;
				real_mode[l] = (do_render[l]) ? linebuffers[trans][__y].mode[l] : DISPMODE_NONE;
				ycount[l]++;
			}
		}
		for(int l = 0; l < 2; l++) {
			__LIKELY_IF(do_render[l]) {
				int prio_l = prio[l];
				int __y = real_y[l];
				#if 0
				__UNLIKELY_IF(((real_mode[l] & DISPMODE_DUP) != 0) && (do_render[l])) {
					int prev_line = linebuffers[trans][__y].prev_y[l];
					__LIKELY_IF((prev_line >= 0) && (prev_line < TOWNS_CRTC_MAX_LINES)){
						int prev_pwidth = linebuffers[trans][prev_line].pixels[l];
						if(prev_pwidth > 0) {
							copy_line(trans, l, prev_line, __y);
						} else {
							clear_line(trans, l, __y);
							do_render[l] = false;
							real_mode[l] = DISPMODE_NONE;
						}
					} else {
						clear_line(trans, l, __y);
						do_render[l] = false;
						real_mode[l] = DISPMODE_NONE;
					}
				}
			#endif
			}
		}
		if((do_render[0]) || (do_render[1])) {
			disp_y++;
			memset(lbuffer1, 0x00, sizeof(lbuffer1));
			memset(abuffer1, 0xff, sizeof(abuffer1));
			memset(lbuffer0, 0x00, sizeof(lbuffer0));
			memset(abuffer0, 0xff, sizeof(abuffer0));
		}/* else {
			continue;
			}*/
		scrntype_t* pix_array[2] = {lbuffer0, lbuffer1};
		scrntype_t* alpha_array[2] = {abuffer0, abuffer1};
		
		bool do_mix[2] = {false};
		int bitshift[2] = {0};
		bool is_hloop[2] = {false};
		int rendered_words[2] = {0};
		if(is_single_tmp) {
			do_render[1] = false;
		}
		for(int l = 0; l < 2; l++) {
			bool is_transparent = false;
			int prio_l = (is_single_tmp) ? 0 : (prio[l] & 1); // Will Fix
			is_transparent = ((do_render[0]) && (do_render[1]) && (prio_l == 0)) ? true : false;			
			if(do_render[l]) {

				scrntype_t* pix = pix_array[prio_l];
				scrntype_t* alpha = alpha_array[prio_l];
				int yyy = real_y[l]; // ToDo: Reduce rendering costs.
				bitshift[prio_l] = linebuffers[trans][yyy].bitoffset[l];
				is_hloop[prio_l] = (linebuffers[trans][yyy].is_hloop[l] != 0) ? true : false;
				switch(real_mode[l] & ~(DISPMODE_DUP)) {
				case DISPMODE_256:
					if(prio_l == 0) {
						do_mix[0] = render_256(trans, pix, yyy, rendered_words[0]);
					}
					break;
				case DISPMODE_32768:
					do_mix[prio_l] = render_32768(trans, pix, alpha, yyy, l, is_transparent, do_alpha, rendered_words[prio_l]);
					break;
				case DISPMODE_16:
					do_mix[prio_l] = render_16(trans, pix, alpha, yyy, l, is_transparent, do_alpha, rendered_words[prio_l]);
					break;
				default:
					break;
				}
			}
		}
//		if(y == 128) {
//			out_debug_log(_T("MIX: %d %d width=%d"), do_mix0, do_mix1, width);
//		}
		
		__LIKELY_IF((do_mix[0]) && (do_mix[1])) {
			if(bitshift[0] > bitshift[1]) {
				bitshift[0] = bitshift[0] - bitshift[1];
				bitshift[1] = 0;
			} else if(bitshift[1] > bitshift[0]) {
				bitshift[0] = 0;
				bitshift[1] = bitshift[1] - bitshift[0];
			} else {
				bitshift[0] = 0;
				bitshift[1] = 0;
			}
//			__UNLIKELY_IF(is_transparent[1]) {
//				memset(abuffer1, 0xff, sizeof(abuffer1));
//			}
			mix_screen(y, width, do_mix[0], do_mix[1], bitshift[0], bitshift[1], rendered_words[0], rendered_words[1], is_hloop[0], is_hloop[1]);
		} else {
			__LIKELY_IF(do_mix[0]) {
				//memset(abuffer0, 0xff, sizeof(abuffer0));
				mix_screen(y, width, do_mix[0], false, bitshift[0], 0, rendered_words[0], 0, is_hloop[0], false);
			} else if(do_mix[1]) {
				//memset(abuffer1, 0xff, sizeof(abuffer1));
				mix_screen(y, width, false, do_mix[1], 0, bitshift[1], 0, rendered_words[1], false, is_hloop[1]);
			}
			// ToDo: Clear VRAM?
		}


	}
	return;
}

void TOWNS_CRTC::copy_line(const int trans, int layer, const int from_y, const int to_y)
{
	__UNLIKELY_IF(from_y == to_y) {
		return;
	}
	__UNLIKELY_IF(from_y < 0) {
		return;
	}
	__UNLIKELY_IF(to_y < 0) {
		return;
	}
	layer &= 1;
	linebuffers[trans][to_y].mode[layer] = 	linebuffers[trans][from_y].mode[layer];
	linebuffers[trans][to_y].is_hloop[layer] = linebuffers[trans][from_y].is_hloop[layer];
	linebuffers[trans][to_y].mag[layer] = linebuffers[trans][from_y].mag[layer];
	linebuffers[trans][to_y].r50_planemask[layer] = linebuffers[trans][from_y].r50_planemask[layer];
	linebuffers[trans][to_y].crtout[layer] = linebuffers[trans][from_y].crtout[layer];
	linebuffers[trans][to_y].pixels[layer] = linebuffers[trans][from_y].pixels[layer];
	linebuffers[trans][to_y].num[layer] = linebuffers[trans][from_y].num[layer];
	linebuffers[trans][to_y].bitoffset[layer] = linebuffers[trans][from_y].bitoffset[layer];

	
	__DECL_ALIGNED(32) uint8_t cache[32]; // 8 * 2
	for(int x = 0; (x < TOWNS_CRTC_MAX_PIXELS * 2); x += 32) {
		__DECL_VECTORIZED_LOOP
		for(int xx = 0; xx < 32; xx++) {
			cache[xx] = linebuffers[trans][from_y].pixels_layer[layer][x + xx];
		}
		__DECL_VECTORIZED_LOOP
		for(int xx = 0; xx < 32; xx++) {
			linebuffers[trans][to_y].pixels_layer[layer][x + xx] = cache[xx];
		}
	}
	memcpy(&(linebuffers[trans][to_y].palettes[layer]),
		   &(linebuffers[trans][from_y].palettes[layer]),
		   sizeof(palette_backup_t));

}
	
void TOWNS_CRTC::clear_line(const int trans, int layer, const int y)
{
	layer &= 1;
	__DECL_ALIGNED(32) uint8_t clrdata[32]; // 8 * 2
	if((linebuffers[trans][y].mode[layer] & ~(DISPMODE_DUP)) == DISPMODE_32768) {
		__DECL_VECTORIZED_LOOP
		for(int ii = 0; ii < 32; ii += 2) {
			clrdata[ii + 0] = 0x00;
			clrdata[ii + 1] = 0x80;
		}
	} else {
		__DECL_VECTORIZED_LOOP
		for(int ii = 0; ii < 32; ii++) {
			clrdata[ii] = 0x00;
		}
	}
	for(int x = 0; (x < TOWNS_CRTC_MAX_PIXELS * 2); x += 32) {
		__DECL_VECTORIZED_LOOP
		for(int xx = 0; xx < 32; xx++) {
			linebuffers[trans][y].pixels_layer[layer][x + xx] = clrdata[xx];
		}
	}
	
}
void TOWNS_CRTC::pre_transfer_line(int layer, int line)
{
	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;

	layer &= 1;
	uint8_t prio;
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];

	prio = video_out_regs[FMTOWNS::VOUTREG_PRIO];
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i += 2) {
		linebuffers[trans][line].mode[i + layer] = DISPMODE_NONE;
		linebuffers[trans][line].pixels[i + layer] = 0;
		linebuffers[trans][line].is_hloop[i + layer] = NOT_LOOP;
		linebuffers[trans][line].mag[i + layer] = 0;
		linebuffers[trans][line].num[i + layer] = layer;
	}
	linebuffers[trans][line].r50_planemask[layer] = r50_planemask;
	int disp_prio[2] = {0, 1};
	if(!(is_single_tmp)) {
		if((prio & 0x01) == 0) {
			disp_prio[0] = 0; // Front
			disp_prio[1] = 1; // Back
		} else {
			disp_prio[0] = 1;
			disp_prio[1] = 0;
		}
	}
//	out_debug_log("LINE %d CTRL=%02X \n", line, ctrl);
	
	int mode_tmp = real_display_mode[layer];
	bool disp = frame_in[layer];
	if(is_single_tmp) {
		// ToDo: High resolution.
		if(layer != 0) {
			disp = false;
		} else if((mode_tmp == DISPMODE_16)) {
			disp = false;
		}
	} else {
		if(mode_tmp == DISPMODE_256) {
			disp = false;
		}
	}
	
	linebuffers[trans][line].num[layer] = disp_prio[layer];
	linebuffers[trans][line].crtout[layer] = ((crtout_fmr[layer]) /*&& (crtout_towns[0]) */&& (disp)) ? 0xff : 0x00;
	linebuffers[trans][line].mode[layer] = mode_tmp;
	// Copy (sampling) palettes
	clear_line(trans, layer, line);
	
	switch(mode_tmp) {
	case DISPMODE_256:
		memcpy(&(linebuffers[trans][line].palettes[layer].raw[0][0]), &(apalette_256_rgb[0][0]), sizeof(uint8_t) * 256 * 4); // Copy RAW
		memcpy(&(linebuffers[trans][line].palettes[layer].pixels[0]), &(apalette_256_pixel[0]), sizeof(scrntype_t) * 256); // Copy Pixels
		break;
	case DISPMODE_16:
		memcpy(&(linebuffers[trans][line].palettes[layer].raw[0][0]), &(apalette_16_rgb[layer][0][0]), sizeof(uint8_t) * 16 * 4); // Copy RAW
		memcpy(&(linebuffers[trans][line].palettes[layer].pixels[0]), &(apalette_16_pixel[layer][0]), sizeof(scrntype_t) * 16); // Copy Pixels
		break;
	default:
		memset(&(linebuffers[trans][line].palettes[layer].raw[0][0]), 0x00, sizeof(uint8_t) * 256 * 4);
		memset(&(linebuffers[trans][line].palettes[layer].pixels[0]), 0x00, sizeof(scrntype_t) * 256);
		break;
	}
	// Fill by skelton colors;

}

void TOWNS_CRTC::transfer_line(int layer, int line)
{
	int l = layer;
	bool to_disp = true; // Dummy
	static const uint32_t address_add[2] =  {0x00000000, 0x00040000};
	uint8_t ctrl, prio;
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];
	
	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;
	__UNLIKELY_IF(d_vram == nullptr) return;
	__UNLIKELY_IF(layer < 0) return;
	__UNLIKELY_IF(layer > 1) return;
	__UNLIKELY_IF(!(frame_in[l])) return;
	__UNLIKELY_IF(linebuffers[trans] == nullptr) return;
	if((is_single_tmp) && (layer != 0)) return;
	// Update some parameters per line. 20230731 K.O
	uint32_t __vstart_addr  = vstart_addr[l];
	uint32_t __line_offset  = line_offset[l];
	uint32_t address_mask = 0x0003ffff;

	uint32_t lo = __line_offset;
	//uint32_t hscroll_mask = ((lo == 128) || (lo == 256)) ? lo : 0xffffffff;
	uint32_t hscroll_mask = 0xffffffff; // ToDo.
	uint32_t magx = zoom_factor_horiz[l];

	int hoffset   = hoffset_val[l];
	int bit_shift = max(0, hbitshift_val[l]);
	int64_t hwidth = hwidth_val[l];
	
	// FAx
	// Note: Re-Wrote related by Tsugaru. 20230806 K.O
	if(is_single_tmp) {
		address_mask = 0x0007ffff;
	}

	uint32_t skip_zoom = zoom_raw_horiz[l];
	int64_t pixels = hwidth * ((is_single_tmp) ? 2 : 1);
	int64_t bit_shift64 = bit_shift;
	
	recalc_width_by_clock(magx, pixels);
	recalc_offset_by_clock(skip_zoom, hoffset, bit_shift64);
	uint32_t address_shift;
	int32_t  hskip_bytes;
	switch(linebuffers[trans][line].mode[l]) {
	case DISPMODE_32768:
		if(is_single_tmp) {
			address_shift = 3; // FM-Towns Manual P.145
			hskip_bytes = bit_shift64 << 1;
		} else {
			address_shift = 2; // FM-Towns Manual P.145
			hskip_bytes = bit_shift64 << 1;
		}
		break;
	case DISPMODE_256:
		address_shift = 3; // FM-Towns Manual P.145
		hskip_bytes = bit_shift64;
		break;
	case DISPMODE_16:
	default:
		address_shift = 2; //  Default is 16 colors; FM-Towns Manual P.145		
		hskip_bytes = bit_shift64 >> 1;
		break;
	}
	//! Note:
	//! - Below is from Tsugaru, commit 1a442831 .
	//! - I wonder sprite offset effects every display mode at page1,
	//!   and FMR's address offset register effects every display mode at page0
	//! - -- 20230715 K.O
	uint32_t page_offset;
	if(l == 0) {
		page_offset = ((r50_pagesel != 0) ? 0x20000 : 0);
	} else {
		page_offset = sprite_offset;
	}
	__UNLIKELY_IF(pixels < 0) {
		pixels = 0;
	} else if(pixels > TOWNS_CRTC_MAX_PIXELS) {
		pixels = TOWNS_CRTC_MAX_PIXELS;
	}
	__LIKELY_IF((linebuffers[trans][line].crtout[l] != 0)){
		__LIKELY_IF(/*(pixels >= magx) && */(magx != 0)){
			//__UNLIKELY_IF(pixels >= TOWNS_CRTC_MAX_PIXELS) pixels = TOWNS_CRTC_MAX_PIXELS;
			uint32_t pixels_bak = pixels;
			linebuffers[trans][line].bitoffset[l] =  hoffset;
			linebuffers[trans][line].pixels[l] = pixels;
			linebuffers[trans][line].mag[l] = magx; // ToDo: Real magnif
			linebuffers[trans][line].is_hloop[l] = NOT_LOOP;
			pixels = ((pixels_bak << 16) / skip_zoom) >> 15;
			bool is_256 = false;
			uint32_t tr_bytes = 0;
			switch(linebuffers[trans][line].mode[l]) {
			case DISPMODE_32768:
				tr_bytes = pixels << 1;
				//lo = lo << 1;
				break;
			case DISPMODE_16:
				tr_bytes = pixels >> 1;
				//lo = lo >> 1;
				break;
			case DISPMODE_256:
				is_256 = true;
				tr_bytes = pixels;
				break;
			default:
				to_disp = false;
				break;
			}
			#if 0
			if((line > 0)) {
				int prev_mode = linebuffers[trans][line - 1].mode[l] & ~(DISPMODE_DUP);
				int prev_line = linebuffers[trans][line - 1].prev_y[l];
				int cur_mode = linebuffers[trans][line].mode[l] & ~(DISPMODE_DUP);
				if((cur_mode != DISPMODE_NONE) && (cur_mode == prev_mode) && (to_disp)) {
					if((zoom_count_vert[l] < zoom_factor_vert[l])) {
						// Skip lines.
						linebuffers[trans][line].mode[l] = cur_mode | DISPMODE_DUP;
						__LIKELY_IF(prev_line >= 0) {
							linebuffers[trans][line].prev_y[l] = prev_line;
						}
						if(zoom_count_vert[l] > 0) {
							zoom_count_vert[l]--;
						}
						if(zoom_count_vert[l] <= 0) {
							zoom_count_vert[l] = zoom_factor_vert[l];
							head_address[l] += lo;
						}
						return;
					}
				}
			}
			#endif
			if(to_disp) {
				uint32_t offset = __vstart_addr; // ToDo: Larger VRAM
				uint32_t head_tmp = head_address[l];
				offset = offset + head_tmp;
				bool is_xwrap = false;
				offset <<= address_shift;
				offset += hskip_bytes;
				uint32_t lo2 = lo * ((is_single_tmp) ? 4 : 8);
				linebuffers[trans][line].prev_y[l] = line;

				__UNLIKELY_IF((lo2 == 1024) || (lo2 == 512)) {
					__LIKELY_IF(!(is_256)) {
						hscroll_mask = lo2 - 1;
						is_xwrap = true;
					}
				}

				__UNLIKELY_IF(is_interlaced[trans][l]) {
					if(interlace_field) {
						offset = offset + (frame_offset_bak[l] << address_shift);
					}
				}

				offset = ((offset + page_offset) & address_mask);
				// ToDo: FO1 Offset Value.
				// ToDo: Will Fix
				__LIKELY_IF(!(is_single_tmp)) {
					offset = offset + address_add[l];
				}
				d_vram->get_data_from_vram(((is_single_tmp) || (is_256)), offset, tr_bytes, &(linebuffers[trans][line].pixels_layer[l][0]), hscroll_mask);
				__LIKELY_IF(is_xwrap) {
					linebuffers[trans][line].is_hloop[l] = IS_LOOP;
				}
			}
		}
	} else {
		to_disp = false;
	}
	if(zoom_count_vert[l] > 0) {
		zoom_count_vert[l]--;
	}
	if(zoom_count_vert[l] <= 0) {
		zoom_count_vert[l] = zoom_factor_vert[l];
		// ToDo: Interlace
		//if((to_disp)) {
			head_address[l] += lo;
			//head_address[l] &= (address_mask >> address_shift);
		//}
	}
}

void TOWNS_CRTC::set_crtc_parameters_from_regs()
{
	int trans = render_linebuf.load() & display_linebuf_mask;
	calc_screen_parameters();  // Re-Calculate general display parameters.
	update_horiz_khz();
	
	copy_regs_v(); // Calculate display parameters per layer.
	copy_regs_h(); // Calculate display parameters per layer.
	
	force_recalc_crtc_param(); // Calculate parameter around HSYNC.
	
	vst1_count = vst1 << 1;
	vst2_count = vst2 << 1;

	__UNLIKELY_IF(vst1_count >= vst2_count) {
		vst2_count = vst1_count + 1;
	}
	calc_pixels_lines(); 

	lines_per_frame = vst_reg;

	eet_count = eet;
	horiz_us = horiz_us_next;
	
	double horiz_ref = horiz_us;

	frame_us = ((double)lines_per_frame) * horiz_ref; // VST
	if(frame_us <= 0.0) {
		frame_us = 1.0e6 / FRAMES_PER_SEC;
	}

	set_frames_per_sec(1.0e6 / frame_us);
	set_lines_per_frame(lines_per_frame);
}

void TOWNS_CRTC::begin_of_display()
{
	int trans = render_linebuf & display_linebuf_mask;

//	make_crtout_from_fda0h(crtout_reg);
//	make_crtout_from_044a(video_out_regs[FMTOWNS::VOUTREG_CTRL]);
	update_control_registers(trans);
	recalc_cr0(regs[TOWNS_CRTC_REG_DISPMODE], true);
	make_dispmode(is_single_layer[trans], real_display_mode[0], real_display_mode[1]);
	
	calc_zoom_regs(regs[TOWNS_CRTC_REG_ZOOM]);
	set_crtc_parameters_from_regs();
	
	for(int layer = 0; layer < 2; layer++) {
		horiz_start_us[layer] = horiz_start_us_next[layer];
		horiz_end_us[layer] = horiz_end_us_next[layer];
	}
	
	vst[trans] = max_lines;
	hst[trans] = pixels_per_line;
	
	for(int i = 0; i < 2; i++) {
		voffset[trans][i] = voffset_val[i];
	}
	for(int i = 0; i < 2; i++) {
		zoom_count_vert[i] = zoom_factor_vert[i];
		frame_offset_bak[i] = frame_offset[i];
		is_interlaced[trans][i] = layer_is_interlaced(i);
	}

	for(int yy = 0; yy < TOWNS_CRTC_MAX_LINES; yy++) {
		for(int i = 0; i < 2; i++) {
			// Maybe initialize.
			linebuffers[trans][yy].is_hloop[i] = NOT_LOOP;
			linebuffers[trans][yy].pixels[i] = pixels_per_line;
			linebuffers[trans][yy].mag[i] = 1;
			linebuffers[trans][yy].num[i] = i;
			linebuffers[trans][yy].mode[i] = DISPMODE_NONE;
			linebuffers[trans][yy].crtout[i] = 0;
			linebuffers[trans][yy].bitoffset[i] = 0;
			linebuffers[trans][yy].prev_y[i] = -1;
		}
	}
	// Check whether Interlaced.
	__UNLIKELY_IF((is_interlaced[trans][0]) || (is_interlaced[trans][1])) {
		interlace_field = !interlace_field;
	} else {
		interlace_field = false;
	}
	
}

void TOWNS_CRTC::event_pre_frame()
{
//	make_crtout_from_fda0h(crtout_reg);
//	make_crtout_from_044a(video_out_regs[FMTOWNS::VOUTREG_CTRL]);
	for(int i = 0; i < 2; i++) {
		hdisp[i] = false;
		frame_in[i] = false;
		head_address[i] = 0;
		update_vstart(i);	
		update_line_offset(i);
	}
	display_linebuf = render_linebuf.load();
	__LIKELY_IF(display_enabled) {
		render_linebuf++;
		render_linebuf &= display_linebuf_mask;
	}

	/*!<
	 @note 20231230 K.O -- Belows are written in Japanese (mey be or not be temporally).
	 以下、FM Towns Technical data book (「赤本」)の Section 4.7 「CRTC周辺のハードウエアの仕組み」による。
	 1. CSPのevent.cpp では、フレームが始まると:
	     a. DEVICE_FOO::event_pre_frame() の処理
		 b. frames_per_sec と lines_per_frame が更新してないかどうかチェック→タイミング変更処理
		 c. DEVICE_FOO::event_frame() の処理
		 d. DEVICE_BAR::event_vline(cur_vline, clocks[cur_vline]) の処理を全ライン行う
		 e. 最後のLineまで終わったら、a.にもどる
	    という処理を行って1フレームをエミュレートしている。
	2. 実マシンでは:
	     a. VSTART
		 b. VST1 時間でVSYNC立ち上がり
		 c. VST2 時間でVSYNC立ち下がり　→ここから事実上表示が始まる(いわゆるひとつの垂直同期信号)
		 d. 画面レイヤーx (x=0 or 1) は、VDSXからはじまる(ラインオフセット)
		    → VDSx < VST2 の場合、VST2までは表示されない！！(多分な)
		 e. 1ラインは、VDSx からカウントが始まる:
		 e-0. 水平同期間隔は、HST + 1 clocks.
		 e-1. HSW1の間、HSYNCパルスが立ち上がる →水平同期信号
		 e-2. HSTARTからHDSxまでの間、画面表示は始まらない → HOFFSETx
		 e-3. HDExで、表示が終了 → HDEx > (HST + 1) の場合は、たぶん(HST + 1)で表示が切れる
		 f. VDEx まで **マスターラインカウントが**達したら、そこで表示終了 → VDEx > (VST + 1) の場合は、多分下が切れる
		 g. なお、a~cの間は、水平同期信号は事実上逆論理になり、HSW2の間、HSYNCパルスが立ち上がるようになる。
	 3. 1. と 2.を比較すると、
	     - CSPではVBLANK期間が考慮されていない
	 4. ということになった場合の解決策は以下の感じになるか？ 
		 - ダミーの表示期間を設定する必要がある？
		 → set_vm_screen_size() と EVENT::set_lines_per_frame() は完全に分離する必要がある
		    - 4.a. EVENT::set_lines_per_frame() は、vst_reg に依存させる
			- 4.b. OSD::set_vm_screen_size(H, W) は、
			  max(HDE0 - HDS0, HDE1 - HDS1), max(VDE0 - VDS0, VDE1 - VDS1)
			  で設定すればいいかな？(´・ω・｀)
			  H, V方向のスケーリングが必要(と言うか、ここらへんはCRTCで計算すりゃいいか)
			- 4.c. 表示絡みのロジックは以下のような感じ？
			    VLINE < VST1 : VSYNC=LOW, HSYNC=NEGATIVE
				VLINE >= VST1 && VLINE < VST2 : VSYNC=HIGH, HSYNC=NEGATIVE
				VSYNC割り込みは、VST1時点か?VST2時点か？それとも、CRTC::event_pre_frame()でやっちゃうか？
				VLINE >= VST2 : 表示可能とする。
				VLINE >= VDSx : レイヤーx表示可能。
				VLINE >  VDEx : レイヤーx表示不可。
	 ---- こんなんでましたけど(´・ω・｀) ---- 
	*/
	begin_of_display();
   
	// Reset VSYNC
	vert_line_count = -1;
	hsync = false;
	vsync = false;
	reset_vsync(); // Auto interrupt off.
}

uint32_t TOWNS_CRTC::get_sprite_offset()
{
	if(d_sprite != NULL) {
		return (d_sprite->read_signal(SIG_TOWNS_SPRITE_BANK) != 0) ? 0x20000 : 0x00000;
	}
	return 0x00000;
}
void TOWNS_CRTC::event_frame()
{
	// Clear all frame buffer (of this) every turn.20230716 K.O
	horiz_width_posi_us = horiz_width_posi_us_next;
	horiz_width_nega_us = horiz_width_nega_us_next;
	
	display_enabled = display_enabled_pre;

	
/*	display_remain++;
	if(display_remain.load() > display_linebuf_mask) {
	display_remain = display_linebuf_mask;
	display_linebuf = (render_linebuf.load() - display_linebuf_mask) & display_linebuf_mask;
	}
*/

	__LIKELY_IF(vst1_count >= vst2_count) {
		hsync = true;
		set_vsync(true);
	}	
	// Set ZOOM factor.
	// ToDo: EET
	cancel_event_by_id(event_hsync);
	for(int layer = 0; layer < 2; layer++) {
		cancel_event_by_id(event_hdisp[layer]);
	}
	// Rendering TEXT.
	sprite_offset = get_sprite_offset();
}

void TOWNS_CRTC::event_vline(int v, int clock)
{
	__UNLIKELY_IF((v < 0)) {
		for(int i = 0; i < 2; i++) {
			frame_in[i] = false;
		}
		hsync = false;
		vsync = false;
		reset_vsync();
		return;
	}
	cancel_event_by_id(event_hsync);

	int __max_lines = max_lines;
	double usec = 0.0;
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];
	
	hsync = true;
	if(v < vst2_count) {
		__UNLIKELY_IF(v == vst1_count) { // Normally, vst1 < vst2.
			set_vsync(true);
		}
		usec = horiz_width_nega_us;
		for(int i = 0; i < 2; i++) {
			hdisp[i] = false;  // HDISP should turn off until HDSx.
			frame_in[i] = false;
		}
	} else {
		usec = horiz_width_posi_us;
		// Make frame_in[layer]
		bool fin_bak[2] = {false};
		
		for(int i = 0; i < 2; i++) {
			fin_bak[i] = frame_in[i];
			hdisp[i] = false; // HDISP should turn off until HDSx.
			__LIKELY_IF((v >= vds[i]) && (v <= vde[i]) && (v < lines_per_frame) && (display_enabled)) {
				__UNLIKELY_IF((is_single_tmp) && (i != 0)) {
					frame_in[i] = false;
				} else {
					frame_in[i] = true;
				}
			} else {
				frame_in[i] = false;
			}
		}
		__UNLIKELY_IF(v == vst2_count) {
			set_vsync(false);
		}
		// Update vstart (by FAx) and line offset (by LOx) when frame_in[layer] has changed.
		// -- 20240314 K.O
		for(int i = 0; i < 2; i++) {
			__UNLIKELY_IF(fin_bak[i] != frame_in[i]) {
				update_vstart(i);	
				update_line_offset(i);
				recalc_hdisp_from_crtc_params(i, horiz_start_us[i], horiz_end_us[i]);
				__LIKELY_IF(d_sprite != NULL) {
					if((i == 0) && !(is_single_tmp)) {
						// Text rendering starts related by LO0. 20240314 K.O
						d_sprite->write_signal(SIG_TOWNS_SPRITE_TEXT_RENDER, 0xffffffff, 0xffffffff);
					}
				}
			}
		}
		// Check frame_in[layer]
		if(frame_in[0] || frame_in[1]) {
			vert_line_count++;
			// Move HDISP feature to EVENT_HSYNC of event_callback(). 20240225 K.O
			if(vert_line_count >= __max_lines) {
				//set_vsync(false);
				hsync = false;
				frame_in[0] = false;
				frame_in[1] = false;
			}
		}
	}
	cancel_event_by_id(event_hsync);
	__UNLIKELY_IF(usec > horiz_us) {
		usec = horiz_us - 0.01; // Insert previous of Hstart. 
	}
	if((v < lines_per_frame) && (usec > 0.0) && (hsync)) {
		register_event(this, EVENT_HSYNC_OFF, usec, false, &event_hsync);
	} else {
		hsync = false;
	}
}

void TOWNS_CRTC::calc_zoom_regs(uint16_t val)
{
	uint8_t zfv[2];
	uint8_t zfh[2];
	pair16_t pd;
	regs[TOWNS_CRTC_REG_ZOOM] = val;
	pd.w = val;
	zfv[0] = ((pd.b.l & 0xf0) >> 4);
	zfh[0] = (pd.b.l & 0x0f);
	zfv[1] = ((pd.b.h & 0xf0) >> 4);
	zfh[1] = (pd.b.h & 0x0f);

	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_single_tmp = is_single_layer[trans];
	for(int layer = 0; layer < 2; layer++) {
		uint32_t h = zfh[layer] + 1;
		uint32_t v = zfv[layer] + 1;
		zoom_raw_vert[layer]  = v;
		zoom_raw_horiz[layer] = h;
		__UNLIKELY_IF(horiz_khz < 16) {
			if(is_single_tmp) {
				v *= 2;
			} else {
				v *= 4;
			}
		} else if((clksel == 0x03) && (hst_reg == 0x029d)) {
			// From Tsugaru: 
			// VING games use this settings.  Apparently zoom-x needs to be interpreted as 4+(pageZoom&15).
			// Chase HQ        HST=029DH  ZOOM=1111H  Zoom2X=5
			// Viewpoint       HST=029DH  ZOOM=1111H  Zoom2X=5
			// Pu Li Ru La     HST=029DH  ZOOM=1111H  Zoom2X=5
			// Splatter House  HST=029DH  ZOOM=1111H  Zoom2X=5
			// Operation Wolf  N/A
			// New Zealand Story  N/A
			// Alshark Opening HST=029DH  ZOOM=0000H  Zoom2X=2
			// Freeware Collection 8 Oh!FM TOWNS Cover Picture Collection  HST=029DH  ZOOM=0000H  Zoom2X=2
			h = 2 + 3 * zfh[layer];
			v  *= 2;
		} else {
			h *= 2;
			v *= 2;
		}
		zoom_factor_vert[layer]  = v;
		zoom_factor_horiz[layer] = h;
	}
}

// Around HSW1 to VST
bool TOWNS_CRTC::calc_screen_parameters(void)
{
	uint16_t hst_reg_bak = hst_reg;
	uint16_t vst_reg_bak = vst_reg;
	hsw1 = regs[TOWNS_CRTC_REG_HSW1] & 0x00fe;
	hsw2 = regs[TOWNS_CRTC_REG_HSW2] & 0x00fe;
	hst_reg  = (regs[TOWNS_CRTC_REG_HST] & 0x07fe) + 1;
	vst1 = regs[TOWNS_CRTC_REG_VST1] & 0x1f;
	vst2 = regs[TOWNS_CRTC_REG_VST2] & 0x1f;
	eet  = regs[TOWNS_CRTC_REG_EET]  & 0x1f;
	vst_reg = (regs[TOWNS_CRTC_REG_VST] & 0x07ff) + 1;

	if((hst_reg_bak != hst_reg) || (vst_reg_bak != vst_reg)) {
		// Need to change frame rate.
		return true;
	}
	return false;
}


void TOWNS_CRTC::event_callback(int event_id, int err)
{
	const int trans = render_linebuf & display_linebuf_mask;
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
	switch(event_id) {
	case EVENT_HSYNC_OFF:
		event_hsync = -1;
		hsync = false;
		// 20240225 K.O; From crtc/crtc.h of Tsugaru:
		/*! It is my guess.  The rectangle in which the image is drawn is defined only by
		  HDSx, VDSx, HDEx, and VDEx.  But, what about HAJx?  FM TOWNS Technical Guidebook [2]
		  fell short of explaining the meaning of HAJx.

		  From my observation, probably it is what happens.

		  CRTC starts scanning VRAM after HAJx*clocks after falling edge of HSYNC.
		  But, it really starts drawing at HDSx.  If this interpretation is correct
		  VRAM bytes for
		  (HDSx-HAJx)
		  pixels in 1x scale should be skipped for each line.  The number of bytes 
		  skipped should be:
		  (HDSx-HAJx)*bytesPerPixel/zoomX
		  This function returns (HDSx-HAJx).
		*/
		if(vert_line_count < vst[trans]) {
			for(int layer = 0; layer < 2; layer++) {
				cancel_event_by_id(event_hdisp[layer]);
				//pre_transfer_line(layer, vert_line_count);
				if(frame_in[layer]) {
					double usec = horiz_start_us[layer];
					__UNLIKELY_IF(usec >= horiz_us) {
						usec = horiz_us - crtc_clock; // Push before next line.
					}
					usec = usec - horiz_width_posi_us;
					__UNLIKELY_IF(usec < crtc_clock) {
						usec = crtc_clock;
					}
					register_event(this, EVENT_HDS0 + layer, usec, false, &(event_hdisp[layer]));
				}
			}
		}
		break;
	case EVENT_HDS0:
	case EVENT_HDS1:
		event_hdisp[event_id - EVENT_HDS0] = -1;
		if(frame_in[event_id - EVENT_HDS0]){
			int layer = event_id - EVENT_HDS0;
			int __max_lines = vst[trans];
			hdisp[layer] = true;
			// Start to render on HDSx. 20240101 K.O
			// Update VSTART/LO at th end of frame.
			//update_vstart(layer);	
			__LIKELY_IF((vert_line_count < __max_lines) && (vert_line_count >= 0)) {
				pre_transfer_line(layer, vert_line_count);
				//transfer_line(layer, vert_line_count);
			}
			double end_us = horiz_end_us[layer];
			double usec;
			__UNLIKELY_IF(end_us > (horiz_us - (crtc_clock * 2))) {
				end_us = horiz_us - crtc_clock; // Push before next line.
			}
			end_us = end_us - horiz_width_posi_us;
			if(end_us > horiz_start_us[layer]) {
				end_us = end_us - horiz_start_us[layer];
			} else {
				end_us = 0.0;
			}
			__UNLIKELY_IF(end_us <= crtc_clock) {
				usec = crtc_clock;
			} else {
				usec = end_us;
			}
			register_event(this, EVENT_HDE0 + layer, usec, false, &(event_hdisp[layer]));
		}
		break;
	case EVENT_HDE0:
	case EVENT_HDE1:
		event_hdisp[event_id - EVENT_HDE0] = -1;
		{
			int __max_lines = vst[trans];
			int layer = event_id - EVENT_HDE0;
			hdisp[layer] = false;
			if(frame_in[layer]) {
				//pre_transfer_line(layer, vert_line_count);
				transfer_line(layer, vert_line_count);
			}
		}
		break;
	default:
		break;
	}
}



bool TOWNS_CRTC::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool TOWNS_CRTC::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == nullptr) return false;
	if(buffer_len == 0) return false;

	_TCHAR paramstr[2048] = {0};
	double horiz_khz_tmp = (hst_reg == 0) ? (1.0e3 / crtc_clock) : (1.0e3 / (crtc_clock * (double)hst_reg));
	static const _TCHAR *modes_list[4]   = { _T("NONE "), _T("256  "), _T("32768"), _T("16   ") };
	bool is_single_tmp;
	int mode_layer0, mode_layer1;
	make_dispmode(is_single_tmp, mode_layer0, mode_layer1);

	double horiz_ref = horiz_us;
	my_stprintf_s(paramstr, sizeof(paramstr) / sizeof(_TCHAR),
				  _T("\n")
				  _T("DISPLAY: %s / VCOUNT=%d / FRAMES PER SEC=%6g / FRAME uS=%6g\n")
				  _T("CLOCK=%6gMHz / FREQ=%4gKHz\n")
				  _T("SINGLE LAYER:%s COLORS: L0=%s L1=%s PRIORITY: %s \n")
				  _T("LINES PER FRAME=%d / PIXELS PER LINE=%d / MAX LINE=%d\n")
				  _T("\n")
				  _T("RAW VALUES: VST REG=%d VST1=%d VST2=%d / HST REG=%04d HSW1=%d HSW2=%d\n\n")
				  _T("EET   uS=%6g /")
				  _T("VST1  uS=%6g / VST2  uS=%6g\n")
				  _T("HORIZ uS=%6g / POSI  uS=%6g / NEGA uS=%6g\n")
				  _T("VERT  START uS [0]=%6g [1]=%6g / END   uS [0]=%6g [1]=%6g\n")
				  _T("HORIZ START uS [0]=%6g [1]=%6g / END   uS [0]=%6g [1]=%6g\n\n")
				  , (display_enabled) ? _T("ON ") : _T("OFF"), vert_line_count
				  , 1.0e6 / frame_us , frame_us, 1.0 / crtc_clock, horiz_khz_tmp
				  , (is_single_tmp) ? _T("YES") : _T("NO ")
				  , modes_list[mode_layer0 & 3]
				  , (is_single_tmp) ? _T("NONE ") : modes_list[mode_layer1 & 3]
				  , ((video_out_regs[FMTOWNS::VOUTREG_PRIO] & 0x01) != 0) ? _T("L1 > L0") : _T("L0 > L1")
				  , lines_per_frame, pixels_per_line, max_lines
				  , vst_reg, vst1, vst2, hst_reg, hsw1, hsw2
				  , ((double)eet_count) * horiz_ref
				  , ((double)vst1_count) * horiz_ref, ((double)vst2_count) * horiz_ref
				  , horiz_ref, horiz_width_posi_us, horiz_width_nega_us
				  , ((double)vds[0]) * horiz_ref, ((double)vds[1]) * horiz_ref
				  , ((double)vde[0]) * horiz_ref, ((double)vde[1]) * horiz_ref
				  , horiz_start_us[0], horiz_start_us[1], horiz_end_us[0], horiz_end_us[1]
		);

	_TCHAR layerstr[2][1024] = {0};
	for(int layer = 0; layer < 2; layer++) {
		my_stprintf_s(layerstr[layer], (sizeof(layerstr) / 2) / sizeof(_TCHAR) - 1,
					  _T("LAYER %d: VDS=%d VDE=%d HDS=%d HAJ=%d HDE=%d\n")
					  _T("          WIDTH =%d HSTART=%d HOFFSET=%d BITSHIFT=%d\n")
					  _T("          HEIGHT=%d VOFFSET=%d\n")
					  _T("          VSTART=%04X LINE OFFSET=%04X FRAME OFFSET=%04X\n")
					  , layer
					  , vds[layer], vde[layer], hds[layer], haj[layer], hde[layer]
					  , hwidth_val[layer], hstart_val[layer], hoffset_val[layer], hbitshift_val[layer]
					  , vheight_val[layer], voffset_val[layer]
					  , vstart_addr[layer], line_offset[layer], frame_offset[layer]
			);
	}
	_TCHAR regstr[1024] = {0};
	my_stprintf_s(regstr, sizeof(regstr) / sizeof(_TCHAR),
				  _T("STATUS REG(#30): %04X\n")
				  _T("REGS:     +0     +1     +2    +3    +4    +5    +6    +7\n")
				  _T("------------------------------------------------------\n")
				  , read_reg30()
		);

	for(int r = 0; r < 32; r += 8) {
		_TCHAR tmps[32] = {0};
		my_stprintf_s(tmps, sizeof(tmps) / sizeof(_TCHAR), "+%02d:   ", r);
		my_tcscat_s(regstr, sizeof(regstr) / sizeof(_TCHAR), tmps);
		for(int q = 0; q < 8; q++) {
			if((r + q) == 30) {
				my_stprintf_s(tmps, sizeof(tmps) / sizeof(_TCHAR), _T("%04X "), read_reg30());
			} else {
				my_stprintf_s(tmps, sizeof(tmps) / sizeof(_TCHAR), _T("%04X "), regs[r + q]);
			}
			my_tcscat_s(regstr, sizeof(regstr) / sizeof(_TCHAR), tmps);
		}
		my_tcscat_s(regstr, sizeof(regstr) / sizeof(_TCHAR), _T("\n"));
	}
	_TCHAR regstr2[1024] = {0};
	my_stprintf_s(regstr2, sizeof(regstr2) / sizeof(_TCHAR),
				  _T("SPRITE OFFSET(PAGE1): %06X\n")
				  _T("R50:    PAGESEL=%d  PLANEMASK=%01X DPALETTE CHANGED=%s\n")
				  _T("CRT:    OUT0(FMR, TOWNS) = %s, %s OUT1(FMR, TOWNS) = %s, %s\n")
				  _T("OUTREG: CTRL=%02X PRIO=%02X\n")
				  _T("CRTC CH=%d\n")
				  , sprite_offset
				  , r50_pagesel, r50_planemask, (dpalette_changed) ? _T("YES") : _T("NO ")
				  , (crtout_fmr[0]) ? _T("ON ") : _T("OFF"), (crtout_towns[0]) ? _T("ON ") : _T("OFF")
				  , (crtout_fmr[1]) ? _T("ON ") : _T("OFF"), (crtout_towns[1]) ? _T("ON ") : _T("OFF")
				  , video_out_regs[FMTOWNS::VOUTREG_CTRL], video_out_regs[FMTOWNS::VOUTREG_PRIO]
				  , crtc_ch
		);

	my_stprintf_s(buffer, buffer_len,
				  _T("%s")
				  _T("ZOOM[0] V=%3g H=%3g VCOUNT=%d / ZOOM[1] V=%3g H=%3g VCOUNT=%d\n")
				  _T("%s%s")
				  _T("VSYNC=%s / FRAME IN[0]=%s / [1]=%s\n")
				  _T("HSYNC=%s / HDISP[0]=%s / [1]=%s\n\n")
				  _T("%s")
				  _T("%s")
				  , paramstr
//				  , line_count[0], line_count[1]
				  , (double)zoom_factor_vert[0], (double)zoom_factor_horiz[0] / 2.0, zoom_count_vert[0]
				  , (double)zoom_factor_vert[1], (double)zoom_factor_horiz[1] / 2.0, zoom_count_vert[1]
				  , layerstr[0], layerstr[1]
				  ,	(vsync) ? _T("YES") : _T("NO ")
				  , (frame_in[0]) ? _T("YES") : _T("NO ")
				  , (frame_in[1]) ? _T("YES") : _T("NO ")
				  , (hsync) ? _T("YES") : _T("NO ")
				  , (hdisp[0]) ? _T("YES") : _T("NO ")
				  , (hdisp[1]) ? _T("YES") : _T("NO ")
				  , regstr2
				  , regstr
		);
	return true;
}

uint32_t TOWNS_CRTC::read_signal(int ch)
{
	uint32_t d;
	switch(ch) {
	case SIG_TOWNS_CRTC_HSYNC:
		return (hsync) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_VSYNC:
		return (vsync) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_FIELD:
		return (interlace_field) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_VDISP0:
		return (frame_in[0]) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_VDISP1:
		return (frame_in[1]) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_HDISP0:
		return (hdisp[0]) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_HDISP1:
		return (hdisp[1]) ? 0xffffffff : 0;
		break;
	case SIG_TOWNS_CRTC_MMIO_CFF82H:
		d = 0x40;
		d = d | ((r50_planemask & 0x08) << 2);
		d = d | (r50_planemask & 0x07);
		d = d | (r50_pagesel << 4);
		return d;
		break;
	case SIG_TOWNS_CRTC_MMIO_CFF86H:
		d = ((vsync) ? 0x04 : 0) | ((hsync) ? 0x80 : 0) | 0x10;
		return d;
		break;
	case SIG_TOWNS_CRTC_COMPATIBLE_MMIO:
		return (is_compatible) ? 0xffffffff : 0x00000000;
		break;
	case SIG_TOWNS_CRTC_REG_LO0:
	case SIG_TOWNS_CRTC_REG_LO1:
		return (uint32_t)(regs[((ch - SIG_TOWNS_CRTC_REG_LO0) * 4) + TOWNS_CRTC_REG_LO0]);
		break;
	case SIG_TOWNS_CRTC_R50_PAGESEL:
		return ((r50_pagesel != 0) ? 0xffffffff : 0);
		break;
	default:
		break;
	}
	return 0;
}

void TOWNS_CRTC::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_TOWNS_CRTC_MMIO_CFF82H) {
//		out_debug_log(_T("CF882H=%02X"), data & 0xff);
		r50_planemask = ((data & 0x20) >> 2) | (data & 0x07);
		r50_pagesel = ((data & 0x10) != 0) ? 1 : 0;
	} else if(ch == SIG_TOWNS_CRTC_COMPATIBLE_MMIO) {
		is_compatible = ((data & mask) != 0) ? true : false;
	} else if(ch == SIG_TOWNS_CRTC_ADD_VAL_FO1) {
		fo1_offset_value = data & 0xffff;
	}
}


#define STATE_VERSION	17

bool TOWNS_CRTC::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(machine_id);
	state_fio->StateValue(cpu_id);
	state_fio->StateValue(is_compatible);

	state_fio->StateValue(lines_per_frame);
	state_fio->StateValue(max_lines);
	state_fio->StateValue(pixels_per_line);
	state_fio->StateValue(vert_line_count);
	
	state_fio->StateValue(vsync);
	state_fio->StateValue(hsync);
	state_fio->StateArray(hdisp, sizeof(hdisp), 1);
	state_fio->StateArray(frame_in, sizeof(frame_in), 1);
	
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateArray(regs_written, sizeof(regs_written), 1);
	state_fio->StateValue(crtc_ch);
	state_fio->StateValue(interlace_field);

	state_fio->StateArray(timing_changed, sizeof(timing_changed), 1);
	state_fio->StateArray(address_changed, sizeof(address_changed), 1);
	state_fio->StateArray(mode_changed, sizeof(mode_changed), 1);

	state_fio->StateValue(display_enabled);
	state_fio->StateValue(display_enabled_pre);
	
	state_fio->StateValue(crtc_clock);
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(cpu_clocks);


	state_fio->StateArray(vstart_addr, sizeof(vstart_addr), 1);
	state_fio->StateValue(hsw1);
	state_fio->StateValue(hsw2);
	state_fio->StateValue(hst_reg);
	state_fio->StateValue(vst1);
	state_fio->StateValue(vst2);
	state_fio->StateValue(vst_reg);
	state_fio->StateValue(eet);

	state_fio->StateArray(frame_offset, sizeof(frame_offset), 1);
	state_fio->StateArray(line_offset, sizeof(line_offset), 1);

	state_fio->StateArray(frame_offset_bak, sizeof(frame_offset_bak), 1);

	state_fio->StateArray(head_address, sizeof(head_address), 1);

	state_fio->StateArray(zoom_raw_vert, sizeof(zoom_raw_vert), 1);
	state_fio->StateArray(zoom_raw_horiz, sizeof(zoom_raw_horiz), 1);
	state_fio->StateArray(zoom_factor_vert, sizeof(zoom_factor_vert), 1);
	state_fio->StateArray(zoom_factor_horiz, sizeof(zoom_factor_horiz), 1);
	state_fio->StateArray(zoom_count_vert, sizeof(zoom_count_vert), 1);
	state_fio->StateArray(line_count, sizeof(line_count), 1);

	state_fio->StateValue(fo1_offset_value);
	
	state_fio->StateValue(r50_planemask);
	state_fio->StateValue(r50_pagesel);
	state_fio->StateArray(dpalette_regs, sizeof(dpalette_regs), 1);
	state_fio->StateValue(dpalette_changed);

	state_fio->StateValue(video_brightness);

	state_fio->StateValue(voutreg_num);
	state_fio->StateArray(video_out_regs, sizeof(video_out_regs), 1);
	
	state_fio->StateValue(crtout_reg);
	state_fio->StateArray(crtout_fmr, sizeof(crtout_fmr), 1);
	state_fio->StateArray(crtout_towns, sizeof(crtout_towns), 1);
	state_fio->StateValue(apalette_code);
	for(int l = 0; l < 2; l++) {
		for(int i = 0; i < 16; i++) {
			state_fio->StateValue(apalette_16_rgb[l][i][TOWNS_CRTC_PALETTE_R]);
			state_fio->StateValue(apalette_16_rgb[l][i][TOWNS_CRTC_PALETTE_G]);
			state_fio->StateValue(apalette_16_rgb[l][i][TOWNS_CRTC_PALETTE_B]);
		}
	}
	for(int i = 0; i < 256; i++) {
		state_fio->StateValue(apalette_256_rgb[i][TOWNS_CRTC_PALETTE_R]);
		state_fio->StateValue(apalette_256_rgb[i][TOWNS_CRTC_PALETTE_G]);
		state_fio->StateValue(apalette_256_rgb[i][TOWNS_CRTC_PALETTE_B]);
	}


	state_fio->StateValue(sprite_offset);

	if(loading) {
		render_linebuf = 0;
	}
	int trans = render_linebuf.load() & display_linebuf_mask;
	bool is_interlaced_tmp[2] = {is_interlaced[trans][0], is_interlaced[trans][1]};
	bool is_single_layer_tmp = is_single_layer[trans];

	state_fio->StateValue(frame_us);
	state_fio->StateValue(is_single_layer_tmp);

	state_fio->StateArray(display_mode, sizeof(display_mode), 1);
	state_fio->StateArray(real_display_mode, sizeof(real_display_mode), 1);
	state_fio->StateArray(is_interlaced_tmp, sizeof(is_interlaced_tmp), 1);
	state_fio->StateArray(horiz_start_us, sizeof(horiz_start_us), 1);
	state_fio->StateArray(horiz_end_us, sizeof(horiz_end_us), 1);

	state_fio->StateArray(vds, sizeof(vds), 1);
	state_fio->StateArray(vde, sizeof(vde), 1);
	state_fio->StateArray(voffset_val, sizeof(voffset_val), 1);
	state_fio->StateArray(vheight_val, sizeof(vheight_val), 1);

	state_fio->StateArray(hds, sizeof(hds), 1);
	state_fio->StateArray(hde, sizeof(hde), 1);
	state_fio->StateArray(haj, sizeof(haj), 1);
	
	state_fio->StateArray(hstart_val, sizeof(hstart_val), 1);
	state_fio->StateArray(hwidth_val, sizeof(hwidth_val), 1);
	state_fio->StateArray(hbitshift_val, sizeof(hbitshift_val), 1);
	state_fio->StateArray(hoffset_val, sizeof(hoffset_val), 1);

	state_fio->StateValue(horiz_width_posi_us);
	state_fio->StateValue(horiz_width_nega_us);
	state_fio->StateValue(horiz_us);
	state_fio->StateValue(horiz_us_next);
	
	// Save frame buffer.
	for(int y = 0; y < TOWNS_CRTC_MAX_LINES; y++) {
		state_fio->StateArray(linebuffers[trans][y].mode, sizeof(uint8_t) * 4, 1);
		state_fio->StateArray(linebuffers[trans][y].is_hloop, sizeof(uint8_t) * 4, 1);
		state_fio->StateArray(linebuffers[trans][y].mag, sizeof(int8_t) * 4, 1);
		state_fio->StateArray(linebuffers[trans][y].r50_planemask, sizeof(uint8_t) * 2, 1);
		state_fio->StateArray(linebuffers[trans][y].crtout, sizeof(uint8_t) * 2, 1);
		state_fio->StateArray(linebuffers[trans][y].pixels, sizeof(int32_t) * 4, 1);
		state_fio->StateArray(linebuffers[trans][y].num,  sizeof(int32_t) * 4, 1);
		state_fio->StateArray(linebuffers[trans][y].bitoffset,  sizeof(int32_t) * 2, 1);
		state_fio->StateArray(linebuffers[trans][y].prev_y,  sizeof(int32_t) * 2, 1);
		state_fio->StateArray(&(linebuffers[trans][y].pixels_layer[0][0]), TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t), 1);
		state_fio->StateArray(&(linebuffers[trans][y].pixels_layer[1][0]), TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t), 1);
		// Palette
		for(int pl = 0; pl < 2; pl++) {
			state_fio->StateArray(&(linebuffers[trans][y].palettes[pl].raw[0][0]), sizeof(uint8_t) * 256 * 4, 1);
			state_fio->StateArray(&(linebuffers[trans][y].palettes[pl].pixels[0]), sizeof(scrntype_t) * 256, 1);
		}
	}
	
	state_fio->StateValue(event_hsync);
	state_fio->StateArray(event_hdisp, sizeof(event_hdisp), 1);

	if(loading) {
		for(int i = 0; i < 16; i++) {
			calc_apalette16(0, i);
			calc_apalette16(1, i);
		}
		for(int i = 0; i < 256; i++) {
			calc_apalette256(i);
		}
		display_linebuf = 0;
		for(int i = 0; i <= display_linebuf_mask; i++) {
			if(i != 0) {
				memcpy(&(linebuffers[i][0]), &(linebuffers[0][0]), sizeof(linebuffer_t) * TOWNS_CRTC_MAX_LINES);
			}
			
			// Duplicate registers.
			update_control_registers(i);
		}
		req_recalc = false;
		// ToDo: Save these values??
		recalc_cr0(regs[TOWNS_CRTC_REG_DISPMODE], false);
		vst1_count = vst1 << 1;
		vst2_count = vst2 << 1;
		__UNLIKELY_IF(vst1_count >= vst2_count) {
			vst2_count = vst1_count + 1;
		}
		for(int l = 0; l < 2; l++) {
			recalc_hdisp_from_crtc_params(l, horiz_start_us_next[l], horiz_end_us_next[l]);
		}
		calc_pixels_lines();
		eet_count = eet;
		make_crtout_from_fda0h(crtout_reg);
		make_crtout_from_044a(video_out_regs[FMTOWNS::VOUTREG_CTRL]);
		
		for(int i = 0; i <= display_linebuf_mask; i++) {
			hst[i] = pixels_per_line;
			vst[i] = max_lines;
			is_single_layer[i] = is_single_layer_tmp;
			for(int l = 0; l < 2; l++) {
				voffset[i][l] = voffset_val[l];
				is_interlaced[i][l] = is_interlaced_tmp[l];
			}
			update_control_registers(i);
		}

	}
	return true;
}
}
