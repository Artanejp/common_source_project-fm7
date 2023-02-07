/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/
#include "../vm.h"
#include "../../common.h"

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
	set_lines_per_frame(512);
	//set_pixels_per_line(640);

	crtc_clock = 1.0e6 / 28.6363e6;
	set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	register_frame_event(this);
	voutreg_ctrl = 0x15;
	voutreg_prio = 0x00;
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
//	memset(regs, 0, sizeof(regs));
	crtc_ch = 0;
	sprite_offset = 0x00000;
	is_sprite = false;
	// initial settings for 1st frame
	req_recalc = false;
//	crtc_clock = 28.6363e6; // OK?
	interlace_field = false;
	is_compatible = true;

	line_count[0] = line_count[1] = 0;
	vert_line_count = -1;
	display_linebuf = 0;

	r50_planemask = 0x0f;
	r50_pagesel = 0;
	dpalette_changed = true;
	for(int i = 0; i < 8; i++) {
		dpalette_regs[i] = i;
	}
	apalette_code = 0;
	apalette_b = 0;
	apalette_r = 0;
	apalette_g = 0;
	for(int i = 0; i < 16; i++) {
		uint16_t r = ((i & 2) != 0) ? 0x7f : 0;
		uint16_t g = ((i & 4) != 0) ? 0x7f : 0;
		uint16_t b = ((i & 1) != 0) ? 0x7f : 0;

		if((i & 8) != 0) {
			r <<= 1;
			b <<= 1;
			g <<= 1;
			if(r != 0) {
				r |= 0x1;
			}
			if(g != 0) {
				g |= 0x1;
			}
			if(b != 0) {
				b |= 0x1;
			}
		}
		apalette_16_rgb[0][i][TOWNS_CRTC_PALETTE_R] = r;
		apalette_16_rgb[0][i][TOWNS_CRTC_PALETTE_G] = g;
		apalette_16_rgb[0][i][TOWNS_CRTC_PALETTE_B] = b;

		apalette_16_rgb[1][i][TOWNS_CRTC_PALETTE_R] = r;
		apalette_16_rgb[1][i][TOWNS_CRTC_PALETTE_G] = g;
		apalette_16_rgb[1][i][TOWNS_CRTC_PALETTE_B] = b;
		if(i == 0) {
			apalette_16_pixel[0][0] = RGBA_COLOR(0, 0, 0, 0);
			apalette_16_pixel[1][0] = RGBA_COLOR(0, 0, 0, 0);
		} else {
			apalette_16_pixel[0][i] = RGBA_COLOR(r, g, b, 0xff);
			apalette_16_pixel[1][i] = RGBA_COLOR(r, g, b, 0xff);
		}
	}
	for(int i = 0; i < 256; i++) {
		uint8_t r = (i & 0x38) << 2;
		uint8_t g = i & 0xc0;
		uint8_t b = (i & 0x07) << 5;
		__LIKELY_IF(r != 0) r |= 0x1f;
		__LIKELY_IF(b != 0) b |= 0x1f;
		__LIKELY_IF(g != 0) g |= 0x3f;
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
		crtout[i] = true;
		crtout_top[i] = true;
	}
	crtout_reg = 0x0f;
	for(int i = 0; i < 2; i++) {
		frame_offset[i] = 0;
		line_offset[i] = 80;
	}
	for(int i = 0; i < 2; i++) {
		frame_offset_bak[i] = frame_offset[i];
		line_offset_bak[i] = line_offset[i];
	}
	for(int i = 0; i < 2; i++) {
		vstart_addr[i] = 0;
		hstart_words[i] = 0;
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
	set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	set_lines_per_frame(512);
	//set_pixels_per_line(640);

	write_signals(&outputs_int_vsync, 0x0);

	hst_tmp = 640;
	vst_tmp = 400;
	for(int i = 0; i < 4; i++) {
		hst[i] = hst_tmp;
		vst[i] = vst_tmp;
	}
	memset(tvram_snapshot, 0x00, sizeof(tvram_snapshot));
	//osd->set_vm_screen_size((hst <= SCREEN_WIDTH) ? hst : SCREEN_WIDTH, (vst <= SCREEN_HEIGHT) ? vst : SCREEN_HEIGHT, -1, -1,  WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
	//emu->set_vm_screen_lines(vst);
	// For DEBUG Only
	// Mode 19
	force_recalc_crtc_param();

//	register_event(this, EVENT_CRTC_VSTART, vstart_us, false, &event_id_vstart);
}

void TOWNS_CRTC::cancel_event_by_id(int& event_num)
{
	if(event_num > -1) cancel_event(this, event_num);
	event_num = -1;
}

