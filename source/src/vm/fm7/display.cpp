/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#include "../../fileio.h"
#include "fm7_display.h"
#if defined(_FM77AV_VARIANTS)
# include "mb61vh010.h"
#endif
extern "C" {
  extern void initvramtbl_4096_vec(void);
  extern void detachvramtbl_4096_vec(void);
  extern void PutBlank(uint32 *p, int height);
  extern void CreateVirtualVram8_Line(uint8 *src, uint32 *p, int ybegin, uint32 *pal);
  extern void CreateVirtualVram8_WindowedLine(uint8 *vram_1, uint8 *vram_w, uint32 *p, int ybegin, int xbegin, int xend, uint32 *pal);
}


DISPLAY::DISPLAY(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
}

DISPLAY::~DISPLAY()
{

}

void DISPLAY::reset_cpuonly()
{
	int i;
	if(hblank_event_id >= 0) cancel_event(this, hblank_event_id);
	if(hdisp_event_id >= 0) cancel_event(this, hdisp_event_id);
	if(vsync_event_id >= 0) cancel_event(this, vsync_event_id);
	if(vstart_event_id >= 0) cancel_event(this, vstart_event_id);
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
	
	subcpu_resetreq = false;
	keyboard->reset();
#if defined(_FM77AV_VARIANTS)
	key_rxrdy = false;
	key_ack = false;
	alu->reset();
	subrom_bank_using = subrom_bank;
	offset_point_bank1 = 0;
	use_alu = false;
	subcpu_resetreq = false;
	apalette_index.d = 0;
	nmi_enable = true;
#endif   
	//for(i = 0; i < 8; i++) set_dpalette(i, i);
	crt_flag = true;
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	keyboard->write_signal(SIG_FM7KEY_SET_INSLED, 0x01, 0x01);
   	memset(gvram, 0x00, sizeof(gvram));
	
	vblank = false;
	vsync = false;
	hblank = false;
	
	halt_flag = false;
 	mainio->write_signal(SIG_FM7_SUB_HALT, 0x00, 0xff);

	sub_run = true;
	sub_busy = true;
	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0xff , 0xff);

	vram_wrote = true;
	clr_count = 0;
	register_event(this, EVENT_FM7SUB_VSTART, 1.0, false, &vstart_event_id);   
   
	//mainio->write_signal(FM7_MAINIO_BEEP, 0x00, 0x01);
	firq_mask = (mainio->read_signal(FM7_MAINIO_KEYBOARDIRQ_MASK) != 0) ? false : true;
	//firq_mask = false;
	key_firq_req = false;	//firq_mask = true;
   
	mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 0x00 , 0xff);
	subcpu->reset();
}


void DISPLAY::reset()
{
	int i;
	uint32 subclock;
	
	if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	nmi_event_id = -1;

#if defined(_FM77AV_VARIANTS)
	display_page = 0;
	active_page = 0;
	cgrom_bank = 0;
	subrom_bank = 0;
	subrom_bank_using = 0;
	mode320 = false;
	power_on_reset = true;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = i & 0xf00;
		analog_palette_b[i] = i & 0x00f;
	}
	offset_point_bank1 = 0;
	use_alu = false;
	subcpu_resetreq = false;
	apalette_index.d = 0;
	nmi_enable = true;
	//key_rxrdy = false;
	//key_ack = true;
#endif
	display_mode = DISPLAY_MODE_8_200L;
	for(i = 0; i < 8; i++) set_dpalette(i, i);
	offset_77av = false;
	vram_accessflag = false;
	
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	memset(io_w_latch, 0x00, sizeof(io_w_latch));
#if defined(_FM77AV_VARIANTS)
	for(i = 0; i < 8; i++) io_w_latch[i + 0x13] = 0x80;
#endif 
	//printf("SUB:Reset.\n");
	cancel_request = false;
	cancel_bak = false;
	irq_backup = false;
	displine = 0;
	vblank_count = 0;
	
	set_cyclesteal(config.dipswitch & FM7_DIPSW_CYCLESTEAL); // CYCLE STEAL = bit0.
	window_low = 0;
	window_high = 200;
	window_xbegin = 0;
	window_xend = 80;
	window_opened = false;
	offset_point = 0;
	tmp_offset_point.d = 0;
	offset_changed = true;

	if(clock_fast) {
		subclock = SUBCLOCK_NORMAL;
	} else {
		subclock = SUBCLOCK_SLOW;
	}
	if(is_cyclesteal || !(vram_accessflag)) {
		//
	} else {
		if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) == 0) subclock = subclock / 3;
	}
	p_vm->set_cpu_clock(subcpu, subclock);
	prev_clock = subclock;
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
   
	do_attention = false;
	mainio->write_signal(FM7_MAINIO_SUB_ATTENTION, 0x00, 0x01);
   
	reset_cpuonly();
}

void DISPLAY::update_config()
{
	uint32 subclock;
	set_cyclesteal(config.dipswitch & FM7_DIPSW_CYCLESTEAL); // CYCLE STEAL = bit0.
	switch(config.cpu_type) {
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
	if(clock_fast) {
		subclock = SUBCLOCK_NORMAL;
	} else {
		subclock = SUBCLOCK_SLOW;
	}
	if(is_cyclesteal || !(vram_accessflag)) {
		//
	} else {
		if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) == 0) subclock = subclock / 3;
	}
	p_vm->set_cpu_clock(subcpu, subclock);
	prev_clock = subclock;
}

inline void DISPLAY::GETVRAM_8_200L(int yoff, scrntype *p, uint32 mask)
{
	register uint8 b, r, g;
	register uint32 dot;
	uint32 yoff_d;
#if defined(_FM77AV_VARIANTS)
	if(display_page == 1) { // Is this dirty?
		yoff_d = offset_point_bank1;
	} else {
		yoff_d = offset_point;
	}
#else
	yoff_d = offset_point;
#endif	
	if(!offset_77av) {
		yoff_d &= 0x3fe0;
	}
	yoff_d = (yoff + yoff_d) & 0x3fff;
	b = r = g = 0;
#if defined(_FM77AV_VARIANTS)
	if(display_page == 1) {
		if(mask & 0x01) b = gvram[yoff_d + 0x0c000];
		if(mask & 0x02) r = gvram[yoff_d + 0x10000];
		if(mask & 0x04) g = gvram[yoff_d + 0x14000];
	} else {
		if(mask & 0x01) b = gvram[yoff_d + 0x00000];
		if(mask & 0x02) r = gvram[yoff_d + 0x04000];
		if(mask & 0x04) g = gvram[yoff_d + 0x08000];
	}
#else
	if(mask & 0x01) b = gvram[yoff_d + 0x00000];
	if(mask & 0x02) r = gvram[yoff_d + 0x04000];
	if(mask & 0x04) g = gvram[yoff_d + 0x08000];
#endif	
	dot = ((g & 0x80) >> 5) | ((r & 0x80) >> 6) | ((b & 0x80) >> 7);
	p[0] = dpalette_pixel[dot];
	dot = ((g & 0x40) >> 4) | ((r & 0x40) >> 5) | ((b & 0x40) >> 6);
	p[1] = dpalette_pixel[dot];
	dot = ((g & 0x20) >> 3) | ((r & 0x20) >> 4) | ((b & 0x20) >> 5);
	p[2] = dpalette_pixel[dot];
	dot = ((g & 0x10) >> 2) | ((r & 0x10) >> 3) | ((b & 0x10) >> 4);
	p[3] = dpalette_pixel[dot];
					
	dot = ((g & 0x8) >> 1) | ((r & 0x8) >> 2) | ((b & 0x8) >> 3);
	p[4] = dpalette_pixel[dot];
	dot = (g & 0x4) | ((r & 0x4) >> 1) | ((b & 0x4) >> 2);
	p[5] = dpalette_pixel[dot];
	dot = ((g & 0x2) << 1) | (r & 0x2) | ((b & 0x2) >> 1);
	p[6] = dpalette_pixel[dot];
	dot = ((g & 0x1) << 2) | ((r & 0x1) << 1) | (b & 0x1);
	p[7] = dpalette_pixel[dot];
}

