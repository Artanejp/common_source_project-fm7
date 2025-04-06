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

#include "./crtc.h"
#include "./crtc/crtc_eventnum.h"

#include "./sprite.h"
#include "../debugger.h"

namespace FMTOWNS {

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
	for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i++) {
		memset(&(linebuffers[i][0]), 0x00, sizeof(linebuffer_t) * TOWNS_CRTC_MAX_LINES);
	}
	// register events

	register_frame_event(this);
	register_vline_event(this);
	video_out_regs[FMTOWNS::VOUTREG_CTRL] = 0x15;
	video_out_regs[FMTOWNS::VOUTREG_PRIO] = 0x00;
	video_out_regs[FMTOWNS::VOUTREG_2] = 0x00;
	video_out_regs[FMTOWNS::VOUTREG_3] = 0x00;
	for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i++) {
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
	frame_us = 1.0e6 / FRAMES_PER_SEC;

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
	req_update_cr1 = true;
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

	// Clear Linebuffer.
	for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i++) {
		memset(&(linebuffers[i][0]), 0x00, sizeof(linebuffer_t) * TOWNS_CRTC_MAX_LINES);
	}

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
		is_interlaced[i] = false;
	}
	
	for(int i = 0; i < 2; i++) {
		vstart_addr[i] = 0;
		head_address[i] = 0;
		voffset_val[i] = 0;
	}
	clear_event(this, event_hsync);
	for(int i = 0; i < 2; i++) {
		clear_event(this, event_hdisp[i]);
	}
	// Register vstart
	odd_field = false;
	begin_of_display();
	
	for(int layer = 0; layer < 2; layer++) {
		update_vstart(layer);	
		update_line_offset(layer);
	}
	for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i++) {
		hst[i] = pixels_per_line;
		vst[i] = max_lines;
		is_single_layer[i] = is_single_layer[0];
		update_control_registers(i);
	}
	// Odd and Even.
	for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i += 2) {
		for(int layer = 0; layer < 2; layer++) {
			this_layer_is_interlaced[i + 0][layer] = is_interlaced[layer];
			this_layer_is_interlaced[i + 1][layer] = is_interlaced[layer];
		}
	}

	for(int i = 0; i < 2; i++) {
		frame_offset_bak[i] = frame_offset[i];
	}

}

// I/Os
// Palette.
void TOWNS_CRTC::reset_vsync()
{
	if(vsync) {
		vsync = false;
		write_signals(&outputs_int_vsync, 0);
	}
}

void TOWNS_CRTC::set_vsync(bool val)
{
	bool vsync_bak = vsync;
	vsync = val;
	if(vsync_bak != val) {
		write_signals(&outputs_int_vsync, (val) ? 0x00000000 : 0xffffffff);
	}
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
			#if 1
			int trans = render_linebuf.load() & display_linebuf_mask;
			bool is_single_tmp = is_single_layer[trans];
			if((real_display_mode[1] == DISPMODE_32768) && !(is_single_tmp) && (line_offset[1] == 128)) {
				d_sprite->write_signal(SIG_TOWNS_SPRITE_VSYNC, 0xffffffff, 0xffffffff);
			}
			#endif
			#if 0
			else if((real_display_mode[1] == DISPMODE_16) && !(is_single_tmp) /*&& (line_offset[1] == 80)*/) {
				// OK?
				d_sprite->write_signal(SIG_TOWNS_SPRITE_TEXT_RENDER, 0xffffffff, 0xffffffff);

			}
			#endif
				
		}
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

