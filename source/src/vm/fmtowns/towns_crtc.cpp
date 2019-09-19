/*
	Skelton for retropc emulator

	Author : Kyuma Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2016.12.28 -

	[ FM-Towns CRTC ]
	History: 2016.12.28 Initial from HD46505 .
*/

#include "towns_crtc.h"

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
	event_id_vstart = -1;
	event_id_vst1 = -1;
	event_id_vst2 = -1;
	event_id_vblank = -1;

	for(int i = 0; i < 2; i++) {
		// ToDo: Allocate at external buffer (when using compute shaders).
		linebuffers[i] = malloc(sizeof(linebuffer_t ) * TOWNS_CRTC_MAX_LINES);
		if(linebuffers[i] != NULL) {
			for(int l = 0; l < TOWNS_CRTC_MAX_LINES; l++) {
				memset(&(linebuffers[i][l]), 0x00, sizeof(linebuffer_t));
			}
		}
	}
	// register events
	//register_frame_event(this);
	//register_vline_event(this);
	
}

void TOWNS_CRTC::release()
{
	for(int i = 0; i < 2; i++) {
		// ToDo: Allocate at external buffer (when using compute shaders).
		if(linebuffers[i] != NULL) free(linebuffers[i]);
	}
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
	static const double clocks[] = {
		28.6363e6, 24.5454e6, 25.175e6, 21.0525e6
	};
	if(crtc_clock[clksel] != crtc_clock) {
		crtc_clock = crtc_clock[clksel];
		force_recalc_crtc_param();
	}
}

void TOWNS_CRTC::force_recalc_crtc_param(void)
{
	horiz_width_posi_us = crtc_clock * ((double)(regs[0] & 0x00fe)); // HSW1
	horiz_width_nega_us = crtc_clock * ((double)(regs[1] & 0x00fe)); // HSW2
	horiz_us = crtc_clock * ((double)((regs[4] & 0x07fe) + 1)); // HST
	vsync_pre_us = ((double)(regs[5] & 0x1f)) * horiz_us; // VST1

	double horiz_ref = horiz_us / 2.0;
	
	vst2_us = ((double)(regs[6] & 0x1f)) * horiz_ref; // VST2
	eet_us = ((double)(regs[7] & 0x1f)) * horiz_ref;  // EET
	frame_us = ((double)(regs[8] & 0x07ff)) * horiz_ref; // VST

	for(int layer = 0; layer < 2; layer++) {
		vert_start_us[layer] =  ((double)(regs[(layer << 1) + 13] & 0x07ff)) * horiz_ref;   // VDSx
		vert_end_us[layer] =    ((double)(regs[(layer << 1) + 13 + 1] & 0x07ff)) * horiz_ref; // VDEx
		horiz_start_us[layer] = ((double)(regs[(layer << 1) + 9] & 0x07ff)) * crtc_clock ;   // HDSx
		horiz_end_us[layer] =   ((double)(regs[(layer << 1) + 9 + 1] & 0x07ff)) * crtc_clock ;   // HDEx
	}
		
}


void TONWS_CRTC::write_io8(uint32_t addr, uint32_t data)
{
	if(addr == 0x0440) {
		ch = data & 0x1f;
	} else if(addr == 0x0442) {
		pair16_t rdata;
		rdata.w = regs[ch];
		rdata.l = (uint8_t)data;
		write_io16(addr, rdata.w);
	} else if(addr == 0x0443) {
		pair16_t rdata;
		rdata.w = regs[ch];
		rdata.h = (uint8_t)data;
		write_io16(addr, rdata.w);
	} 		
}