#if defined(_FM77AV_VARIANTS)
inline void DISPLAY::GETVRAM_4096(int yoff, scrntype *p, uint32 mask)
{
	register uint32 b3, r3, g3;
	register scrntype b, r, g;
	uint32 idx;;
	scrntype pixel;
	uint32 yoff_d1, yoff_d2;

	if(offset_77av) {
		yoff_d1 = offset_point;
		yoff_d2 = offset_point_bank1;
	} else {
		yoff_d1 = offset_point & 0x1fe0;
		yoff_d2 = offset_point_bank1 & 0x1fe0;
	}
	yoff_d1 = (yoff + yoff_d1) & 0x1fff;
	yoff_d2 = (yoff + yoff_d2) & 0x1fff;

	b3  = gvram[yoff_d1] << 24;
	b3 |= gvram[yoff_d1 + 0x02000] << 16;
	r3  = gvram[yoff_d1 + 0x04000] << 24;
	r3 |= gvram[yoff_d1 + 0x06000] << 16;
		
	g3  = gvram[yoff_d1 + 0x08000] << 24;
	g3 |= gvram[yoff_d1 + 0x0a000] << 16;
		
	b3 |= gvram[yoff_d2 + 0x0c000] << 8;
	b3 |= gvram[yoff_d2 + 0x0e000] << 0;
		
	r3 |= gvram[yoff_d2 + 0x10000] << 8;
	r3 |= gvram[yoff_d2 + 0x12000] << 0;
	g3 |= gvram[yoff_d2 + 0x14000] << 8;
	g3 |= gvram[yoff_d2 + 0x16000] << 0;
   
	g = ((g3 & (0x80 << 24)) ? 0x800 : 0) | ((g3 & (0x80 << 16)) ? 0x400 : 0) | ((g3 & (0x80 << 8)) ? 0x200 : 0) | ((g3 & 0x80) ? 0x100 : 0);
	r = ((r3 & (0x80 << 24)) ? 0x80  : 0) | ((r3 & (0x80 << 16)) ? 0x40  : 0) | ((r3 & (0x80 << 8)) ? 0x20  : 0) | ((r3 & 0x80) ? 0x10  : 0);
	b = ((b3 & (0x80 << 24)) ? 0x8   : 0) | ((b3 & (0x80 << 16)) ? 0x4   : 0) | ((b3 & (0x80 << 8)) ? 0x2   : 0) | ((b3 & 0x80) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[0] = pixel;
	p[1] = pixel;

	
	g = ((g3 & (0x40 << 24)) ? 0x800 : 0) | ((g3 & (0x40 << 16)) ? 0x400 : 0) | ((g3 & (0x40 << 8)) ? 0x200 : 0) | ((g3 & 0x40) ? 0x100 : 0);
	r = ((r3 & (0x40 << 24)) ? 0x80  : 0) | ((r3 & (0x40 << 16)) ? 0x40  : 0) | ((r3 & (0x40 << 8)) ? 0x20  : 0) | ((r3 & 0x40) ? 0x10  : 0);
	b = ((b3 & (0x40 << 24)) ? 0x8   : 0) | ((b3 & (0x40 << 16)) ? 0x4   : 0) | ((b3 & (0x40 << 8)) ? 0x2   : 0) | ((b3 & 0x40) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[2] = pixel;
	p[3] = pixel;

	g = ((g3 & (0x20 << 24)) ? 0x800 : 0) | ((g3 & (0x20 << 16)) ? 0x400 : 0) | ((g3 & (0x20 << 8)) ? 0x200 : 0) | ((g3 & 0x20) ? 0x100 : 0);
	r = ((r3 & (0x20 << 24)) ? 0x80  : 0) | ((r3 & (0x20 << 16)) ? 0x40  : 0) | ((r3 & (0x20 << 8)) ? 0x20  : 0) | ((r3 & 0x20) ? 0x10  : 0);
	b = ((b3 & (0x20 << 24)) ? 0x8   : 0) | ((b3 & (0x20 << 16)) ? 0x4   : 0) | ((b3 & (0x20 << 8)) ? 0x2   : 0) | ((b3 & 0x20) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[4] = pixel;
	p[5] = pixel;

	g = ((g3 & (0x10 << 24)) ? 0x800 : 0) | ((g3 & (0x10 << 16)) ? 0x400 : 0) | ((g3 & (0x10 << 8)) ? 0x200 : 0) | ((g3 & 0x10) ? 0x100 : 0);
	r = ((r3 & (0x10 << 24)) ? 0x80  : 0) | ((r3 & (0x10 << 16)) ? 0x40  : 0) | ((r3 & (0x10 << 8)) ? 0x20  : 0) | ((r3 & 0x10) ? 0x10  : 0);
	b = ((b3 & (0x10 << 24)) ? 0x8   : 0) | ((b3 & (0x10 << 16)) ? 0x4   : 0) | ((b3 & (0x10 << 8)) ? 0x2   : 0) | ((b3 & 0x10) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[6] = pixel;
	p[7] = pixel;


	g = ((g3 & (0x8 << 24)) ? 0x800 : 0) | ((g3 & (0x8 << 16)) ? 0x400 : 0) | ((g3 & (0x8 << 8)) ? 0x200 : 0) | ((g3 & 0x8) ? 0x100 : 0);
	r = ((r3 & (0x8 << 24)) ? 0x80  : 0) | ((r3 & (0x8 << 16)) ? 0x40  : 0) | ((r3 & (0x8 << 8)) ? 0x20  : 0) | ((r3 & 0x8) ? 0x10  : 0);
	b = ((b3 & (0x8 << 24)) ? 0x8   : 0) | ((b3 & (0x8 << 16)) ? 0x4   : 0) | ((b3 & (0x8 << 8)) ? 0x2   : 0) | ((b3 & 0x8) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[8] = pixel;
	p[9] = pixel;

	
	g = ((g3 & (0x4 << 24)) ? 0x800 : 0) | ((g3 & (0x4 << 16)) ? 0x400 : 0) | ((g3 & (0x4 << 8)) ? 0x200 : 0) | ((g3 & 0x4) ? 0x100 : 0);
	r = ((r3 & (0x4 << 24)) ? 0x80  : 0) | ((r3 & (0x4 << 16)) ? 0x40  : 0) | ((r3 & (0x4 << 8)) ? 0x20  : 0) | ((r3 & 0x4) ? 0x10  : 0);
	b = ((b3 & (0x4 << 24)) ? 0x8   : 0) | ((b3 & (0x4 << 16)) ? 0x4   : 0) | ((b3 & (0x4 << 8)) ? 0x2   : 0) | ((b3 & 0x4) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[10] = pixel;
	p[11] = pixel;

	g = ((g3 & (0x2 << 24)) ? 0x800 : 0) | ((g3 & (0x2 << 16)) ? 0x400 : 0) | ((g3 & (0x2 << 8)) ? 0x200 : 0) | ((g3 & 0x2) ? 0x100 : 0);
	r = ((r3 & (0x2 << 24)) ? 0x80  : 0) | ((r3 & (0x2 << 16)) ? 0x40  : 0) | ((r3 & (0x2 << 8)) ? 0x20  : 0) | ((r3 & 0x2) ? 0x10  : 0);
	b = ((b3 & (0x2 << 24)) ? 0x8   : 0) | ((b3 & (0x2 << 16)) ? 0x4   : 0) | ((b3 & (0x2 << 8)) ? 0x2   : 0) | ((b3 & 0x2) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[12] = pixel;
	p[13] = pixel;

	g = ((g3 & (0x1 << 24)) ? 0x800 : 0) | ((g3 & (0x1 << 16)) ? 0x400 : 0) | ((g3 & (0x1 << 8)) ? 0x200 : 0) | ((g3 & 0x1) ? 0x100 : 0);
	r = ((r3 & (0x1 << 24)) ? 0x80  : 0) | ((r3 & (0x1 << 16)) ? 0x40  : 0) | ((r3 & (0x1 << 8)) ? 0x20  : 0) | ((r3 & 0x1) ? 0x10  : 0);
	b = ((b3 & (0x1 << 24)) ? 0x8   : 0) | ((b3 & (0x1 << 16)) ? 0x4   : 0) | ((b3 & (0x1 << 8)) ? 0x2   : 0) | ((b3 & 0x1) ? 0x1   : 0);
	   
	idx = (g  | b | r ) & mask;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	pixel = RGB_COLOR(r, g, b);
	p[14] = pixel;
	p[15] = pixel;
}
#endif

void DISPLAY::draw_screen()
{
	int y;
	int x;
	int height = (display_mode == DISPLAY_MODE_8_400L) ? 400 : 200;
	scrntype *p, *pp;
	register int yoff;
	uint32 planesize;
	register uint32 rgbmask;
	
	if(!vram_wrote) return;
	vram_wrote = false;   
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_8_400L_TEXT)) {
		planesize = 0x8000;
	} else if((display_mode == DISPLAY_MODE_8_200L) || (display_mode == DISPLAY_MODE_8_200L_TEXT)) {
		planesize = 0x4000;
	} else if((display_mode == DISPLAY_MODE_4096) || (display_mode == DISPLAY_MODE_256k)) {
		planesize = 0x2000;
	} else {
		return;
	}
	  // Set blank
	if(!crt_flag) {
		for(y = 0; y < 400; y++) {
			memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
		return;
	}
	if(display_mode == DISPLAY_MODE_8_200L) {
		yoff = 0;
		rgbmask = ~multimode_dispmask;
		for(y = 0; y < 400; y += 2) {
			p = emu->screen_buffer(y);
			pp = p;
			for(x = 0; x < 10; x++) {
			  GETVRAM_8_200L(yoff + 0, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_200L(yoff + 1, p, rgbmask);
			  p += 8;

  			  GETVRAM_8_200L(yoff + 2, p, rgbmask);
			  p += 8;

			  GETVRAM_8_200L(yoff + 3, p, rgbmask);
			  p += 8;

			  GETVRAM_8_200L(yoff + 4, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_200L(yoff + 5, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_200L(yoff + 6, p, rgbmask);
			  p += 8;
			  
			  GETVRAM_8_200L(yoff + 7, p, rgbmask);
			  p += 8;
			  yoff += 8;
			}
			if(config.scan_line == 0) {
				memcpy((void *)emu->screen_buffer(y + 1), pp, 640 * sizeof(scrntype));
			} else {
				memset((void *)emu->screen_buffer(y + 1), 0x00, 640 * sizeof(scrntype));
			}
		}
		return;
	}
#if defined(_FM77AV_VARIANTS)
	if(display_mode == DISPLAY_MODE_4096) {
		uint32 mask = 0;
		yoff = 0;
		rgbmask = multimode_dispmask;
		if((rgbmask & 0x01) == 0) mask = 0x00f;
		if((rgbmask & 0x02) == 0) mask = mask | 0x0f0;
		if((rgbmask & 0x04) == 0) mask = mask | 0xf00;
		for(y = 0; y < 400; y += 2) {
			p = emu->screen_buffer(y);
			pp = p;
			yoff = y * (40 / 2);
			for(x = 0; x < 5; x++) {
				GETVRAM_4096(yoff + 0, p, mask);
				p += 16;
			  
				GETVRAM_4096(yoff + 1, p, mask);
				p += 16;
				
				GETVRAM_4096(yoff + 2, p, mask);
				p += 16;

				GETVRAM_4096(yoff + 3, p, mask);
				p += 16;
				
				GETVRAM_4096(yoff + 4, p, mask);
				p += 16;
			  
				GETVRAM_4096(yoff + 5, p, mask);
				p += 16;
				
				GETVRAM_4096(yoff + 6, p, mask);
				p += 16;

				GETVRAM_4096(yoff + 7, p, mask);
				p += 16;
				yoff += 8;
			}
			if(config.scan_line == 0) {
				memcpy((void *)emu->screen_buffer(y + 1), pp, 640 * sizeof(scrntype));
			} else {
				memset((void *)emu->screen_buffer(y + 1), 0x00, 640 * sizeof(scrntype));
			}
		}
		return;
	}
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	else { // 256k
		for(y = 0; y < height; y++) {
			p = &pvram[y * pitch];
			if(((y < window_low) && (y > window_high)) || (!window_opened)) {
				CreateVirtualVram256k_Line(p, y, offset, multimode_dispmask);
			} else {
				CreateVirtualVram256k_WindowedLine(p, y, offset, window_xbegin, window_xend,
								   multimode_dispmask);
			}
		}
	}
#endif // _FM77AV40
#endif //_FM77AV_VARIANTS
}

void DISPLAY::do_irq(bool flag)
{
	subcpu->write_signal(SIG_CPU_IRQ, flag ? 1: 0, 1);
}

void DISPLAY::do_firq(bool flag)
{
	subcpu->write_signal(SIG_CPU_FIRQ, flag ? 1: 0, 1);
}

void DISPLAY::do_nmi(bool flag)
{
#if defined(_FM77AV_VARIANTS)
	if(!nmi_enable && flag) flag = false;
#endif
	subcpu->write_signal(SIG_CPU_NMI, flag ? 1 : 0, 1);
}

void DISPLAY::set_multimode(uint8 val)
{
	multimode_accessmask = val & 0x07;
	multimode_dispmask = (val & 0x70) >> 4;
	vram_wrote = true;
}

uint8 DISPLAY::get_multimode(void)
{
	uint8 val;
	val = multimode_accessmask & 0x07;
	val |= ((multimode_dispmask << 4) & 0x70);
	val |= 0x80;
	return val;
}

uint8 DISPLAY::get_cpuaccessmask(void)
{
	return multimode_accessmask & 0x07;
}

void DISPLAY::set_dpalette(uint32 addr, uint8 val)
{
	scrntype r, g, b;
	addr &= 7;
	dpalette_data[addr] = val | 0xf8; //0b11111000;
	vram_wrote = true;
	b =  ((val & 0x01) != 0x00)? 255 : 0x00;
	r =  ((val & 0x02) != 0x00)? 255 : 0x00;
	g =  ((val & 0x04) != 0x00)? 255 : 0x00;
	
	dpalette_pixel[addr] = RGB_COLOR(r, g, b);
}

uint8 DISPLAY::get_dpalette(uint32 addr)
{
	uint8 data;
	addr = addr & 7;
	
	data = dpalette_data[addr];
	return data;
}

void DISPLAY::halt_subcpu(void)
{
	subcpu->write_signal(SIG_CPU_BUSREQ, 0x01, 0x01);
}

void DISPLAY::go_subcpu(void)
{
	subcpu->write_signal(SIG_CPU_BUSREQ, 0x00, 0x01);
}

void DISPLAY::enter_display(void)
{
	uint32 subclock;
	if(clock_fast) {
		subclock = SUBCLOCK_NORMAL;
	} else {
		subclock = SUBCLOCK_SLOW;
	}
   
	if(!is_cyclesteal && vram_accessflag) {
		if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) == 0) subclock = subclock / 3;
	}
	//halt_subcpu();
	if(prev_clock != subclock) p_vm->set_cpu_clock(subcpu, subclock);
	prev_clock = subclock;
}

void DISPLAY::leave_display(void)
{
}

void DISPLAY::halt_subsystem(void)
{
  	sub_run = false;
	halt_flag = false;
  	halt_subcpu();
}

void DISPLAY::restart_subsystem(void)
{
	sub_run = true;
	halt_flag = false;
	go_subcpu();
}

//SUB:D408:R
void DISPLAY::set_crtflag(void)
{
	crt_flag = true;
	vram_wrote = true;
}

//SUB:D408:W
void DISPLAY::reset_crtflag(void)
{
	crt_flag = false;
	vram_wrote = true;
}

//SUB:D402:R
uint8 DISPLAY::acknowledge_irq(void)
{
	cancel_request = false;
	do_irq(false);
	return 0xff;
}

//SUB:D403:R
uint8 DISPLAY::beep(void)
{
	mainio->write_signal(FM7_MAINIO_BEEP, 0x01, 0x01);
	return 0xff; // True?
}


// SUB:D404 : R 
uint8 DISPLAY::attention_irq(void)
{
	do_attention = true;
	mainio->write_signal(FM7_MAINIO_SUB_ATTENTION, 0x01, 0x01);
	return 0xff;
}

// SUB:D405:W
void DISPLAY::set_cyclesteal(uint8 val)
{
#if !defined(_FM8)
	vram_wrote = true;
	val &= 0x01;
	if(val != 0) {
		is_cyclesteal = true;
	} else {
		is_cyclesteal = false;
	}
#endif
}

//SUB:D409:R
uint8 DISPLAY::set_vramaccess(void)
{
	vram_accessflag = true;
	return 0xff;
}

//SUB:D409:W
void DISPLAY::reset_vramaccess(void)
{
	vram_accessflag = false;
}

//SUB:D40A:R
uint8 DISPLAY::reset_subbusy(void)
{
	sub_busy = false;
	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0x00 , 0xff);
	return 0xff;
}

//SUB:D40A:W
void DISPLAY::set_subbusy(void)
{
	sub_busy = true;
	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0xff , 0xff);
}


#if defined(_FM77AV_VARIANTS)
// D410
void DISPLAY::alu_write_cmdreg(uint32 val)
{
	alu->write_data8(ALU_CMDREG, val);
	if((val & 0x80) != 0) {
		use_alu = true;
	} else {
		use_alu = false;
	}
}

// D411
void DISPLAY::alu_write_logical_color(uint8 val)
{
	uint32 data = (uint32)val;
	alu->write_data8(ALU_LOGICAL_COLOR, data);
}

// D412
void DISPLAY::alu_write_mask_reg(uint8 val)
{
	uint32 data = (uint32)val;
	alu->write_data8(ALU_WRITE_MASKREG, data);
}

// D413 - D41A
void DISPLAY::alu_write_cmpdata_reg(int addr, uint8 val)
{
	uint32 data = (uint32)val;
	addr = addr & 7;
	alu->write_data8(ALU_CMPDATA_REG + addr, data);
}

// D41B
void DISPLAY::alu_write_disable_reg(uint8 val)
{
	uint32 data = (uint32)val;
	data = (data & 0x07) | 0x08;
	alu->write_data8(ALU_BANK_DISABLE, data);
}

// D41C - D41F
void DISPLAY::alu_write_tilepaint_data(int addr, uint8 val)
{
	uint32 data = (uint32)val;
	switch(addr) {
		case 0: // $D41C
			alu->write_data8(ALU_TILEPAINT_B, data);
			break;
		case 1: // $D41D
			alu->write_data8(ALU_TILEPAINT_R, data);
			break;
		case 2: // $D41E
			alu->write_data8(ALU_TILEPAINT_G, data);
			break;
		case 3: // xxxx
			//alu->write_data8(SIG_FM7_ALU_TILEPAINT_L, data, 0xff);
			alu->write_data8(ALU_TILEPAINT_L, 0xff);
			break;
	}
}

// D420
void DISPLAY::alu_write_offsetreg_hi(uint8 val)
{
	if(display_mode == DISPLAY_MODE_8_400L) {
		alu->write_data8(ALU_OFFSET_REG_HIGH, val & 0x7f);
	} else {
		alu->write_data8(ALU_OFFSET_REG_HIGH, val & 0x3f);
	}
}
 
// D421
void DISPLAY::alu_write_offsetreg_lo(uint8 val)
{
	alu->write_data8(ALU_OFFSET_REG_LO, val);
}

// D422
void DISPLAY::alu_write_linepattern_hi(uint8 val)
{
	alu->write_data8(ALU_LINEPATTERN_REG_HIGH, val);
}

// D423
void DISPLAY::alu_write_linepattern_lo(uint8 val)
{
	alu->write_data8(ALU_LINEPATTERN_REG_LO, val);
}

// D424-D42B
void DISPLAY::alu_write_line_position(int addr, uint8 val)
{
	uint32 data = (uint32)val;
	switch(addr) {
		case 0:  
			alu->write_data8(ALU_LINEPOS_START_X_HIGH, data & 0x03); 
			break;
		case 1:  
			alu->write_data8(ALU_LINEPOS_START_X_LOW, data); 
			break;
		case 2:  
			alu->write_data8(ALU_LINEPOS_START_Y_HIGH, data & 0x01); 
			break;
		case 3:  
			alu->write_data8(ALU_LINEPOS_START_Y_LOW, data); 
			break;
		case 4:  
			alu->write_data8(ALU_LINEPOS_END_X_HIGH, data & 0x03); 
			break;
		case 5:  
			alu->write_data8(ALU_LINEPOS_END_X_LOW, data); 
			break;
  		case 6:  
			alu->write_data8(ALU_LINEPOS_END_Y_HIGH, data & 0x01); 
			break;
		case 7:  
			alu->write_data8(ALU_LINEPOS_END_Y_LOW, data);
			break;
	}
}

// D42E :  AV40
void DISPLAY::select_sub_bank(uint8 val)
{
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
	kanji_level2 = ((val & 0x80) == 0) ? false : true;
 #endif
}

// D42F
void DISPLAY::select_vram_bank_av40(uint8 val)
{

}


//SUB:D430:R
uint8 DISPLAY::get_miscreg(void)
{
	uint8 ret;
#if defined(_FM77AV_VARIANTS)
	//ret = 0x6a;
	ret = 0x00;
	if(!hblank && !vblank) ret |= 0x80;
	//if(!hblank) ret |= 0x80;
	if(vsync) ret |= 0x04;
	if(alu->read_signal(SIG_ALU_BUSYSTAT) == 0) ret |= 0x10;
	if(!power_on_reset) ret |= 0x01;
#else // 77 or older.
	ret = 0xff;
#endif
	return ret;
}
//SUB:D430:W
void DISPLAY::set_miscreg(uint8 val)
{
	int y;
	int old_display_page = display_page;
	nmi_enable = ((val & 0x80) == 0) ? true : false;
	//nmi_enable = ((val & 0x80) != 0) ? true : false;
	if((val & 0x40) == 0) {
		display_page = 0;
	} else {
		display_page = 1;
	}
	if(display_page != old_display_page) {
		if((display_mode != DISPLAY_MODE_4096) &&
		   (display_mode != DISPLAY_MODE_256k)) {
			for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
	}
	active_page = ((val & 0x20) == 0) ? 0 : 1;
	if((val & 0x04) == 0) {
		offset_77av = false;
	} else {
		offset_77av = true;
	}
	cgrom_bank = val & 0x03;
	
}
//SUB : D431 : R
uint8 DISPLAY::get_key_encoder(void)
{
	return keyboard->read_data8(0x31);
}
void DISPLAY::put_key_encoder(uint8 data)
{
	return keyboard->write_data8(0x31, data);
}

uint8 DISPLAY::get_key_encoder_status(void)
{
	uint8 data = 0xff;
	if(key_rxrdy) {
		data &= 0x7f;
	}
	if(!key_ack) {
		data &= 0xfe;
	}
	// Digityze : bit0 = '0' when waiting,
	return data;
}


// Main: FD13
void DISPLAY::set_monitor_bank(uint8 var)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if((var & 0x04) != 0){
		monitor_ram = true;
	} else {
		monitor_ram = false;
	}
#endif
	subrom_bank = var & 0x03;
	subcpu_resetreq = false;
	vram_wrote = true;
	subrom_bank_using = subrom_bank;
	power_on_reset = false;
	this->reset_cpuonly();
	//restart_subsystem();
}


// FD30
void DISPLAY::set_apalette_index_hi(uint8 val)
{
	apalette_index.b.h = val & 0x0f;
}

// FD31
void DISPLAY::set_apalette_index_lo(uint8 val)
{
	apalette_index.b.l = val;
}

// FD32
void DISPLAY::set_apalette_b(uint8 val)
{
	uint16 index;
	index = apalette_index.w.l;
	analog_palette_b[index] = (val & 0x0f) << 4;
	vram_wrote = true;
}

// FD33
void DISPLAY::set_apalette_r(uint8 val)
{
	uint16 index;
	index = apalette_index.w.l;
	analog_palette_r[index] = (val & 0x0f) << 4;
	vram_wrote = true;
}

// FD34
void DISPLAY::set_apalette_g(uint8 val)
{
	uint16 index;
	index = apalette_index.w.l;
	analog_palette_g[index] = (val & 0x0f) << 4;
	vram_wrote = true;
}

#endif // _FM77AV_VARIANTS

// Test header
#if !defined(_MSC_VER)
#include <SDL2/SDL.h>
#endif

// Timing values from XM7 . Thanks Ryu.
void DISPLAY::event_callback(int event_id, int err)
{
	double usec;
	bool f;
	switch(event_id) {
  		case EVENT_FM7SUB_DISPLAY_NMI: // per 20.00ms
#if defined(_FM77AV_VARIANTS)
			if(nmi_enable) {
				do_nmi(true);
			} else {
				do_nmi(false);
			}
#else
				do_nmi(true);
#endif
		break;
  		case EVENT_FM7SUB_DISPLAY_NMI_OFF: // per 20.00ms
			do_nmi(false);
			break;
		case EVENT_FM7SUB_HDISP:
			hblank = false;
			vblank = false;
			vsync = false;
			usec = 39.5;
			if(display_mode == DISPLAY_MODE_8_400L) usec = 30.0;
			register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hblank_event_id); // NEXT CYCLE_
			break;
		case EVENT_FM7SUB_HBLANK:
			hblank = true;
			vblank = false;
			vsync = false;
			f = false;
			//if(!is_cyclesteal && vram_accessflag)  leave_display();
			if(display_mode == DISPLAY_MODE_8_400L) {
				if(displine <= 400) f = true;
			} else {
		                if(displine <= 200) f = true;
                        }
			if(f) {
				if(display_mode == DISPLAY_MODE_8_400L) {
					usec = 11.0;
				} else {
					usec = 24.0;
				}
				register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id);
			}
	   
			displine++;
 //                       } else {
 //                             if(f) {
 //                                       if(display_mode == DISPLAY_MODE_8_400L) {
 //                                               usec = 30.0;
 //                                       } else {
 //                                               usec = 39.0;
 //                                       }
 //                                       register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id); // NEXT CYCLE_
 //                               } 
 //                               displine++;
//                        }
//#endif
                        break;
                case EVENT_FM7SUB_VSTART: // Call first.
                        vblank = true;
                        vsync = false;
                        hblank = false;
                        displine = 0;
                        leave_display();
                        // Parameter from XM7/VM/display.c , thanks, Ryu.
                        if(vblank_count != 0) {
                                //printf("VBLANK(1): %d\n", SDL_GetTicks());
                                if(display_mode == DISPLAY_MODE_8_400L) {
                                        usec = (0.98 + 16.4) * 1000.0;
                                } else {
                                        usec = (1.91 + 12.7) * 1000.0;
                                }
                                register_event(this, EVENT_FM7SUB_VSYNC, usec, false, &vsync_event_id);

                                if(display_mode == DISPLAY_MODE_8_400L) {
                                        usec = 1.65 * 1000.0;
                                } else {
                                        usec = 3.94 * 1000.0;
                                }
                                register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hblank_event_id); // NEXT CYCLE_
				vblank_count = 0;
                        } else {
                                //printf("VBLANK(0): %d\n", SDL_GetTicks());
                                if(display_mode == DISPLAY_MODE_8_400L) {
                                        usec = 0.34 * 1000.0;
                                } else {
                                        usec = 1.52 * 1000.0;
                                }
                                register_event(this, EVENT_FM7SUB_VSYNC, usec, false, &vsync_event_id); // NEXT CYCLE_
                                vblank_count++;

                        }                         
                        break;
                case EVENT_FM7SUB_VSYNC:
                        vblank = true;
                        hblank = false;
                        vsync = true;
                        //printf("VSYNC: %d\n", SDL_GetTicks());
                        if(display_mode == DISPLAY_MODE_8_400L) {
                                usec = 0.33 * 1000.0; 
                        } else {
                                usec = 0.51 * 1000.0;
                        }
                        register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
                        break;
                 case EVENT_FM7SUB_CLR:
                        set_subbusy();
                        break;
        }
}

void DISPLAY::event_frame()
{
	enter_display();
}

void DISPLAY::event_vline(int v, int clock)
{
#if 0
        double usec;
	if(v < 400) { 
		hblank = false;
		vblank = false;
		vsync = false;
		enter_display();
		if(display_mode == DISPLAY_MODE_8_400L) {
			register_event(this, EVENT_FM7SUB_HBLANK, 30.0, false, &hdisp_event_id); // NEXT CYCLE_
		} else {
			if((v & 1) == 0) register_event(this, EVENT_FM7SUB_HBLANK, 39.5, false, &hdisp_event_id); // NEXT CYCLE_
		}		  
	} else {
		vblank = true;
		vsync = false;
		if(display_mode == DISPLAY_MODE_8_400L) {
			usec = 0.98 * 1000.0;
		} else {
			usec = 1.91 * 1000.0;
		}
		register_event(this, EVENT_FM7SUB_VSYNC, usec, false, &vsync_event_id);
	}
#endif  
}


uint32 DISPLAY::read_signal(int id)
{
	uint32 retval;
	switch(id) {
		case SIG_FM7_SUB_HALT:
		case SIG_DISPLAY_HALT:
			return (halt_flag) ? 0xffffffff : 0;
			break;
		case SIG_DISPLAY_BUSY:
			return (sub_busy) ? 0x80 : 0;
		 	break;
		case SIG_DISPLAY_MULTIPAGE:
			return multimode_accessmask;
			break;
		case SIG_DISPLAY_PLANES:
			return 3;
			break;
		case SIG_DISPLAY_VSYNC:
			retval = (vsync) ? 0x01 : 0x00;
			return retval;
			break;
		case SIG_DISPLAY_DISPLAY:
			retval = (!hblank && !vblank) ? 0x02: 0x00;
			//retval = (!hblank) ? 0x02: 0x00;
			return retval;
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_DISPLAY_MODE320:
			retval = (mode320) ? 0x40: 0x00;
			return retval;
			break;
#endif
		case SIG_DISPLAY_Y_HEIGHT:
#if defined(_FM77AV_VARIANTS)
			retval = 200;
#else
		  	retval = 200;
#endif		  
			return retval;
			break;
		case SIG_DISPLAY_X_WIDTH:
#if defined(_FM77AV_VARIANTS)
			retval = (mode320) ? 40 : 80;
#else
		  	retval = 640;
#endif		  
			return retval;
			break;
		default:
			return 0;
			break;
	}
   
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	bool flag = ((data & mask) != 0);
	int y;
	switch(id) {
		case SIG_FM7_SUB_HALT:
			if(flag) {
				sub_busy = true;
				mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0xff, 0xff);
			}
			halt_flag = flag;
			mainio->write_signal(SIG_FM7_SUB_HALT, data, mask);
			break;
       		case SIG_DISPLAY_HALT:
			if(flag) {
				halt_subsystem();
			} else {
				restart_subsystem();
			}
			break;
		case SIG_FM7_SUB_CANCEL:
			if(flag) {
				cancel_request = true;
				do_irq(true);
			}
			break;
		case SIG_DISPLAY_CLOCK:
			if(clock_fast != flag) {
				uint32 clk;
				if(flag) {
					clk = SUBCLOCK_NORMAL;
				} else {
					clk = SUBCLOCK_SLOW;
				}
				if(is_cyclesteal || !(vram_accessflag)) {
				  //
				} else {
					if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) == 0) clk = clk / 3;
				}
				if(clk != prev_clock) p_vm->set_cpu_clock(subcpu, clk);
				clock_fast = flag;
				prev_clock = clk;
			}
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_BANK: // Main: FD13
			set_monitor_bank(data & 0xff);
			break;
		case SIG_FM7KEY_RXRDY: // D432 bit7
			key_rxrdy = ((data & mask) != 0);
			break;
		case SIG_FM7KEY_ACK: // D432 bit 0
			key_ack = ((data & mask) != 0);
			break;
		case SIG_DISPLAY_EXTRA_MODE: // FD04 bit 4, 3
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			{
				int oldmode = display_mode;
				mode400line = ((data & 0x08) != 0);
				mode256k = ((data & 0x10) != 0);
				if(mode400line) {
					display_mode = DISPLAY_MODE_8_400L;
				} else if(mode256k) {
					display_mode = DISPLAY_MODE_256k;
				} else {
					display_mode = (mode320)? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
				}
				if(oldmode != display_mode) {
					for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
				}
			}
			kanji_sub = ((data & 0x20) != 0);
#endif
			break;
		case SIG_DISPLAY_MODE320: // FD12 bit 6
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if((!mode400line) && (!mode256k)){
				if(mode320 != flag) {
					for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
				}
				mode320 = flag;
				display_mode = (mode320) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
			}
#else
			if(mode320 != flag) {
				for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
			}
			mode320 = flag;
			display_mode = (mode320 == true) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
			vram_wrote = true;
			//printf("MODE320: %d\n", display_mode);
#endif
			break;
#endif // _FM77AV_VARIANTS
		case SIG_DISPLAY_MULTIPAGE:
	  		set_multimode(data);
			break;
		case SIG_FM7_SUB_KEY_MASK:
			if(firq_mask != flag) {
				do_firq((!flag) & key_firq_req);
			}
			firq_mask = !flag;
			break;
		case SIG_FM7_SUB_KEY_FIRQ:
			key_firq_req = flag;
			if((key_firq_req) && (!firq_mask)) {
				do_firq(true);
			} else {
				do_firq(false);
			}
			break;
		case SIG_FM7_SUB_USE_CLR:
	   		if(flag) {
				clr_count = data & 0x03;
			} else {
				clr_count = 0;
			}
			break;
		default:
			break;
	}
}
   