void TOWNS_CRTC::begin_of_display()
{
	int trans = render_linebuf.load() & display_linebuf_mask;
	int trans_old = (trans - 1) & display_linebuf_mask;
	bool odd_field_new = ((trans & 1) != 0) ? true : false;

	// Update Display priorites.
	bool need_change_mode = (!(odd_field_new) || (!(is_interlaced[0]) && !(is_interlaced[1])));
	if(need_change_mode) {
		update_control_registers(trans);
		recalc_cr0(regs[TOWNS_CRTC_REG_DISPMODE], true);
		make_dispmode(is_single_layer[trans], real_display_mode[0], real_display_mode[1]);
	} else {
		priority_cache[trans] = priority_cache[trans_old];
		control_cache[trans] = control_cache[trans_old];
		make_dispmode(is_single_layer[trans_old], real_display_mode[0], real_display_mode[1]);
	}
	
	calc_zoom_regs(regs[TOWNS_CRTC_REG_ZOOM]);
	if(need_change_mode) {
		set_crtc_parameters_from_regs();
		for(int layer = 0; layer < 2; layer++) {
			horiz_start_us[layer] = horiz_start_us_next[layer];
			horiz_end_us[layer] = horiz_end_us_next[layer];
		}
	}
	
	vst[trans] = max_lines;
	hst[trans] = pixels_per_line;
	
	for(int i = 0; i < 2; i++) {
		zoom_count_vert[i] = zoom_factor_vert[i];
		frame_offset_bak[i] = frame_offset[i];
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

	if(!(odd_field_new)) { // Check interlace when even frame timing.
		for(int layer = 0; layer < 2; layer++) {
			is_interlaced[layer] = layer_is_interlaced(layer);
		}
	}
	for(int layer = 0; layer < 2; layer++) {
		this_layer_is_interlaced[trans][layer] = is_interlaced[layer];
	}
	
}

void TOWNS_CRTC::event_pre_frame()
{
	for(int i = 0; i < 2; i++) {
		hdisp[i] = false;
		frame_in[i] = false;
		head_address[i] = 0;
	}
//	display_linebuf = render_linebuf.load();
//	__LIKELY_IF(display_enabled) {
		render_linebuf++;
		render_linebuf &= display_linebuf_mask;
//	}

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
	reset_vsync(); // Force interrupt off.
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
//	display_enabled = display_enabled_pre;
//	__LIKELY_IF(display_enabled) {
//		render_linebuf++;
//		render_linebuf &= display_linebuf_mask;
//	}
//	begin_of_display();
	
	odd_field = ((render_linebuf.load() & 1) != 0) ? true : false;
	// Clear all frame buffer (of this) every turn.20230716 K.O
	horiz_width_posi_us = horiz_width_posi_us_next;
	horiz_width_nega_us = horiz_width_nega_us_next;
	
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
	clear_event(this, event_hsync);
	for(int layer = 0; layer < 2; layer++) {
		clear_event(this, event_hdisp[layer]);
	}
	// Rendering TEXT.
	sprite_offset = get_sprite_offset();
#if 1
	__LIKELY_IF(d_sprite != NULL) {
		int trans = render_linebuf.load() & display_linebuf_mask;
		bool is_single_tmp = is_single_layer[trans];
		if((real_display_mode[1] == DISPMODE_16) && !(is_single_tmp) /*&& (line_offset[1] == 80)*/) {
			// OK?
			d_sprite->write_signal(SIG_TOWNS_SPRITE_TEXT_RENDER, 0xffffffff, 0xffffffff);
			
		}
	}
#endif
}

void TOWNS_CRTC::event_vline(int v, int clock)
{
	__UNLIKELY_IF((v < 0)) {
		for(int i = 0; i < 2; i++) {
			frame_in[i] = false;
		}
		hsync = false;
		reset_vsync();
		display_linebuf = render_linebuf.load();
		return;
	}
	clear_event(this, event_hsync);

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
		__UNLIKELY_IF(v == vst2_count) {
			set_vsync(false);
			for(int i = 0; i < 2; i++) {
				// Need to update on vert offset
				update_regs_v(i);
			}
		}
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
		// Update vstart (by FAx) and line offset (by LOx) when frame_in[layer] has changed.
		// -- 20240314 K.O
		for(int i = 0; i < 2; i++) {
			__UNLIKELY_IF((fin_bak[i] != frame_in[i]) && (frame_in[i])) {
				update_vstart(i);
				update_line_offset(i);
				// Need to update on vert offset
				//recalc_hdisp_from_crtc_params(i, horiz_start_us[i], horiz_end_us[i]);
				
				#if 0
				if((d_sprite != NULL) && (i == 1)) {
					if((real_display_mode[1] == DISPMODE_32768) && !(is_single_tmp) && (line_offset[1] == 128)) {
						d_sprite->write_signal(SIG_TOWNS_SPRITE_VSYNC, 0xffffffff, 0xffffffff);
					} else
					if((real_display_mode[1] == DISPMODE_16) && !(is_single_tmp) /*&& (line_offset[1] == 80)*/) {
						// OK?
						d_sprite->write_signal(SIG_TOWNS_SPRITE_TEXT_RENDER, 0xffffffff, 0xffffffff);
						
					}
				}
				#endif
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
				display_linebuf = render_linebuf.load();
				reset_vsync(); // Auto interrupt off.
			}
		} else if((fin_bak[0]) || (fin_bak[1])) {
			// Last of line
			display_linebuf = render_linebuf.load();
			reset_vsync(); // Auto interrupt off.
		}
	}
	clear_event(this, event_hsync);
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
	const int trans = render_linebuf.load() & display_linebuf_mask;
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
				clear_event(this, event_hdisp[layer]);
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
		__UNLIKELY_IF((is_interlaced[0]) || (is_interlaced[1])) {
			return (odd_field) ? 0xffffffff : 0;
		}
		return 0; // Not Interlaced
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

#define STATE_VERSION	19

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
	state_fio->StateValue(req_update_cr1);
	
	state_fio->StateValue(lines_per_frame);
	state_fio->StateValue(max_lines);
	state_fio->StateValue(pixels_per_line);
	state_fio->StateValue(vert_line_count);

	state_fio->StateValue(odd_field);
	state_fio->StateValue(vsync);
	state_fio->StateValue(hsync);
	state_fio->StateArray(hdisp, sizeof(hdisp), 1);
	state_fio->StateArray(frame_in, sizeof(frame_in), 1);
	
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateArray(regs_written, sizeof(regs_written), 1);
	state_fio->StateValue(crtc_ch);


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

	int trans0[2];
	if(loading) {
		if(odd_field) {
			render_linebuf = 1;
			display_linebuf = 0;
			trans0[0] = 0;
			trans0[1] = 1;
		} else {
			render_linebuf = 0;
			display_linebuf = 1;
			trans0[0] = 1;
			trans0[1] = 0;
		}
	} else {
		trans0[1] = render_linebuf.load() & display_linebuf_mask;
		trans0[0] = (trans0[1] - 1) & display_linebuf_mask;;
	}
	bool is_single_layer_tmp = is_single_layer[trans0[1]];

	state_fio->StateValue(frame_us);
	state_fio->StateValue(is_single_layer_tmp);
	state_fio->StateArray(display_mode, sizeof(display_mode), 1);
	state_fio->StateArray(real_display_mode, sizeof(real_display_mode), 1);
	state_fio->StateArray(is_interlaced, sizeof(is_interlaced), 1);
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
	for(int _t = 0; _t < 2; _t++) {
		int trans = trans0[_t];
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
		for(int _t = 2; _t < FMTOWNS::CRTC_BUFFER_NUM; _t++) {
			// Copy buffers
			my_memcpy(&(linebuffers[_t][0]), &(linebuffers[_t - 2][0]), sizeof(linebuffer_t) * TOWNS_CRTC_MAX_LINES);
		}
		for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i++) {
			// Duplicate registers.
			update_control_registers(i);
		}

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
		
		for(int i = 0; i < FMTOWNS::CRTC_BUFFER_NUM; i++) {
			hst[i] = pixels_per_line;
			vst[i] = max_lines;
			is_single_layer[i] = is_single_layer_tmp;
			update_control_registers(i);
			for(int layer = 0; layer < 2; layer++) {
				this_layer_is_interlaced[i][layer] = false;
			}
		}
		for(int layer = 0; layer < 2; layer++) {
			this_layer_is_interlaced[0][layer] = is_interlaced[layer];
			this_layer_is_interlaced[1][layer] = is_interlaced[layer];
		}
	}
	return true;

}
}
