/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/
#include "../../common.h"

#include "towns_crtc.h"
#include "towns_vram.h"
#include "towns_sprite.h"

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
	event_id_vblank = -1;
	event_id_vstart = -1;
	event_id_hstart = -1;
	for(int i = 0; i < 2; i++) {
		event_id_vds[i] = -1;
		event_id_vde[i] = -1;
		event_id_hds[i] = -1;
		event_id_hde[i] = -1;
	}		
	for(int i = 0; i < 4; i++) {
		// ToDo: Allocate at external buffer (when using compute shaders).
		linebuffers[i] = (linebuffer_t *)malloc(sizeof(linebuffer_t ) * TOWNS_CRTC_MAX_LINES);
		if(linebuffers[i] != NULL) {
			for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
				memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
			}
		}
	}
	// register events
	set_lines_per_frame(512);
	//set_pixels_per_line(640);

	crtc_clock = 1.0e6 / 28.6363e6;
	set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	register_frame_event(this);
	voutreg_ctrl = 0x1f;
	voutreg_prio = 0x00;
	
}

void TOWNS_CRTC::release()
{
	for(int i = 0; i < 4; i++) {
		// ToDo: Allocate at external buffer (when using compute shaders).
		if(linebuffers[i] != NULL) free(linebuffers[i]);
	}
}

void TOWNS_CRTC::reset()
{
	// initialize
	display_enabled = false;
	vblank = vsync = hsync = true;
	
//	memset(regs, 0, sizeof(regs));
	crtc_ch = 0;
	
	// initial settings for 1st frame
	req_recalc = false;
	sprite_disp_page = 0; // OK?
	sprite_enabled = false;
//	crtc_clock = 28.6363e6; // OK?
	interlace_field = false;
	
	line_count[0] = line_count[1] = 0;
	vert_line_count = -1;
	display_linebuf = 0;
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
	for(int i = 0; i < 4; i++) {
		frame_offset[i] = 0;
		line_offset[i] = 640;
		vstart_addr[i] = 0;
		hstart_words[i] = 0;
		head_address[i] = 0;
	}
	if(event_id_hsync  >= 0) cancel_event(this, event_id_hsync);
	if(event_id_hsw    >= 0) cancel_event(this, event_id_hsw);
	if(event_id_vsync  >= 0) cancel_event(this, event_id_vsync);
	if(event_id_vstart >= 0) cancel_event(this, event_id_vstart);
	if(event_id_vst1   >= 0) cancel_event(this, event_id_vst1);
	if(event_id_vst2   >= 0) cancel_event(this, event_id_vst2);
	if(event_id_vblank >= 0) cancel_event(this, event_id_vblank);
	if(event_id_hstart >= 0) cancel_event(this, event_id_hstart);

	for(int i = 0; i < 2; i++) {
		if(event_id_vds[i] >= 0) cancel_event(this, event_id_vds[i]);
		if(event_id_hds[i] >= 0) cancel_event(this, event_id_hds[i]);
		if(event_id_vde[i] >= 0) cancel_event(this, event_id_vde[i]);
		if(event_id_hde[i] >= 0) cancel_event(this, event_id_hde[i]);
	}
	event_id_hsw = -1;
	event_id_vsync = -1;
	event_id_vstart = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vblank = -1;
	event_id_hstart = -1;
	for(int i = 0; i < 2; i++) {
		event_id_vds[i] = -1;
		event_id_hds[i] = -1;
		event_id_vde[i] = -1;
		event_id_hde[i] = -1;
	}

	// Register vstart
	pixels_per_line = 640;
	lines_per_frame = 400;
	set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	set_lines_per_frame(512);
	//set_pixels_per_line(640);
	
	set_vsync(vsync, true);
	// For DEBUG Only
	// Mode 19
#if 1
	const uint16_t reg_default[] =
		{ 0x0060, 0x02c0, 0x0000, 0x0000, 0x031f, 0x0000, 0x0004, 0x0000,
		  0x0419, 0x008a, 0x018a, 0x008a, 0x030a, 0x0046, 0x0246, 0x0046,
		  0x0406, 0x0000, 0x008a, 0x0000, 0x0080, 0x0000, 0x008a, 0x0000,
		  0x0080, 0x0058, 0x0001, 0x0000, 0x000d, 0x0002, 0x0000, 0x0192};
	for(int i = 0; i < 32; i++) {
		write_io16(0x0440, i);
		write_io16(0x0442, reg_default[i]);
	}
	write_io16(0x0440, 0);
#else
	force_recalc_crtc_param();
#endif
//	register_event(this, EVENT_CRTC_VSTART, vstart_us, false, &event_id_vstart);
}