uint8 DISPLAY::read_vram_8_200l(uint32 addr, uint32 offset)
{
	uint32 page_offset = 0;
	uint32 pagemod;
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) {
		page_offset = 0xc000;
	}
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_page) page_offset += 0x18000;
#endif
	pagemod = addr & 0xc000;
	return gvram[(((addr + offset) & 0x3fff) | pagemod) + page_offset];
}

uint8 DISPLAY::read_vram_l4_400l(uint32 addr, uint32 offset)
{
#if defined(_FM77L4)
	if(addr < 0x8000) {
		if(workram) {
			raddr = addr & 0x3fff;
			if((multimode_accessmask & 0x04) == 0) {
				return gvram[0x8000 + (raddr + offset) & 0x7fff];
			}
			return 0xff;
		}
		pagemod = addr & 0x4000;
		return gvram[((addr + offset) & mask) | pagemod];
	} else if(addr < 0x9800) {
		return textvram[addr & 0x0fff];
	} else { // $9800-$bfff
		return subrom_l4[addr - 0x9800];
	}
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_8_400l(uint32 addr, uint32 offset)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 color = vram_bank & 0x03;
	uint32 raddr;
	
	if((multimode_accessmask & (1 << color)) != 0) {
		return 0xff;
	} else {
		raddr = addr & 0xbfff;
		switch(color) {
			case 0:
				raddr = raddr & 0x7fff;
				raddr = (raddr + offset) & 0x7fff;
				if(vram_page) raddr = raddr + 0x18000;
				break;
			case 1:
				if(vram_bank) {
					raddr = (raddr & 0x3fff) + 0x4000;
				} else {
					raddr = (raddr & 0xbfff) - 0x8000;
				}
				raddr = (raddr + offset) & 0x7fff;
				raddr = raddr + 0x8000;
				if(vram_page) raddr = raddr + 0x18000;
				break;
			case 2:
				if(vram_bank) {
					raddr = raddr & 0x7fff;
				} else {
					return 0xff;
				}
				raddr = (raddr + offset) & 0x7fff;
				raddr = raddr + 0x10000;
				if(vram_page) raddr = raddr + 0x18000;
				break;
			default:
				return 0xff;
				break;
			}
			return gvram[raddr];
	}
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_4096(uint32 addr, uint32 offset)
{
#if defined(_FM77AV_VARIANTS)
	uint32 page_offset = 0;
	uint32 pagemod;
	uint32 color;
	
	if(active_page != 0) {
		page_offset = 0xc000;
	}
	pagemod = addr & 0xe000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_page) page_offset += 0x18000;
	if(vram_bank) {
		if(pagemod < 0x4000) {
			color = 1; // r
		} else {
			color = 2; // g
		}
	} else {
		if(pagemod >= 0x8000) {
			color = 1; // r
		}
	}
	if((multimode_accessmask & (1 << color)) == 0) {
		return gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset];
	}
	return 0xff;
