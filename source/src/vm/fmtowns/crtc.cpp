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
	EVENT_CRTC_VSTART = 1,
	EVENT_CRTC_VST1   = 2,
	EVENT_CRTC_VST2   = 3,
	EVENT_CRTC_HSTART = 4,
	EVENT_CRTC_HEND   = 5,
	EVENT_CRTC_HSW    = 6,
	EVENT_CRTC_EET    = 7,
	EVENT_CRTC_VDS    = 32,
	EVENT_CRTC_VDE    = 34,
	EVENT_CRTC_HDS    = 36,
	EVENT_CRTC_HDE    = 38,

};


void TOWNS_CRTC::initialize()
{
	memset(regs, 0, sizeof(regs));
	memset(regs_written, 0, sizeof(regs_written));
	set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	set_lines_per_frame(512); // Its dummy.

	line_count[0] = line_count[1] = 0;
	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_changed[1][i] = true;
	}
	event_id_hsync = -1;
	event_id_hsw = -1;
	event_id_vsync = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vstart = -1;
	event_id_hstart = -1;
	for(int i = 0; i < 2; i++) {
		event_id_vds[i] = -1;
		event_id_vde[i] = -1;
		event_id_hds[i] = -1;
		event_id_hde[i] = -1;
	}
	for(int i = 0; i < 4; i++) {
		for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
			memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
		}
	}
	// register events
	//set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	//set_lines_per_frame(400);
	//set_pixels_per_line(640);

	register_frame_event(this);
	register_vline_event(this);
	voutreg_ctrl = 0x15;
	voutreg_prio = 0x00;
	is_single_layer = false;
}

void TOWNS_CRTC::release()
{
}

void TOWNS_CRTC::reset()
{
	// initialize
	display_enabled = true;
	vsync = hsync = false;
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
	is_sprite = false;
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
	render_linebuf  = 0;

	r50_planemask = 0x0f;
	r50_pagesel = 0;
	crtout_reg = 0x0f;
	make_crtout_from_fda0h(crtout_reg);

	int dummy_mode0, dummy_mode1;
	voutreg_ctrl = 0x15;
	voutreg_prio = 0x00;
	make_dispmode(is_single_layer, dummy_mode0, dummy_mode1);

	dpalette_changed = true;
	for(int i = 0; i < 8; i++) {
		dpalette_regs[i] = i;
	}
	apalette_code = 0;
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

	for(int i = 0; i < TOWNS_CRTC_MAX_LINES; i++) {
		line_changed[0][i] = true;
		line_changed[1][i] = true;
	}
	for(int i = 0; i < 2; i++) {
		timing_changed[i] = true;
		address_changed[i] = true;
		mode_changed[i] = true;

		impose_mode[i] = false; // OK?
		carry_enable[i] = false; //OK?
	}
	for(int i = 0; i < 2; i++) {
		zoom_factor_vert[i] = 1;
		zoom_factor_horiz[i] = 1;
		zoom_count_vert[i] = 1;
	}

	for(int i = 0; i < 2; i++) {
		frame_offset[i] = 0;
		line_offset[i] = 80;
	}
	for(int i = 0; i < 2; i++) {
		vstart_addr[i] = 0;
		head_address[i] = 0;
		vert_offset_tmp[i] = 0;
		horiz_offset_tmp[i] = 0;
	}
	cancel_event_by_id(event_id_hsync);
	cancel_event_by_id(event_id_hsw);
	cancel_event_by_id(event_id_vsync);
	cancel_event_by_id(event_id_vstart);
	cancel_event_by_id(event_id_vst1);
	cancel_event_by_id(event_id_vst2);
	cancel_event_by_id(event_id_hstart);

	for(int i = 0; i < 2; i++) {
		cancel_event_by_id(event_id_vds[i]);
		cancel_event_by_id(event_id_hds[i]);
		cancel_event_by_id(event_id_vde[i]);
		cancel_event_by_id(event_id_hde[i]);
	}
	// Register vstart
	pixels_per_line = 640;
	lines_per_frame = 400;
	max_lines = 400;

	write_signals(&outputs_int_vsync, 0x0);

	for(int i = 0; i < 4; i++) {
		hst[i] = 640;
		vst[i] = 400;
	}

	memset(tvram_snapshot, 0x00, sizeof(tvram_snapshot));
	//osd->set_vm_screen_size((hst <= SCREEN_WIDTH) ? hst : SCREEN_WIDTH, (vst <= SCREEN_HEIGHT) ? vst : SCREEN_HEIGHT, -1, -1,  WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
	//emu->set_vm_screen_lines(vst);
	// For DEBUG Only
	// Mode 19
	for(int layer = 0; layer < 2; layer++) {
		vert_start_us[layer] =  0.0;
		vert_end_us[layer] =    frame_us;

		horiz_start_us[layer] = 0.0;
		horiz_end_us[layer] =   frame_us / ((double)lines_per_frame);
	}
	frame_us = NAN;
	copy_regs();
	calc_screen_parameters();
	force_recalc_crtc_param();
	update_horiz_khz();
	calc_zoom_regs(regs[TOWNS_CRTC_REG_ZOOM]);
	calc_pixels_lines();
	set_lines_per_frame(lines_per_frame);
	for(int i = 0; i < 2; i++) {
		frame_offset_bak[i] = frame_offset[i];
		line_offset_bak[i] = line_offset[i];
	}
//	register_event(this, EVENT_CRTC_VSTART, vstart_us, false, &event_id_vstart);
}

void TOWNS_CRTC::cancel_event_by_id(int& event_num)
{
	if(event_num > -1) cancel_event(this, event_num);
	event_num = -1;
}

void TOWNS_CRTC::reset_vsync()
{
	write_signals(&outputs_int_vsync, 0);
	vsync = true;
}
void TOWNS_CRTC::set_vsync(bool val)
{
	write_signals(&outputs_int_vsync, (val) ? 0x00000000 : 0xffffffff);
	vsync = val;
}
void TOWNS_CRTC::restart_display()
{
	// ToDo
}

void TOWNS_CRTC::stop_display()
{
	// ToDo
}