void TOWNS_CRTC::set_vsync(bool val, bool force)
{
	if((vsync != val) || (force)) {
		write_signals(&outputs_int_vsync, (val) ? 0xffffffff : 0x00000000);
		vsync = val;
	}
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

// CRTC register #29
void TOWNS_CRTC::set_crtc_clock(uint16_t val)
{
	scsel = (val & 0x0c) >> 2;
	clksel = val & 0x03;
	static const double clocks[] = {
		28.6363e6, 24.5454e6, 25.175e6, 21.0525e6
	};
	if(clocks[clksel] != crtc_clock) {
		crtc_clock = 1.0e6 / clocks[clksel];
		req_recalc = true;
	}
}

void TOWNS_CRTC::force_recalc_crtc_param(void)
{
	horiz_width_posi_us = crtc_clock * ((double)(regs[0] & 0x00fe)); // HSW1
	horiz_width_nega_us = crtc_clock * ((double)(regs[1] & 0x00fe)); // HSW2
	horiz_us = crtc_clock * ((double)((regs[4] & 0x07fe) + 1)); // HST
	vst1_us = ((double)(regs[5] & 0x1f)) * horiz_us; // VST1
	
	double horiz_ref = horiz_us / 2.0;
	
	vst2_us = ((double)(regs[6] & 0x1f)) * horiz_ref; // VST2
	eet_us = ((double)(regs[7] & 0x1f)) * horiz_ref;  // EET
	frame_us = ((double)(regs[8] & 0x07ff)) * horiz_ref; // VST
	if(frame_us > 0.0) {
		set_frames_per_sec(1.0e6 / frame_us); // Its dummy.
	} else {
		frame_us = 1.0e6 / FRAMES_PER_SEC;
		set_frames_per_sec(FRAMES_PER_SEC); // Its dummy.
	}		
//	out_debug_log(_T("RECALC PARAM: horiz_us=%f frame_us=%f"), horiz_us, frame_us);
	for(int layer = 0; layer < 2; layer++) {
		vert_start_us[layer] =  ((double)(regs[(layer << 1) + 13] & 0x07ff)) * horiz_ref;   // VDSx
		vert_end_us[layer] =    ((double)(regs[(layer << 1) + 13 + 1] & 0x07ff)) * horiz_ref; // VDEx
		horiz_start_us[layer] = ((double)(regs[(layer << 1) + 9] & 0x07ff)) * crtc_clock ;   // HDSx
		horiz_end_us[layer] =   ((double)(regs[(layer << 1) + 9 + 1] & 0x07ff)) * crtc_clock ;   // HDEx
	}
#if 1
	out_debug_log(_T("RECALC: CRTC_CLOCK=%f MHz FPS=%f"), 1.0 / crtc_clock, 1.0e6 / frame_us);
	_TCHAR sdata[32 * 5];
	_TCHAR sdata2[8];
	for(int q = 0; q < 4; q++) {
		memset(sdata, 0x00, sizeof(sdata));
		for(int r = 0; r < 8; r++) {
			my_sprintf_s(sdata2, 8, "%04X ", regs[r + q * 8]);
			my_tcscat_s(sdata, sizeof(sdata) / sizeof(_TCHAR), sdata2);
		}
		out_debug_log(_T("RECALC: regs[%02d..%02d]= %s"), q * 8, q * 8 + 7, sdata);
	}
	out_debug_log(_T("RECALC: HORIZ_us=%f HORIZ_WIDTH_P_us=%f HORIZ_WIDTH_N_us=%f"), horiz_us, horiz_width_posi_us, horiz_width_nega_us);
	out_debug_log(_T("RECALC: VST1_us=%f VST2_us=%f EET_us=%f frame_us=%f"), vst1_us, vst2_us, eet_us, frame_us);
	for(int q = 0; q < 2; q++) {
		out_debug_log(_T("RECALC: LAYER%d: VERT_START_us=%f VERT_END_us=%f HORIZ_START_us=%f HORIZ_END_us=%f"), q, vert_start_us[q], vert_end_us[q], horiz_start_us[q], horiz_end_us[q]);
	}
#endif
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
	data |= ((frame_in[1])   ?  0x8000 : 0);
	data |= ((frame_in[0])   ?  0x4000 : 0);
	data |= ((hdisp[1])      ?  0x2000 : 0);
	data |= ((hdisp[0])      ?  0x1000 : 0);
	//data |= ((eet)          ?  0x0800 : 0);
	data |= ((vsync)         ?  0x0400 : 0);
	data |= (!(hsync)        ?  0x0200 : 0);
	//data |= ((video_in)     ? 0x0100 : 0);
	//data |= ((half_tone)    ? 0x0008 : 0);
	//data |= ((sync_enable)  ? 0x0004 : 0);
	//data |= ((vcard_enable) ? 0x0002 : 0);
	//data |= ((sub_carry)    ? 0x0001 : 0);
	return data;
}

uint32_t TOWNS_CRTC::read_io16(uint32_t addr)
{
//	out_debug_log(_T("READ16 ADDR=%04x"), addr);
	switch(addr & 0xfffe) {
	case 0x0442:
		if(crtc_ch == 30) {
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
	case 0x044c:
		{
			uint16_t d = 0x7c;
			d = d | ((dpalette_changed) ? 0x80 : 0x00);
			if(d_sprite != NULL) {
				d = d | ((d_sprite->read_signal(SIG_TOWNS_SPRITE_BUSY) != 0) ? 0x02 : 0x00);
			}
			d = d | ((sprite_disp_page != 0) ? 0x01 : 0x00);
			dpalette_changed = false;
			return d;
		}
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
			uint8_t d = 0xfc;
			d = d | ((vsync) ? 0x01 : 0);
			d = d | ((hsync) ? 0x02 : 0);
			return d;
		}
		break;
	} 
	return 0xff;
}

bool TOWNS_CRTC::render_32768(scrntype_t* dst, scrntype_t *mask, int y, int width, int layer, bool do_alpha)
{
//	out_debug_log("RENDER_32768 Y=%d LAYER=%d WIDTH=%d DST=%08X MASK=%08X ALPHA=%d", y, layer, width,dst, mask, do_alpha);
	if(dst == NULL) return false;
	
	int trans = display_linebuf & 3;
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	int num = linebuffers[trans][y].num[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = mask;
	if(magx < 1) return false;
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) != 0) {
			pwidth++;
		}
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
		for(int i = 0; i < 8; i++) {
//			ptmp16.read_2bytes_le_from(p);
			ptmp16.b.l = *p++;
			ptmp16.b.h = *p++;
			pbuf[i] = ptmp16.w;
//			p += 2;
		}
		for(int i = 0; i < 8; i++) {
			rbuf[i] = pbuf[i];
			gbuf[i] = pbuf[i];
			bbuf[i] = pbuf[i];
		}			
		for(int i = 0; i < 8; i++) {
			rbuf[i] = rbuf[i] >> 5;
			gbuf[i] = gbuf[i] >> 10;
		}			
		for(int i = 0; i < 8; i++) {
			rbuf[i] = rbuf[i] & 0x1f;
			gbuf[i] = gbuf[i] & 0x1f;
			bbuf[i] = bbuf[i] & 0x1f;
		}
		for(int i = 0; i < 8; i++) {
			rbuf[i] <<= 3;
			gbuf[i] <<= 3;
			bbuf[i] <<= 3;
		}
		if(do_alpha) {
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
		if(magx == 1) {
			for(int i = 0; i < 8; i++) {
				*q++ = sbuf[i];
			}
			if(r2 != NULL) {
				for(int i = 0; i < rwidth; i++) {
					*r2++ = abuf[i];
				}
			}
			k += 8;
			if(k >= width) break;
		} else {
			for(int i = 0; i < 8; i++) {
				int kbak = k;
				for(int j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					k++;
					if(k >= width) break;
				}
				if(r2 != NULL) {
					for(int j = 0; j < magx; j++) {
						*r2++ = abuf[i];
						kbak++;
						if(kbak >= width) break;
					}
				}
				if(k >= width) break;
			}
		}
	}
	if(k >= width) return true;
	
	if((pwidth & 7) != 0) {
		for(int i = 0; i < rwidth; i++) {
			ptmp16.read_2bytes_le_from(p);
			pbuf[i] = ptmp16.w;
			p += 2;
		}
		for(int i = 0; i < rwidth; i++) {
			rbuf[i] = pbuf[i];
			gbuf[i] = pbuf[i];
			bbuf[i] = pbuf[i];
		}			
		for(int i = 0; i < rwidth; i++) {
			rbuf[i] = (rbuf[i] >> 5) & 0x1f;
			gbuf[i] = (gbuf[i] >> 10) & 0x1f;
			bbuf[i] = bbuf[i] & 0x1f;
		}
		for(int i = 0; i < rwidth; i++) {
			rbuf[i] <<= 3;
			gbuf[i] <<= 3;
			bbuf[i] <<= 3;
		}
		if(do_alpha) {
			for(int i = 0; i < rwidth; i++) {
				a2buf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
			}
			for(int i = 0; i < rwidth; i++) {
				sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], a2buf[i]);
			}
		} else {
			for(int i = 0; i < rwidth; i++) {
				abuf[i] = (pbuf[i] & 0x8000) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
			}
			for(int i = 0; i < rwidth; i++) {
				sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], 255);
			}
		}			
		if(magx == 1) {
			for(int i = 0; i < rwidth; i++) {
				*q++ = sbuf[i];
			}
			if(r2 != NULL) {
				for(int i = 0; i < rwidth; i++) {
					*r2++ = abuf[i];
				}
			}
			k += 8;
			if(k >= width) return true;
		} else {
			for(int i = 0; i < rwidth; i++) {
				for(int j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					if(r2 != NULL) {
						*r2++ = abuf[i];
					}
					k++;
					if(k >= width) break;
				}
				if(k >= width) return true;
			}
		}
	}
	return true;
}