#else
	return gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset];
#endif
#endif
	return 0xff;
}

uint8 DISPLAY::read_vram_256k(uint32 addr, uint32 offset)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 page_offset;
	uint32 pagemod;
	uint32 color; // b
	if(!vram_page && !vram_bank) {
		color = 0;
		page_offset = 0;
	} else if(vram_page && !vram_bank) {
		color = 1;
		page_offset = 0xc000;
	} else if(!vram_page && vram_bank) {
		color = 2;
		page_offset = 0x18000;
	} else {
		return 0xff;
	}
	if((multimode_accessmask & (1 << color)) == 0) {
		pagemod = addr & 0xe000;
		return gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset];
	}
#endif
	return 0xff;
}


uint8 DISPLAY::read_mmio(uint32 addr)
{
	uint32 retval = 0xff;
	uint32 raddr;	
#if !defined(_FM77AV_VARIANTS)
	raddr = (addr - 0xd400) & 0x000f;
#else
	raddr = (addr - 0xd400) & 0x003f;
#endif
	//if(addr >= 0x02) printf("SUB: IOREAD PC=%04x, ADDR=%02x\n", subcpu->get_pc(), addr);
	switch(raddr) {
		case 0x00: // Read keyboard
			retval = (keyboard->read_data8(0x00) != 0) ? 0xff : 0x7f;
			break;
		case 0x01: // Read keyboard
			retval = keyboard->read_data8(0x01) & 0xff;
			key_firq_req = false;
			do_firq(false);
			mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 0x00, 0xff);
			break;
		case 0x02: // Acknowledge
			acknowledge_irq();
			break;
		case 0x03:
			beep();
			break;
		case 0x04:
			attention_irq();
			break;
	        
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		case 0x06:
		  //if(!kanjisub) return 0xff;
				
 #if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
			if(kanji_level2) {
				retval = kanjiclass2->read_data8(kanji2_addr.w.l << 1);
			} else {
				retval = kanjiclass1->read_data8(kanji1_addr.w.l << 1);
			}
 #elif defined(_FM77_VARIANTS) // _FM77L4
			retval = kanjiclass1->read_data8(kanji1_addr.w.l << 1);
 #else
			retval = 0xff;
#endif				
			break;
		case 0x07:
 #if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
			//if(!kanjisub) return 0xff;
			if(kanji_level2) {
				retval = kanjiclass2->read_data8((kanji2_addr.w.l << 1) + 1);
			} else {
				retval = kanjiclass1->read_data8((kanji1_addr.w.l << 1) + 1);
			}
 #elif defined(_FM77_VARIANTS) // _FM77L4
			retval = kanjiclass1->read_data8((kanji1_addr.w.l << 1) + 1);
 #else
			retval = 0xff;
 #endif				
			break;
#endif
		case 0x08:
			set_crtflag();
			break;
		case 0x09:
			retval = set_vramaccess();
			break;
		case 0x0a:
			reset_subbusy();
			break;
		case 0x0d:
			keyboard->write_signal(SIG_FM7KEY_SET_INSLED, 0x01, 0x01);
			break;
#if defined(_FM77AV_VARIANTS)
		// ALU
		case 0x10:
			retval = alu->read_data8(ALU_CMDREG);
			break;
		case 0x11:
			retval = alu->read_data8(ALU_LOGICAL_COLOR);
			break;
		case 0x12:
			retval = alu->read_data8(ALU_WRITE_MASKREG);
			break;
		case 0x13:
			retval = alu->read_data8(ALU_CMP_STATUS_REG);
			break;
		case 0x1b:
			retval = alu->read_data8(ALU_BANK_DISABLE);
			break;
		// MISC
		case 0x30:
			retval = get_miscreg();
			break;
		// KEY ENCODER.
		case 0x31:
			retval = get_key_encoder();
			break;
		case 0x32:
			retval = get_key_encoder_status();
			break;
#endif				
		default:
			break;
	}
	return (uint8)retval;
}