void TOWNS_CRTC::set_vsync(bool val, bool pre_vsync)
{
	write_signals(&outputs_int_vsync, (val) ? 0x00000000 : 0xffffffff);
	vsync = pre_vsync;
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
void TOWNS_CRTC::calc_apalette(int index)
{
	int layer = 0;
	switch(voutreg_prio & 0x30) {
	case 0x00:
		calc_apalette16(0, index & 0x0f);
		break;
	case 0x20:
		calc_apalette16(1, index & 0x0f);
		break;
	default:
		calc_apalette256(index & 0xff);
		break;
	}
}

void TOWNS_CRTC::calc_apalette16(int layer, int index)
{
	index = index & 0x0f;
	apalette_r = apalette_16_rgb[layer][index][TOWNS_CRTC_PALETTE_R] & 0xf0;
	apalette_g = apalette_16_rgb[layer][index][TOWNS_CRTC_PALETTE_G] & 0xf0;
	apalette_b = apalette_16_rgb[layer][index][TOWNS_CRTC_PALETTE_B] & 0xf0;

	__UNLIKELY_IF(index == 0) {
		apalette_16_pixel[layer][index] = RGBA_COLOR(0, 0, 0, 0); // ??
	} else {
		apalette_16_pixel[layer][index] = RGBA_COLOR(apalette_r, apalette_g , apalette_b, 0xff);
	}
}

void TOWNS_CRTC::calc_apalette256(int index)
{
	index = index & 255;
	apalette_r = apalette_256_rgb[index][TOWNS_CRTC_PALETTE_R];
	apalette_g = apalette_256_rgb[index][TOWNS_CRTC_PALETTE_G];
	apalette_b = apalette_256_rgb[index][TOWNS_CRTC_PALETTE_B];
	__UNLIKELY_IF(index == 0) {
		apalette_256_pixel[index] = RGBA_COLOR(0, 0, 0, 0); // ??
	} else {
		apalette_256_pixel[index] = RGBA_COLOR(apalette_r, apalette_g, apalette_b, 0xff);
	}
}

void TOWNS_CRTC::set_apalette_r(uint8_t val)
{
	switch(voutreg_prio & 0x30) {
	case 0x00:
		apalette_16_rgb[0][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_R] = val;
		calc_apalette16(0, (int)apalette_code);
		break;
	case 0x20:
		apalette_16_rgb[1][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_R] = val;
		calc_apalette16(1, (int)apalette_code);
		break;
	default:
		apalette_256_rgb[apalette_code & 0xff][TOWNS_CRTC_PALETTE_R] = val;
		calc_apalette256((int)apalette_code % 256);
		break;
	}
}

void TOWNS_CRTC::set_apalette_g(uint8_t val)
{
	switch(voutreg_prio & 0x30) {
	case 0x00:
		apalette_16_rgb[0][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_G] = val;
		calc_apalette16(0, (int)apalette_code);
		break;
	case 0x20:
		apalette_16_rgb[1][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_G] = val;
		calc_apalette16(1, (int)apalette_code);
		break;
	default:
		apalette_256_rgb[apalette_code & 0xff][TOWNS_CRTC_PALETTE_G] = val;
		calc_apalette256((int)apalette_code % 256);
		break;
	}
}

void TOWNS_CRTC::set_apalette_b(uint8_t val)
{
	switch(voutreg_prio & 0x30) {
	case 0x00:
		apalette_16_rgb[0][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_B] = val;
		calc_apalette16(0, (int)apalette_code);
		break;
	case 0x20:
		apalette_16_rgb[1][apalette_code & 0x0f][TOWNS_CRTC_PALETTE_B] = val;
		calc_apalette16(1, (int)apalette_code);
		break;
	default:
		apalette_256_rgb[apalette_code & 0xff][TOWNS_CRTC_PALETTE_B] = val;
		calc_apalette256((int)apalette_code % 256);
		break;
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


void TOWNS_CRTC::set_apalette_num(uint8_t val)
{
	apalette_code = ((int)val) % 256;
}

// CRTC register #29
void TOWNS_CRTC::set_crtc_clock(uint16_t val)
{
	scsel = (val & 0x0c) >> 2;
	clksel = val & 0x03;
	static const double clocks[] = {
		28.6363e6, 24.5454e6, 25.175e6, 21.0525e6
	};
	__UNLIKELY_IF((1.0e6 / clocks[clksel]) != crtc_clock) {
		crtc_clock = 1.0e6 / clocks[clksel];
		req_recalc = true;
	}
}

void TOWNS_CRTC::force_recalc_crtc_param(void)
{
	int hst_bak = hst_tmp;
	int vst_bak = vst_tmp;
	horiz_width_posi_us = crtc_clock * ((double)(regs[0] & 0x00fe)); // HSW1
	horiz_width_nega_us = crtc_clock * ((double)(regs[1] & 0x00fe)); // HSW2
	horiz_us = crtc_clock * ((double)((regs[4] & 0x07fe) + 1)); // HST
	double horiz_ref = horiz_us / 2.0;

	vst1_us = ((double)(regs[5] & 0x1f)) * horiz_ref; // VST1
	vst2_us = ((double)(regs[6] & 0x1f)) * horiz_ref; // VST2
	eet_us = ((double)(regs[7] & 0x1f)) * horiz_ref;  // EET
	frame_us = ((double)(regs[8] & 0x07ff)) * horiz_ref; // VST
	__LIKELY_IF(frame_us > 0.0) {
		set_frames_per_sec(1.0e6 / frame_us); // Its dummy.
	} else {
		frame_us = 1.0e6 / FRAMES_PER_SEC;
		set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	}
//	out_debug_log(_T("RECALC PARAM: horiz_us=%f frame_us=%f"), horiz_us, frame_us);
	for(int layer = 0; layer < 2; layer++) {
		vert_offset_tmp[layer] = (int)(regs[(layer << 1) + 13] & 0x07ff) - (int)(regs[6] & 0x1f);
		horiz_offset_tmp[layer] = (int)(regs[(layer << 1) + 9] & 0x07ff) - (int)(regs[0] & 0x00fe);
		vert_start_us[layer] =  ((double)(regs[(layer << 1) + 13] & 0x07ff)) * horiz_ref;   // VDSx
		vert_end_us[layer] =    ((double)(regs[(layer << 1) + 13 + 1] & 0x07ff)) * horiz_ref; // VDEx
		horiz_start_us[layer] = ((double)(regs[(layer << 1) + 9] & 0x07ff)) * crtc_clock ;   // HDSx
		horiz_end_us[layer] =   ((double)(regs[(layer << 1) + 9 + 1] & 0x07ff)) * crtc_clock ;   // HDEx
	}

	hst_tmp = (regs[4] & 0x7fe) + 1;
	vst_tmp =  (regs[8] & 0x7ff) + 1;
	if((voutreg_ctrl & 0x10) == 0) {
		// Single layer
		uint32_t vst_an = (int)(regs[14] & 0x07ff) - (int)(regs[13] & 0x07ff);
		if(vst_an <= 1) vst_an = 2;
		uint32_t hst_an = (int)(regs[10] & 0x07ff) - (int)(regs[9] & 0x07ff);
		if(hst_an <= 8) hst_an = 8;
		if(vst_an > 768) vst_an >>= 1;
//		hst_tmp = (hst_tmp < hst_an) ? hst_tmp : hst_an;
//		vst_tmp = (vst_tmp < vst_an) ? vst_tmp : vst_an;
		hst_tmp = hst_an;
		vst_tmp = vst_an;
	} else {
		uint32_t vst_an = (int)(regs[14] & 0x07ff) - (int)(regs[13] & 0x07ff);
		if(vst_an <= 1) vst_an = 2;
		uint32_t hst_an = (int)(regs[10] & 0x07ff) - (int)(regs[9] & 0x07ff);
		if(hst_an <= 8) hst_an = 8;
		uint32_t vst_an2 = (int)(regs[16] & 0x07ff) - (int)(regs[15] & 0x07ff);
		if(vst_an2 <= 1) vst_an2 = 2;
		uint32_t hst_an2 = (int)(regs[12] & 0x07ff) - (int)(regs[11] & 0x07ff);
		if(hst_an2 <= 8) hst_an2 = 8;
		if(vst_an < vst_an2) vst_an = vst_an2;
		if(hst_an < hst_an2) hst_an = hst_an2;
		vst_an >>= 1;
		/*if(vst_tmp > vst_an) */ vst_tmp = vst_an;
		/*if(hst_tmp > hst_an) */ hst_tmp = hst_an;
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
	case 0x0442 :
		{
			pair16_t rdata;
			rdata.w = regs[crtc_ch];
			rdata.b.l = (uint8_t)data;
			write_io16(addr, rdata.w);
		}
		break;
	case 0x0443:
		{
			pair16_t rdata;
			rdata.w = regs[crtc_ch];
			rdata.b.h = (uint8_t)data;
			write_io16(addr, rdata.w);
		}
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
		set_apalette_num(data);
		break;
	case 0xfd91:
		break;
	case 0xfd92:
		set_apalette_b(data);
		break;
	case 0xfd93:
		break;
	case 0xfd94:
		set_apalette_r(data);
		break;
	case 0xfd95:
		break;
	case 0xfd96:
		set_apalette_g(data);
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
		crtout[0] = ((data & 0x0c) != 0) ? true : false;
		crtout[1] = ((data & 0x03) != 0) ? true : false;
		crtout_reg = data & 0x0f;
		break;
	}
}

void TOWNS_CRTC::write_io16(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE16 ADDR=%04x DATA=%04x"), addr, data);
	switch(addr & 0xfffe) {
	case 0x0442:
		{
			if(crtc_ch < 32) {
				if((crtc_ch < 0x09) && ((crtc_ch >= 0x04) || (crtc_ch <= 0x01))) { // HSW1..VST
					if(regs[crtc_ch] != (uint16_t)data) {
						req_recalc = true;
					}
				} else if(crtc_ch < 0x11) { // HDS0..VDE1
					uint8_t localch = ((crtc_ch  - 0x09) / 2) & 1;
					if(regs[crtc_ch] != (uint16_t)data) {
						timing_changed[localch] = true;
						// ToDo: Crtc_Change render parameter within displaying.
						req_recalc = true;
					}
				}else if(crtc_ch < 0x19) { // FA0..LO1
					uint8_t localch = (crtc_ch - 0x11) / 4;
					uint8_t localreg = (crtc_ch - 0x11) & 3;
//					if(regs[crtc_ch] != (uint16_t)data) {
						address_changed[localch] = true;
						switch(localreg) {
						case 0: // FAx
							vstart_addr[localch] = (uint32_t)(data & 0xffff);
							break;
						case 1: // HAJx
							hstart_words[localch] = (uint32_t)(data & 0x07ff);
							break;
						case 2: // FOx
							frame_offset[localch] = (uint32_t)(data & 0xffff);
							break;
						case 3: // LOx
							line_offset[localch] = (uint32_t)(data & 0xffff);
							break;
						}
//					}
				} else  { // All reg
//					if(regs[crtc_ch] != (uint16_t)data) {
						switch(crtc_ch - 0x19) {
						case 0: // EHAJ
						case 1: // EVAJ
							// ToDo: External SYNC.
							break;
						case 2: // ZOOM
							{
								uint8_t zfv[2];
								uint8_t zfh[2];
								pair16_t pd;
								pd.w = (uint16_t)data;
								zfv[0] = ((pd.b.l & 0xf0) >> 4) + 1;
								zfh[0] = (pd.b.l & 0x0f) + 1;
								zfv[1] = ((pd.b.h & 0xf0) >> 4) + 1;
								zfh[1] = (pd.b.h & 0x0f) + 1;
								/*
								if((zfv[0] != zoom_factor_vert[0]) || (zfh[0] != zoom_factor_horiz[0])) {
									timing_changed[0] = true;
									address_changed[0] = true;
									if(zfv[0] != zoom_factor_vert[0]) zoom_count_vert[0] = zfv[0];
								}
								if((zfv[1] != zoom_factor_vert[1]) || (zfh[1] != zoom_factor_horiz[1])) {
									timing_changed[1] = true;
									address_changed[1] = true;
									if(zfv[1] != zoom_factor_vert[1]) zoom_count_vert[1] = zfv[1];
								}
								*/
								for(int i = 0; i < 2; i++) {
									zoom_factor_vert[i]  = zfv[i];
									zoom_factor_horiz[i] = zfh[i];
									zoom_count_vert[i]  = zfv[i];
									timing_changed[i] = true;
									address_changed[i] = true;
								}
//							}
							break;
						case 3: // CR0
//							if(regs[crtc_ch] != data) {
								if((data & 0x8000) == 0) {
									// START BIT
									restart_display();
								} else {
									stop_display();
								}
								if((data & 0x4000) == 0) {
									// ESYN BIT
									// EXTERNAL SYNC OFF
								} else {
									// EXTERNAL SYNC ON
								}
								impose_mode[1]  = ((data & 0x0080) == 0);
								impose_mode[0]  = ((data & 0x0040) == 0);
								carry_enable[1] = ((data & 0x0020) != 0);
								carry_enable[0] = ((data & 0x0010) != 0);
								uint8_t dmode[2];
								dmode[0] = data & 0x03;
								dmode[1] = (data & 0x0c) >> 2;
								for(int i = 0; i < 2; i++) {
									if(dmode[0] != display_mode[0]) {
										mode_changed[0] = true;
										notify_mode_changed(i, dmode[i]);
									}
									display_mode[i] = dmode[i];
								}
//							}
							break;
						case 4: // CR1
							set_crtc_clock((uint16_t)data);
							break;
						case 5: // Reserved(REG#30)
							break;
						case 6: // CR2
							// ToDo: External trigger.
							break;
						}
					}
				}
				regs[crtc_ch] = (uint16_t)data;
			}
		}
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
	data = (data & 0xff00 ) | (regs[30] & 0x00ff);
	return data;
}

uint32_t TOWNS_CRTC::read_io16(uint32_t addr)
{
//	out_debug_log(_T("READ16 ADDR=%04x"), addr);
	if((addr >= 0xfd90) && (addr <= 0xfd97)) {
		switch(addr) {
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
		}
		return 0xff;
	}
	switch(addr & 0xfffe) {
	case 0x0442:
/*		if(crtc_ch == 21) { // FO1
			return ((regs[21] & 0x7fff) + fo1_offset_value);
			} else */if(crtc_ch == 30) {
			return (uint32_t)read_reg30();
		} else {
			return regs[crtc_ch];
		}
		break;
	default:
		return read_io8(addr & 0xfffe);
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
			if(crtc_ch == 30) {
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
			if(crtc_ch == 30) {
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

bool TOWNS_CRTC::render_32768(scrntype_t* dst, scrntype_t *mask, int y, int layer, bool do_alpha)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	int trans = (display_linebuf == 0) ? display_linebuf_mask : ((display_linebuf - 1) & display_linebuf_mask);
//	int trans = display_linebuf & 3;
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	int num = linebuffers[trans][y].num[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = mask;
	__UNLIKELY_IF(pwidth <= 0) return false;
	__UNLIKELY_IF(magx < 1) return false;
	const int width = ((hst[trans] + 16 * magx) > TOWNS_CRTC_MAX_PIXELS) ? TOWNS_CRTC_MAX_PIXELS : (hst[trans] + 16 * magx);
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) != 0) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) != 0) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 0) return false;
	if(y == 128) {
		//out_debug_log("RENDER_32768 Y=%d LAYER=%d PWIDTH=%d WIDTH=%d DST=%08X MASK=%08X ALPHA=%d", y, layer, pwidth, width, dst, mask, do_alpha);
	}
	__DECL_ALIGNED(16) uint16_t pbuf[8];
	__DECL_ALIGNED(16) uint16_t rbuf[8];
	__DECL_ALIGNED(16) uint16_t gbuf[8];
	__DECL_ALIGNED(16) uint16_t bbuf[8];
	__DECL_ALIGNED(32) scrntype_t sbuf[8];
	__DECL_ALIGNED(32) scrntype_t abuf[8];
	__DECL_ALIGNED(32) uint8_t a2buf[8];
	pair16_t ptmp16;
	int rwidth = pwidth & 7;

	int k = 0;

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
			for(int i = 0; i < 8; i++) {
				a2buf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
			}
			for(int i = 0; i < 8; i++) {
				sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], a2buf[i]);
			}
		} else {
			for(int i = 0; i < 8; i++) {
				abuf[i] = (pbuf[i] & 0x8000) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
			}
			for(int i = 0; i < 8; i++) {
				sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], 255);
			}
		}
		__LIKELY_IF(((magx << 3) + k) <= width) {
			switch(magx) {
			case 1:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					q[i] = sbuf[i];
				}
				q += 8;
				__LIKELY_IF(r2 != nullptr) {
__DECL_VECTORIZED_LOOP
					for(int i = 0; i < 8; i++) {
						r2[i] = abuf[i];
					}
					r2 += 8;
				}
				k += 8;
				break;
			case 2:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 16; i++) {
					q[i] = sbuf[i >> 1];
				}
				q += 16;
				__LIKELY_IF(r2 != nullptr) {
__DECL_VECTORIZED_LOOP
					for(int i = 0; i < 16; i++) {
						r2[i] = abuf[i >> 1];
					}
					r2 += 16;
				}
				k += 16;
				break;
			case 4:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 32; i++) {
					q[i] = sbuf[i >> 2];
				}
				q += 32;
				__LIKELY_IF(r2 != nullptr) {
__DECL_VECTORIZED_LOOP
					for(int i = 0; i < 32; i++) {
						r2[i] = abuf[i >> 2];
					}
					r2 += 32;
				}
				k += 32;
				break;
			default:
				for(int i = 0; i < 8; i++) {
					for(int ii = 0; ii < magx; ii++) {
						*q++ = sbuf[i];
					}

				}
				__LIKELY_IF(r2 != nullptr) {
__DECL_VECTORIZED_LOOP
					for(int i = 0; i < 8; i++) {
						for(int ii = 0; ii < magx; ii++) {
							*r2++ = abuf[i];
						}
					}
				}
				k += (magx << 3);
				break;
			}
		} else {
			int kbak = k;
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				for(int j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					kbak++;
					if(kbak >= width) break;
				}
			}
			__LIKELY_IF(r2 != nullptr) {
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 8; i++) {
					for(int j = 0; j < magx; j++) {
						*r2++ = abuf[i];
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
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				a2buf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], a2buf[i]);
			}
		} else {
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				abuf[i] = (pbuf[i] & 0x8000) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 8; i++) {
				sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], 255);
			}
		}
		__UNLIKELY_IF(magx == 1) {
			for(int i = 0; i < rwidth; i++) {
				*q++ = sbuf[i];
			}
			__LIKELY_IF(r2 != nullptr) {
				for(int i = 0; i < rwidth; i++) {
					*r2++ = abuf[i];
				}
			}
			k += 8;
			__UNLIKELY_IF(k >= width) return true;
		} else if(magx == 2) {
			for(int i = 0; i < rwidth; i++) {
				q[0] = sbuf[i];
				q[1] = sbuf[i];
				q += 2;
			}
			if(r2 != nullptr) {
				for(int i = 0; i < rwidth; i++) {
					r2[0] = abuf[i];
					r2[1] = abuf[i];
					r2 += 2;
				}
			}
			k += 16;
			__UNLIKELY_IF(k >= width) return true;
		} else {
			for(int i = 0; i < rwidth; i++) {
				for(int j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					if(r2 != nullptr) {
						*r2++ = abuf[i];
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

bool TOWNS_CRTC::render_256(scrntype_t* dst, int y)
{
	// 256 colors
	__UNLIKELY_IF(dst == nullptr) return false;
	int trans = (display_linebuf == 0) ? display_linebuf_mask : ((display_linebuf - 1) & display_linebuf_mask);
//	int trans = display_linebuf & 3;
	int magx = linebuffers[trans][y].mag[0];
	int pwidth = linebuffers[trans][y].pixels[0];
	int num = linebuffers[trans][y].num[0];
	uint8_t *p = linebuffers[trans][y].pixels_layer[0];
	int bitshift0 = linebuffers[trans][y].bitshift[0];
	scrntype_t* q = dst;
	__UNLIKELY_IF(pwidth <= 0) return false;
	const int width = ((hst[trans] + 16 * magx) > TOWNS_CRTC_MAX_PIXELS) ? TOWNS_CRTC_MAX_PIXELS : (hst[trans] + 16 * magx);
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) != 0) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) != 0) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}
	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 0) return false;

	__DECL_ALIGNED(32)  scrntype_t apal256[256];
	my_memcpy(apal256, apalette_256_pixel, sizeof(scrntype_t) * 256);

	__DECL_ALIGNED(16) uint8_t pbuf[16];
	__DECL_ALIGNED(32) scrntype_t sbuf[16];