bool TOWNS_CRTC::render_256(scrntype_t* dst, int y, int width)
{
	// 256 colors
	if(dst == NULL) return false;
	int trans = display_linebuf & 3;
	int magx = linebuffers[trans]->mag[0];
	int pwidth = linebuffers[trans]->pixels[0];
	int num = linebuffers[trans]->num[0];
	uint8_t *p = linebuffers[trans]->pixels_layer[0];
	__DECL_ALIGNED(32)  scrntype_t apal256[256];
	
	d_vram->get_analog_palette(2, apal256);
	
	__DECL_ALIGNED(16) uint8_t pbuf[16];
	__DECL_ALIGNED(32) scrntype_t sbuf[16];
	if(magx < 1) {
		return false;;
	}
	if(magx == 1) {
		if(pwidth > width) pwidth = width;
		if(pwidth < 1) pwidth = 1;
		for(int x = 0; x < (pwidth >> 4); x++) {
			// ToDo: Start position
			for(int i = 0; i < 16; i++) {
				pbuf[i] = *p++;
			}
			int xx = x << 4;
			for(int i = 0; i < 16; i++) {
				dst[xx++] = apal256[pbuf[i]];
			}
		}
		if((pwidth & 15) != 0) {
			int xx = pwidth & ~15;
			int w = pwidth & 15;
			for(int i = 0; i < w; i++) {
				pbuf[i] = *p++;
			}
			for(int i = 0; i < w; i++) {
				dst[xx++] = apal256[pbuf[i]];
			}
		}
	} else {
		if((pwidth * magx) > width) {
			pwidth = width / magx;
			if((width % magx) != 0) pwidth++;
		}
		int k = 0;
		for(int x = 0; x < (pwidth >> 4); x++) {
			// ToDo: Start position
			for(int i = 0; i < 16; i++) {
				pbuf[i] = *p++;
			}
			for(int i = 0; i < 16; i++) {
				sbuf[i] = apal256[pbuf[i]];
			}
			for(int i = 0; i < 16; i++) {
				scrntype_t s = sbuf[i];
				for(int j = 0; j < magx; j++) {
					lbuffer0[k++] = s;
				}
			}
		}
		if((pwidth & 15) != 0) {
			for(int i = 0; i < (pwidth & 15); i++) {
				pbuf[i] = *p++;
			}
			for(int i = 0; i < (pwidth & 15); i++) {
				sbuf[i] = apal256[pbuf[i]];
			}
			for(int i = 0; i < (pwidth & 15); i++) {
				scrntype_t s = sbuf[i];
				for(int j = 0; j < magx; j++) {
					lbuffer0[k++] = s;
					if(k >= width) break;
				}
				if(k > width) break;
			}
		}
	}
	return true;
}
bool TOWNS_CRTC::render_16(scrntype_t* dst, scrntype_t *mask, scrntype_t* pal, int y, int width, int layer, bool do_alpha)
{
	if(dst == NULL) return false;
	
	int trans = display_linebuf & 3;
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	//int num = linebuffers[trans][y].num[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r2 = mask;

	if(magx < 1) return false;
	if((pwidth * magx) > width) {
		pwidth = width / magx;
		if((width % magx) != 0) {
			pwidth++;
		}
	}
	__DECL_ALIGNED(16) uint8_t pbuf[8];
	__DECL_ALIGNED(16) uint8_t hlbuf[16];
	__DECL_ALIGNED(32) scrntype_t sbuf[16];
	__DECL_ALIGNED(32) scrntype_t abuf[16];
	__DECL_ALIGNED(32) scrntype_t a2buf[16];
	scrntype_t __DECL_ALIGNED(32)  palbuf[16];
	const scrntype_t amask2 = RGBA_COLOR(255, 255, 255, 0);
	
	if(pal == NULL) {
		for(int i = 0; i < 16; i++) {
			uint8_t r, g,b;
			r = ((i & 2) != 0) ? (((i & 8) != 0) ? 255 : 128) : 0;
			g = ((i & 4) != 0) ? (((i & 8) != 0) ? 255 : 128) : 0;
			b = ((i & 1) != 0) ? (((i & 8) != 0) ? 255 : 128) : 0;
			palbuf[i] = RGBA_COLOR(r, g, b, 255);
		}
	} else {
		for(int i = 0; i < 16; i++) {
			palbuf[i] = pal[i];
		}
	}
	palbuf[0] = RGBA_COLOR(0, 0, 0, 0);
	int k = 0;
	for(int x = 0; x < (pwidth >> 3); x++) {
		for(int i = 0; i < 8; i++) {
			pbuf[i] = *p++;
		}
		for(int i = 0; i < 16; i += 2) {
			hlbuf[i] = pbuf[i >> 1];
		}			
		for(int i = 0; i < 16; i += 2) {
			hlbuf[i + 1] = hlbuf[i];
		}			
		for(int i = 0; i < 16; i += 2) {
			hlbuf[i] >>= 4;
		}
		for(int i = 0; i < 16; i++) {
			hlbuf[i] &= 15;
		}

		if(do_alpha) {
			for(int i = 0; i < 16; i++) {
				sbuf[i] = palbuf[hlbuf[i]];
			}
		} else {
			for(int i = 0; i < 16; i++) {
				abuf[i] = (hlbuf[i] == 0) ? RGBA_COLOR(0, 0, 0, 0): RGBA_COLOR(255, 255, 255, 255);
				sbuf[i] = palbuf[hlbuf[i]];
			}
		}
		
		if(do_alpha) {
			if(magx == 1) {
				for(int i = 0; i < 16; i++) {
					*q++ = sbuf[i];
				}
				k += 16;
				if(k >= width) break;
			} else {
				for(int i = 0; i < 16; i++) {
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[i];
						k++;
						if(k >= width) break;
					}
				}
				if(k >= width) break;
			}
		} else {
			if(magx == 1) {
				for(int i = 0; i < 16; i++) {
					*q++ = sbuf[i];
				}
				if(r2 != NULL) {
					for(int i = 0; i < 16; i++) {
						*r2++ = abuf[i];
					}
				}
				k += 16;
				if(k >= width) break;
			} else {
				for(int i = 0; i < 16; i++) {
					int kbak = k;
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[i];
						k++;
						if(k >= width) break;
					}
					if(r2 != NULL) {
						for(int j = 0; j < magx; j++) {
							*r2++ = abuf[i];
							kbak++;
							if(kbak >= width) break;
						}
					}
					if(k >= width) break;
				}
			}
		}
	}
	if(k >= width) return true;
	uint8_t tmpp;
	uint8_t tmph;
	uint8_t tmpl;
	scrntype_t ah, al;
	int rwidth = pwidth & 7;
	if(rwidth > 0) {
		for(int x = 0; x < rwidth; x++) {
			tmpp = *p++;
			tmph = tmpp >> 4;
			tmpl = tmpp & 0x0f;
			sbuf[0] = palbuf[tmph];
			sbuf[1] = palbuf[tmpl];

			if(do_alpha) {
				if(magx == 1) {
					*q++ = sbuf[0];
					k++;
					if(k >= width) break;
					*q++ = sbuf[1];
					k++;
					if(k >= width) break;
				} else {
					for(int xx = 0; xx < magx; xx++) {
						*q++ = sbuf[0];
						k++;
						if(k >= width) break;
					}
					if(k >= width) break;
					for(int xx = 0; xx < magx; xx++) {
						*q++ = sbuf[1];
						k++;
						if(k >= width) break;
					}
					if(k >= width) break;
				}					
			} else {				
				ah = (tmph == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
				al = (tmpl == 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
				if(magx == 1) {
					*q++ = sbuf[0];
					if(r2 != NULL) {
						*r2++ = ah;
					}
					k++;
					if(k >= width) break;
					*q++ = sbuf[1];
					if(r2 != NULL) {
						*r2++ = al;
					}
					k++;
					if(k >= width) break;
				} else {
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[0];
						if(r2 != NULL) {
							*r2++ = ah;
						}
						k++;
						if(k >= width) break;
					}
					for(int j = 0; j < magx; j++) {
						*q++ = sbuf[1];
						if(r2 != NULL) {
							*r2++ = al;
						}
						k++;
						if(k >= width) break;
					}
					if(k >= width) break;
				}
			}
		}
	}
	return true;
}