uint32 DISPLAY::read_data8(uint32 addr)
{
	uint32 raddr = addr;
	uint32 mask = 0x3fff;
	uint32 offset;
	uint32 dummy;
	uint32 color = (addr & 0x0c000) >> 14;
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) {
		offset = offset_point_bank1 & 0x7fff;
	} else {
		offset = offset_point & 0x7fff; 
	}
	if(!offset_77av) {
		offset = offset & 0x7fe0;
	}
#else
	offset = offset_point & 0x7fe0;
#endif
	if(addr < 0xc000) {
#if defined(_FM77AV_VARIANTS)
		if(use_alu) {
			dummy = alu->read_data8((addr & 0x3fff) + ALU_WRITE_PROXY);
			return dummy;
		}
		if(!(is_cyclesteal | vram_accessflag)) return 0xff;
		if((multimode_accessmask & (1 << color)) != 0) return 0xff;
		
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			return (uint32)read_vram_8_400l(addr, offset);
		} else if(display_mode == DISPLAY_MODE_256k) {
			return (uint32)read_vram_256k(addr, offset);
		} else	if(display_mode == DISPLAY_MODE_4096) {
			return (uint32)read_vram_4096(addr, offset);
		} else { // 8Colors, 200LINE
			return (uint32)read_vram_8_200l(addr, offset);
		}