void TOWNS_CRTC::notify_mode_changed(int layer, uint8_t mode)
{
		// ToDo
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
		switch(voutreg_prio & 0x30) {
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
	switch(voutreg_prio & 0x30) {
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
	switch(voutreg_prio & 0x30) {
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
	switch(voutreg_prio & 0x30) {
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

void TOWNS_CRTC::copy_regs()
{
	for(int layer = 0; layer < 2; layer++) {
		vds[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_VDS0] & 0x07ff;
		vde[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_VDE0] & 0x07ff;

		hds[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_HDS0] & 0x07ff;
		hde[layer] = regs[(layer * 2) + TOWNS_CRTC_REG_HDE0] & 0x07ff;
		haj[layer] = regs[(layer * 4) + TOWNS_CRTC_REG_HAJ0] & 0x07ff;
		vstart_addr[layer]  = regs[(layer * 4) + TOWNS_CRTC_REG_FA0]  & 0xffff;
		line_offset[layer]  = regs[(layer * 4) + TOWNS_CRTC_REG_LO0]  & 0xffff;

		frame_offset[layer] = regs[(layer * 4) + TOWNS_CRTC_REG_FO0]  & 0xffff;
#if 0
		vert_offset_tmp[layer] = (vds[layer] > vst2) ? (vds[layer] - vst2) : 0;
		horiz_offset_tmp[layer] = (hds[layer] > hsw1) ? (hds[layer] - hsw1) : 0;
		hstart_reg[layer] = (hds[layer] > haj[layer]) ? hds[layer] : haj[layer];
		hwidth_reg[layer] = (hde[layer] > hstart_reg[layer]) ? (hde[layer] - hstart_reg[layer]) : 0;

		vheight_reg[layer] = (vde[layer] > vds[layer]) ? (vde[layer] - vds[layer]) : 0;
#else
		// From Tsugaru
		int hstart_tmp = (int)(max(hds[layer], haj[layer]));
		int hoffset_tmp = max(0, (int)(haj[layer]) - (int)(hds[layer]));
		int hend_tmp =   (int)(hde[layer]);
		int vstart_tmp = (int)(vds[layer]);
		int vend_tmp =   (int)(vde[layer]);
		int hwidth_tmp = max(0, (int)(hde[layer]) - (int)(hds[layer]));
		int vheight_tmp = max(0, (int)(vde[layer]) - (int)(vds[layer]));
		switch(clksel) {
		case 0x00: // 28.6363MHz
			hstart_tmp = (hstart_tmp - 0x0129) >> 1;
			hoffset_tmp = hoffset_tmp >> 1;
			vstart_tmp = (vstart_tmp - 0x002a) >> 1;
			//hwidth_tmp >>= 1;
			//vheight_tmp >>= 1;
			break;
		case 0x01:
			if(hst_reg == 0x031f) {
				hstart_tmp = hstart_tmp - 0x008a;
			} else {
				hstart_tmp = (hstart_tmp - 0x00e7) >> 1;
				hoffset_tmp = hoffset_tmp >> 1;
			}
			vstart_tmp = (vstart_tmp - 0x002a) >> 1;
			//vheight_tmp >>= 1;
			break;
		case 0x02:
			hstart_tmp = hstart_tmp - 0x008a;
			vstart_tmp =  (vstart_tmp - 0x0046) >> 1;
			//vheight_tmp >>= 1;
			break;
		case 0x03:
			if(hst_reg != 0x029d) {
				hstart_tmp = hstart_tmp - 0x009c;
				vstart_tmp =  (vstart_tmp - 0x0040) >> 1;
			} else {
				hstart_tmp = hstart_tmp - 0x008a;
				vstart_tmp =  (vstart_tmp - 0x0046) >> 1;
			}
			//vheight_tmp >>= 1;
			break;
		default:
			hstart_tmp = 0;
			vstart_tmp = 0;
			break;
		}
		//hstart_reg[layer] = hstart_tmp;
		if(horiz_khz <= 16) {
			hstart_tmp = hstart_tmp >> 1;
			hoffset_tmp = hoffset_tmp >> 1;
			hwidth_tmp >>= 1;
		}
		hstart_reg[layer] = hstart_tmp;
		horiz_offset_tmp[layer] = hoffset_tmp;
		vert_offset_tmp[layer] = vstart_tmp;
		hwidth_reg[layer]  = hwidth_tmp;
		vheight_reg[layer] = vheight_tmp;
#endif
	}
}

void TOWNS_CRTC::calc_pixels_lines()
{
	int _width[2];
	int _height[2];

	for(int l = 0; l < 2; l++) {
		_width[l] = hwidth_reg[l];
		_height[l] = vheight_reg[l];
		if(frame_offset[l] == 0) {
			_height[l] /= 2;
		}
		if((clksel == 0x03) && (hst_reg == 0x029d)) {
			uint32_t magx = zoom_factor_horiz[l];
			if(magx >= 5) {
				_width[l] = (((_width[l]<< 16) * magx) / 4) >> 16;
			}
		}
	}

	if(is_single_layer) {
		// Single layer
		lines_per_frame = _height[0];
		pixels_per_line = _width[0];
		//lines_per_frame >>= 1;
	} else {
		lines_per_frame = max(_height[0], _height[1]);
		pixels_per_line = max(_width[0], _width[1]);
		//lines_per_frame >>= 1;
	}
	// ToDo: High resolution MODE.

	lines_per_frame = min(512, lines_per_frame);
//	pixels_per_line = min(640, pixels_per_line);
	__UNLIKELY_IF(pixels_per_line >= TOWNS_CRTC_MAX_PIXELS) pixels_per_line = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(lines_per_frame >= TOWNS_CRTC_MAX_LINES) lines_per_frame = TOWNS_CRTC_MAX_LINES;
}

void TOWNS_CRTC::force_recalc_crtc_param(void)
{
	horiz_width_posi_us_next = crtc_clock * ((double)hsw1); // HSW1
	horiz_width_nega_us_next = crtc_clock * ((double)hsw2); // HSW2
	horiz_us_next = crtc_clock * ((double)hst_reg); // HST
	double horiz_ref = horiz_us_next / 2.0;

	vst1_us = ((double)vst1) * horiz_ref; // VST1
	vst2_us = ((double)vst2) * horiz_ref; // VST2
	eet_us  = ((double)eet) * horiz_ref;  // EET
	double frame_us_tmp = frame_us;
	frame_us = ((double)vst_reg) * horiz_ref; // VST
	__UNLIKELY_IF(frame_us_tmp != frame_us) {
		__LIKELY_IF(frame_us > 0.0) {
			set_frames_per_sec(1.0e6 / frame_us); // Its dummy.
		} else {
			frame_us = 1.0e6 / FRAMES_PER_SEC;
			set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
		}
	}
//	out_debug_log(_T("RECALC PARAM: horiz_us=%f frame_us=%f"), horiz_us, frame_us);

	for(int layer = 0; layer < 2; layer++) {
		vert_start_us[layer] =  ((double)vds[layer]) * horiz_ref;   // VDSx
		vert_end_us[layer] =    ((double)vde[layer]) * horiz_ref; // VDEx

		horiz_start_us_next[layer] = ((double)hstart_reg[layer]) * crtc_clock;
		horiz_end_us_next[layer] =   ((double)hde[layer]) * crtc_clock ;   // HDEx
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
		voutreg_num = data & 0x01;
		break;
	case 0x044a:
		if(voutreg_num == 0) {
			voutreg_ctrl = data;
		} else if(voutreg_num == 1) {
			voutreg_prio = data;
		}
		break;
	case 0x044c:
		break;
	case 0x05ca:
		write_signals(&outputs_int_vsync, 0x00000000); // Clear VSYNC
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
		if(calc_screen_parameters()) {
			force_recalc_crtc_param();
		}
		break;
	case TOWNS_CRTC_REG_HDS0:
	case TOWNS_CRTC_REG_HDE0:
	case TOWNS_CRTC_REG_HDS1:
	case TOWNS_CRTC_REG_HDE1:
	case TOWNS_CRTC_REG_VDS0:
	case TOWNS_CRTC_REG_VDE0:
	case TOWNS_CRTC_REG_VDS1:
	case TOWNS_CRTC_REG_VDE1:
	{
		//if(reg_bak != data) {
		copy_regs();
		//}
	}
	break;
	case TOWNS_CRTC_REG_FA0:
	case TOWNS_CRTC_REG_HAJ0:
	case TOWNS_CRTC_REG_FO0:
	case TOWNS_CRTC_REG_LO0:
	case TOWNS_CRTC_REG_FA1:
	case TOWNS_CRTC_REG_HAJ1:
	case TOWNS_CRTC_REG_FO1:
	case TOWNS_CRTC_REG_LO1:
	{
		//if(reg_bak != data) {
		copy_regs();
		//}
	}
	break;
	case TOWNS_CRTC_REG_EHAJ:
		// ToDo
		break;
	case TOWNS_CRTC_REG_EVAJ:
		// ToDo
		break;
	case TOWNS_CRTC_REG_ZOOM:
		calc_zoom_regs((uint16_t)data);
		break;
	case TOWNS_CRTC_REG_DISPMODE: // CR0
		recalc_cr0(data, false);
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


void TOWNS_CRTC::write_io16(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE16 ADDR=%04x DATA=%04x"), addr, data);
	switch(addr & 0xfffe) {
	case 0x0442:
		update_crtc_reg(crtc_ch, data);
		break;
	case 0xfd90:
		set_apalette(TOWNS_CRTC_PALETTE_INDEX, data & 0xff, false);
		set_apalette(TOWNS_CRTC_PALETTE_B, (data >> 8) & 0xff, true);
		break;
	case 0xfd92:
		set_apalette(TOWNS_CRTC_PALETTE_B, data & 0xff, false);
		set_apalette(TOWNS_CRTC_PALETTE_R, (data >> 8) & 0xff, true);
		break;
	case 0xfd94:
		set_apalette(TOWNS_CRTC_PALETTE_R, data & 0xff, false);
		set_apalette(TOWNS_CRTC_PALETTE_G, (data >> 8) & 0xff, true);
		break;
	case 0xfd96:
		set_apalette(TOWNS_CRTC_PALETTE_G, data  & 0xff, true);
		break;
	default:
		write_io8(addr & 0xfffe, data);
	}
}

uint16_t TOWNS_CRTC::read_reg30()
{
	uint16_t data = 0x00f0;
	data |= ((frame_in[1])     ?  0x8000 : 0);
	data |= ((frame_in[0])     ?  0x4000 : 0);
	data |= ((hdisp[1])        ?  0x2000 : 0);
	data |= ((hdisp[0])        ?  0x1000 : 0);
	data |= ((interlace_field) ?  0x0800 : 0);
	data |= ((vsync)           ?  0x0400 : 0);
	data |= ((hsync)           ?  0x0200 : 0);
	//data |= ((video_in)     ? 0x0100 : 0);
	//data |= ((half_tone)    ? 0x0008 : 0);
	//data |= ((sync_enable)  ? 0x0004 : 0);
	//data |= ((vcard_enable) ? 0x0002 : 0);
	//data |= ((sub_carry)    ? 0x0001 : 0);
//	data = (data & 0xff00 ) | (regs[TOWNS_CRTC_REG_DUMMY] & 0x00ff);

	return data;
}

uint32_t TOWNS_CRTC::read_io16(uint32_t addr)
{
	switch(addr & 0xfffe) {
	case 0x0442:
/*		if(crtc_ch == 21) { // FO1
			return ((regs[TOWNS_CRTC_REG_FO1] & 0x7fff) + fo1_offset_value);
			} else */if(crtc_ch == TOWNS_CRTC_REG_DUMMY) {
			return (uint32_t)read_reg30();
		} else {
			return regs[crtc_ch];
		}
		break;
	default:
		return read_io8(addr);
		break;
	}
	return 0xffff;
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
		if(voutreg_num == 0) {
			return voutreg_ctrl;
		} else if(voutreg_num == 1) {
			return voutreg_prio;
		}
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

bool TOWNS_CRTC::render_32768(int trans, scrntype_t* dst, scrntype_t *mask, int y, int layer, bool is_transparent, bool do_alpha, int& rendered_pixels)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	int num = linebuffers[trans][y].num[layer];
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
				sbuf.set(i, RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], 255));
			}
		}
		__LIKELY_IF((((magx << 3) + k) <= width) && !(odd_mag)) {
			q = scaling_store(q, &sbuf, magx, 1, width_tmp1);
			r2 = scaling_store(r2, &abuf, magx, 1, width_tmp2);
			k += (magx << 3);
			__UNLIKELY_IF((width_tmp1 <= 0) || (width_tmp2 <= 0)) break;
		} else {
			int kbak = k;
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				scrntype_t dd = sbuf.at(i);
				for(int j = 0; j < magx_tmp[i]; j++) {
					*q++ = dd;
					kbak++;
					if(kbak >= width) break;
				}
			}
			__LIKELY_IF(r2 != nullptr) {
__DECL_VECTORIZED_LOOP
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
	int num = linebuffers[trans][y].num[0];
	uint8_t *p = linebuffers[trans][y].pixels_layer[0];
	int bitshift0 = linebuffers[trans][y].bitshift[0];
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

	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	//int num = linebuffers[trans][y].num[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = mask;
	__UNLIKELY_IF(pwidth <= 0) return false;

	bool odd_mag = (((magx & 1) != 0) && (magx > 2)) ? true : false;
	__DECL_ALIGNED(16) int magx_tmp[16];
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		magx_tmp[i] = (magx + (i & 1)) / 2;
	}

	const int width = ((hst[trans] * 2 + 16 * magx) > (TOWNS_CRTC_MAX_PIXELS * 2)) ? (TOWNS_CRTC_MAX_PIXELS * 2) : (hst[trans] * 2 + 16 * magx);
	rendered_pixels = pwidth * 2;

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

	__DECL_ALIGNED(16) uint8_t pbuf[8];
	__DECL_ALIGNED(16) uint8_t hlbuf[16];
	__DECL_ALIGNED(16) uint8_t mbuf[16];
	csp_vector8<scrntype_t> sbuf[2];
	csp_vector8<scrntype_t> abuf[2];

	__DECL_ALIGNED(32)  scrntype_t  palbuf[16];
	uint8_t pmask = linebuffers[trans][y].r50_planemask & 0x0f;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		mbuf[i] = pmask;
	}
	scrntype_t *pal = &(linebuffers[trans][y].palettes[layer].pixels[0]);
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		palbuf[i] = pal[i];
	}
	int k = 0;

	size_t width_tmp1 = pwidth << 1;
	size_t width_tmp2 = pwidth << 1;

	for(int x = 0; x < (pwidth >> 3); x++) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 8; i++) {
			pbuf[i] = *p++;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i += 2) {
			hlbuf[i] = pbuf[i >> 1];
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i += 2) {
			hlbuf[i + 1] = hlbuf[i];
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i += 2) {
			hlbuf[i + 1] >>= 4;
		}
	__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			hlbuf[i] &= mbuf[i];
		}
		if(do_alpha) {
			if(is_transparent) {
				int i1 = 0;
				for(size_t i0 = 0; i0 < 2; i0++) {
					for(size_t i = 0; i < 8; i++) {
						sbuf[i0].set(i, (hlbuf[i1] == 0) ? RGBA_COLOR(0, 0, 0, 0) : palbuf[hlbuf[i1]]);
						i1++;
					}
				}
			} else {
				int i1 = 0;
				for(size_t i0 = 0; i0 < 2; i0++) {
					for(size_t i = 0; i < 8; i++) {
						sbuf[i0].set(i, palbuf[hlbuf[i1]]);
						i1++;
					}
				}
			}
		} else {
			if(is_transparent) {
				int i1 = 0;
				for(int i0 = 0; i0 < 2; i0++) {
					for(int i = 0; i < 8; i++) {
						abuf[i0].set(i, (hlbuf[i1] == 0) ? RGBA_COLOR(0, 0, 0, 0x00) : RGBA_COLOR(0xff, 0xff, 0xff, 0xff));
						i1++;
					}
				}
			} else {
				for(int i0 = 0; i0 < 2; i0++) {
					abuf[i0].fill(RGBA_COLOR(0xff, 0xff, 0xff, 0xff));
				}
			}
			size_t i1 = 0;
			for(int i0 = 0; i0 < 2; i0++) {
				for(size_t i = 0; i < 8; i++) {
					sbuf[i0].set(i, palbuf[hlbuf[i1]]);
					i1++;
				}
			}
		}
		int kbak = k;
		__LIKELY_IF((((magx << 4) + k) <= width) && !(odd_mag)) {
			__LIKELY_IF((width_tmp1 >= 16)) {
				q = scaling_store(q, sbuf, magx, 2, width_tmp1);
				__LIKELY_IF(!(do_alpha) && (r2 != nullptr)) {
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
				__LIKELY_IF((r2 != NULL) && !(do_alpha)) {
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
	void TOWNS_CRTC::mix_screen(int y, int width, bool do_mix0, bool do_mix1, int bitshift0, int bitshift1, int words0, int words1)
{
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(width <= 0) return;

	scrntype_t *pp = osd->get_vm_screen_buffer(y);
	if(y == 128) {
		//out_debug_log(_T("MIX_SCREEN Y=%d WIDTH=%d DST=%08X"), y, width, pp);
	}
	__LIKELY_IF(pp != nullptr) {
		if((do_mix0) && (do_mix1)) {
			// alpha blending
			//csp_vector8<scrntype_t>pixbuf0;
			//csp_vector8<scrntype_t>pixbuf1;
			//csp_vector8<scrntype_t>maskbuf_front;
			//csp_vector8<scrntype_t>maskbuf_back;

			int of0 = max(0, bitshift0);
			int of1 = max(0, bitshift1);
			for(int xx = 0; xx < width; xx += 8) {
				csp_vector8<scrntype_t>pixbuf0((scrntype_t)0);
				csp_vector8<scrntype_t>pixbuf1((scrntype_t)0);
				csp_vector8<scrntype_t>maskbuf_front((scrntype_t)0);
				csp_vector8<scrntype_t>maskbuf_back((scrntype_t)0);

//				maskbuf_front.fill(0);
//				pixbuf0.fill(0);
//				pixbuf1.fill(0);
				scrntype_t *px1 = &(lbuffer1[xx]);
				__LIKELY_IF(words1 >= 8) {
					__LIKELY_IF(xx >= of1){
						pixbuf1.load(px1);
						words1 -= 8;
					} else if((xx + 8) > of1) {
						size_t ofset1 = of1 - xx;
						//size_t bytes1 = xx + 8 - of1;
						pixbuf1.load_offset(px1, ofset1, 8); // ToDo
						words1 -= (8 - ofset1);
						//words1 -= 8;
					}
				} else if(words1 > 0) {
					__LIKELY_IF(xx >= of1) {
						pixbuf1.load_limited(px1, words1);
						words1 = 0;
					}
				}
				scrntype_t *px0 = &(lbuffer0[xx]);
				scrntype_t *ax0 = &(abuffer0[xx]);
				__LIKELY_IF(words0 >= 8) {
					__LIKELY_IF(xx >= of0) {
						pixbuf0.load(px0);
						maskbuf_front.load(ax0);
						words0 -= 8;
					} else if((xx + 8) > of0) {
						size_t ofset0 = of0 - xx;
						size_t bytes0 = xx + 8 - of0;
						pixbuf0.load_offset(px0, ofset0); // ToDo
						maskbuf_front.load_offset(ax0, ofset0); // ToDo
						words0 -= (8 - ofset0);
						//words0 -= 8;
					}
				} else if(words0 > 0) {
					__LIKELY_IF(xx >= of0) {
						pixbuf0.load_limited(px0, words0); // ToDo
						maskbuf_front.load_limited(ax0, words0); // ToDo
						words0 = 0;
					}
				}
				maskbuf_back.negate(maskbuf_front); // Replacement of "~"
				pixbuf1 &= maskbuf_back;
				pixbuf0 &= maskbuf_front;
				pixbuf0 |= pixbuf1;

				pixbuf0.store(pp);
				pp += 8;
			}
		} else if(do_mix0) {
			__LIKELY_IF(bitshift0 <= 0) {
				int words_new = min(words0, width);
				__LIKELY_IF(words_new > 0) {
					transfer_pixels(pp, lbuffer0, words_new);
				}
				if(words_new >= 0) {
					for(int xx = words_new; xx < width; xx++) {
						pp[xx] = RGBA_COLOR(0, 0, 0, 255);
					}
				}
			} else if(bitshift0 < width) {
				int words_new = min(words0, width - bitshift0);
				int xx = 0;
				for(xx = 0; xx < bitshift0; xx++) {
					pp[xx] = RGBA_COLOR(0, 0, 0, 255);
				}
				for(int ii = 0; ii < words_new; ii++) {
					pp[xx] = lbuffer0[ii];
					xx++;
				}
				__LIKELY_IF(xx < width) {
					for(; xx < width; xx++) {
						pp[xx] = RGBA_COLOR(0, 0, 0, 255);
					}
				}
			} else {
				for(int xx = 0; xx < width; xx++) {
					pp[xx] = RGBA_COLOR(0, 0, 0, 255);
				}
			}
		} else if(do_mix1) {
			__LIKELY_IF(bitshift1 <= 0) {
				int words_new = min(words1, width);
				__LIKELY_IF(words_new > 0) {
					transfer_pixels(pp, lbuffer1, words_new);
				}
				if(words_new >= 0) {
					for(int xx = words_new; xx < width; xx++) {
						pp[xx] = RGBA_COLOR(0, 0, 0, 255);
					}
				}
			} else if(bitshift1 < width) {
				int words_new = min(words1, width - bitshift1);
				int xx = 0;
				for(xx = 0; xx < bitshift1; xx++) {
					pp[xx] = RGBA_COLOR(0, 0, 0, 255);
				}
				for(int ii = 0; ii < words_new; ii++) {
					pp[xx] = lbuffer1[ii];
					xx++;
				}
				__LIKELY_IF(xx < width) {
					for(; xx < width; xx++) {
						pp[xx] = RGBA_COLOR(0, 0, 0, 255);
					}
				}
			} else {
				for(int xx = 0; xx < width; xx++) {
					pp[xx] = RGBA_COLOR(0, 0, 0, 255);
				}
			}
		} else {
			for(int xx = 0; xx < width; xx++) {
				pp[xx] = RGBA_COLOR(0, 0, 0, 255);
			}
		}
	}
}

void TOWNS_CRTC::draw_screen()
{
	//int _l = (display_linebuf + 1) & display_linebuf_mask;
	int _l = (render_linebuf - 1) & display_linebuf_mask;
	if(_l != render_linebuf) {
		display_linebuf = _l;
	}
	//int trans = (render_linebuf - 1) & display_linebuf_mask;
	int trans = display_linebuf & display_linebuf_mask;

	bool do_alpha = false; // ToDo: Hardware alpha rendaring.
	__UNLIKELY_IF(d_vram == nullptr) {
		return;
	}
	int lines = vst[trans];
	int width = hst[trans];
	// Will remove.
	__UNLIKELY_IF(lines <= 0) lines = 1;
	__UNLIKELY_IF(width <= 16) width = 16;

	__UNLIKELY_IF(lines > TOWNS_CRTC_MAX_LINES) lines = TOWNS_CRTC_MAX_LINES;
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	osd->set_vm_screen_size(width, lines, SCREEN_WIDTH, SCREEN_HEIGHT,  -1, -1);
	osd->set_vm_screen_lines(lines);
//	if((lines != vst[trans2]) || (width != hst[trans])) {
//		return; // Wait (a frame) if surface attributes are changed
//	}


	for(int y = 0; y < lines; y++) {
		bool do_render[2] = { false };
		int prio[2];
		// Clear buffers per every lines.
		memset(lbuffer1, 0x00, sizeof(lbuffer1));
		memset(abuffer1, 0xff, sizeof(abuffer1));
		memset(lbuffer0, 0x00, sizeof(lbuffer0));
		memset(abuffer0, 0xff, sizeof(abuffer0));
		for(int i = 0; i < 2; i++) {
			prio[i] = linebuffers[trans][y].num[i];
			if(prio[i] >= 0) {
				prio[i] &= 1;
				do_render[i] = (linebuffers[trans][y].crtout[prio[i]] != 0) ? true : false;
			}
		}
		bool do_mix[2] = { false };
		int rendered_words[2] = {0};
		for(int l = 0; l < 2; l++) {
			if(do_render[l]) {
				bool is_transparent = ((do_render[0]) && (do_render[1]) && (l == 0)) ? true : false;
				scrntype_t* pix = (l == 0) ? lbuffer0 : lbuffer1;
				scrntype_t* alpha = (l == 0) ? abuffer0 : abuffer1;

				switch(linebuffers[trans][y].mode[prio[l]]) {
				case DISPMODE_256:
					if(l == 0) {
						do_mix[l] = render_256(trans, pix, y, rendered_words[l]);
					}
					break;
				case DISPMODE_32768:
					do_mix[l] = render_32768(trans, pix, alpha, y, prio[l], is_transparent, do_alpha, rendered_words[l]);
					break;
				case DISPMODE_16:
					do_mix[l] = render_16(trans, pix, alpha, y, prio[l], is_transparent, do_alpha, rendered_words[l]);
					break;
				default:
					break;
				}
			}
		}
//		if(y == 128) {
//			out_debug_log(_T("MIX: %d %d width=%d"), do_mix0, do_mix1, width);
//		}
		int bitshift[2];
		for(int l = 0; l < 2; l++) {
			bitshift[l] = linebuffers[trans][y].bitshift[prio[l]];
		}
		mix_screen(y, width, do_mix[0], do_mix[1], bitshift[0], bitshift[1], rendered_words[0], rendered_words[1]);
	}
	return;
}

// From MAME 0.216
// ToDo: Will refine.
uint32_t TOWNS_CRTC::get_font_address(uint32_t c, uint8_t &attr)
{
	static const uint32_t addr_base_jis = 0x00000;
	static const uint32_t addr_base_ank = 0x3d800;
	uint32_t romaddr = 0;
	attr = tvram_snapshot[c + 1];
	switch(attr & 0xc0) {
	case 0x00:
		{
			uint8_t ank = tvram_snapshot[c];
			romaddr = addr_base_ank + (ank * 16);
		}
		break;
	case 0x40:
		{ // KANJI LEFT
			pair32_t jis;
			jis.b.h = tvram_snapshot[c + 0x2000]; // CA000-CAFFF
			jis.b.l = tvram_snapshot[c + 0x2001]; // CA000-CAFFF
			if(jis.b.h < 0x30) {
				romaddr =
					(((uint32_t)(jis.b.l & 0x1f)) << 4) |
					((uint32_t)((jis.b.l - 0x20) & 0x20) << 8) |
					((uint32_t)((jis.b.l - 0x20) & 0x40) << 6) |
					(((uint32_t)(jis.b.h & 0x07)) << 9);
				romaddr <<= 1;
			} else if(jis.b.h < 0x70) {
				romaddr =
					(((uint32_t)(jis.b.l & 0x1f)) << 5) +
					((uint32_t)((jis.b.l - 0x20) & 0x60) << 9) +
					((uint32_t)((jis.b.h & 0x0f)) << 10) +
					((uint32_t)((jis.b.h - 0x30) & 0x70) * 0xc00) +
					0x8000;
			} else {
				romaddr =
					(((uint32_t)(jis.b.l & 0x1f)) << 4) |
					((uint32_t)((jis.b.l - 0x20) & 0x20) << 8) |
					((uint32_t)((jis.b.l - 0x20) & 0x40) << 6) |
					(((uint32_t)(jis.b.h & 0x07)) << 9);
				romaddr <<= 1;
				romaddr |= 0x38000;
			}
			romaddr = addr_base_jis + romaddr;
		}
		break;
	default: // KANJI RIGHT or ILLEGAL
		return 0;
	}
	return romaddr;
}

void TOWNS_CRTC::render_text()
{
	int c = 0;
	uint32_t plane_offset = 0x40000 + ((r50_pagesel != 0) ? 0x20000 : 0x00000);
	for(int y = 0; y < 25; y++) {
		uint32_t linesize = regs[TOWNS_CRTC_REG_LO1] * 4;
		uint32_t addr_of = y * (linesize * 16);
		if(c >= 0x1000) break;
		uint32_t romaddr = 0;
		for(int x = 0; x < 80; x++) {
			uint8_t attr;
			uint32_t t = get_font_address(c, attr);
			if(((attr & 0xc0) == 0) || ((attr & 0xc0) == 0x40)) {
				// ANK OR KANJI LEFT
				romaddr = t;
			} else if((attr & 0xc0) == 0x80) {
				// KANJI RIGHT
				romaddr = romaddr + 1;
			} else {
				// Illegal
				addr_of = (addr_of + 4) & 0x3ffff;
				c += 2;
				continue;
			}
			// Get data
			uint32_t color = attr & 0x07;
			uint8_t tmpdata = 0;
			if(attr & 0x20) color |= 0x08;

			// Do render
//			out_debug_log("ROMADDR=%08X", romaddr);
			uint32_t of = addr_of;
			for(int column = 0; column < 16; column++) {
				__LIKELY_IF(d_font != nullptr) {
					if((attr & 0xc0) == 0) {
						// ANK
						tmpdata = d_font->read_direct_data8(column + romaddr);
					} else {
						tmpdata = d_font->read_direct_data8(column * 2 + romaddr);
					}
				}
				if(attr & 0x08)
				{
					tmpdata = ~tmpdata;
				}
				uint32_t pix = 0;
				uint8_t *p = d_vram->get_vram_address(of + plane_offset);
				__LIKELY_IF(p != nullptr) {

					d_vram->lock();
__DECL_VECTORIZED_LOOP
					for(int nb = 0; nb < 8; nb += 2) {
//						pix = 0;
						pix = ((tmpdata & 0x80) != 0) ? color : 0;
						pix = pix | (((tmpdata & 0x40) != 0) ? (color << 4) : 0);
						tmpdata <<= 2;
						*p++ = pix;
					}
					d_vram->unlock();
				}
				of = (of + linesize) & 0x3ffff;
			}
//		_leave0:
			addr_of = (addr_of + 4) & 0x3ffff;
			c += 2;
		}
	}
}

void TOWNS_CRTC::pre_transfer_line(int line)
{
	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;

	uint8_t prio;
	int trans = render_linebuf & display_linebuf_mask;

	prio = voutreg_prio;
	__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		linebuffers[trans][line].mode[i] = DISPMODE_NONE;
		linebuffers[trans][line].pixels[i] = 0;
		linebuffers[trans][line].mag[i] = 0;
		linebuffers[trans][line].num[i] = -1;
	}
	linebuffers[trans][line].r50_planemask = r50_planemask;

	int disp_prio[2] = {-1};
	if((prio & 0x01) == 0) {
		disp_prio[0] = 0; // Front
		disp_prio[1] = 1; // Back
	} else {
		disp_prio[0] = 1;
		disp_prio[1] = 0;
	}
//	out_debug_log("LINE %d CTRL=%02X \n", line, ctrl);
	bool to_disp[2] = { false, false};
	int mode_layer0 = DISPMODE_NONE;
	int mode_layer1 = DISPMODE_NONE;

	make_dispmode(is_single_layer, mode_layer0, mode_layer1);
	to_disp[0] = (mode_layer0 != DISPMODE_NONE) ? true : false;
	to_disp[1] = (mode_layer1 != DISPMODE_NONE) ? true : false;
	if(is_single_layer) {
		// One layer mode
		bool disp = frame_in[0];
		to_disp[1] = false;
//		__UNLIKELY_IF((horiz_end_us[0] <= 0.0) || (horiz_end_us[0] <= horiz_start_us[0])) {
//			disp = false;
//		}
//			if(vert_offset_tmp[0] > line) {
//				disp = false;
//			}
		mode_layer1 = DISPMODE_NONE;
		__UNLIKELY_IF(!(disp)) {
			mode_layer0 = DISPMODE_NONE;
		}
		linebuffers[trans][line].num[0] = 0;
		linebuffers[trans][line].crtout[0] = ((crtout[0]) && (to_disp[0]) && (disp)) ? 0xff : 0x00;
		linebuffers[trans][line].mode[0] = mode_layer0;

		linebuffers[trans][line].crtout[1] = 0x00;
		linebuffers[trans][line].mode[1] = DISPMODE_NONE;
		linebuffers[trans][line].num[1] = -1;
	} else {
		int modes_tmp[2] = {mode_layer0, mode_layer1};
		for(int l = 0; l < 2; l++) {
			linebuffers[trans][line].num[l] = disp_prio[l];
			linebuffers[trans][line].mode[l] = modes_tmp[l];
		}
		for(int l = 0; l < 2; l++) {
			//int realpage = disp_prio[l];
			int realpage = l;
			bool disp = ((frame_in[realpage]) && (to_disp[realpage]));
			if(disp_prio[l] >= 0) {
				disp = ((disp) && (crtout[realpage]));
			} else {
				disp = false;
			}
//			__UNLIKELY_IF((horiz_end_us[realpage] <= 0.0) || (horiz_end_us[realpage] <= horiz_start_us[realpage])) {
//				disp = false;
//			}
			linebuffers[trans][line].crtout[realpage] =
				(disp) ? 0xff : 0x00;
		}
	}
	for(int l = 0; l < 2; l++) {
		switch(linebuffers[trans][line].mode[l]) {
		case DISPMODE_256:
			memcpy(&(linebuffers[trans][line].palettes[l].raw[0][0]), &(apalette_256_rgb[0][0]), sizeof(uint8_t) * 256 * 4); // Copy RAW
			memcpy(&(linebuffers[trans][line].palettes[l].pixels[0]), &(apalette_256_pixel[0]), sizeof(scrntype_t) * 256); // Copy Pixels
			break;
		case DISPMODE_16:
			memcpy(&(linebuffers[trans][line].palettes[l].raw[0][0]), &(apalette_16_rgb[l][0][0]), sizeof(uint8_t) * 16 * 4); // Copy RAW
			memcpy(&(linebuffers[trans][line].palettes[l].pixels[0]), &(apalette_16_pixel[l][0]), sizeof(scrntype_t) * 16); // Copy Pixels
			break;
		default:
			memset(&(linebuffers[trans][line].palettes[l].raw[0][0]), 0x00, sizeof(uint8_t) * 256 * 4);
			memset(&(linebuffers[trans][line].palettes[l].pixels[0]), 0x00, sizeof(scrntype_t) * 256);
			break;
		}
	}
	// Fill by skelton colors;
	for(int l = 0; l < 2; l++) {
		uint32_t *p = (uint32_t*)(&(linebuffers[trans][line].pixels_layer[l][0]));
		__DECL_ALIGNED(16) pair32_t pix[8];
		if(!(to_disp[l])) {
			if(linebuffers[trans][line].mode[l] == DISPMODE_32768) {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					pix[i].w.l = 0x8000;
					pix[i].w.h = 0x8000;
				}
			} else {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					pix[i].d = 0x00000000;
				}
			}
			for(int x = 0; x < (TOWNS_CRTC_MAX_PIXELS >> 1); x += 8) {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					p[i] = pix[i].d; // Clear color
				}
				p += 8;
			}
		}
	}

}