// This function does alpha-blending.
// If CSP support hardware-accelalations, will support.
// (i.e: Hardware Alpha blending, Hardware rendaring...)
void TOWNS_CRTC::mix_screen(int y, int width, bool do_mix0, bool do_mix1)
{
	if(width >= TOWNS_CRTC_MAX_PIXELS) return;
	if(width <= 0) return;
	
	scrntype_t *pp = emu->get_screen_buffer(y);
	if(pp != NULL) {
		if((do_mix0) && (do_mix1)) {
			// alpha blending
			__DECL_ALIGNED(32) scrntype_t pixbuf0[8];
			__DECL_ALIGNED(32) scrntype_t pixbuf1[8];
			__DECL_ALIGNED(32) scrntype_t maskbuf_front[8];
			__DECL_ALIGNED(32) scrntype_t maskbuf_back[8];
			for(int xx = 0; xx < width; xx += 8) {
				scrntype_t *px1 = &(lbuffer1[xx]);
				scrntype_t *ax = &(abuffer0[xx]);
				for(int ii = 0; ii < 8; ii++) {
					pixbuf1[ii] = px1[ii];
					maskbuf_front[ii] = ax[ii];
					maskbuf_back[ii] = ~maskbuf_front[ii];
				}
				scrntype_t *px0 = &(lbuffer0[xx]);
				
				for(int ii = 0; ii < 8; ii++) {
					pixbuf0[ii] = px0[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					pixbuf1[ii] = pixbuf1[ii] & maskbuf_back[ii];
					pixbuf0[ii] = pixbuf0[ii] & maskbuf_front[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					pixbuf0[ii] = pixbuf0[ii] | pixbuf1[ii];
				}
				for(int ii = 0; ii < 8; ii++) {
					*pp++ = pixbuf0[ii];
				}
			}
			scrntype_t pix0, pix1, mask0, mask1;
			int xptr = width & 0x7f8; // Maximum 2048 pixs
			for(int ii = 0; ii < (width & 7); ii++) {
				pix0 = lbuffer0[ii + xptr];
				pix1 = lbuffer1[ii + xptr];
				mask0 = abuffer0[ii + xptr];
				mask1 = ~mask0;
				pix0 = pix0 & mask0;
				pix1 = pix1 & mask1;
				pix0 = pix0 | pix1;
				*pp++ = pix0;
			}
		} else if(do_mix0) {
			my_memcpy(pp, lbuffer0, width * sizeof(scrntype_t));
		} else if(do_mix1) {
			my_memcpy(pp, lbuffer1, width * sizeof(scrntype_t));
		} else {
			memset(pp, 0x00, width * sizeof(scrntype_t));
		}
	}
}

void TOWNS_CRTC::draw_screen()
{
	int trans = (display_linebuf == 0) ? 3 : ((display_linebuf - 1) & 3);
	bool do_alpha = false; // ToDo: Hardware alpha rendaring.
	if((linebuffers[trans] == NULL) || (d_vram == NULL)) {
		//display_linebuf = (display_linebuf + 1) & 3;
		return;
	}
	int lines = lines_per_frame;
	int width = pixels_per_line;
	// Will remove.
	if(lines <= 0) lines = 1;
	if(width <= 16) width = 16;
	
	if(lines > TOWNS_CRTC_MAX_LINES) lines = TOWNS_CRTC_MAX_LINES;
	if(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	
	memset(lbuffer1, 0x00, sizeof(lbuffer1));
	memset(abuffer1, 0xff, sizeof(abuffer1));
	memset(lbuffer0, 0x00, sizeof(lbuffer0));
	memset(abuffer0, 0xff, sizeof(abuffer0));
	
	for(int y = 0; y < lines; y++) {
		bool do_mix0 = false;
		bool do_mix1 = false;
		if(linebuffers[trans]->mode[0] == DISPMODE_256) {
			do_mix0 = render_256(lbuffer0, y, width);
			do_mix1 = false;
		} else {
			__DECL_ALIGNED(32)  scrntype_t apal16[2][16];
			d_vram->get_analog_palette(0, &(apal16[0][0]));
			d_vram->get_analog_palette(1, &(apal16[1][0]));
			if(linebuffers[trans]->mode[1] == DISPMODE_16) { // Lower layer
				do_mix1 = render_16(lbuffer1, abuffer1, &(apal16[linebuffers[trans]->num[1]][0]), y, width, 1, do_alpha);
			} else if(linebuffers[trans]->mode[1] == DISPMODE_32768) { // Lower layer
				do_mix1 = render_32768(lbuffer1, abuffer1, y, width, 1, do_alpha);
			}
			// Upper layer
			if(linebuffers[trans]->mode[0] == DISPMODE_16) { 
				do_mix0 = render_16(lbuffer0, abuffer0, &(apal16[linebuffers[trans]->num[0]][0]), y, width, 0, do_alpha);
			} else if(linebuffers[trans]->mode[0] == DISPMODE_32768) { 
				do_mix0 = render_32768(lbuffer0, abuffer0, y, width, 0, do_alpha);
			}
		}
		mix_screen(y, width, do_mix0, do_mix1);
	}
	
	//display_linebuf = (display_linebuf + 1) & 3;
	return;
}
	
void TOWNS_CRTC::transfer_line(int line)
{
	if(line < 0) return;
	if(line >= TOWNS_CRTC_MAX_LINES) return;
	if(d_vram == NULL) return;
	uint8_t ctrl, prio;
	ctrl = voutreg_ctrl;
	prio = voutreg_prio;
	
	//int trans = (display_linebuf - 1) & 3;
	int trans = display_linebuf & 3;
	if(linebuffers[trans] == NULL) return;
	for(int i = 0; i < 4; i++) {
		linebuffers[trans][line].mode[i] = 0;
		linebuffers[trans][line].pixels[i] = 0;
		linebuffers[trans][line].mag[i] = 0;
		linebuffers[trans][line].num[i] = -1;
	}

	int page0, page1;
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
	if((ctrl & 0x10) == 0) { // One layer mode
		bool to_disp = false;
		linebuffers[trans][line].num[0] = 0;
		if(!(frame_in[0])) return;
		int line_shift = 1;
		if((horiz_end_us[0] <= 0.0) || (horiz_end_us[0] <= horiz_start_us[0])) return;
		switch(ctrl & 0x0f) {
		case 0x0a:
			linebuffers[trans][line].mode[0] = DISPMODE_256;
			to_disp = true;
			break;
		case 0x0f:
			linebuffers[trans][line].mode[0] = DISPMODE_32768;
			to_disp = true;
			break;
		}
		if(to_disp) {
			// ToDo: Sprite mode.
			uint32_t offset = vstart_addr[0];
			offset = offset + head_address[0];
			if(hstart_words[0] >= regs[9]) {
				offset = offset + hstart_words[0] - regs[9];
			}
			offset <<= 3;
			offset = offset & 0x7ffff; // OK?
			// ToDo: HAJ0, LO0
			uint16_t _begin = regs[9]; // HDS0
			uint16_t _end = regs[10];  // HDE0
			if(_begin < _end) {
				int words = _end - _begin;
				if(hstart_words[0] >= regs[9]) {
					words = words - (hstart_words[0] - regs[9]);
				}
				uint8_t magx = zoom_factor_horiz[0];
				uint8_t *p = d_vram->get_vram_address(offset);
//				words <<= 1;
				if((p != NULL) && (words > magx) && (magx != 0)){
//					out_debug_log("LINE=%d HEAD=%08X LINE_OFFSET=%d VRAM=%08X words=%d magx=%d", line, head_address[0], line_offset[0], p, words, magx);
					if(words >= TOWNS_CRTC_MAX_PIXELS) words = TOWNS_CRTC_MAX_PIXELS;
					switch(linebuffers[trans][line].mode[0]) {
					case DISPMODE_32768:
						// OK?
//						if(words > (line_offset[0] << 1)) words = line_offset[0] << 1;
						linebuffers[trans][line].pixels[0] = words;
						linebuffers[trans][line].mag[0] = magx;  // ToDo: Real magnify
						line_shift = 3;
						memcpy(linebuffers[trans][line].pixels_layer[0], p, words << 1);
						did_transfer[0] = true;
						break;
					case DISPMODE_256:
						linebuffers[trans][line].pixels[0] = words;
						linebuffers[trans][line].mag[0] = magx;
						line_shift = 2;
						memcpy(linebuffers[trans][line].pixels_layer[0], p, words);
						did_transfer[0] = true;
						break;
					}
					
				}
			}
		}
		memset(linebuffers[trans][line].pixels_layer[1], 0x00, TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t));
		if(!(did_transfer[0])) {
			memset(linebuffers[trans][line].pixels_layer[0], 0x00, TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t));
		}
		if(zoom_count_vert[0] > 0) {
			zoom_count_vert[0]--;
		}
		if(zoom_count_vert[0] <= 0) {
			zoom_count_vert[0] = zoom_factor_vert[0];
			// ToDo: Interlace
			if(to_disp) {
				head_address[0] += (line_offset[0] << line_shift);
			}
		}
	} else { // Two layers.
		bool to_disp[2] = {false, false};
		uint8_t ctrl_b = ctrl;
		linebuffers[trans][line].num[0] = page0;
		linebuffers[trans][line].num[1] = page1;
//		out_debug_log("2LAYER MODE: page0/1 = %d/%d FRAMEIN=%d/%d", page0, page1, frame_in[0], frame_in[1]);
		// ToDo: Sprite mode.
		for(int l = 0; l < 2; l++) {
			bool disp = frame_in[l];
			if((horiz_end_us[l] <= 0.0) || (horiz_end_us[l] <= horiz_start_us[l])) {
				disp = false;
			}
//			out_debug_log("LAYER=%d CTRL_B=%02X DISP=%d START=%f END=%f", l, ctrl_b, disp, horiz_start_us[l], horiz_end_us[l]);
			if(disp) {
				switch(ctrl_b & 0x03) {
				case 0x01:
					linebuffers[trans][line].mode[l] = DISPMODE_16;
					to_disp[l] = true;
					break;
				case 0x03: 
					linebuffers[trans][line].mode[l] = DISPMODE_32768;
					to_disp[l] = true;
					break;
				}
			}
			ctrl_b >>= 2;
//			linebuffers[trans][line].mode[l] = DISPMODE_32768;
		}
		int line_shift;
		for(int l = 0; l < 2; l++) {
			if(to_disp[l]) {
				line_shift = 1;
				uint32_t offset = vstart_addr[l];
				offset = offset + head_address[l];
				if(hstart_words[l] >= regs[9 + l * 2]) {
					offset = offset + (hstart_words[l] - regs[9 + l * 2]);
				}
				offset <<= 2;
				offset = offset & 0x3ffff; // OK?
				if(l != 0) offset += 0x40000;
				// ToDo: HAJ0, LO0
				uint16_t _begin = regs[9 + l * 2]; // HDSx
				uint16_t _end = regs[10 + l * 2];  // HDEx
				if(_begin < _end) {
					int words = _end - _begin;
					if(hstart_words[l] >= _begin) {
						words = words - (hstart_words[l] - _begin);
					}
					uint8_t magx = zoom_factor_horiz[l];
					uint8_t *p = d_vram->get_vram_address(offset);
//					out_debug_log("LINE=%d LAYER=%d HEAD=%08X LINE_OFFSET=%d VRAM=%08X words=%d magx=%d", line, l, head_address[l], line_offset[l], p, words, magx);
					if((p != NULL) && (words >= magx) && (magx != 0)){
						if(words >= TOWNS_CRTC_MAX_PIXELS) words = TOWNS_CRTC_MAX_PIXELS;
						switch(linebuffers[trans][line].mode[l]) {
						case DISPMODE_32768:
//							if(words > (line_offset[l] << 1)) words = line_offset[l] << 1;
							linebuffers[trans][line].pixels[l] = words;
							linebuffers[trans][line].mag[l] = magx << 1; // ToDo: Real magnify
							memcpy(linebuffers[trans][line].pixels_layer[l], p, words << 1);
							did_transfer[l] = true;
							line_shift = 3;
							break;
						case DISPMODE_16:
							linebuffers[trans][line].pixels[l] = words;
							linebuffers[trans][line].mag[l] = magx;
							memcpy(linebuffers[trans][line].pixels_layer[l], p, words >> 1);
							line_shift = 1;
							did_transfer[l] = true;
							break;
						}
					}
				}
			}
			if(zoom_count_vert[l] > 0) {
				zoom_count_vert[l]--;
			}
			if(zoom_count_vert[l] <= 0) {
				zoom_count_vert[l] = zoom_factor_vert[l];
				// ToDo: Interlace
				if(to_disp[l]) {
					head_address[l] += (line_offset[l] << line_shift);
				}
			}
			if(!(did_transfer[l])) {
				// Fill clear value.
				memset(linebuffers[trans][line].pixels_layer[l], 0x00, TOWNS_CRTC_MAX_PIXELS * sizeof(uint16_t));
			}
		}
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
	if(req_recalc) {
		force_recalc_crtc_param();
	}
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
	}
	if(lb != lines_per_frame) {
		set_lines_per_frame(lines_per_frame);
	}
	if(pb != pixels_per_line) {
		// ToDo: Resize texture.
		//set_pixels_per_line(pixels_per_line);
	}
}

void TOWNS_CRTC::event_frame()
{
	display_linebuf = (display_linebuf + 1) & 3; // Incremant per vstart
	lines_per_frame = max_lines;
//		if(req_recalc) {
//			force_recalc_crtc_param();
//		}
	line_count[0] = line_count[1] = 0;
	vert_line_count = -1;
	hsync = false;
//	out_debug_log(_T("FRAME EVENT LINES=%d FRAMEus=%f Hus=%f VST1us=%f VST2us=%f"),lines_per_frame, frame_us, horiz_us, vst1_us, vst2_us);
	for(int i = 0; i < 2; i++) {
		hdisp[i] = false;
		zoom_count_vert[i] = zoom_factor_vert[i];
	}
	// ToDo: EET
	//register_event(this, EVENT_CRTC_VSTART, frame_us, false, &event_id_frame); // EVENT_VSTART MOVED TO event_frame().
	if(d_sprite != NULL) {
		d_sprite->write_signal(SIG_TOWNS_SPRITE_CALL_VSTART, 0xffffffff, 0xffffffff);
	}

	if(event_id_vst1 > -1) {
		cancel_event(this, event_id_vst1);
		event_id_vst1 = -1;
	}
	if(event_id_vst2 > -1) {
		cancel_event(this, event_id_vst2);
		event_id_vst2 = -1;
	}
	if(vst1_us > 0.0) {
		set_vsync(false, true);
		register_event(this, EVENT_CRTC_VST1, vst1_us, false, &event_id_vst1); // VST1
	} else {
		set_vsync(true, true);
	}
	if(vst2_us > 0.0) {
		register_event(this, EVENT_CRTC_VST2, vst2_us, false, &event_id_vst2);
	}
	for(int i = 0; i < 2; i++) {
		frame_in[i] = false;
		if(event_id_vds[i] != -1) {
			cancel_event(this, event_id_vds[i]);
		}
		if(event_id_vde[i] != -1) {
			cancel_event(this, event_id_vde[i]);
		}
		if(vert_start_us[i] > 0.0) {
			register_event(this, EVENT_CRTC_VDS + i, vert_start_us[i], false, &event_id_vds[i]); // VDSx
		}
		if(vert_end_us[i] > 0.0) {
			register_event(this, EVENT_CRTC_VDE + i, vert_end_us[i],   false, &event_id_vde[i]); // VDEx
		}
		head_address[i] = 0;
	}
		
	if(event_id_hstart != -1) {
		cancel_event(this, event_id_hstart);
		event_id_hstart = -1;
	}
	if(event_id_hsw != -1) {
		cancel_event(this, event_id_hsw);
		event_id_hsw = -1;
	}
	if(horiz_us > 0.0) {
		register_event(this, EVENT_CRTC_HSTART, horiz_us, false, &event_id_hstart); // HSTART
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
		set_vsync(true, false);
		event_id_vst1 = -1;
	} else if (event_id == EVENT_CRTC_VST2) {
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
		if(frame_in[0] || frame_in[1]) {
			vert_line_count++;
			if(vert_line_count < lines_per_frame) {
				transfer_line(vert_line_count); // Tranfer before line.
			}
		}
		if(d_sprite != NULL) {
			d_sprite->write_signal(SIG_TOWNS_SPRITE_CALL_HSYNC, vert_line_count, 0xffffffff);
		}
		hdisp[0] = false;
		hdisp[1] = false;
		if(event_id_hsw != -1) {
			cancel_event(this, event_id_hsw);
			event_id_hsw = -1;
		}
		if(!vsync) {
			hsync = true;
			register_event(this, EVENT_CRTC_HSW, horiz_width_posi_us, false, &event_id_hsw); // VDEx
		} else {
			hsync = true;
			register_event(this, EVENT_CRTC_HSW, horiz_width_nega_us, false, &event_id_hsw); // VDEx
		}
		for(int i = 0; i < 2; i++) {
			if(event_id_hds[i] != -1) {
				cancel_event(this, event_id_hds[i]);
			}
			event_id_hds[i] = -1;
			if(event_id_hde[i] != -1) {
				cancel_event(this, event_id_hde[i]);
			}
			event_id_hde[i] = -1;
			
			if(horiz_start_us[i] > 0.0) {
				register_event(this, EVENT_CRTC_HDS + i, horiz_start_us[i], false, &event_id_hds[i]); // HDS0
			} else {
				hdisp[i] = true;
			}
			if((horiz_end_us[i] > 0.0) && (horiz_end_us[i] > horiz_start_us[i])) {
				register_event(this, EVENT_CRTC_HDE + i, horiz_end_us[i], false, &event_id_hde[i]); // HDS0
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
		event_id_hde[layer] = -1;
	}

}

uint32_t TOWNS_CRTC::read_signal(int ch)
{
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
	case SIG_TOWNS_CRTC_MMIO_CF882H:
		{
			uint8_t d;
			d = ((r50_planemask & 0x08) != 0) ? 0x60 : 0x40;
			d = d | (r50_planemask & 0x07);
			d = d | (r50_pagesel << 4);
			return d;
		}
		break;
	}
	return 0;
}
		
void TOWNS_CRTC::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_TOWNS_CRTC_MMIO_CF882H) {
		r50_planemask = ((data & 0x20) >> 2) | (data & 0x07);
		r50_pagesel = ((data & 0x10) != 0) ? 1 : 0;
	} else if(ch == SIG_TOWNS_CRTC_SPRITE_DISP) {
		sprite_disp_page = data & 1; // OK?
	} else if(ch == SIG_TOWNS_CRTC_SPRITE_USING) {
		sprite_enabled = ((data & mask) != 0) ? true : false; // OK?
	}
}