#else		
		if(mode320) {
			return (uint32)read_vram_4096(addr, offset);
		} else {
			return (uint32)read_vram_8_200l(addr, offset);
		}
#endif		
		return 0xff;
#else //_FM77L4
		if(display_mode == DISPLAY_MODE_8_400L) {
			return (uint32)read_vram_l4_400l(addr, offset);
		} else {
			uint32 color = (addr & 0x0c000) >> 14;
			if((multimode_accessmask & (1 << color)) != 0) return 0xff;
			return (uint32)read_vram_8_200l(addr, offset);
		}
		return 0xff;
#endif
	} else if(addr < 0xd000) { 
		raddr = addr - 0xc000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(submon_bank == 4) {
			if(cgram_bank >= 1) {
				return (uint32)submem_cgram[((cgram_bank - 1) << 12) | raddr];
			}
		}
#endif
		return console_ram[raddr];
	} else if(addr < 0xd380) {
		raddr = addr - 0xd000;
		return work_ram[raddr];
	} else if(addr < 0xd400) {
		raddr = addr - 0xd380;
		return shared_ram[raddr];
	} else 	if(addr < 0xd800) {
		return read_mmio(addr);
	} else if(addr < 0x10000) {
#if !defined(_FM77AV_VARIANTS)
		return subsys_c[addr - 0xd800];
#else
		if(addr < 0xe000) {
			//if(subrom_bank_using == 0) return subsys_c[addr - 0xd800];
			return subsys_cg[(addr - 0xd800) + cgrom_bank * 0x800];
		} else if(addr < 0x10000) {
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
			if(monitor_ram) {
				return subsys_ram[addr - 0xe000]; //FIXME
			}
#endif		
			switch(subrom_bank_using) {
				case 0: // SUBSYS_C
					return subsys_c[addr - 0xd800];
					break;
		        	case 1:
					return subsys_a[addr - 0xe000];
					break;
		        	case 2:
					return subsys_b[addr - 0xe000];
					break;
				default:
					return subsys_cg[addr - 0xe000];
					break;
			}
		}
#endif
	} else if((addr >= FM7_SUBMEM_OFFSET_DPALETTE) && (addr < (FM7_SUBMEM_OFFSET_DPALETTE + 8))) {
		return dpalette_data[addr - FM7_SUBMEM_OFFSET_DPALETTE];
	}