void TOWNS_CRTC::transfer_line(int line, int layer)
{
	int l = layer;	bool to_disp = true; // Dummy
	static const uint32_t address_add[2] =  {0x00000000, 0x00040000};
	uint8_t page_16mode = r50_pagesel;
	uint8_t ctrl, prio;
	int trans = render_linebuf & display_linebuf_mask;
	uint32_t address_shift = 0;
	uint32_t address_mask = 0x0003ffff;

	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;
	__UNLIKELY_IF(d_vram == nullptr) return;
	__UNLIKELY_IF(layer < 0) return;
	__UNLIKELY_IF(layer > 1) return;
	__UNLIKELY_IF(!(frame_in[layer])) return;
	__UNLIKELY_IF(linebuffers[trans] == nullptr) return;
/*	__UNLIKELY_IF((horiz_end_us[layer] <= 0.0) || (horiz_end_us[layer] < horiz_start_us[layer])) {
		return;
	}
*/

	/*
	if(linebuffers[trans][line].crtout[0] == 0) {
		linebuffers[trans][line].mode[page0] = DISPMODE_NONE;
		to_disp[0] = false;
	}
	if(linebuffers[trans][line].crtout[1] == 0) {
		linebuffers[trans][line].mode[page1] = DISPMODE_NONE;
		to_disp[1] = false;
		}*/

	// Update some parameters per line. 20230731 K.O
	hds[l] = regs[(l * 2) + TOWNS_CRTC_REG_HDS0] & 0x07ff;
	hde[l] = regs[(l * 2) + TOWNS_CRTC_REG_HDE0] & 0x07ff;
	haj[l] = regs[(l * 4) + TOWNS_CRTC_REG_HAJ0] & 0x07ff;
	vstart_addr[l]  = regs[(l * 4) + TOWNS_CRTC_REG_FA0]  & 0xffff;
	line_offset[l]  = regs[(l * 4) + TOWNS_CRTC_REG_LO0]  & 0xffff;

	int bit_shift = max(0, (int)hds[l] - (int)haj[l]);
	uint32_t magx = zoom_factor_horiz[l];
	// FAx
	// Note: Re-Wrote related by Tsugaru. 20230806 K.O
	uint32_t offset = vstart_addr[l]; // ToDo: Larger VRAM
	uint32_t lo = line_offset[l] * ((is_single_layer) ? 8 : 4);
	uint32_t hscroll_mask = ((lo == 512) || (lo == 1024)) ? (lo - 1) : 0xffffffff;
	uint32_t hskip_pixels;
	if((is_single_layer) && (layer == 0)) {
		address_mask = 0x0007ffff;
	} else {
		address_mask = 0x0003ffff;
	}
	uint32_t skip_zoom = ((regs[TOWNS_CRTC_REG_ZOOM] >> ((l == 0) ? 0 : 8)) & 0x0f) + 1;
	hskip_pixels = (bit_shift << 16) / skip_zoom;
	uint32_t hskip_bytes = 0;
	//hskip_pixels = (bit_shift * 2) / magx;
	switch(linebuffers[trans][line].mode[l]) {
	case DISPMODE_32768:
		if(is_single_layer) {
			address_shift = 3; // FM-Towns Manual P.145
			hskip_bytes = hskip_pixels >> (16 - (1 + 1)); // 4 bytes per pixel ?
		} else {
			address_shift = 2; // FM-Towns Manual P.145
			hskip_bytes = hskip_pixels >> (16 - 1); // 2 bytes per pixel ?
		}
		break;
	case DISPMODE_16:
		address_shift = 2; // FM-Towns Manual P.145
		hskip_bytes = hskip_pixels >> (16 + 1); // 0.5 bytes per pixel ?
		break;
	case DISPMODE_256:
		address_shift = 3; // FM-Towns Manual P.145
		hskip_bytes = hskip_pixels >> 16; // 1 bytes per pixel ?
		break;
	default:
		break;
	}
	uint32_t shift_mask = (1 << address_shift) - 1;
	offset <<= address_shift;
//	if(horiz_khz > 15) {
//		// Maybe LOW RESOLUTION, Will fix.20201115 K.O
//		offset >>= 1;
//	}

	//! Note:
	//! - Below is from Tsugaru, commit 1a442831 .
	//! - I wonder sprite offset effects every display mode at page1,
	//!   and FMR's address offset register effects every display mode at page0
	//! - -- 20230715 K.O
	uint32_t page_offset;
	if(l == 0) {
		page_offset = ((page_16mode != 0) ? 0x20000 : 0);
	} else {
		page_offset = sprite_offset;
	}
//			out_debug_log(_T("LINE=%d BEGIN=%d END=%d"), line, _begin, _end);
	uint32_t pixels = hwidth_reg[l] * ((is_single_layer) ? 2 : 1);
	if((clksel == 0x03) && (hst_reg == 0x029d) && (magx >= 5)) {
		pixels = (((pixels << 16) * magx) / 4) >> 16;
	}

	__LIKELY_IF((pixels > 0) && (linebuffers[trans][line].crtout[l] != 0)){
		__LIKELY_IF(/*(pixels >= magx) && */(magx != 0)){
			//__UNLIKELY_IF(pixels >= TOWNS_CRTC_MAX_PIXELS) pixels = TOWNS_CRTC_MAX_PIXELS;
			uint32_t pixels_bak = pixels;
			linebuffers[trans][line].bitshift[l] =  max(0, (int)(horiz_offset_tmp[l]));
			linebuffers[trans][line].pixels[l] = pixels;
			linebuffers[trans][line].mag[l] = magx; // ToDo: Real magnif
			pixels = ((pixels_bak << 16) / magx) >> 15;
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
			if(to_disp) {
				offset = (offset + hskip_bytes) & address_mask;
				if((trans & 1) != 0) offset += (frame_offset_bak[l] << address_shift);

				if(l == 1) {
					offset += (fo1_offset_value << address_shift);
				}
				offset += page_offset;
				offset += head_address[l];
				offset &= address_mask;
				offset += address_add[l];

				// ToDo: Will Fix
				offset = offset & 0x0007ffff;
				d_vram->get_data_from_vram(((is_single_layer) || (is_256)), offset, tr_bytes, &(linebuffers[trans][line].pixels_layer[l][0]));
			}
		}
	} else {
		to_disp = false;
	}
	if(zoom_count_vert[l] > 0) {
		zoom_count_vert[l]--;
	}
	if(zoom_count_vert[l] <= 0) {
		zoom_count_vert[l] = zoom_factor_vert[l] / 2;
		// ToDo: Interlace
		if(to_disp) {
			head_address[l] += lo;
			//head_address[l] += line_offset[l];
		}
	}
}