//	out_debug_log(_T("Y=%d MAGX=%d WIDTH=%d pWIDTH=%d"), y, magx, width, pwidth);
	__UNLIKELY_IF(magx < 1) {
		return false;
	}
	__UNLIKELY_IF(pwidth < 1) pwidth = 1;
	int xx = 0;
	int k = 0;
	for(int x = 0; x < (pwidth >> 4); x++) {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			pbuf[i] = p[i];
		}
		p += 16;
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			sbuf[i] = apal256[pbuf[i]];
		}
		int kbak = k;
		if(((magx << 4) + k) <= width) {
			switch(magx) {
			case 1:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 16; i++) {
					q[i] = sbuf[i];
				}
				k += 16;
				q += 16;
				break;
			case 2:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 32; i++) {
					q[i] = sbuf[i >> 1];
				}
				k += 32;
				q += 32;
				break;
			case 4:
__DECL_VECTORIZED_LOOP
				for(int i = 0, j = 0; i < 16; i++, j += 4) {
					q[j + 0] = sbuf[i];
					q[j + 1] = sbuf[i];
					q[j + 2] = sbuf[i];
					q[j + 3] = sbuf[i];
				}
				k += 64;
				q += 64;
				break;
			default:
				for(int i = 0; i < 16; i++) {
					for(int j = 0; j < magx; j++) {
						q[j] = sbuf[i];
					}
					q += magx;
					k += magx;
				}
				break;
			}
		} else {
			for(int i = 0; i < 16; i++) {
				for(int j = 0; j < magx; j++) {
					q[j] = sbuf[i];
					k++;
					if(k >= width) break;
				}
				if(k >= width) break;
				q += magx;
			}
		}
	}
	__LIKELY_IF(k >= width) return true;
	int w = pwidth & 0x0f;
	__UNLIKELY_IF(w != 0) {
		for(int i = 0; i < w; i++) {
			pbuf[i] = p[i];
		}
		for(int i = 0; i < w; i++) {
			sbuf[i] = apal256[pbuf[i]];
		}
		if(((magx * w) + k) <= width) {
			switch(magx) {
			case 1:
				for(int i = 0; i < w; i++) {
					q[i] = sbuf[i];
				}
				k += w;
				q += w;
				break;
			case 2:
				for(int i = 0, j = 0; i < w; i++, j += 2) {
					q[j + 0] = sbuf[i];
					q[j + 1] = sbuf[i];
				}
				k += (w << 1);
				q += (w << 1);
				break;
			case 4:
				for(int i = 0, j = 0; i < w; i++, j += 4) {
					q[j + 0] = sbuf[i];
					q[j + 1] = sbuf[i];
					q[j + 2] = sbuf[i];
					q[j + 3] = sbuf[i];
				}
				k += (w << 2);
				q += (w << 2);
				break;
			default:
				for(int i = 0; i < w; i++) {
					for(int j = 0; j < magx; j++) {
						q[j] = sbuf[i];
					}
					q += magx;
					k += magx;
				}
				break;
			}
		} else {
			for(int i = 0; i < w; i++) {
				for(int j = 0; j < magx; j++) {
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

bool TOWNS_CRTC::render_16(scrntype_t* dst, scrntype_t *mask, scrntype_t* pal, int y, int layer, bool do_alpha)
{
	__UNLIKELY_IF(dst == nullptr) return false;

	int trans = (display_linebuf == 0) ? display_linebuf_mask : ((display_linebuf - 1) & display_linebuf_mask);
//	int trans = display_linebuf & 3;
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	//int num = linebuffers[trans][y].num[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = mask;
	__UNLIKELY_IF(pwidth <= 0) return false;
	__UNLIKELY_IF(magx < 1) return false;
	const int width = ((hst[trans] + 16 * magx) > TOWNS_CRTC_MAX_PIXELS) ? TOWNS_CRTC_MAX_PIXELS : (hst[trans] + 16 * magx);
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) != 0) {
			pwidth++;
		}
	} else {
		if((pwidth % magx) != 0) {
			pwidth = pwidth / magx + 1;
		} else {
			pwidth = pwidth / magx;
		}
	}	__UNLIKELY_IF(pwidth > TOWNS_CRTC_MAX_PIXELS) pwidth = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(pwidth <= 1) return false;

	__DECL_ALIGNED(16) uint8_t pbuf[8];
	__DECL_ALIGNED(16) uint8_t hlbuf[16];
	__DECL_ALIGNED(16) uint8_t mbuf[16];
	__DECL_ALIGNED(32) scrntype_t sbuf[16];
	__DECL_ALIGNED(32) scrntype_t abuf[16];
	__DECL_ALIGNED(32) scrntype_t a2buf[16];

	__DECL_ALIGNED(32)  scrntype_t  palbuf[16];
	uint8_t pmask = linebuffers[trans][y].r50_planemask & 0x0f;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		mbuf[i] = pmask;
	}
	__UNLIKELY_IF(pal == nullptr) {
		__DECL_ALIGNED(16) uint8_t tmp_r[16];
		__DECL_ALIGNED(16) uint8_t tmp_g[16];
		__DECL_ALIGNED(16) uint8_t tmp_b[16];
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			tmp_r[i] = ((i & 2) != 0) ? (((i & 8) != 0) ? 255 : 128) : 0;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			tmp_g[i] = ((i & 4) != 0) ? (((i & 8) != 0) ? 255 : 128) : 0;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			tmp_b[i] = ((i & 1) != 0) ? (((i & 8) != 0) ? 255 : 128) : 0;
		}
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			palbuf[i] = RGBA_COLOR(tmp_r[i], tmp_g[i], tmp_b[i], 255);
		}
	} else {
__DECL_VECTORIZED_LOOP
		for(int i = 0; i < 16; i++) {
			palbuf[i] = pal[i];
		}
	}
	palbuf[0] = RGBA_COLOR(0, 0, 0, 0);
	__DECL_ALIGNED(32) static const scrntype_t maskdata[16] =
	{
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

	int k = 0;
	pwidth = pwidth / 2; // 20201210 K.O
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
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i++) {
				sbuf[i] = palbuf[hlbuf[i]];
			}
		} else {
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i++) {
//				abuf[i] = (hlbuf[i] == 0) ? RGBA_COLOR(0, 0, 0, 0): RGBA_COLOR(255, 255, 255, 255);
				abuf[i] = maskdata[hlbuf[i]];
			}
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i++) {
				sbuf[i] = palbuf[hlbuf[i]];
			}
		}
		int kbak = k;
		__LIKELY_IF(((magx << 4) + k) <= width) {
			switch(magx) {
			case 1:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 16; i++) {
					q[i] = sbuf[i];
				}
				k += 16;
				q += 16;
				break;
			case 2:
__DECL_VECTORIZED_LOOP
				for(int i = 0; i < 32; i++) {
					q[i] = sbuf[i >> 1];
				}
				k += 32;
				q += 32;
				break;
			case 4:
__DECL_VECTORIZED_LOOP
				for(int i = 0, j = 0; i < 16; i++, j+= 4) {
					q[j + 0] = sbuf[i];
					q[j + 1] = sbuf[i];
					q[j + 2] = sbuf[i];
					q[j + 3] = sbuf[i];
				}
				k += 64;
				q += 64;
				break;
			default:
				for(int i = 0; i < 16; i++) {
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[i];
					}
					k += magx;
					if(k >= width) break;
				}
				break;
			}
		} else {
			for(int i = 0; i < 16; i++) {
				for(int j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					k++;
					__UNLIKELY_IF(k >= width) break;
				}
				__UNLIKELY_IF(k >= width) break;
			}
		}
		if(!(do_alpha) && (r2 != nullptr)) {
			if(((magx << 4) + k) <= width) {
				switch(magx) {
				case 1:
__DECL_VECTORIZED_LOOP
					for(int i = 0; i < 16; i++) {
						r2[i] = abuf[i];
					}
					r2 += 16;
					break;
				case 2:
__DECL_VECTORIZED_LOOP
					for(int i = 0; i < 32; i++) {
						r2[i] = abuf[i >> 1];
					}
					r2 += 32;
					break;
				case 4:
__DECL_VECTORIZED_LOOP
					for(int i = 0, j = 0; i < 16; i++, j += 4) {
						r2[j + 0] = abuf[i];
						r2[j + 1] = abuf[i];
						r2[j + 2] = abuf[i];
						r2[j + 3] = abuf[i];
					}
					r2 += 64;
					break;
				default:
					for(int i = 0; i < 16; i++) {
						for(int j = 0; j < magx; j++) {
							*r2++ = abuf[i];
						}
						kbak += magx;
						__UNLIKELY_IF(kbak >= width) break;
					}
					break;
				}
			} else {
				for(int i = 0; i < 16; i++) {
					for(int j = 0; j < magx; j++) {
						*r2++ = abuf[i];
						kbak++;
						__UNLIKELY_IF(kbak >= width) break;
					}
					__UNLIKELY_IF(kbak >= width) break;
				}
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
	__UNLIKELY_IF(rwidth > 0) {
		for(int x = 0; x < rwidth; x++) {
			tmpp = *p++;
			tmph = tmpp >> 4;
			tmpl = tmpp & 0x0f;
			sbuf[0] = palbuf[tmph];
			sbuf[1] = palbuf[tmpl];

			__UNLIKELY_IF(do_alpha) {
				if(magx == 1) {
					*q++ = sbuf[0];
					k++;
					__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					*q++ = sbuf[1];
					k++;
					__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
				} else {
					for(int xx = 0; xx < magx; xx++) {
						*q++ = sbuf[0];
						k++;
						__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					}
					__UNLIKELY_IF(k >= width) break;
					for(int xx = 0; xx < magx; xx++) {
						*q++ = sbuf[1];
						k++;
						__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					}
					__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
				}
			} else {
				ah = (tmph == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
				al = (tmpl == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
				if(magx == 1) {
					*q++ = sbuf[0];
					__LIKELY_IF(r2 != nullptr) {
						*r2++ = ah;
					}
					k++;
					__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					*q++ = sbuf[1];
					__LIKELY_IF(r2 != nullptr) {
						*r2++ = al;
					}
					k++;
					__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
				} else {
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[0];
						__LIKELY_IF(r2 != nullptr) {
							*r2++ = ah;
						}
						k++;
						__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					}
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[1];
						__LIKELY_IF(r2 != nullptr) {
							*r2++ = al;
						}
						k++;
						__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
					}
					__UNLIKELY_IF(k >= TOWNS_CRTC_MAX_PIXELS) break;
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
void TOWNS_CRTC::mix_screen(int y, int width, bool do_mix0, bool do_mix1)
{
	__UNLIKELY_IF(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(width <= 0) return;

	int trans = (display_linebuf == 0) ? display_linebuf_mask : ((display_linebuf - 1) & display_linebuf_mask);

	int bitshift0 = linebuffers[trans][y].bitshift[0];
	int bitshift1 = linebuffers[trans][y].bitshift[1];
	scrntype_t *pp = osd->get_vm_screen_buffer(y);
	if(y == 128) {
		//out_debug_log(_T("MIX_SCREEN Y=%d WIDTH=%d DST=%08X"), y, width, pp);
	}
	/*
	if(width < -(bitshift0)) {
		do_mix0 = false;
	} else if(width < bitshift0) {
		do_mix0 = false;
	}
	if(width < -(bitshift1)) {
		do_mix1 = false;
	} else if(width < bitshift1) {
		do_mix1 = false;
	}
	*/
	__LIKELY_IF(pp != nullptr) {
		if((do_mix0) && (do_mix1)) {
			// alpha blending
			__DECL_ALIGNED(32) scrntype_t pixbuf0[8];
			__DECL_ALIGNED(32) scrntype_t pixbuf1[8];
			__DECL_ALIGNED(32) scrntype_t maskbuf_front[8];
			__DECL_ALIGNED(32) scrntype_t maskbuf_back[8];
			int of0 = 0;
			int of1 = 0;
			if(bitshift0 < 0) {
				of0 = -bitshift0;
			} else if(bitshift0 > 0) {
				of0 = bitshift0;
			}
			if(bitshift1 < 0) {
				of1 = -bitshift1;
			} else if(bitshift0 > 0) {
				of1 = bitshift1;
			}
			for(int xx = 0; xx < width; xx += 8) {
__DECL_VECTORIZED_LOOP
				for(int ii = 0; ii < 8; ii++) {
					pixbuf1[ii] = 0;
					pixbuf0[ii] = 0;
					maskbuf_front[ii] = 0;
				}
				scrntype_t *px1 = &(lbuffer1[xx + of1]);
__DECL_VECTORIZED_LOOP
				for(int ii = 0; ii < 8; ii++) {
						pixbuf1[ii] = px1[ii];
				}
				scrntype_t *px0 = &(lbuffer0[xx + of0]);
				scrntype_t *ax = &(abuffer0[xx + of0]);
				for(int ii = 0; ii < 8; ii++) {
					pixbuf0[ii] = px0[ii];
					maskbuf_front[ii] = ax[ii];
				}
__DECL_VECTORIZED_LOOP
				for(int ii = 0; ii < 8; ii++) {
					maskbuf_back[ii] = ~maskbuf_front[ii];
				}
__DECL_VECTORIZED_LOOP
				for(int ii = 0; ii < 8; ii++) {
					pixbuf1[ii] = pixbuf1[ii] & maskbuf_back[ii];
					pixbuf0[ii] = pixbuf0[ii] & maskbuf_front[ii];
				}
__DECL_VECTORIZED_LOOP
				for(int ii = 0; ii < 8; ii++) {
					pixbuf0[ii] = pixbuf0[ii] | pixbuf1[ii];
				}
__DECL_VECTORIZED_LOOP
				for(int ii = 0; ii < 8; ii++) {
					pp[ii] = pixbuf0[ii];
				}
				pp += 8;
			}
#if 0
			int rrwidth = width & 7;
			if(rrwidth > 0) {
				scrntype_t pix0, pix1, mask0, mask1;
				int xptr = width & 0x7f8; // Maximum 2048 pixs
				scrntype_t *px1 = &(lbuffer1[xptr + of1]);
				scrntype_t *px0 = &(lbuffer0[xptr + of0]);
				scrntype_t *ax = &(abuffer0[xptr + of0]);

				for(int ii = 0; ii < rrwidth; ii++) {
					pix0 = px0[ii];
					mask0 = ax[ii];
					pix1 = px1[ii];
					mask1 = ~mask0;
					pix0 = pix0 & mask0;
					pix1 = pix1 & mask1;
					pix0 = pix0 | pix1;
					pp[ii]	= pix0;
				}
			}
#endif
		} else if(do_mix0) {
			if(bitshift0 < 0) {
				if(-(bitshift0) < width) {
					transfer_pixels(pp, &(lbuffer0[-bitshift0]), width);
				}
			} else if(bitshift0 > 0) {
				transfer_pixels(pp, &(lbuffer1[bitshift0]), width);
			} else {
				transfer_pixels(pp, lbuffer0, width);
			}
		} else __LIKELY_IF(do_mix1) {
			if(bitshift1 < 0) {
				if(-(bitshift1) < width) {
					transfer_pixels(pp, &(lbuffer1[-bitshift1]), width);
				}
			} else if(bitshift1 > 0) {
				transfer_pixels(pp, &(lbuffer1[bitshift1]), width);
			} else {
				transfer_pixels(pp, lbuffer1, width);
			}
		} else {
			memset(pp, 0x00, width * sizeof(scrntype_t));
		}
	}
}

void TOWNS_CRTC::draw_screen()
{
	int trans = (display_linebuf == 0) ? display_linebuf_mask : ((display_linebuf - 1) & display_linebuf_mask);
	//int trans2 = ((display_linebuf - 2) & display_linebuf_mask);
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

	memset(lbuffer1, 0x00, sizeof(lbuffer1));
	memset(abuffer1, 0xff, sizeof(abuffer1));
	memset(lbuffer0, 0x00, sizeof(lbuffer0));
	memset(abuffer0, 0xff, sizeof(abuffer0));
	bool is_single = false;
	if((voutreg_ctrl & 0x10) == 0) is_single = true;
	__DECL_ALIGNED(32)  scrntype_t apal16[2][16];
	my_memcpy(apal16[0], apalette_16_pixel[0], sizeof(scrntype_t) * 16);
	my_memcpy(apal16[1], apalette_16_pixel[1], sizeof(scrntype_t) * 16);

	for(int y = 0; y < lines; y++) {
		bool do_mix0 = false;
		bool do_mix1 = false;
		if(is_single) {
			if(linebuffers[trans][y].crtout[0] != 0) {
				switch(linebuffers[trans][y].mode[0]) {
				case DISPMODE_256:
					do_mix0 = render_256(lbuffer0, y);
					break;
				case DISPMODE_32768:
					do_mix0 = render_32768(lbuffer0, abuffer0, y, 0, do_alpha);
					break;
				default: // 16 Colors mode don't allow with single layer mode.
					break;
				}
			}

		} else {
			int prio0 = linebuffers[trans][y].num[0];
			int prio1 = linebuffers[trans][y].num[1];
			if(linebuffers[trans][y].crtout[prio1] != 0) {
				switch(linebuffers[trans][y].mode[prio1]) {
				case DISPMODE_16:
					do_mix1 = render_16(lbuffer1, abuffer1, &(apal16[prio1][0]), y, linebuffers[trans][y].num[1], do_alpha);
					break;
				case DISPMODE_32768:
					do_mix1 = render_32768(lbuffer1, abuffer1, y, prio1, do_alpha);
					break;
				default: // 256 Colors mode don't allow in 2 layers mode.
					do_mix1 = false;
					break;
				}
			}
			// Upper layer
			if(linebuffers[trans][y].crtout[prio0] != 0){
				switch(linebuffers[trans][y].mode[prio0]) {
				case DISPMODE_16:
					do_mix0 = render_16(lbuffer0, abuffer0, &(apal16[prio0][0]), y, prio0, do_alpha);
					break;
				case DISPMODE_32768:
					do_mix0 = render_32768(lbuffer0, abuffer0, y, prio0, do_alpha);
					break;
				default: // 256 Colors mode don't allow in 2 layers mode.
					do_mix0 = false;
					break;
				}
			}
		}
//		if(y == 128) {
//			out_debug_log(_T("MIX: %d %d width=%d"), do_mix0, do_mix1, width);
//		}
		mix_screen(y, width, do_mix0, do_mix1);
	}

	//display_linebuf = (display_linebuf + 1) & 3;
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
//	uint32_t linesize = regs[24] * 4;
	int c = 0;
	uint32_t plane_offset = 0x40000 + ((r50_pagesel != 0) ? 0x20000 : 0x00000);
	for(int y = 0; y < 25; y++) {
		uint32_t linesize = regs[24] * 4;
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
void TOWNS_CRTC::transfer_line(int line)
{
	__UNLIKELY_IF(line < 0) return;
	__UNLIKELY_IF(line >= TOWNS_CRTC_MAX_LINES) return;
	__UNLIKELY_IF(d_vram == nullptr) return;
	uint8_t ctrl, prio;
	ctrl = voutreg_ctrl;
	prio = voutreg_prio;

	//int trans = (display_linebuf - 1) & 3;
	int trans = display_linebuf & display_linebuf_mask;
	__UNLIKELY_IF(linebuffers[trans] == nullptr) return;

__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 4; i++) {
		linebuffers[trans][line].mode[i] = 0;
		linebuffers[trans][line].pixels[i] = 0;
		linebuffers[trans][line].mag[i] = 0;
		linebuffers[trans][line].num[i] = -1;
	}

	int page0, page1;
	linebuffers[trans][line].r50_planemask = r50_planemask;
	linebuffers[trans][line].prio = prio;
	if((prio & 0x01) == 0) {
		page0 = 0; // Front
		page1 = 1; // Back
	} else {
		page0 = 1;
		page1 = 0;
	}
//	out_debug_log("LINE %d CTRL=%02X \n", line, ctrl);
	bool did_transfer[2] = { false, false };
	bool to_disp[2] = { false, false};
	uint32_t address_shift[2] = { 0, 0};
	uint32_t address_mask[2] = {0x0003ffff, 0x0003ffff};
	static const uint32_t address_add[2] =  {0x00000000, 0x00040000};
	uint8_t ctrl_b = ctrl;
	linebuffers[trans][line].num[0] = page0;
	linebuffers[trans][line].num[1] = page1;
	linebuffers[trans][line].mode[0] = DISPMODE_16;
	linebuffers[trans][line].mode[1] = DISPMODE_16;
	uint8_t page_16mode = r50_pagesel;

	for(int l = 0; l < 2; l++) {
		if((ctrl & 0x10) == 0) { // One layer mode
			linebuffers[trans][line].num[0] = 0;
			linebuffers[trans][line].num[1] = 0;
			linebuffers[trans][line].crtout[0] = (crtout_top[0]) ? 0xff : 0x00;
			linebuffers[trans][line].crtout[1] = 0;
			bool disp = frame_in[0];
			__UNLIKELY_IF((horiz_end_us[0] <= 0.0) || (horiz_end_us[0] <= horiz_start_us[0])) {
				disp = false;
			}
//			if(vert_offset_tmp[0] > line) {
//				disp = false;
//			}
			__LIKELY_IF(disp) {
				switch(ctrl & 0x0f) {
				case 0x0a:
					linebuffers[trans][line].mode[0] = DISPMODE_256;
					address_shift[0] = 3; // FM-Towns Manual P.145
					to_disp[0] = true;
					address_mask[0] = 0x7ffff;
					break;
				case 0x0f:
					linebuffers[trans][line].mode[0] = DISPMODE_32768;
					to_disp[0] = true;
					address_shift[0] = 3; // FM-Towns Manual P.145
					address_mask[0] = 0x7ffff;
					break;
				default:
					linebuffers[trans][line].mode[0] = DISPMODE_NONE;
					to_disp[0] = false;
					break;
				}
			}
			if(l == 0) break;
		} else { // Two layer mode
			linebuffers[trans][line].num[0] = page0;
			linebuffers[trans][line].num[1] = page1;
			linebuffers[trans][line].crtout[page0] = (crtout_top[page0]) ? 0xff : 0x00;
			linebuffers[trans][line].crtout[page1] = (crtout_top[page1]) ? 0xff : 0x00;
			bool disp = frame_in[l];
			__UNLIKELY_IF((horiz_end_us[l] <= 0.0) || (horiz_end_us[l] <= horiz_start_us[l])) {
				disp = false;
			}
//			if(vert_offset_tmp[l] > line) {
//				disp = false;
//			}
//			out_debug_log("LAYER=%d CTRL_B=%02X DISP=%d START=%f END=%f", l, ctrl_b, disp, horiz_start_us[l], horiz_end_us[l]);
			__LIKELY_IF(disp) {
				switch(ctrl_b & 0x03) {
				case 0x01:
					linebuffers[trans][line].mode[l] = DISPMODE_16;
					to_disp[l] = true;
					address_shift[l] = 2; // FM-Towns Manual P.145
					break;
				case 0x03:
					linebuffers[trans][line].mode[l] = DISPMODE_32768;
					to_disp[l] = true;
					address_shift[l] = 2; // FM-Towns Manual P.145
					break;
				default:
					linebuffers[trans][line].mode[l] = DISPMODE_NONE;
					to_disp[l] = false;
					break;
				}
			}
			ctrl_b >>= 2;
		}
	}
	/*
	if(linebuffers[trans][line].crtout[0] == 0) {
		linebuffers[trans][line].mode[page0] = DISPMODE_NONE;
		to_disp[0] = false;
	}
	if(linebuffers[trans][line].crtout[1] == 0) {
		linebuffers[trans][line].mode[page1] = DISPMODE_NONE;
		to_disp[1] = false;
		}*/
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

	for(int l = 0; l < 2; l++) {
		__LIKELY_IF(to_disp[l]) {
			uint16_t _begin = regs[9 + l * 2] & 0x7ff; // HDSx
			uint16_t _end = regs[10 + l * 2] & 0x7ff;  // HDEx
			int ashift = address_shift[l];
			uint32_t shift_mask = (1 << ashift) - 1;

			int bit_shift = 0;
			uint16_t haj = hstart_words[l];
			// FAx
			int  offset = (int)vstart_addr[l]; // ToDo: Larger VRAM
			bit_shift = (int)haj - (int)_begin;
			if(horiz_us >= (1.0e6 / 16.6e3)) {
				// Maybe LOW RESOLUTION, Will fix.20201115 K.O
				bit_shift >>= 1;
			}
			offset = (offset + ((-bit_shift) >> ashift)) & (((ctrl & 0x10) == 0) ? 0x7ffff : 0x3ffff);
#if 0
			int _x = (int)max((unsigned int)_begin, (unsigned int)haj);
			int _y = (int)(regs[13 + l * 2] & 0x3ff); // VDSx
			switch(clksel) {
			case 0:
				_x = (_x - 0x129) >> 1;
				_y = (_y - 0x2a) >> 1;
				break;
			case 1:
				if((regs[4] & 0x3ff) == 0x31f) { // HST
					_x = _x - 0x8a;
				} else {
					_x = (_x - 0xe7) >> 1;
				}
				_y = (_y - 0x2a) >> 1;
				break;
			case 2:
				_x = _x - 0x8a;
				_y = (_y - 0x46) >> 1;
				break;
			case 3:
				if((regs[4] & 0x3ff) != 0x29d) { // HST
					_x = _x - 0x9c;
					_y = (_y - 0x40) >> 1;
				} else {
					_x = _x - 0x8a;
					_y = (_y - 0x46) >> 1;
				}
				break;
			default:
				_x = 0;
				_y = 0;
				break;
			}
			if(_x < 0) _x = 0;
			if(_y < 0) _y = 0;
#endif
			offset = offset + (int)head_address[l];
			offset <<= ashift;
//			bit_shift = _x >> ashift;
			if((trans & 1) != 0) offset = offset + (frame_offset_bak[l] <<= ashift);
			if(l == 1) {
				offset = offset + (fo1_offset_value <<= ashift);
			}
			if(linebuffers[trans][line].mode[l] == DISPMODE_16) {
				offset += ((page_16mode != 0) ? 0x20000 : 0);
			}
			if(l == 1) {
				if((is_sprite) && (linebuffers[trans][line].mode[l] == DISPMODE_32768)) {
					offset += sprite_offset;
				}
			}
			offset = offset & address_mask[l]; // OK?
			offset += address_add[l];
//			out_debug_log(_T("LINE=%d BEGIN=%d END=%d"), line, _begin, _end);
			__LIKELY_IF(_begin < _end) {
				int pixels = _end - _begin;
				uint8_t magx = zoom_factor_horiz[l];

				int npixels = pixels;
//				if((npixels % magx) != 0) npixels++;
				__LIKELY_IF((pixels >= magx) && (magx != 0)){
					if(bit_shift < 0) {
//						pixels += (-bit_shift * magx);
						bit_shift = 0;
					} else if(bit_shift > 0) {
						//pixels += (bit_shift * magx);
						bit_shift = 0;
					}
					__UNLIKELY_IF(pixels >= TOWNS_CRTC_MAX_PIXELS) pixels = TOWNS_CRTC_MAX_PIXELS;
					linebuffers[trans][line].bitshift[l] = bit_shift * magx;
					linebuffers[trans][line].pixels[l] = pixels;
					linebuffers[trans][line].mag[l] = magx; // ToDo: Real magnif
					if((pixels % magx) != 0) {
						npixels = pixels / magx + 2;
					} else {
						npixels = pixels / magx;
					}
//					out_debug_log(_T("LAYER=%d Y=%d NPIXELS=%d"), l, line, npixels);
					__UNLIKELY_IF(npixels >= TOWNS_CRTC_MAX_PIXELS) npixels = TOWNS_CRTC_MAX_PIXELS;
					bool is_256 = false;
					uint32_t tr_bytes = 0;
					switch(linebuffers[trans][line].mode[l]) {
					case DISPMODE_32768:
						tr_bytes = npixels << 1;
						break;
					case DISPMODE_16:
						tr_bytes = npixels >> 1;
						break;
					case DISPMODE_256:
						is_256 = true;
						tr_bytes = npixels;
						break;
					}
					if(is_256) {
						linebuffers[trans][line].pixels[0] = pixels;
						linebuffers[trans][line].mag[0] = magx; // ToDo: Real magnif
					}
					uint8_t *p = d_vram->get_vram_address(offset);
					__LIKELY_IF((tr_bytes > 0) && (p != nullptr)) {
						d_vram->lock();
						__UNLIKELY_IF(((offset & address_mask[l]) + tr_bytes) > address_mask[l]) {
							// Wrap
							int __left = (address_mask[l] + 1 - (offset & address_mask[l]));
							my_memcpy(&(linebuffers[trans][line].pixels_layer[(is_256) ? 0 : l][0])
								   , p
								   , __left);
							offset  = ((page_16mode != 0) ? 0x20000 : 0);
							offset += address_add[l];

							uint8_t *p2 = d_vram->get_vram_address(offset);
							my_memcpy(&(linebuffers[trans][line].pixels_layer[(is_256) ? 0 : l][__left])
								   , p2
								   , tr_bytes - __left);
						} else {
							my_memcpy(&(linebuffers[trans][line].pixels_layer[(is_256) ? 0 : l][0]), p, tr_bytes);
						}
						d_vram->unlock();

					}
					did_transfer[l] = true;
				}
			}
			if(zoom_count_vert[l] > 0) {
				zoom_count_vert[l]--;
			}
			if(zoom_count_vert[l] <= 0) {
				zoom_count_vert[l] = zoom_factor_vert[l];
				// ToDo: Interlace
				if(to_disp[l]) {
					head_address[l] += line_offset_bak[l];
				}
			}
		}
		__UNLIKELY_IF((l >= 0) && ((ctrl & 0x10) == 0)) break; // Single layers.
	}
}


void TOWNS_CRTC::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{

	max_lines = new_lines_per_frame;
	frames_per_sec = new_frames_per_sec;
	cpu_clocks = new_clocks;
	req_recalc = true;
}

void TOWNS_CRTC::event_pre_frame()
{
	interlace_field = !interlace_field;
	for(int i = 0; i < 2; i++) {
		crtout_top[i] = crtout[i];
		hdisp[i] = false;
		zoom_count_vert[i] = zoom_factor_vert[i];
	}
//	if(req_recalc) {
		force_recalc_crtc_param();
//	}
	int pb = pixels_per_line;
	int lb = lines_per_frame;
	if((voutreg_ctrl & 0x10) == 0) {
		// Single layer
		lines_per_frame = (int)(regs[14] & 0x07ff) - (int)(regs[13] & 0x07ff);
		if(lines_per_frame <= 1) lines_per_frame = 2;
		pixels_per_line = (int)(regs[10] & 0x07ff) - (int)(regs[9] & 0x07ff);
		if(pixels_per_line <= 8) pixels_per_line = 8;
		lines_per_frame >>= 1;
	} else {
		int l0 = (int)(regs[14] & 0x07ff) - (int)(regs[13] & 0x07ff);
		int l1 = (int)(regs[16] & 0x07ff) - (int)(regs[15] & 0x07ff);
		int w0 = (int)(regs[10] & 0x07ff) - (int)(regs[9] & 0x07ff);
		int w1 = (int)(regs[12] & 0x07ff) - (int)(regs[11] & 0x07ff);
		if(l0 > l1) {
			lines_per_frame = l0;
		} else {
			lines_per_frame = l1;
		}
		if(w0 > w1) {
			pixels_per_line = w0;
		} else {
			pixels_per_line = w1;
		}
		if(lines_per_frame <= 1) lines_per_frame = 1;
		if(pixels_per_line <= 8) pixels_per_line = 8;
		lines_per_frame >>= 1;
	}
	__UNLIKELY_IF(pixels_per_line >= TOWNS_CRTC_MAX_PIXELS) pixels_per_line = TOWNS_CRTC_MAX_PIXELS;
	__UNLIKELY_IF(lines_per_frame >= TOWNS_CRTC_MAX_LINES) lines_per_frame = TOWNS_CRTC_MAX_LINES;
	__UNLIKELY_IF(lb != lines_per_frame) {
		set_lines_per_frame(lines_per_frame);
	}
	for(int i = 0; i < 2; i++) {
		frame_offset_bak[i] = frame_offset[i];
		line_offset_bak[i] = line_offset[i];
	}
//	if(pb != pixels_per_line) {
		// ToDo: Resize texture.
		//set_pixels_per_line(pixels_per_line);
//	}
	// Render VRAM
	if(d_sprite->read_signal(SIG_TOWNS_SPRITE_TVRAM_ENABLED) != 0) {
		d_sprite->get_tvram_snapshot(tvram_snapshot);
		render_text();
	}
}

void TOWNS_CRTC::event_frame()
{
	display_linebuf = (display_linebuf + 1) & display_linebuf_mask; // Incremant per vstart
	hst[display_linebuf] = hst_tmp;
	vst[display_linebuf] = vst_tmp;
	lines_per_frame = max_lines;

	line_count[0] = line_count[1] = 0;
	vert_line_count = -1;
	hsync = false;

	//! @note at event_pre_frame() at SPRITE, clear VRAM if enabled.
	is_sprite = (d_sprite->read_signal(SIG_TOWNS_SPRITE_ENABLED) != 0) ? true : false;
	if(is_sprite) {
		sprite_offset = (d_sprite->read_signal(SIG_TOWNS_SPRITE_DISP_PAGE1) != 0) ? 0x20000 : 0x00000;
	} else {
		sprite_offset = 0x00000;
	}
	// ToDo: EET
	//register_event(this, EVENT_CRTC_VSTART, frame_us, false, &event_id_frame); // EVENT_VSTART MOVED TO event_frame().
	cancel_event_by_id(event_id_vst1);
	cancel_event_by_id(event_id_vst2);

	set_vsync(false, true);

	if(vst1_us > 0.0) {
		register_event(this, EVENT_CRTC_VST1, vst1_us, false, &event_id_vst1); // VST1
	}
	if(vst2_us > 0.0) {
		register_event(this, EVENT_CRTC_VST2, vst2_us, false, &event_id_vst2);
	} else if(vst1_us <= 0.0) {
		set_vsync(true, true);
		set_vsync(false, false);
	}

	for(int i = 0; i < 2; i++) {
		frame_in[i] = false;
		cancel_event_by_id(event_id_vds[i]);
		cancel_event_by_id(event_id_vde[i]);
		if(vert_start_us[i] > 0.0) {
			register_event(this, EVENT_CRTC_VDS + i, vert_start_us[i], false, &event_id_vds[i]); // VDSx
		}
		if(vert_end_us[i] > 0.0) {
			register_event(this, EVENT_CRTC_VDE + i, vert_end_us[i],   false, &event_id_vde[i]); // VDEx
		}
		head_address[i] = 0;
	}

	cancel_event_by_id(event_id_hstart);
	cancel_event_by_id(event_id_hsw);
	if(horiz_us > 0.0) {
		register_event(this, EVENT_CRTC_HSTART, horiz_us, false, &event_id_hstart); // HSTART
	}
}

bool TOWNS_CRTC::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}

bool TOWNS_CRTC::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == nullptr) return false;

	_TCHAR paramstr[2048] = {0};
	my_stprintf_s(paramstr, sizeof(paramstr) / sizeof(_TCHAR),
				  _T("\n")
				  _T("DISPLAY: %s / VCOUNT=%d / FRAMES PER SEC=%6g / FRAME uS=%6g / CLOCK=%6gMHz\n")
				  _T("LINES PER FRAME=%d / PIXELS PER LINE=%d / MAX LINE=%d\n")
				  _T("EET   uS=%6g /")
				  _T("VST1  uS=%6g / VST2  uS=%6g\n")
				  _T("HORIZ uS=%6g / POSI  uS=%6g / NEGA  uS=%6g\n")
				  _T("VERT  START uS [0]=%6g [1]=%6g / END   uS [0]=%6g [1]=%6g\n")
				  _T("HORIZ START uS [0]=%6g [1]=%6g / END   uS [0]=%6g [1]=%6g\n\n")
				  , (display_enabled) ? _T("ON ") : _T("OFF"), vert_line_count
				  , frames_per_sec, frame_us, 1.0 / crtc_clock
				  , lines_per_frame, pixels_per_line, max_lines
				  , eet_us
				  , vst1_us, vst2_us
				  , horiz_us, horiz_width_posi_us, horiz_width_nega_us
				  , vert_start_us[0], vert_start_us[1], vert_end_us[0], vert_end_us[1]
				  , horiz_start_us[0], horiz_start_us[1], horiz_end_us[0], horiz_end_us[1]
		);

	_TCHAR regstr[1024] = {0};
	my_stprintf_s(regstr, sizeof(regstr) / sizeof(_TCHAR),
				  _T("REGS:     +0     +1     +2    +3    +4    +5    +6    +7\n")
				  _T("------------------------------------------------------\n")
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
				  _T("ZOOM[0] V=%d H=%d VCOUNT=%d / ZOOM[1] V=%d H=%d VCOUNT=%d\n")
				  _T("VSYNC=%s / FRAME IN[0]=%s / [1]=%s\n")
				  _T("HSYNC=%s / HDISP[0]=%s / [1]=%s\n\n")
				  _T("%s")
				  _T("%s")
				  , paramstr
//				  , line_count[0], line_count[1]
				  , zoom_factor_vert[0], zoom_factor_horiz[0], zoom_count_vert[0]
				  , zoom_factor_vert[1], zoom_factor_horiz[1], zoom_count_vert[1]
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
		set_vsync(true, true);
		if(vst2_us <= 0.0) {
			set_vsync(false, false);
		}
		event_id_vst1 = -1;
	} else if (event_id == EVENT_CRTC_VST2) {
		if(vst1_us <= 0.0) {
			set_vsync(true, true);
		}
		set_vsync(false, false);
		event_id_vst2 = -1;
	} else if(eid2 == EVENT_CRTC_VDS) { // Display start
		int layer = event_id & 1;
		frame_in[layer] = true;
		// DO ofset line?
		event_id_vds[layer] = -1;
		zoom_count_vert[layer] = zoom_factor_vert[layer];
	} else if(eid2 == EVENT_CRTC_VDE) { // Display end
		int layer = event_id & 1;
		frame_in[layer] = false;
		event_id_vde[layer] = -1;

		// DO ofset line?
	} else if(event_id == EVENT_CRTC_HSTART) {
		// Do render
		event_id_hstart = -1;
		__LIKELY_IF(d_sprite != nullptr) {
			d_sprite->write_signal(SIG_TOWNS_SPRITE_HOOK_VLINE, vert_line_count, 0xffffffff);
		}
		if(frame_in[0] || frame_in[1]) {
			vert_line_count++;
			if(vert_line_count < lines_per_frame) {
				transfer_line(vert_line_count); // Tranfer before line.
			}
		}
		hdisp[0] = false;
		hdisp[1] = false;
		cancel_event_by_id(event_id_hsw);
		hsync = true;
		if(!vsync) {
			register_event(this, EVENT_CRTC_HSW, horiz_width_posi_us, false, &event_id_hsw); // VDEx
		} else {
			register_event(this, EVENT_CRTC_HSW, horiz_width_nega_us, false, &event_id_hsw); // VDEx
		}
		for(int i = 0; i < 2; i++) {
			cancel_event_by_id(event_id_hds[i]);
			cancel_event_by_id(event_id_hde[i]);

			if(horiz_start_us[i] > 0.0) {
				register_event(this, EVENT_CRTC_HDS + i, horiz_start_us[i], false, &event_id_hds[i]); // HDS0
			} else {
				hdisp[i] = true;
			}
			if((horiz_end_us[i] > 0.0) && (horiz_end_us[i] > horiz_start_us[i])) {
				register_event(this, EVENT_CRTC_HDE + i, horiz_end_us[i], false, &event_id_hde[i]); // HDS0
			} else {
				hdisp[i] = false;
			}
		}
		register_event(this, EVENT_CRTC_HSTART, horiz_us, false, &event_id_hstart); // HSTART
	} else if(event_id == EVENT_CRTC_HSW) {
		hsync = false;
		event_id_hsw = -1;
	} else if(eid2 == EVENT_CRTC_HDS) {
		int layer = event_id & 1;
		hdisp[layer] = true;
		if((horiz_end_us[layer] <= 0.0) || (horiz_end_us[layer] <= horiz_start_us[layer])) {
			hdisp[layer] = false;
		}
		event_id_hds[layer] = -1;
	} else if(eid2 == EVENT_CRTC_HDE) {
		int layer = event_id & 1;
		hdisp[layer] = false;
//		if(!(hdisp[0]) && !(hdisp[1])) hsync = false;
		event_id_hde[layer] = -1;
	}

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


#define STATE_VERSION	5

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
	state_fio->StateArray(display_mode, sizeof(display_mode), 1);

	state_fio->StateValue(display_enabled);
	state_fio->StateValue(crtc_clock);
	state_fio->StateValue(max_lines);
	state_fio->StateValue(frames_per_sec);
	state_fio->StateValue(cpu_clocks);

	state_fio->StateArray(vstart_addr, sizeof(vstart_addr), 1);
	state_fio->StateArray(hstart_words, sizeof(hstart_words), 1);
	state_fio->StateArray(hend_words, sizeof(hend_words), 1);

	state_fio->StateArray(frame_offset, sizeof(frame_offset), 1);
	state_fio->StateArray(line_offset, sizeof(line_offset), 1);

	state_fio->StateArray(frame_offset_bak, sizeof(frame_offset_bak), 1);
	state_fio->StateArray(line_offset_bak, sizeof(line_offset_bak), 1);

	state_fio->StateArray(head_address, sizeof(head_address), 1);
	state_fio->StateArray(impose_mode,  sizeof(impose_mode), 1);
	state_fio->StateArray(carry_enable, sizeof(carry_enable), 1);

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
	state_fio->StateArray(crtout_top, sizeof(crtout_top), 1);


	state_fio->StateValue(pixels_per_line);
	state_fio->StateValue(lines_per_frame);

	state_fio->StateValue(apalette_code);
	state_fio->StateValue(apalette_b);
	state_fio->StateValue(apalette_r);
	state_fio->StateValue(apalette_g);
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

	//state_fio->StateValue(display_linebuf);

	if(loading) {
		for(int i = 0; i < 16; i++) {
			calc_apalette16(0, i);
			calc_apalette16(1, i);
		}
		for(int i = 0; i < 256; i++) {
			calc_apalette256(i);
		}
		vst_tmp = 400;
		hst_tmp = 640;
		for(int i = 0; i < 4; i++) {
			for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
				memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
			}
			vst[i] = vst_tmp;
			hst[i] = hst_tmp;
		}
		display_linebuf = 0;
		req_recalc = false;
		force_recalc_crtc_param();
	}
	return true;
}
}