#if defined(_FM77AV_VARIANTS)
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0xc000))) {
		return read_vram_8_200l(addr - DISPLAY_VRAM_DIRECT_ACCESS, offset);
	}
	
#endif
	return 0xff;
}	


void DISPLAY::write_vram_8_200l(uint32 addr, uint32 offset, uint32 data)
{
	uint32 page_offset = 0;
	uint32 pagemod;
	uint8 val8 = data & 0xff;
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) {
		page_offset = 0xc000;
	}
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_page) page_offset += 0x18000;
#endif
	pagemod = addr & 0xc000;
	gvram[(((addr + offset) & 0x3fff) | pagemod) + page_offset] = val8;
	vram_wrote = true;
}

void DISPLAY::write_vram_l4_400l(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77L4)
	if(addr < 0x8000) {
		if(workram) {
			raddr = addr & 0x3fff;
			if((multimode_accessmask & 0x04) == 0) {
				gvram[0x8000 + (raddr + offset) & 0x7fff] = (uint8)data;
			}
			return;
		}
		pagemod = addr & 0x4000;
		gvram[((addr + offset) & mask) | pagemod] = (uint8)data;
	} else if(addr < 0x9800) {
	  textvram[addr & 0x0fff] = (uint8)data;
	} else { // $9800-$bfff
		//return subrom_l4[addr - 0x9800];
	}
	return;
#endif	
}

void DISPLAY::write_vram_8_400l(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 color = vram_bank & 0x03;
	uint32 raddr;
	
	if((multimode_accessmask & (1 << color)) != 0) {
		return;
	} else {
		raddr = addr & 0xbfff;
		switch(color) {
			case 0:
				raddr = raddr & 0x7fff;
				raddr = (raddr + offset) & 0x7fff;
				if(vram_page) raddr = raddr + 0x18000;
				break;
			case 1:
				if(vram_bank) {
					raddr = (raddr & 0x3fff) + 0x4000;
				} else {
					raddr = (raddr & 0xbfff) - 0x8000;
				}
				raddr = (raddr + offset) & 0x7fff;
				raddr = raddr + 0x8000;
				if(vram_page) raddr = raddr + 0x18000;
				break;
			case 2:
				if(vram_bank) {
					raddr = raddr & 0x7fff;
				} else {
					return;
				}
				raddr = (raddr + offset) & 0x7fff;
				raddr = raddr + 0x10000;
				if(vram_page) raddr = raddr + 0x18000;
				break;
			default:
				return;
				break;
			}
			gvram[raddr] = val8;
			return;
	}
#endif
}

void DISPLAY::write_vram_4096(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV_VARIANTS)
	uint32 page_offset = 0;
	uint32 pagemod;
	uint32 color;
	
	if(active_page != 0) {
		page_offset = 0xc000;
	}
	pagemod = addr & 0xe000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_page) page_offset += 0x18000;
	if(vram_bank) {
		if(pagemod < 0x4000) {
			color = 1; // r
		} else {
			color = 2; // g
		}
	} else {
		if(pagemod >= 0x8000) {
			color = 1; // r
		}
	}
	if((multimode_accessmask & (1 << color)) == 0) {
		gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset] = (uint8)data;
	}
#else
	gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset] = (uint8)data;
	vram_wrote = true;
#endif
#endif
}

void DISPLAY::write_vram_256k(uint32 addr, uint32 offset, uint32 data)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32 page_offset;
	uint32 pagemod;
	uint32 color; // b
	if(!vram_page && !vram_bank) {
		color = 0;
		page_offset = 0;
	} else if(vram_page && !vram_bank) {
		color = 1;
		page_offset = 0xc000;
	} else if(!vram_page && vram_bank) {
		color = 2;
		page_offset = 0x18000;
	} else {
		return;
	}
	if((multimode_accessmask & (1 << color)) == 0) {
		pagemod = addr & 0xe000;
		vram_wrote = true;
		gvram[(((addr + offset) & 0x1fff) | pagemod) + page_offset] = (uint8)data;
	}
	return;
#endif
}

void DISPLAY::write_mmio(uint32 addr, uint32 data)
{
	uint32 raddr;
	uint8 rval = 0;
#if !defined(_FM77AV_VARIANTS)
	addr = addr & 0x000f;
#else
	addr = addr & 0x003f;
#endif
	io_w_latch[addr] = (uint8)data;
	//if(addr >= 0x02) printf("SUB: IOWRITE PC=%04x, ADDR=%02x DATA=%02x\n", subcpu->get_pc(), addr, val8);
	switch(addr) {
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4)
		// FM77 SPECIFIED
		case 0x05:
			set_cyclesteal((uint8)data);
			break;
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		// KANJI
		case 0x06:
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
			//if(!kanjisub) break;
			if(kanji_level2) {
				kanji2_addr.b.h = (uint8)data;
				mainio->write_signal(FM7_MAINIO_KANJI2_ADDR_HIGH, data, 0xff);
			} else {
				kanji1_addr.b.h = (uint8)data;
				mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_HIGH, data, 0xff);
			}
#else
			//if(!kanjisub) break;
			kanji1_addr.b.h = (uint8)data;
			mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_HIGH, data, 0xff);
#endif				
			break;
		case 0x07:
			//printf("KANJI LO=%02x\n", data);
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
			//if(!kanjisub) break;
			if(kanji_level2) {
				kanji2_addr.b.l = (uint8)data;
				mainio->write_signal(FM7_MAINIO_KANJI2_ADDR_LOW, data, 0xff);
			} else {
				kanji1_addr.b.l = (uint8)data;
				mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_LOW, data, 0xff);
			}
#else
			kanji1_addr.b.l = (uint8)data;
			mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_LOW, data, 0xff);
#endif
			break;
#endif
		// CRT OFF
		case 0x08:
			reset_crtflag();
			break;
		// VRAM ACCESS
		case 0x09:
			reset_vramaccess();
			break;
		// BUSY
		case 0x0a:
			if(clr_count <= 0) {
				set_subbusy();
			} else { // Read once when using clr_foo() to set busy flag.
				double usec = (1000.0 * 1000.0) / 999000.0;
				if(mainio->read_data8(FM7_MAINIO_CLOCKMODE) != FM7_MAINCLOCK_SLOW) usec = (1000.0 * 1000.0) / 2000000.0;
			 	if(!is_cyclesteal) usec = usec * 3.0;
				usec = (double)clr_count * usec;
				register_event(this, EVENT_FM7SUB_CLR, usec, false, NULL); // NEXT CYCLE_
				reset_subbusy();
				clr_count = 0;
			}
			break;
		// LED
		case 0x0d:
			keyboard->write_signal(SIG_FM7KEY_SET_INSLED, 0x00, 0x01);
			break;
		// OFFSET
		case 0x0e:
			rval = (uint8)data;
			tmp_offset_point.b.h = rval;
			offset_changed = !offset_changed;
			if(offset_changed) {
				vram_wrote = true;
#if defined(_FM77AV_VARIANTS)
				if(active_page != 0) {
					offset_point_bank1 = tmp_offset_point.w.l;
				} else {
					offset_point = tmp_offset_point.w.l;
				}
#else
				offset_point = tmp_offset_point.w.l;
#endif				   
			}
			break;
		case 0x0f:
			rval = (uint8)data;
			tmp_offset_point.b.l = rval;
			offset_changed = !offset_changed;
			if(offset_changed) {
				vram_wrote = true;
#if defined(_FM77AV_VARIANTS)
				if(active_page != 0) {
					offset_point_bank1 = tmp_offset_point.w.l;
				} else {
					offset_point = tmp_offset_point.w.l;
				}
#else
				offset_point = tmp_offset_point.w.l;
#endif
			}
 			break;
