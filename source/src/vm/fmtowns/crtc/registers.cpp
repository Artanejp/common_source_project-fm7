/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2025.03.27 -

	[ FM-Towns CRTC around CRTC Registers ]
	History: 2025.03.27 Split from crtc.cpp .
*/

#include "../../vm.h"
#include "../../../common.h"

#include "../crtc.h"

namespace FMTOWNS {

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
		req_update_cr1 = true;
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
	
void TOWNS_CRTC::set_crtc_parameters_from_regs()
{
	calc_screen_parameters();  // Re-Calculate general display parameters.
	if(req_update_cr1) {
		set_crtc_clock(false);
	} else {
		update_horiz_khz();
	}
	
	copy_regs_v(); // Calculate display parameters per layer.
	copy_regs_h(); // Calculate display parameters per layer.
	
	force_recalc_crtc_param(); // Calculate parameter around HSYNC.
	
	vst1_count = vst1 << 1;
	vst2_count = vst2 << 1;

	__UNLIKELY_IF(vst1_count >= vst2_count) {
		vst2_count = vst1_count + 1;
	}
	calc_pixels_lines(); 

	int _lines = vst_reg;

	eet_count = eet;
	horiz_us = horiz_us_next;
	
	double horiz_ref = horiz_us;

	frame_us = ((double)_lines) * horiz_ref; // VST
	if(frame_us <= 0.0) {
		frame_us = 1.0e6 / FRAMES_PER_SEC;
	}

	set_frames_per_sec(1.0e6 / frame_us);
	set_lines_per_frame(_lines);
}
	
// CRTC register #29
void TOWNS_CRTC::set_crtc_clock(bool force)
{
	uint16_t val = regs[TOWNS_CRTC_REG_CLK];
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
	req_update_cr1 = false;
}

uint16_t TOWNS_CRTC::read_reg30()
{
	//uint16_t data = 0x00f0;
	uint16_t data = 0x0000;

	#if 0 /* Why is this... Ported from Tsugaru (；´Д｀) */
	data |= (!(vsync)             ?  0x8000 : 0);
	data |= (!(vsync)             ?  0x4000 : 0);
	data |= (!(hsync)             ?  0x2000 : 0);
	data |= (!(hsync)             ?  0x1000 : 0);
	data |= ((false)              ?  0x0800 : 0);
	#else
	data |= ((frame_in[1])        ?  0x8000 : 0);
	data |= ((frame_in[0])        ?  0x4000 : 0);
	data |= ((hdisp[1])           ?  0x2000 : 0);
	data |= ((hdisp[0])           ?  0x1000 : 0);
	data |= ((odd_field)          ?  0x0800 : 0);
	#endif
	data |= ((vsync)              ?  0x0400 : 0);
	data |= ((hsync)              ?  0x0200 : 0);
	//data |= ((video_in)           ? 0x0100 : 0);
	//data |= ((half_tone)          ? 0x0008 : 0);
	//data |= ((sync_enable)        ? 0x0004 : 0);
	//data |= ((vcard_enable)       ? 0x0002 : 0);
	//data |= ((sub_carry)          ? 0x0001 : 0);
	data = (data & 0xff00 ) | (regs[TOWNS_CRTC_REG_DUMMY] & 0x00ff);

	return data;
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


void TOWNS_CRTC::update_horiz_khz()
{
	double horiz_us_tmp;
	__LIKELY_IF(hst_reg != 0) {
		horiz_us_tmp = crtc_clock * (double)hst_reg;
	} else {
		horiz_us_tmp = crtc_clock;
	}
	horiz_khz = std::lrint(1.0e3 / horiz_us_tmp);
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
		max_lines = vheight_val[0] + voffset_val[0];
	} else {
		max_lines = max((vheight_val[0] + voffset_val[0]) , (vheight_val[1] + voffset_val[1]));
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
}

void TOWNS_CRTC::recalc_cr0(uint16_t cr0, bool calc_only)
{
	if(!(calc_only)) {
		if((cr0 & 0x8000) != 0) {
			// START BIT
			restart_display();
		} else {
			stop_display();
		}
	}
	if((cr0 & 0x4000) == 0) {
		// ESYN BIT
		// EXTERNAL SYNC OFF
	} else {
		// EXTERNAL SYNC ON
	}
	impose_mode[1]  = ((cr0 & 0x0080) == 0);
	impose_mode[0]  = ((cr0 & 0x0040) == 0);
	carry_enable[1] = ((cr0 & 0x0020) != 0);
	carry_enable[0] = ((cr0 & 0x0010) != 0);

	uint8_t dmode[2];
	dmode[0] = cr0 & 0x03;
	dmode[1] = (cr0 & 0x0c) >> 2;
	for(int i = 0; i < 2; i++) {
		__UNLIKELY_IF(dmode[i] != display_mode[i]) {
			notify_mode_changed(i, dmode[i]);
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

}