void TOWNS_CRTC::write_io16(uint32_t addr, uint32_t data)
{
	if((addr & 0xfffe) == 0x0442) {
		if(ch < 32) {
			if((ch < 0x09) && ((ch >= 0x04) || (ch <= 0x01))) { // HSW1..VST
				if(regs[ch] != (uint16_t)data) {
					force_recalc_crtc_param();
				}
			} else if(ch < 0x11) { // HDS0..VDE1
				uint8_t localch = ((ch  - 0x09) / 2) & 1;
				if(regs[ch] != (uint16_t)data) {
					timing_changed[localch] = true;
					// ToDo: Change render parameter within displaying.
					force_recalc_crtc_param();
				}
			}else if(ch < 0x19) { // FA0..LO1
				uint8_t localch = (ch - 0x11) / 4;
				uint8_t localreg = (ch - 0x11) & 3;
				if(regs[ch] != (uint16_t)data) {
					address_changed[localch] = true;
					switch(localreg) {
					case 0: // FAx
						vstart_addr[localch] = (uint32_t)(data & 0xffff);
						break;
					case 1: // HAJx
						hstart_words[localch] = (uint32_t)(data & 0x07ff);
						break;
					case 2: // FOx
						frame_offet[localch] = (uint32_t)(data & 0xffff);
						break;
					case 3: // LOx
						line_offet[localch] = (uint32_t)(data & 0xffff);
						break;
					}					
				}
			} else  { // All reg
				if(regs[ch] != (uint16_t)data) {
					switch(ch - 0x19) {
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
							zfv[0] = ((pd.l & 0xf0) >> 4) + 1;
							zfh[0] = (pd.l & 0x0f) + 1;
							zfv[1] = ((pd.h & 0xf0) >> 4) + 1;
							zfh[1] = (pd.h & 0x0f) + 1;
							if((zfv[0] != zoom_factor_vert[0]) || (zfh[0] != zoom_factor_horiz[0])) {
								timing_changed[0] = true;
								address_changed[0] = true;
								if(zfv[0] != zoom_factor_vert[0]) zoom_count_vert[0] = zfv[0];
							}
							if((zfv[1] != zoom_factor_vert[1]) || (zfh[1] != zoom_factor_horiz[1])) {
								timing_changed[0] = true;
								address_changed[0] = true;
								if(zfv[1] != zoom_factor_vert[1]) zoom_count_vert[1] = zfv[1];
							}
							zoom_factor_vert[0]  = zfv[0];
							zoom_factor_horiz[0] = zfh[0];
							zoom_factor_vert[1]  = zfv[1];
							zoom_factor_horiz[1] = zfh[1];
						}
						break;
					case 3: // CR0
						if(regs[ch] != data) {
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
						}
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
			regs[ch] = (uint16_t)data;
		}
	} else if(addr == 0x0440) {
		ch = data & 0x1f;
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
	
}

uint32_t TOWNS_CRTC::read_io16(uint32_t addr)
{
	addr = addr & 0xfffe;
	if(addr == 0x0440) {
		return (uint32_t)ch;
	} else if(addr == 0x0442) {
		if(ch == 30) {
			return (uint32_t)read_reg30();
		} else {
			return regs[ch];
		}
	}
	return 0xffff;
}

uint32_t TOWNS_CRTC::read_io8(uint32_t addr)
{
	if(addr == 0x0440) {
		return (uint32_t)ch;
	} else if(addr == 0x0442) {
		pair16_t d;
		if(ch == 30) {
			d.w = read_reg32();
		} else {
			d.w = regs[ch];
		}
		return (uint32_t)(d.l);
	} else if(addr == 0x0443) {
		pair16_t d;
		if(ch == 30) {
			d.w = read_reg32();
		} else {
			d.w = regs[ch];
		}
		return (uint32_t)(d.h);
	}
	return 0xff;
}

void TOWND_CRTC::render_32768(scrntype_t* dst, scrntype_t *mask, uint8_t* src, int y, int width, int layer)
{
	int magx = linebuffers[trans][y].mag[layer];
	int pwidth = linebuffers[trans][y].pixels[layer];
	int num = linebuffers[trans][y].num[layer];
	uint8_t *p = linebuffers[trans][y].pixels_layer[layer];
	scrntype_t *q = dst;
	scrntype_t *r = mask;
	if(magx < 1) return;
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
	int k = 0;
	for(x = 0; x < (pwidth >> 3); x++) {
		for(int i = 0; i < 8; i++) {
			pbuf[i] = read_2bytes_le_from(p);
			p += 2;
		}
		for(int i = 0; i < 8; i++) {
			rbuf[i] = pbuf[i];
			gbuf[i] = pbuf[i];
			bbuf[i] = pbuf[i];
		}			
		for(int i = 0; i < 8; i++) {
			rbuf[i] = (rbuf[i] >> 5) & 0x1f;
			gbuf[i] = (gbuf[i] >> 10) & 0x1f;
			bbuf[i] = bbuf[i] & 0x1f;
		}
		for(int i = 0; i < 8; i++) {
			rbuf[i] <<= 3;
			gbuf[i] <<= 3;
			bbuf[i] <<= 3;
		}
		for(int i = 0; i < 8; i++) {
			abuf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
		}
		for(int i = 0; i < 8; i++) {
			sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
		}
		if(magx == 1) {
			for(int i = 0; i < 8; i++) {
				*q++ = sbuf[i];
			}
			for(int i = 0; i < rwidth; i++) {
				*r++ = (rbuf[i] == 0) ? 0 : (scrntype_t)(-1);
			}
			k += 8;
			if(k >= width) break;
		} else {
			for(int i = 0; i < 8; i++) {
				if(j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					*r++ = (rbuf[i] == 0) ? 0 : (scrntype_t)(-1);
					k++;
					if(k >= width) break;
				}
				if(k >= width) break;
			}
		}
	}
	if(k >= width) return;
	if((pwidth & 7) != 0) {
		int rwidth = pwidth & 7;
		for(int i = 0; i < rwidth; i++) {
			pbuf[i] = read_2bytes_le_from(p);
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
		for(int i = 0; i < rwidth; i++) {
			abuf[i] = (pbuf[i] & 0x8000) ? 0 : 255;
		}
		for(int i = 0; i < rwidth; i++) {
			sbuf[i] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
		}
		if(magx == 1) {
			for(int i = 0; i < rwidth; i++) {
				*q++ = sbuf[i];
			}
			for(int i = 0; i < rwidth; i++) {
				*r++ = (rbuf[i] == 0) ? 0 : (scrntype_t)(-1);
			}
			k += 8;
			if(k >= width) break;
		} else {
			for(int i = 0; i < rwidth; i++) {
				if(j = 0; j < magx; j++) {
					*q++ = sbuf[i];
					*r++ = (rbuf[i] == 0) ? 0 : (scrntype_t)(-1);
					k++;
					if(k >= width) break;
				}
				if(k >= width) break;
			}
		}
	}
}

void TOWNS_CRTC::draw_screen()
{
	if(linebuffers[trans] == NULL) return;
	if(d_vram == NULL) return;
	__DECL_ALIGNED(32)  scrntype_t apal16[2][16];
	__DECL_ALIGNED(32)  scrntype_t apal256[256];
	
	{
		vm->lock_vm();
		d_vram->get_analog_palette(0, &(apal16[0][0]));
		d_vram->get_analog_palette(0, &(apal16[1][0]));
		d_vram->get_analog_palette(2, apal256);
		vm->unlock_vm();
	}
	int lines = d_vram->get_screen_height();
	int width = d_vram->get_screen_width();
	if(lines > TOWNS_CRTC_MAX_LINES) lines = TOWNS_CRTC_MAX_LINES;
	if(width > TOWNS_CRTC_MAX_PIXELS) width = TOWNS_CRTC_MAX_PIXELS;
	// ToDo: faster alpha blending.
	__DECL_ALIGNED(32) scrntype_t lbuffer0[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(32) scrntype_t lbuffer1[TOWNS_CRTC_MAX_PIXELS + 16];
	__DECL_ALIGNED(32) scrntype_t abuffer0[TOWNS_CRTC_MAX_PIXELS + 16];
	
	scrntype_t *dst;
	for(int y = 0; y < lines; y++) {
//		memset(lbuffer0, 0x00, sizeof(lbuffer0));
		memset(lbuffer1, 0x00, sizeof(lbuffer1));
//		memset(abuffer0, 0x00, sizeof(abuffer0));

		if(linebuffers[trans].mode[0] == DISPMODE_256) {
			// 256 colors
			int magx = linebuffers[trans].mag[0];
			int pwidth = linebuffers[trans].pixels[0];
			int num = linebuffers[trans].num[0];
			uint8_t *p = linebuffers[trans].pixels_layer[0];
			__DECL_ALIGNED(16) uint8_t pbuf[16];
			__DECL_ALIGNED(32) scrntype_t sbuf[16];
			if(magx < 1) return;
			if(magx == 1) {
				if(pwidth > width) pwidth = width;
				for(int x = 0; x < (pwidth >> 4); x++) {
					// ToDo: Start position
					for(int i = 0; i < 16; i++) {
						pbuf[i] = *p++;
					}
					int xx = x << 4;
					for(int i = 0; i < 16; i++) {
						lbuffer1[xx++] = apal256[pbuf[i]];
					}
				}
				if((pwidth & 15) != 0) {
					int xx = pwidth & ~15;
					int w = pwidth & 15;
					for(int i = 0; i < w; i++) {
						pbuf[i] = p++;
					}
					for(int i = 0; i < w; i++) {
						lbuffer1[xx++] = apal256[pbuf[i]];
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
							lbuffer1[k++] = s;
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
							lbuffer1[k++] = s;
							if(k >= width) break;
						}
						if(k > width) break;
					}
				}
			}
		} else {
			int magx = linebuffers[trans].mag[1];
			int pwidth = linebuffers[trans].pixels[1];
			int num = linebuffers[trans].num[1];
			uint8_t *p = linebuffers[trans].pixels_layer[1];
			__DECL_ALIGNED(16) uint16_t rbuf[16];
			__DECL_ALIGNED(16) uint16_t gbuf[16];
			__DECL_ALIGNED(16) uint16_t bbuf[16];
			__DECL_ALIGNED(16) uint8_t p8buf[16];
			__DECL_ALIGNED(16) uint8_t hlbuf[32];
			__DECL_ALIGNED(32) scrntype_t sbuf[16];
			if(magx < 1) return;
			if(pwidth > width) pwidth = width;
			if(linebuffers[trans].mode[1] == DISPMODE_16) { // Lower layer
				int k = 0;
				for(int x = 0; x < (pwidth >> 5); x++) {
					for(int i = 0; i < 16; i++) {
						p8buf[i] = *p++;
					}
					for(int i = 0; i < 32; i += 2) {
						hlbuf[i + 0] = p8buf[i >> 1] >> 4;
						hlbuf[i + 1] = p8buf[i >> 1] & 0x0f;
					}
					for(int i = 0; i < 32; i++) {
						sbuf[i] = apal16[num][hlbuf[i]];
					}
					if(magx == 1) {
						for(int i = 0; i < 32; i++) {
							lbuffer1[k++] = sbuf[i];
							if(k > width) break;
						}
					} else {
						for(int i = 0; i < 32; i++) {
							for(int j = 0; j < magx; j++) {
								lbuffer1[k++] = sbuf[i];
								if(k > width) break;
							}
							if(k > width) break;
						}
					}
				}
				if((pwidth & 31) >= 2) {
					for(int x = 0; x < ((pwidth & 31) >> 1); x++) {
						p8buf[x] = *p++;
					}
					for(int i = 0; i < (pwidth & 0x1e); i += 2) {
						hlbuf[i + 0] = p8buf[i >> 1] >> 4;
						hlbuf[i + 1] = p8buf[i >> 1] & 0x0f;
					}
					for(int i = 0; i < (pwidth & 0x1f); i++) {
						sbuf[i] = apal16[num][hlbuf[i]];
					}
					if(magx == 1) {
						for(int i = 0; i < (pwidth & 0x1f); i++) {
							lbuffer1[k++] = sbuf[i];
							if(k > width) break;
						}
					} else {
						for(int i = 0; i < (pwidth & 0x1f); i++) {
							for(int j = 0; j < magx; j++) {
								lbuffer1[k++] = sbuf[i];
								if(k > width) break;
							}
							if(k > width) break;
						}
					}
				}
			} else if(linebuffers[trans].mode[1] == DISPMODE_32768) { // Lower layer
				uint16_t* q = (uint16_t*)p;
				int k = 0;
				pair16_t n;
				for(int x = 0; x < (pwidth >> 4); x++) {
					for(int i = 0; i < 16; i++) {
						rbuf[i] = read_2bytes_le_from(p);
						p += 2;
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = rbuf[i];
						bbuf[i] = rbuf[i];
						abuf[i] = rbuf[i];
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = (gbuf[i] >> 10) & 0x1f;
						bbuf[i] = bbuf[i] & 0x1f;
						rbuf[i] = (rbuf[i]  >> 5) & 0x1f;
						abuf[i] = (abuf[i] & 0x8000) ? 0 : 255;
					}
					if(magx == 1) {
						for(int i = 0; i < 16; i++) {
							lbuffer1[k++] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
						}
					} else if(magx > 0) {
						for(int i = 0; i < 16; i++) {
							scrntype_t s = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
							for(int j = 0; j < magx; j++) {
								lbuffer1[k++] = s;
								if(k > width) break;
							}
							if(k > width) break;
						}
					}
				}
				for(int x = 0; x < (pwidth >> 4); x++) {
					for(int i = 0; i < 16; i++) {
						rbuf[i] = read_2bytes_le_from(p);
						p += 2;
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = rbuf[i];
						bbuf[i] = rbuf[i];
						abuf[i] = rbuf[i];
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = (gbuf[i] >> 10) & 0x1f;
						bbuf[i] = bbuf[i] & 0x1f;
						rbuf[i] = (rbuf[i]  >> 5) & 0x1f;
						abuf[i] = (abuf[i] & 0x8000) ? 0 : 255;
					}
					if(magx == 1) {
						for(int i = 0; i < 16; i++) {
							lbuffer1[k++] = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
						}
					} else if(magx > 0) {
						for(int i = 0; i < 16; i++) {
							scrntype_t s = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
							for(int j = 0; j < magx; j++) {
								lbuffer1[k++] = s;
								if(k > width) break;
							}
							if(k > width) break;
						}
					}
				}
			}
			// Upper layer
			magx = linebuffers[trans].mag[0];
			pwidth = linebuffers[trans].pixels[0];
			num = linebuffers[trans].num[0];
			p = linebuffers[trans].pixels_layer[0];
			if(magx < 1) return;
			if(pwidth > width) pwidth = width;
			// ToDo: alpha blending by GPU
			if(linebuffers[trans].mode[0] == DISPMODE_16) { // Upper layer
				int k = 0;
				for(int x = 0; x < (pwidth >> 5); x++) {
					for(int i = 0; i < 16; i++) {
						p8buf[i] = *p++;
					}
					for(int i = 0; i < 32; i += 2) {
						hlbuf[i + 0] = p8buf[i >> 1] >> 4;
						hlbuf[i + 1] = p8buf[i >> 1] & 0x0f;
					}
					for(int i = 0; i < 32; i++) {
						sbuf[i] = apal16[num][hlbuf[i]];
					}
					if(magx == 1) {
						for(int i = 0; i < 32; i++) {
							lbuffer1[k] = (hlbuf[i] == 0) ? lbuffer1[k] : sbuf[i];
							k++;
							if(k > width) break;
						}
					} else {
						for(int i = 0; i < 32; i++) {
							for(int j = 0; j < magx; j++) {
								lbuffer1[k] = (hlbuf[i] == 0) ? lbuffer1[k] : sbuf[i];
								k++;
								if(k > width) break;
							}
							if(k > width) break;
						}
					}
				}
				if((pwidth & 31) >= 2) {
					for(int x = 0; x < ((pwidth & 31) >> 1); x++) {
						p8buf[x] = *p++;
					}
					for(int i = 0; i < (pwidth & 0x1e); i += 2) {
						hlbuf[i + 0] = p8buf[i >> 1] >> 4;
						hlbuf[i + 1] = p8buf[i >> 1] & 0x0f;
					}
					for(int i = 0; i < (pwidth & 0x1f); i++) {
						sbuf[i] = apal16[num][hlbuf[i]];
					}
					if(magx == 1) {
						for(int i = 0; i < (pwidth & 0x1f); i++) {
							lbuffer1[k] = (hlbuf[i] == 0) ? lbuffer1[k] : sbuf[i];
							k++;
							if(k > width) break;
						}
					} else {
						for(int i = 0; i < (pwidth & 0x1f); i++) {
							for(int j = 0; j < magx; j++) {
								lbuffer1[k] = (hlbuf[i] == 0) ? lbuffer1[k] : sbuf[i];
								k++;
								if(k > width) break;
							}
							if(k > width) break;
						}
					}
				}
			} else if(linebuffers[trans].mode[0] == DISPMODE_32768) { // upper layer
				uint16_t* q = (uint16_t*)p;
				int k = 0;
				pair16_t n;
				for(int x = 0; x < (pwidth >> 4); x++) {
					for(int i = 0; i < 16; i++) {
						rbuf[i] = read_2bytes_le_from(p);
						p += 2;
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = rbuf[i];
						bbuf[i] = rbuf[i];
						abuf[i] = rbuf[i];
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = (gbuf[i] >> 10) & 0x1f;
						bbuf[i] = bbuf[i] & 0x1f;
						rbuf[i] = (rbuf[i]  >> 5) & 0x1f;
						abuf[i] = (abuf[i] & 0x8000) ? 0 : 255;
					}
					if(magx == 1) {
						for(int i = 0; i < 16; i++) {
							lbuffer1[k] = (abuf[i] == 0) ? lbuffer1[k] : RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
							k++;
						}
					} else if(magx > 0) {
						for(int i = 0; i < 16; i++) {
							if(!(abuf[i] == 0)) {
								scrntype_t s = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
								for(int j = 0; j < magx; j++) {
									lbuffer1[k++] = s;
									if(k > width) break;
								}
							} else {
								k += magx;
							}
							if(k > width) break;
						}
					}
				}
				for(int x = 0; x < (pwidth >> 4); x++) {
					for(int i = 0; i < 16; i++) {
						rbuf[i] = read_2bytes_le_from(p);
						p += 2;
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = rbuf[i];
						bbuf[i] = rbuf[i];
						abuf[i] = rbuf[i];
					}
					for(int i = 0; i < 16; i++) {
						gbuf[i] = (gbuf[i] >> 10) & 0x1f;
						bbuf[i] = bbuf[i] & 0x1f;
						rbuf[i] = (rbuf[i]  >> 5) & 0x1f;
						abuf[i] = (abuf[i] & 0x8000) ? 0 : 255;
					}
					if(magx == 1) {
						for(int i = 0; i < 16; i++) {
							lbuffer1[k] = (abuf[i] == 0) ? lbuffer1[k] : RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
							k++;
						}
					} else if(magx > 0) {
						for(int i = 0; i < 16; i++) {
							if(abuf[i] != 0) {
								scrntype_t s = RGBA_COLOR(rbuf[i], gbuf[i], bbuf[i], abuf[i]);
								for(int j = 0; j < magx; j++) {
									lbuffer1[k++] = s;
									if(k > width) break;
								}
							} else {
								k += magx;
							}
							if(k > width) break;
						}
					}
				}
			}
		}
		// ToDo: alpha blending
		{
			vm->lock_vm();
			scrntype_t *pp = emu->get_screen_buffer(y);
			if(pp != NULL) {
				my_memcpy(pp, lbuffer1, width * sizeof(scrntype_t));
			}
			vm->unlock_vm();
		}
	}
}
	
void TOWNS_CRTC::transfer_line(int line)
{
	if(line < 0) return;
	if(line >= TOWNS_CRTC_MAX_LINES) return;
	if(d_vram == NULL) return;
	uint8_t ctrl, prio, outc;
	d_vram->get_vramctrl_regs(ctrl, prio, outc);
	
	int trans = ((display_linebuf & 1) == 0) ? 1 : 0;
	if(linebuffers[trans] == NULL) return;
	for(int i = 0; i < 2; i++) {
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
	if((ctrl & 0x10) == 0) { // One layer mode
		bool to_disp = false;
		linebuffers[trans][line].num[0] = 0;
		if(!(frame_in[0])) return;
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
				if((p != NULL) && (words >= magx) && (magx != 0)){
					memcpy(linebuffers[trans][line].pixels_layer[0], p, words / magx);
					switch(linebuffers[trans][line].mode[0]) {
					case DISPMODE_32768:
						linebuffers[trans][line].pixels[0] = words / (magx * 2);
						linebuffers[trans][line].mag[0] = magx;
						break;
					case DISPMODE_256:
						linebuffers[trans][line].pixels[0] = words / (magx * 1);
						linebuffers[trans][line].mag[0] = magx;
						break;
					}
				}
			}
		}
		if(zoom_count_vert[0] > 0) {
			zoom_count_vert[0]--;
		}
		if(zoom_count_vert[0] == 0) {
			zoom_count_vert[0] = zoom_factor_vert[0];
			head_address[0] += frame_offset[0];
		}
	} else { // Two layers.
		bool to_disp[2] = {false, false};
		uint8_t ctrl_b = ctrl;
		linebuffers[trans][line].num[0] = page0;
		linebuffers[trans][line].num[1] = page1;
		for(int l = 0; l < 1; l++) {
			if(frame_in[l]) {
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
		}
		for(int l = 0; l < 1; l++) {
			if((to_disp[l]) && (frame_in[l])) {
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
					if(hstart_words[l] >= regs[9 + l * 2]) {
						words = words - (hstart_words[l] - regs[9 + l * 2]);
					}
					uint8_t magx = zoom_factor_horiz[l];
					uint8_t *p = d_vram->get_vram_address(offset);
					if((p != NULL) && (words >= magx) && (magx != 0)){
						memcpy(linebuffers[trans][line].pixels_layer[l], p, words / magx);
						switch(linebuffers[trans][line].mode[l]) {
						case DISPMODE_32768:
							linebuffers[trans][line].pixels[l] = words / (magx * 2);
							linebuffers[trans][line].mag[l] = magx;
							break;
						case DISPMODE_16:
							linebuffers[trans][line].pixels[l] = (words * 2) / (magx * 1);
							linebuffers[trans][line].mag[l] = magx;
							break;
						}
					}
				}
			}
			if(frame_in[l]) {
				if(zoom_count_vert[l] > 0) {
					zoom_count_vert[l]--;
				}
				if(zoom_count_vert[l] == 0) {
					zoom_count_vert[l] = zoom_factor_vert[l];
					head_address[l] += frame_offset[l];
				}
			}
		}
	}
}

void TOWNS_CRTC::event_pre_frame()
{
	// ToDo: Resize frame buffer.
}

void TOWNS_CRTC::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	cpu_clocks = new_clocks;
	frames_per_sec = new_frames_per_sec;
	max_lines = new_lines_per_frame;
	max_frame_usec = 1.0e6 / frames_per_sec;
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
	if(event_id == EVENT_CRTC_VSTART) {
		d_vram->write_signal(SIG_TOWNS_VRAM_VSTART, 0x01, 0x01);
		line_count[0] = line_count[1] = 0;
		if((horiz_us != next_horiz_us) || (vert_us != next_vert_us)) {
			horiz_us = next_horiz_us;
			vert_us = next_vert_us;
		}
		hsync = false;
		for(int i = 0; i < 2; i++) {
			hdisp[i] = false;
			zoom_count_vert[i] = zoom_factor_vert[i];
		}
		major_line_count = -1;
		// ToDo: EET
		register_event(this, EVENT_CRTC_VSTART, frame_us, false, &event_id_frame);
		if(vert_sync_pre_us >= 0.0) {
			vsync = false;
			register_event(this, EVENT_CRTC_VST1, vert_sync_pre_us, false, &event_id_vst1); // VST1
		} else {
			vsync = true;
		}
		register_event(this, EVENT_CRTC_VST2, vst2_us, false, &event_id_vst2);
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
			} else {
				frame_in[i] = true;
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
		register_event(this, EVENT_CRTC_HSTART, horiz_us, false, &event_id_hstart); // HSTART
	} else if(event_id == EVENT_CRTC_VST1) { // VSYNC
		vsync = true;
	} else if (event_id == EVENT_CRTC_VST2) {
		vsync = false;
		event_id_vstn = -1;
	} else if(eid2 == EVENT_CRTC_VDS) { // Display start
		int layer = event_id & 1;
		frame_in[layer] = true;
		// DO ofset line?
		event_id_vstart[layer] = -1;
		zoom_count_vert[layer] = zoom_factor_vert[layer];
	} else if(eid2 == EVENT_CRTC_VDE) { // Display end
		int layer = event_id & 1;
		frame_in[layer] = false;
		event_id_vend[layer] = -1;
		// DO ofset line?
	} else if(event_id == EVENT_CRTC_HSTART) {
		// Do render
		event_id_hstart = -1;
		major_line_count++;
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
		transfer_line(major_line_count - 1);
	} else if(eid2 == EVENT_CRTC_HDS) {
		int layer = event_id & 1;
		hdisp[layer] = true;
		if((horiz_end_us[i] <= 0.0) || (horiz_end_us[i] <= horiz_start_us[i])) {
			hdisp[layer] = false;
		}
		event_id_hds[layer] = -1;
	} else if(eid2 == EVENT_CRTC_HDE) {
		int layer = event_id & 1;
		hdisp[layer] = false;	
		event_id_hde[layer] = -1;
	}

}

void TOWNS_CRTC::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_TONWS_CRTC_SINGLE_LAYER) {
		one_layer_mode = ((data & mask) == 0);
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

}