#if defined(_FM77AV_VARIANTS)
		// ALU
		case 0x10:
			alu_write_cmdreg(data);
			break;
		case 0x11:
			alu_write_logical_color(data);
			break;
		case 0x12:
			alu_write_mask_reg(data);
			break;
		case 0x1b:
			alu_write_disable_reg(data);
			break;
		case 0x20:
			alu_write_offsetreg_hi(data);
			break;
		case 0x21:
			alu_write_offsetreg_lo(data);
			break;
		case 0x22:
			alu_write_linepattern_hi(data);
			break;
		case 0x23:
			alu_write_linepattern_lo(data);
			break;
		// MISC
		case 0x30:
			set_miscreg(data);
			break;
		// KEYBOARD ENCODER
		case 0x31:
			put_key_encoder(data);
			break;
#endif				
		default:
#if defined(_FM77AV_VARIANTS)
			//ALU
			if((addr >= 0x13) && (addr <= 0x1a)) {
				alu_write_cmpdata_reg(addr - 0x13, data);
			} else if((addr >= 0x1c) && (addr <= 0x1e)) {
				alu_write_tilepaint_data(addr - 0x1c, data);
			} else if((addr >= 0x24) && (addr <= 0x2b)) {
				alu_write_line_position(addr - 0x24, data);
			}
#endif				
			break;
		}
}

void DISPLAY::write_data8(uint32 addr, uint32 data)
{
	uint32 mask = 0x3fff;
	uint8 val8 = data & 0xff;
	uint32 offset;
	uint32 raddr;
	uint8 dummy;
	
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) { // Not 400line
		offset = offset_point_bank1 & 0x7fff;
	} else {
		offset = offset_point & 0x7fff; 
	}
	if(!offset_77av) {
		offset = offset & 0x7fe0;
	}
#else
	offset = offset_point &0x7fe0;
#endif
	if(addr < 0xc000) {
#if defined(_FM77AV_VARIANTS)
		uint32 color;
		if(use_alu) {
			dummy = alu->read_data8((addr & 0x3fff) + ALU_WRITE_PROXY);
			return;
		}
		if(!(is_cyclesteal | vram_accessflag)) return;
		color  = (addr & 0x0c000) >> 14;
		if((multimode_accessmask & (1 << color)) != 0) return;
		
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			write_vram_8_400l(addr, offset, data);
		} else if(display_mode == DISPLAY_MODE_256k) {
			write_vram_256k(addr, offset, data);
		} else	if(display_mode == DISPLAY_MODE_4096) {
			write_vram_4096(addr, offset, data);
		} else { // 8Colors, 200LINE
			write_vram_8_200l(addr, offset, data);
		}
#else		
		if(mode320) {
			write_vram_4096(addr, offset, data);
		} else {
			write_vram_8_200l(addr, offset, data);
		}
#endif		
		return;
#else //_FM77L4
		if(display_mode == DISPLAY_MODE_8_400L) {
			write_vram_l4_400l(addr, offset, data);
		} else {
			uint32 color = (addr & 0x0c000) >> 14;
			if((multimode_accessmask & (1 << color)) != 0) return;
			write_vram_8_200l(addr, offset, data);
		}
		return;
#endif
	} else if(addr < 0xd000) { 
		raddr = addr - 0xc000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(submon_bank == 4) {
			if(cgram_bank >= 1) {
				submem_cgram[((cgram_bank - 1) << 12) | addr] = val8;
				return;
			}
		}
#endif
		console_ram[raddr] = val8;
		return;
	} else if(addr < 0xd380) {
		raddr = addr - 0xd000;
		work_ram[raddr] = val8;
		return;
	} else if(addr < 0xd400) {
		raddr = addr - 0xd380;
		shared_ram[raddr] = val8;
		return;
	} else if(addr < 0xd800) {
		write_mmio(addr, data);
		return;
	} else if(addr < 0x10000) {
#if defined(_FM77AV_VARIANTS)
		//subrom_cg[addr - 0xd800] = val8;
#endif
		return;
	} else if((addr >= FM7_SUBMEM_OFFSET_DPALETTE) && (addr < (FM7_SUBMEM_OFFSET_DPALETTE + 8))) {
		set_dpalette(addr - FM7_SUBMEM_OFFSET_DPALETTE, val8);
		return;
	}
#if defined(_FM77AV_VARIANTS)
	// ANALOG PALETTE
	else if(addr == FM7_SUBMEM_OFFSET_APALETTE_R) {
		set_apalette_r(val8);
		return;
	} else if(addr == FM7_SUBMEM_OFFSET_APALETTE_G) {
		set_apalette_g(val8);
		return;
	} else if(addr == FM7_SUBMEM_OFFSET_APALETTE_B) {
		set_apalette_b(val8);
		return;
	} else if(addr == FM7_SUBMEM_OFFSET_APALETTE_HI) {
		set_apalette_index_hi(val8);
		return;
	} else if(addr == FM7_SUBMEM_OFFSET_APALETTE_LO) {
		set_apalette_index_lo(val8);
		return;
	}
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0x30000))) {
	}
# else
	// ACCESS VIA ALU.
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0x0c000))) {
		write_vram_8_200l(addr - DISPLAY_VRAM_DIRECT_ACCESS, offset, data);
		return;
	}
# endif
#endif
	return;
}	


uint32 DISPLAY::read_bios(const char *name, uint8 *ptr, uint32 size)
{
	FILEIO fio;
	uint32 blocks;
	_TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = emu->bios_path((_TCHAR *)name);
	if(s == NULL) return 0;
  
	if(!fio.Fopen(s, FILEIO_READ_BINARY)) return 0;
	blocks = fio.Fread(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}


void DISPLAY::initialize()
{
	int i;

	memset(gvram, 0x00, sizeof(gvram));
	memset(console_ram, 0x00, sizeof(console_ram));
	memset(work_ram, 0x00, sizeof(work_ram));
	memset(shared_ram, 0xff, sizeof(shared_ram));
	memset(subsys_c, 0xff, sizeof(subsys_c));
   
	diag_load_subrom_c = false;
	if(read_bios(_T("SUBSYS_C.ROM"), subsys_c, 0x2800) >= 0x2800) diag_load_subrom_c = true;
	emu->out_debug_log("SUBSYSTEM ROM Type C READING : %s\n", diag_load_subrom_c ? "OK" : "NG");
 
#if defined(_FM77AV_VARIANTS)
	memset(subsys_a, 0xff, sizeof(subsys_a));
	memset(subsys_b, 0xff, sizeof(subsys_b));
	memset(subsys_cg, 0xff, sizeof(subsys_cg));
	memset(subsys_ram, 0x00, sizeof(subsys_ram));
   
	diag_load_subrom_a = false;
   	if(read_bios(_T("SUBSYS_A.ROM"), subsys_a, 0x2000) >= 0x2000) diag_load_subrom_a = true;
	emu->out_debug_log("SUBSYSTEM ROM Type A READING : %s\n", diag_load_subrom_a ? "OK" : "NG");

	diag_load_subrom_b = false;
   	if(read_bios(_T("SUBSYS_B.ROM"), subsys_b, 0x2000) >= 0x2000) diag_load_subrom_b = true;
	emu->out_debug_log("SUBSYSTEM ROM Type B READING : %s\n", diag_load_subrom_b ? "OK" : "NG");

	diag_load_subrom_cg = false;
   	if(read_bios(_T("SUBSYSCG.ROM"), subsys_cg, 0x2000) >= 0x2000) diag_load_subrom_cg = true;
	emu->out_debug_log("SUBSYSTEM CG ROM READING : %s\n", diag_load_subrom_cg ? "OK" : "NG");
	power_on_reset = true;
	mode320 = false;
#endif
#if defined(_FM77AV_VARIANTS)
	apalette_index.d = 0;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = i & 0xf00;
		analog_palette_b[i] = i & 0x00f;
	}
#endif
	for(i = 0; i < 8; i++) set_dpalette(i, i);
	vram_wrote = true;
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	offset_77av = false;
	display_mode = DISPLAY_MODE_8_200L;
	crt_flag = true;
	switch(config.cpu_type){
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
#if defined(_FM77AV_VARIANTS)
   	subrom_bank = 0;
	subrom_bank_using = 0;
	cgrom_bank = 0;
	kanjisub = false;
	display_page = 0;
	active_page = 0;
	nmi_enable = true;
	offset_point_bank1 = 0;
	use_alu = false;
	subcpu_resetreq = false;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	mode400line = false;
	mode256k = false;
#endif	
#if defined(_FM77_VARIANTS)
	mode400line = false;
#endif
	tmp_offset_point.d = 0;
	offset_point = 0;
	halt_flag = false;
	do_attention = false;
	firq_mask = false;
	key_firq_req = false;
	//register_frame_event(this);
	//register_vline_event(this);
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
	nmi_event_id = -1;
}

void DISPLAY::release()
{
}