void TOWNS_CRTC::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	max_lines = new_lines_per_frame;
	frames_per_sec = new_frames_per_sec;
	cpu_clocks = new_clocks;
	req_recalc = false;
}

void TOWNS_CRTC::event_pre_frame()
{
	interlace_field = !interlace_field;
	int dummy_mode0, dummy_mode1;
	make_dispmode(is_single_layer, dummy_mode0, dummy_mode1);

	for(int i = 0; i < 2; i++) {
		hdisp[i] = false;
	}

	copy_regs();
	calc_screen_parameters();
	force_recalc_crtc_param();
	update_horiz_khz();
	calc_zoom_regs(regs[TOWNS_CRTC_REG_ZOOM]);
	calc_pixels_lines();
	set_lines_per_frame(lines_per_frame);
	// Render VRAM
	if(d_sprite->read_signal(SIG_TOWNS_SPRITE_TVRAM_ENABLED) != 0) {
		d_sprite->get_tvram_snapshot(tvram_snapshot);
		render_text();
	}
}

void TOWNS_CRTC::calc_zoom_regs(uint16_t val)
{
	uint8_t zfv[2];
	uint8_t zfh[2];
	pair16_t pd;
	regs[TOWNS_CRTC_REG_ZOOM] = val;
	pd.w = val;
	zfv[0] = ((pd.b.l & 0xf0) >> 4) + 1;
	zfh[0] = (pd.b.l & 0x0f) + 1;
	zfv[1] = ((pd.b.h & 0xf0) >> 4) + 1;
	zfh[1] = (pd.b.h & 0x0f) + 1;

	for(int layer = 0; layer < 2; layer++) {
		uint32_t h = zfh[layer];
		uint32_t v = zfv[layer];
		if(horiz_khz <= 16) {
			if(!(is_single_layer)) {
				v *= 2;
			} else {
				v *= 4;
				//v *= 2;
			}
		} else if((clksel == 0x03) && (hst_reg == 0x029d)) {
			h = 2 + 3 * (h - 1);
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
	vst_reg = regs[TOWNS_CRTC_REG_VST] & 0x07ff;

	if((hst_reg_bak != hst_reg) || (vst_reg_bak != vst_reg)) {
		// Need to change frame rate.
		return true;
	}
	return false;
}

void TOWNS_CRTC::event_frame()
{
	// This calls after setting new frames parameters.
	horiz_width_posi_us = horiz_width_posi_us_next;
	horiz_width_nega_us = horiz_width_nega_us_next;
	horiz_us = horiz_us_next;
	for(int layer = 0; layer < 2; layer++) {
		horiz_start_us[layer] = horiz_start_us_next[layer];
		horiz_end_us[layer] =   horiz_end_us_next[layer];
	}
	//display_linebuf = render_linebuf & display_linebuf_mask; // Incremant per vstart
	render_linebuf = (render_linebuf + 1) & display_linebuf_mask; // Incremant per vstart

	for(int i = 0; i < 2; i++) {
		zoom_count_vert[i] = zoom_factor_vert[i] / 2;
		frame_offset_bak[i] = frame_offset[i];
		line_offset_bak[i] = line_offset[i];
	}
	int trans = render_linebuf;
	// Clear all frame buffer (of this) every turn.20230716 K.O
	csp_vector8<uint16_t> dat((const uint16_t)0x0000);
	for(int yy = 0; yy < TOWNS_CRTC_MAX_LINES; yy++) {
		for(int i = 0; i < 2; i++) {
			// Maybe initialize.
			linebuffers[trans][yy].pixels[i] = TOWNS_CRTC_MAX_PIXELS;
			linebuffers[trans][yy].mag[i] = 1;
			linebuffers[trans][yy].num[i] = -1;
			linebuffers[trans][yy].mode[i] = DISPMODE_NONE;
			linebuffers[trans][yy].crtout[i] = 0;
			linebuffers[trans][yy].bitshift[i] = 0;
//			uint16_t* p = (uint16_t*)(&(linebuffers[trans][yy].pixels_layer[i][0]));
//			for(int xx = 0; xx < (TOWNS_CRTC_MAX_PIXELS / 8); xx++) {
//				dat.store_aligned(p);
//				p += 8;
//			}
		}
	}

	hst[render_linebuf] = pixels_per_line;
	vst[render_linebuf] = max_lines;
	//lines_per_frame = max_lines;

	line_count[0] = line_count[1] = 0;
	vert_line_count = -1;
	hsync = false;

	//! @note Every event_pre_frame() at SPRITE, clear VRAM if enabled.
	is_sprite = (d_sprite->read_signal(SIG_TOWNS_SPRITE_ENABLED) != 0) ? true : false;
	//! Note:
	//! - Below is from Tsugaru, commit 1a442831 .
	//! - I wonder sprite offset effects every display mode at page1.
	//! - -- 20230715 K.O
	sprite_offset = (d_sprite->read_signal(SIG_TOWNS_SPRITE_DISP_PAGE1) != 0) ? 0x20000 : 0x00000;
	// ToDo: EET
	//register_event(this, EVENT_CRTC_VSTART, frame_us, false, &event_id_frame); // EVENT_VSTART MOVED TO event_frame().
	cancel_event_by_id(event_id_vst1);
	cancel_event_by_id(event_id_vst2);

	// Reset VSYNC
	reset_vsync();
	if(vst1_us > 0.0) {
		register_event(this, EVENT_CRTC_VST1, vst1_us, false, &event_id_vst1);
	} else {
		event_callback(EVENT_CRTC_VST1, 1);
	}
	for(int i = 0; i < 2; i++) {
		frame_in[i] = false;
		cancel_event_by_id(event_id_vds[i]);
		cancel_event_by_id(event_id_vde[i]);
		if(vert_start_us[i] > 0.0) {
			register_event(this, EVENT_CRTC_VDS + i, vert_start_us[i], false, &event_id_vds[i]); // VDSx
		}/* else if(vert_start_us[i] == 0.0) {
			event_callback(EVENT_CRTC_VDS + i, 1);
		}*/
		if(vert_end_us[i] > 0.0) {
			register_event(this, EVENT_CRTC_VDE + i, vert_end_us[i],   false, &event_id_vde[i]); // VDEx
		}
		head_address[i] = 0;
	}

	cancel_event_by_id(event_id_hstart);
	cancel_event_by_id(event_id_hsw);
	if(horiz_us > 0.0) {
		register_event(this, EVENT_CRTC_HSTART, horiz_us, true, &event_id_hstart); // HSTART
	}
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
	// EVENT_VSTART MOVED TO event_frame().
	if(event_id == EVENT_CRTC_VST1) { // VSYNC
		set_vsync(true);
		if((vst2_us > vst1_us) && (vst2_us >= 0.0)) {
			register_event(this, EVENT_CRTC_VST2, vst2_us - vst1_us, false, &event_id_vst2);
		} else {
			set_vsync(false);
		}
		event_id_vst1 = -1;
	} else if (event_id == EVENT_CRTC_VST2) {
		set_vsync(false);
		event_id_vst2 = -1;
	} else if(eid2 == EVENT_CRTC_VDS) { // Display start
		int layer = event_id & 1;
//		if(vsync) {
			frame_in[layer] = true;
//		}
		// DO ofset line?
		event_id_vds[layer] = -1;
		zoom_count_vert[layer] = zoom_factor_vert[layer] / 2;
	} else if(eid2 == EVENT_CRTC_VDE) { // Display end
		int layer = event_id & 1;
		frame_in[layer] = false;
		event_id_vde[layer] = -1;

		// DO ofset line?
	} else if(event_id == EVENT_CRTC_HSTART) {
		vline_hook();
	} else if(event_id == EVENT_CRTC_HSW) {
		hsync = false;
		event_id_hsw = -1;
	} else if(eid2 == EVENT_CRTC_HDS) {
		int layer = event_id & 1;
		hdisp[layer] = true;
/*
		if(frame_in[layer]) {
			if(vert_line_count < max_lines) {
				__LIKELY_IF(pixels_per_line > 0) {
					transfer_line(vert_line_count, layer); // Tranfer before line.
				}
			}
		}
*/
		if((horiz_end_us[layer] <= horiz_start_us[layer]) || (horiz_end_us[layer] <= 0.0)){
			cancel_event_by_id(event_id_hde[layer]);
			event_callback(EVENT_CRTC_HDE + layer, 1);
		}
		event_id_hds[layer] = -1;
	} else if(eid2 == EVENT_CRTC_HDE) {
		int layer = event_id & 1;
		hdisp[layer] = false;
		if(frame_in[layer]) {
			if(vert_line_count < max_lines) {
				__LIKELY_IF(pixels_per_line > 0) {
					transfer_line(vert_line_count, layer); // Tranfer before line.
				}
			}
		}

//		if(!(hdisp[0]) && !(hdisp[1])) hsync = false;
		event_id_hde[layer] = -1;
		cancel_event_by_id(event_id_hds[layer]);
	}

}

void TOWNS_CRTC::vline_hook()
{
	hsync = true;
	if(frame_in[0] || frame_in[1]) {
		register_event(this, EVENT_CRTC_HSW, horiz_width_posi_us, false, &event_id_hsw);
		vert_line_count++;
		if(vert_line_count < max_lines) {
			pre_transfer_line(vert_line_count);
			__LIKELY_IF(d_sprite != nullptr) {
				d_sprite->write_signal(SIG_TOWNS_SPRITE_HOOK_VLINE, vert_line_count, 0xffffffff);
			}
		}
	} else {
		register_event(this, EVENT_CRTC_HSW, horiz_width_nega_us, false, &event_id_hsw);
	}
	for(int layer = 0; layer < 2; layer++) {
		hdisp[layer] = false;
		cancel_event_by_id(event_id_hds[layer]);
		cancel_event_by_id(event_id_hde[layer]);
		if(frame_in[layer]) {
			double hds_us = horiz_start_us[layer];
			double hde_us = horiz_end_us[layer];
			if(hds_us > 0.0) {
				register_event(this, EVENT_CRTC_HDS + layer, hds_us, false, &event_id_hds[layer]); // HDSx
			} else {
				event_callback(EVENT_CRTC_HDS + layer, 1);
			}
			if(hde_us > 0.0) {
				register_event(this, EVENT_CRTC_HDE + layer, hde_us, false, &event_id_hde[layer]); // HDEx
			}
		}
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
	static const _TCHAR *single_modes_list[4] = { _T("NONE "), _T("NONE "), _T("32768"), _T("256  ") };
	static const _TCHAR *twin_modes_list[4]   = { _T("NONE "), _T("32768"), _T("NONE "), _T("16   ") };

	my_stprintf_s(paramstr, sizeof(paramstr) / sizeof(_TCHAR),
				  _T("\n")
				  _T("DISPLAY: %s / VCOUNT=%d / FRAMES PER SEC=%6g / FRAME uS=%6g \n")
				  _T("CLOCK=%6gMHz / FREQ=%4gKHz\n")
				  _T("SINGLE LAYER:%s COLORS: L0=%s L1=%s PRIORITY: %s \n")
				  _T("LINES PER FRAME=%d / PIXELS PER LINE=%d / MAX LINE=%d\n")
				  _T("EET   uS=%6g /")
				  _T("VST1  uS=%6g / VST2  uS=%6g\n")
				  _T("HORIZ uS=%6g / POSI  uS=%6g / NEGA  uS=%6g\n")
				  _T("VERT  START uS [0]=%6g [1]=%6g / END   uS [0]=%6g [1]=%6g\n")
				  _T("HORIZ START uS [0]=%6g [1]=%6g / END   uS [0]=%6g [1]=%6g\n\n")
				  , (display_enabled) ? _T("ON ") : _T("OFF"), vert_line_count
				  , frames_per_sec, frame_us, 1.0 / crtc_clock, horiz_khz_tmp
				  , (is_single_layer) ? _T("YES") : _T("NO ")
				  , (is_single_layer) ? (((voutreg_ctrl & 0x08) != 0) ? single_modes_list[display_mode[0] & 3] : _T("NONE ")) : twin_modes_list[display_mode[0] & 3]
				  , (is_single_layer) ? _T("NONE ") : twin_modes_list[display_mode[1] & 3]
				  , ((voutreg_prio & 0x01) != 0) ? _T("L1 > L0") : _T("L0 > L1")
				  , lines_per_frame, pixels_per_line, max_lines
				  , eet_us
				  , vst1_us, vst2_us
				  , horiz_us, horiz_width_posi_us, horiz_width_nega_us
				  , vert_start_us[0], vert_start_us[1], vert_end_us[0], vert_end_us[1]
				  , horiz_start_us[0], horiz_start_us[1], horiz_end_us[0], horiz_end_us[1]
		);

	_TCHAR layerstr[2][1024] = {0};
	for(int layer = 0; layer < 2; layer++) {
		my_stprintf_s(layerstr[layer], (sizeof(layerstr) / 2) / sizeof(_TCHAR) - 1,
					  _T("LAYER %d: VDS=%d VDE=%d HDS=%d HAJ=%d HDE=%d\n")
					  _T("          WIDTH=%d HOFFSET=%d HSTART=%d HEIGHT=%d VOFFSET=%d\n")
					  _T("          VSTART=%04X LINE OFFSET=%04X FRAME OFFSET=%04X\n")
					  , layer
					  , vds[layer], vde[layer], hds[layer], haj[layer], hde[layer]
					  , hwidth_reg[layer], horiz_offset_tmp[layer],  hstart_reg[layer]
					  , vheight_reg[layer], vert_offset_tmp[layer]
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
				  _T("R50:    PAGESEL=%d  PLANEMASK=%01X DPALETTE CHANGED=%s\n")
				  _T("CRT:    OUT0=%s OUT1=%s ")
				  _T("OUTREG: CTRL=%02X PRIO=%02X\n")
				  _T("CRTC CH=%d\n")
				  , r50_pagesel, r50_planemask, (dpalette_changed) ? _T("YES") : _T("NO ")
				  , (crtout[0]) ? _T("ON ") : _T("OFF"), (crtout[1]) ? _T("ON ") : _T("OFF")
				  , voutreg_ctrl, voutreg_prio
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
				  , (double)zoom_factor_vert[0] / 2.0, (double)zoom_factor_horiz[0] / 2.0, zoom_count_vert[0]
				  , (double)zoom_factor_vert[1] / 2.0, (double)zoom_factor_horiz[1] / 2.0, zoom_count_vert[1]
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
		return (interlace_field != 0) ? 0xffffffff : 0;
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


#define STATE_VERSION	8

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

	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateArray(regs_written, sizeof(regs_written), 1);
	state_fio->StateValue(crtc_ch);
	state_fio->StateValue(interlace_field);

	state_fio->StateArray(timing_changed, sizeof(timing_changed), 1);
	state_fio->StateArray(address_changed, sizeof(address_changed), 1);
	state_fio->StateArray(mode_changed, sizeof(mode_changed), 1);

	state_fio->StateValue(display_enabled);
	state_fio->StateValue(crtc_clock);
	state_fio->StateValue(max_lines);
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(cpu_clocks);

	state_fio->StateArray(vstart_addr, sizeof(vstart_addr), 1);
	state_fio->StateArray(hend_words, sizeof(hend_words), 1);

	state_fio->StateArray(frame_offset, sizeof(frame_offset), 1);
	state_fio->StateArray(line_offset, sizeof(line_offset), 1);

	state_fio->StateArray(frame_offset_bak, sizeof(frame_offset_bak), 1);
	state_fio->StateArray(line_offset_bak, sizeof(line_offset_bak), 1);

	state_fio->StateArray(head_address, sizeof(head_address), 1);

	state_fio->StateArray(zoom_factor_vert, sizeof(zoom_factor_vert), 1);
	state_fio->StateArray(zoom_factor_horiz, sizeof(zoom_factor_horiz), 1);
	state_fio->StateArray(zoom_count_vert, sizeof(zoom_count_vert), 1);
	state_fio->StateArray(line_count, sizeof(line_count), 1);

	state_fio->StateValue(fo1_offset_value);
	state_fio->StateValue(vert_line_count);

	state_fio->StateValue(vsync);
	state_fio->StateValue(hsync);
	state_fio->StateArray(hdisp, sizeof(hdisp), 1);
	state_fio->StateArray(frame_in, sizeof(frame_in), 1);

	state_fio->StateValue(r50_planemask);
	state_fio->StateValue(r50_pagesel);
	state_fio->StateArray(dpalette_regs, sizeof(dpalette_regs), 1);
	state_fio->StateValue(dpalette_changed);

	state_fio->StateValue(video_brightness);

	state_fio->StateValue(voutreg_num);
	state_fio->StateValue(voutreg_ctrl);
	state_fio->StateValue(voutreg_prio);
	state_fio->StateArray(video_out_regs, sizeof(video_out_regs), 1);
	state_fio->StateValue(crtout_reg);
	state_fio->StateArray(crtout, sizeof(crtout), 1);

	state_fio->StateValue(pixels_per_line);
	state_fio->StateValue(lines_per_frame);

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

	state_fio->StateValue(is_sprite);
	state_fio->StateValue(sprite_offset);

	state_fio->StateValue(frame_us);
	state_fio->StateValue(horiz_us);
	state_fio->StateValue(horiz_width_posi_us);
	state_fio->StateValue(horiz_width_nega_us);
	state_fio->StateArray(horiz_start_us, sizeof(horiz_start_us), 1);
	state_fio->StateArray(horiz_end_us, sizeof(horiz_end_us), 1);

	state_fio->StateValue(event_id_hsync);
	state_fio->StateValue(event_id_hsw);
	state_fio->StateValue(event_id_vsync);
	state_fio->StateValue(event_id_vst1);
	state_fio->StateValue(event_id_vst2);
	state_fio->StateValue(event_id_vstart);
	state_fio->StateValue(event_id_hstart);

	state_fio->StateArray(event_id_vds, sizeof(event_id_vds), 1);
	state_fio->StateArray(event_id_vde, sizeof(event_id_vde), 1);
	state_fio->StateArray(event_id_hds, sizeof(event_id_hds), 1);
	state_fio->StateArray(event_id_hde, sizeof(event_id_hde), 1);

	if(loading) {
		for(int i = 0; i < 16; i++) {
			calc_apalette16(0, i);
			calc_apalette16(1, i);
		}
		for(int i = 0; i < 256; i++) {
			calc_apalette256(i);
		}
		for(int i = 0; i < 4; i++) {
			for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
				memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
			}
		}
		int dummy_mode0, dummy_mode1;
		recalc_cr0(regs[TOWNS_CRTC_REG_DISPMODE], true);
		make_dispmode(is_single_layer, dummy_mode0, dummy_mode1);

		display_linebuf = 0;
		render_linebuf = 0;
		req_recalc = false;
		copy_regs();
		calc_screen_parameters();
		force_recalc_crtc_param();
		update_horiz_khz();
		calc_zoom_regs(regs[TOWNS_CRTC_REG_ZOOM]);
		calc_pixels_lines();
		set_lines_per_frame(lines_per_frame);
		for(int i = 0; i < 4; i++) {
			hst[i] = pixels_per_line;
			vst[i] = lines_per_frame;
		}
	}
	return true;
}
}