#define STATE_VERSION	1

bool TOWNS_CRTC::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
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
	state_fio->StateArray(vstart_lines, sizeof(vstart_lines), 1);
	state_fio->StateArray(vend_lines, sizeof(vend_lines), 1);
	state_fio->StateArray(line_offset, sizeof(line_offset), 1);
	state_fio->StateArray(frame_offset, sizeof(frame_offset), 1);
	state_fio->StateArray(head_address, sizeof(head_address), 1);
	state_fio->StateArray(impose_mode,  sizeof(impose_mode), 1);
	state_fio->StateArray(carry_enable, sizeof(carry_enable), 1);

	
	state_fio->StateArray(zoom_factor_vert, sizeof(zoom_factor_vert), 1);
	state_fio->StateArray(zoom_factor_horiz, sizeof(zoom_factor_horiz), 1);
	state_fio->StateArray(zoom_count_vert, sizeof(zoom_count_vert), 1);
	state_fio->StateArray(line_count, sizeof(line_count), 1);

	state_fio->StateValue(vert_line_count);
	
	state_fio->StateValue(vdisp);
	state_fio->StateValue(vblank);
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
	state_fio->StateArray(crtout, sizeof(crtout), 1);

	state_fio->StateValue(sprite_disp_page);
	state_fio->StateValue(sprite_enabled);

	state_fio->StateValue(pixels_per_line);
	state_fio->StateValue(lines_per_frame);

	state_fio->StateValue(event_id_hsync);
	state_fio->StateValue(event_id_hsw);
	state_fio->StateValue(event_id_vsync);
	state_fio->StateValue(event_id_vst1);
	state_fio->StateValue(event_id_vst2);
	state_fio->StateValue(event_id_vblank);
	state_fio->StateValue(event_id_vstart);
	state_fio->StateValue(event_id_hstart);
	
	state_fio->StateArray(event_id_vds, sizeof(event_id_vds), 1);
	state_fio->StateArray(event_id_vde, sizeof(event_id_vde), 1);
	state_fio->StateArray(event_id_hds, sizeof(event_id_hds), 1);
	state_fio->StateArray(event_id_hde, sizeof(event_id_hde), 1);

	//state_fio->StateValue(display_linebuf);

	if(loading) {
		for(int i = 0; i < 4; i++) {
			if(linebuffers[i] == NULL) {
				linebuffers[i] = (linebuffer_t*)malloc(sizeof(linebuffer_t ) * TOWNS_CRTC_MAX_LINES);
			}
			for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
				memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
			}
		}
		display_linebuf = 0;
		req_recalc = false;
		force_recalc_crtc_param();
	}
	return true;
}
}
