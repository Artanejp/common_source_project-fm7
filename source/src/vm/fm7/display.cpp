/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

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
	//	initvramtbl_4096_vec();
}

DISPLAY::~DISPLAY()
{

}

void DISPLAY::reset(void)
{
	int i;
	uint32 subclock;
	
	if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	if(hblank_event_id >= 0) cancel_event(this, hblank_event_id);
	if(hdisp_event_id >= 0) cancel_event(this, hdisp_event_id);
	if(vsync_event_id >= 0) cancel_event(this, vsync_event_id);
	if(vstart_event_id >= 0) cancel_event(this, vstart_event_id);
	if(halt_event_id >= 0) cancel_event(this, halt_event_id);
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	halt_event_id = -1;
#if defined(_FM77AV_VARIANTS)
	display_page = 0;
	active_page = 0;
	cgrom_bank = 0;
	if(!subcpu_resetreq) {
	   //vram_bank = 0;
		subrom_bank = 0;
		subrom_bank_using = 0;
	}
	nmi_enable = true;
	offset_point_bank1 = 0;
	use_alu = false;
#endif
	for(i = 0; i < 8; i++) set_dpalette(i, i);
	offset_77av = false;
	sub_run = true;
	crt_flag = true;
	vram_accessflag = false;
	vram_wait = false;
	clr_count = 0;
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	
	vblank = false;
	vsync = false;
	hblank = true;
	halt_flag = false;
	cancel_request = false;
	firq_backup = false;
	irq_backup = false;
	displine = 0;
	
	set_cyclesteal(config.dipswitch & FM7_DIPSW_CYCLESTEAL); // CYCLE STEAL = bit0.
	vram_wrote = true;
	window_low = 0;
	window_high = 200;
	window_xbegin = 0;
	window_xend = 80;
	window_opened = false;
	offset_point = 0;
	tmp_offset_point = 0;
	offset_changed = true;
	halt_count = 0;

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

	if(!subcpu_resetreq) display_mode = DISPLAY_MODE_8_200L;
	subcpu_resetreq = false;
	
	register_event(this, EVENT_FM7SUB_VSTART, 1.0 * 1000.0, false, &vstart_event_id);   
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	sub_busy = true;
	subcpu->reset();
}

void DISPLAY::update_config(void)
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

inline int DISPLAY::GETVRAM_8_200L(int yoff, scrntype *p, uint32 rgbmask)
{
	register uint8 b, r, g;
	register uint32 dot;
	uint32 yoff_d;
#if defined(_FM77AV_VARIANTS)
	if(display_page == 1) {
		yoff_d = offset_point_bank1;
	} else {
		yoff_d = offset_point;
	}
#else
	yoff_d = offset_point;
#endif	
	if(!offset_77av) {
		yoff_d &= 0x7fe0;
	}
	yoff_d = (yoff + yoff_d) & 0x3fff;
	//if(display_page != active_page) return yoff + 1;
#if defined(_FM77AV_VARIANTS)
	if(display_page == 1) {
		b = gvram[yoff_d + 0x0c000];
		r = gvram[yoff_d + 0x10000];
		g = gvram[yoff_d + 0x14000];
	} else {
		b = gvram[yoff_d + 0x00000];
		r = gvram[yoff_d + 0x04000];
		g = gvram[yoff_d + 0x08000];
	}
#else
	b = gvram[yoff_d + 0x00000];
	r = gvram[yoff_d + 0x04000];
	g = gvram[yoff_d + 0x08000];
#endif	
	dot = ((g & 0x80) >> 5) | ((r & 0x80) >> 6) | ((b & 0x80) >> 7);
	*p++ = dpalette_pixel[dot & rgbmask];
	dot = ((g & 0x40) >> 4) | ((r & 0x40) >> 5) | ((b & 0x40) >> 6);
	*p++ = dpalette_pixel[dot & rgbmask];
	dot = ((g & 0x20) >> 3) | ((r & 0x20) >> 4) | ((b & 0x20) >> 5);
	*p++ = dpalette_pixel[dot & rgbmask];
	dot = ((g & 0x10) >> 2) | ((r & 0x10) >> 3) | ((b & 0x10) >> 4);
	*p++ = dpalette_pixel[dot & rgbmask];
					
	dot = ((g & 0x8) >> 1) | ((r & 0x8) >> 2) | ((b & 0x8) >> 3);
	*p++ = dpalette_pixel[dot & rgbmask];
	dot = (g & 0x4) | ((r & 0x4) >> 1) | ((b & 0x4) >> 2);
	*p++ = dpalette_pixel[dot & rgbmask];
	dot = ((g & 0x2) << 1) | (r & 0x2) | ((b & 0x2) >> 1);
	*p++ = dpalette_pixel[dot & rgbmask];
	dot = ((g & 0x1) << 2) | ((r & 0x1) << 1) | (b & 0x1);
	*p = dpalette_pixel[dot & rgbmask];
	yoff++;
	return yoff;
}

#if defined(_FM77AV_VARIANTS)
inline int DISPLAY::GETVRAM_4096(int yoff, scrntype *p, uint32 rgbmask)
{
	uint32 b0, r0, g0;
	uint32 b1, r1, g1;
	uint32 b2, r2, g2;
	uint32 b3, r3, g3;
	scrntype b, r, g;
	register uint32 dot;
	uint32 mask = rgbmask;
	scrntype bmask, rmask, gmask;
	scrntype pmask, pixel;
	uint32 yoff_d1, yoff_d2;
	int i;

	if(offset_77av) {
		yoff_d1 = offset_point;
		yoff_d2 = offset_point_bank1;
	} else {
		yoff_d1 = offset_point & 0x7fe0;
		yoff_d2 = offset_point_bank1 & 0x7fe0;
	}
	yoff_d1 = (yoff + yoff_d1) & 0x1fff;
	yoff_d2 = (yoff + yoff_d2) & 0x1fff;
	//mask = 0x0fff;
	b3 = gvram[yoff_d1];
	b2 = gvram[yoff_d1 + 0x02000];
	r3 = gvram[yoff_d1 + 0x04000];
	r2 = gvram[yoff_d1 + 0x06000];
		
	g3 = gvram[yoff_d1 + 0x08000];
	g2 = gvram[yoff_d1 + 0x0a000];
		
	b1 = gvram[yoff_d2 + 0x0c000];
	b0 = gvram[yoff_d2 + 0x0e000];
		
	r1 = gvram[yoff_d2 + 0x10000];
	r0 = gvram[yoff_d2 + 0x12000];
	g1 = gvram[yoff_d2 + 0x14000];
	g0 = gvram[yoff_d2 + 0x16000];
	for(i = 0; i < 8; i++) {
	  g = (g3 & 0x80) | ((g2 >> 1) & 0x40) | ((g1 >> 2) & 0x20) | ((g0 >> 3) & 0x10);
	  r = (r3 & 0x80) | ((r2 >> 1) & 0x40) | ((r1 >> 2) & 0x20) | ((r0 >> 3) & 0x10);
	  b = (b3 & 0x80) | ((b2 >> 1) & 0x40) | ((b1 >> 2) & 0x20) | ((b0 >> 3) & 0x10);
	  pixel = ((g << 4) | (b >> 4) | r ) & mask;
	  g = analog_palette_g[pixel] << 4;
	  r = analog_palette_r[pixel] << 4;
	  b = analog_palette_b[pixel] << 4;
	  g3 <<= 1;
	  g2 <<= 1;
	  g1 <<= 1;
	  g0 <<= 1;
	  
	  r3 <<= 1;
	  r2 <<= 1;
	  r1 <<= 1;
	  r0 <<= 1;
	  
	  b3 <<= 1;
	  b2 <<= 1;
	  b1 <<= 1;
	  b0 <<= 1;
	pixel = RGB_COLOR(r, g, b);
	*p++ = pixel;
	*p++ = pixel;
	}	  
	yoff++;
	return yoff;
}
#endif

void DISPLAY::draw_screen(void)
{
	int y;
	int x;
	int i;
	int height = (display_mode == DISPLAY_MODE_8_400L) ? 400 : 200;
	scrntype *p, *pp, *q;
	register int yoff;
	uint32 planesize;
	uint32 offset;
	register uint32 rgbmask;
	//if(!vram_wrote) return;
	
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
		vram_wrote = false;
		for(y = 0; y < 400; y++) {
			memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
		return;
	}
	if(display_mode == DISPLAY_MODE_8_200L) {
		vram_wrote = false;
		yoff = 0;
		rgbmask = ~multimode_dispmask & 0x07;
		for(y = 0; y < 400; y += 2) {
			p = emu->screen_buffer(y);
			pp = p;
			for(x = 0; x < 10; x++) {
			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;
			  
			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;

  			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;

			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;

			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;
			  
			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;
			  
			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;
			  
			  yoff = GETVRAM_8_200L(yoff, p, rgbmask);
			  p += 8;
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
		uint32 mask;
		vram_wrote = false;
		yoff = 0;
		rgbmask = multimode_dispmask;
		if((rgbmask & 0x01) == 0) mask = 0x00f;
		if((rgbmask & 0x02) == 0) mask = mask | 0x0f0;
		if((rgbmask & 0x04) == 0) mask = mask | 0xf00;
		for(y = 0; y < 400; y += 2) {
			p = emu->screen_buffer(y);
			pp = p;
			for(x = 0; x < 5; x++) {
				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;
			  
				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;
				
				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;

				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;
				
				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;
			  
				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;
				
				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;

				yoff = GETVRAM_4096(yoff, p, mask);
				p += 16;

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
	if(irq_backup == flag) return;
	subcpu->write_signal(SIG_CPU_IRQ, flag ? 1: 0, 1);
	irq_backup = flag;
}

void DISPLAY::do_firq(bool flag)
{
	//if(firq_backup == flag) return;
	subcpu->write_signal(SIG_CPU_FIRQ, flag ? 1: 0, 1);
	firq_backup = flag;   
}

void DISPLAY::do_nmi(bool flag)
{
#if defined(_FM77AV_VARIANTS)
	if(!nmi_enable && flag) return;
	if(!nmi_enable) flag = false;
#endif
	subcpu->write_signal(SIG_CPU_NMI, flag ? 1 : 0, 1);
}

void DISPLAY::set_multimode(uint8 val)
{
	uint8 old_a, old_d;
	int i;
	old_d = multimode_dispmask;
	old_a = multimode_accessmask;
	
	multimode_accessmask = val & 0x07;
	multimode_dispmask = (val & 0x70) >> 4;
	if(old_d != multimode_dispmask) {
#if defined(_FM77AV_VARIANTS)
		for(i = 0; i < 4096; i++) calc_apalette(i);
#endif
	}
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
	bool flag = !(sub_run);
	if(flag) {
		sub_busy = true;
		subcpu->write_signal(SIG_CPU_BUSREQ, 0x01, 0x01);
	}
}

void DISPLAY::go_subcpu(void)
{
	bool flag = sub_run;
	if(flag) {
		subcpu->write_signal(SIG_CPU_BUSREQ, 0x00, 0x01);
	}
   
}

void DISPLAY::enter_display(void)
{
	uint32 subclock;
	if(clock_fast) {
		subclock = SUBCLOCK_NORMAL;
	} else {
		subclock = SUBCLOCK_SLOW;
	}
   
	if(is_cyclesteal || !(vram_accessflag)) {
		vram_wait = false;
	} else {
		vram_wait = true;
		if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) == 0) subclock = subclock / 3;
	}
	//halt_subcpu();
	if(prev_clock != subclock) p_vm->set_cpu_clock(subcpu, subclock);
	prev_clock = subclock;
}

void DISPLAY::leave_display(void)
{
	vram_wait = false;
	//go_subcpu();
}

void DISPLAY::halt_subsystem(void)
{
  	sub_run = false;
  	halt_subcpu();
}

void DISPLAY::restart_subsystem(void)
{
	sub_run = true;
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
	//if(cancel_request) this->do_irq(false);
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
	mainio->write_signal(FM7_MAINIO_SUB_ATTENTION, 0x01, 0x01);
	//printf("DISPLAY: ATTENTION TO MAIN\n");
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
	return 0xff;
}

//SUB:D40A:W
void DISPLAY::set_subbusy(void)
{
	sub_busy = true;
}


#if defined(_FM77AV_VARIANTS)
// D410
void DISPLAY::alu_write_cmdreg(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_CMDREG, data);
	if((val & 0x80) != 0) {
		use_alu = true;
	} else {
		use_alu = false;
	}
}

// D411
void DISPLAY::alu_write_logical_color(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_LOGICAL_COLOR, data);
}

// D412
void DISPLAY::alu_write_mask_reg(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_WRITE_MASKREG, data);
}

// D413 - D41A
void DISPLAY::alu_write_cmpdata_reg(int addr, uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	addr = addr & 7;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_CMPDATA_REG + addr, data);
}

// D41B
void DISPLAY::alu_write_disable_reg(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
  
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	data = data & 0x07 | 0x08;
	alu->write_data8(ALU_BANK_DISABLE, data);
}

// D41C - D41F
void DISPLAY::alu_write_tilepaint_data(int addr, uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
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
	bool busyflag;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	if(display_mode == DISPLAY_MODE_8_400L) {
		alu->write_data8(ALU_OFFSET_REG_HIGH, val & 0x7f);
	} else {
		alu->write_data8(ALU_OFFSET_REG_HIGH, val & 0x3f);
	}
}
 
// D421
void DISPLAY::alu_write_offsetreg_lo(uint8 val)
{
	bool busyflag;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_OFFSET_REG_LO, val);
}

// D422
void DISPLAY::alu_write_linepattern_hi(uint8 val)
{
	bool busyflag;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_LINEPATTERN_REG_HIGH, val);
}

// D423
void DISPLAY::alu_write_linepattern_lo(uint8 val)
{
	bool busyflag;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
	alu->write_data8(ALU_LINEPATTERN_REG_LO, val);
}

// D424-D42B
void DISPLAY::alu_write_line_position(int addr, uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	//busyflag = alu->read_signal(SIG_ALU_BUSYSTAT) ? true : false;
	//if(busyflag) return; // DISCARD
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
	ret = 0x6a;
	if(!hblank && !vblank) ret |= 0x80;
	if(vsync) ret |= 0x04;
	if(alu->read_signal(SIG_ALU_BUSYSTAT) == 0) ret |= 0x10;
	if(power_on_reset) ret |= 0x01;
#else // 77 or older.
	ret = 0xff;
#endif
	return ret;
}
//SUB:D430:W
void DISPLAY::set_miscreg(uint8 val)
{
	int y;
	nmi_enable = ((val & 0x80) == 0) ? true : false;
	if((val & 0x40) == 0) {
		if(display_page != 0) {
			for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
		display_page = 0;
	} else {
		if(display_page != 1) {
			for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
		display_page = 1;
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
	return keyboard->read_data8(0x32);
}


// Main: FD13
void DISPLAY::set_monitor_bank(uint8 var)
{
	uint8 subrom_bank_bak = subrom_bank;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if((var & 0x04) != 0){
		monitor_ram = true;
	} else {
		monitor_ram = false;
	}
#endif
	subrom_bank = var & 0x03;
	subrom_bank_using = var & 0x03;
	subcpu_resetreq = true;
	power_on_reset = false;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(halt_flag) {
		this->reset();
		subcpu->reset();
		halt_flag = false;
	} else {
	  //subspu->reset();
		halt_flag = true;
	}
#else
	this->reset();
	//subcpu->reset();
#endif
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

void DISPLAY::calc_apalette(uint32 index)
{
	scrntype r, g, b;
	uint32 mask = 0;
	int i;
	vram_wrote = true;
	r = g = b = 0;

	if((multimode_dispmask & 0x01) == 0) mask  = 0x00f;
	if((multimode_dispmask & 0x02) == 0) mask += 0x0f0;
	if((multimode_dispmask & 0x04) == 0) mask += (256 * 16);
	//for(index = 0; index < 4096; index++) { 
	r = (analog_palette_r[index & mask] & 0x0f) << 4; // + 0x0f?
	g = (analog_palette_g[index & mask] & 0x0f) << 4; // + 0x0f?
	b = (analog_palette_b[index & mask] & 0x0f) << 4; // + 0x0f?
	apalette_pixel[index & mask] = RGB_COLOR(r, g, b);
	//apalette_pixel[index] = RGB_COLOR(r, g, b);

	//}
}

// FD32
void DISPLAY::set_apalette_b(uint8 val)
{
	uint16 index;
	index = apalette_index.w.l;
	analog_palette_b[index] = val & 0x0f;
	calc_apalette(index);
	//printf("APALETTE: baccess %04x %d\n", index, analog_palette_b[index]); 
}

// FD33
void DISPLAY::set_apalette_r(uint8 val)
{
	uint16 index;
	index = apalette_index.w.l;
	analog_palette_r[index] = val & 0x0f;
	calc_apalette(index);
}

// FD34
void DISPLAY::set_apalette_g(uint8 val)
{
	uint16 index;
	index = apalette_index.w.l;
	analog_palette_g[index] = val & 0x0f;
	calc_apalette(index);
}

#endif // _FM77AV_VARIANTS

// Timing values from XM7 . Thanks Ryu.
void DISPLAY::event_callback(int event_id, int err)
{
	double usec;
	uint64 haltclocks;
	bool f;
	switch(event_id) {
  		case EVENT_FM7SUB_DISPLAY_NMI: // per 20.00ms
			do_nmi(true);
			//register_event(this, EVENT_FM7SUB_DISPLAY_NMI_OFF, 1.0 * 1000.0, false, NULL); // NEXT CYCLE_
			break;
  		case EVENT_FM7SUB_DISPLAY_NMI_OFF: // per 20.00ms
			do_nmi(false);
			break;
		case EVENT_FM7SUB_HDISP:
			hblank = false;
			usec = 39.5;
			if(display_mode == DISPLAY_MODE_8_400L) usec = 30.0;
			register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hblank_event_id); // NEXT CYCLE_
			enter_display();
			break;
		case EVENT_FM7SUB_HBLANK:
			hblank = true;
			f = false;
			displine++;
			//if(!is_cyclesteal && vram_accessflag)  leave_display();
			leave_display();
			if(display_mode == DISPLAY_MODE_8_400L) {
				usec = 11.0;
				if(displine < 400) f = true; 
			} else {
				usec = 24.0;
				if(displine < 200) f = true;
			}
			if(f) {
				register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id); // NEXT CYCLE_
			} else {
				// Vblank?
				vblank = true;
				vsync = true;
				if(display_mode == DISPLAY_MODE_8_400L) {
					usec = 0.31 * 1000.0;
				} else {
					usec = 0.51 * 1000.0;
				}
				register_event(this, EVENT_FM7SUB_VSYNC, usec, false, &vsync_event_id); // NEXT CYCLE_
			}
			break;
		case EVENT_FM7SUB_VSTART: // Call first.
			vblank = false;
			vsync = false;
			hblank = true;
			displine = 0;
			leave_display();
			if(display_mode == DISPLAY_MODE_8_400L) {
				usec = 11.0;
			} else {
				usec = 24.0;
			}
			register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id); // NEXT CYCLE_
			break;
		case EVENT_FM7SUB_VSYNC:
			vblank = true;
			vsync = false;
			if(display_mode == DISPLAY_MODE_8_400L) {
				usec = 0.98 * 1000.0;
			} else {
				usec = 1.91 * 1000.0;
			}
			register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
			break;
		case EVENT_FM7SUB_HALT:
	   		//if(!sub_run) {
				//halt_subcpu();
				//mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0x01, 0x01); // BUSY
				//cancel_event(this, halt_event_id);
		   	   	//halt_event_id = -1;
				//halt_flag = true;		   
			//}
	   		break;
		 case EVENT_FM7SUB_CLR:
			set_subbusy();
	   		break;
	}
}

void DISPLAY::event_frame()
{
  
}

void DISPLAY::event_vline(int v, int clock)
{
}

uint32 DISPLAY::read_signal(int id)
{
	uint32 retval;
	switch(id) {
		case SIG_FM7_SUB_HALT:
		case SIG_DISPLAY_HALT:
			return (halt_flag) ? 0 : 0xffffffff;
			break;
		case SIG_DISPLAY_BUSY:
			return (sub_busy) ? 0x80 : 0;
		 	break;
		case SIG_FM7_SUB_MULTIPAGE:
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
			if(cancel_request && flag) {
				sub_run = true;
				subcpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
				//printf("SUB: HALT : CANCEL\n");
				return;
			}
			halt_flag = flag;
			//printf("SUB: HALT : DID STAT=%d\n", flag);   
			break;
       		case SIG_DISPLAY_HALT:
			if(flag) {
				halt_subsystem();
			} else {
#if 0
				if(subcpu_resetreq) {
					vram_wrote = true;
					power_on_reset = false;
					subrom_bank_using = subrom_bank;
					//subcpu->reset();
					this->reset();
					subcpu_resetreq = false;
					//restart_subsystem();
				} else {
					restart_subsystem();
				}
#else
				restart_subsystem();
#endif
			}
			break;
		case SIG_FM7_SUB_CANCEL:
			if(flag) {
				cancel_request = true;
				//printf("MAIN: CANCEL REQUEST TO SUB\n");
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
		  //key_rxrdy = ((data & mask) != 0);
			break;
		case SIG_FM7KEY_ACK: // D432 bit 0
		  //key_ack = ((data & mask) != 0);
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
			//printf("MODE320: %d\n", display_mode);
#endif
			break;
#endif // _FM77AV_VARIANTS
		case SIG_FM7_SUB_MULTIPAGE:
		case SIG_DISPLAY_MULTIPAGE:
	  		set_multimode(data & 0xff);
			break;
		case SIG_FM7_SUB_KEY_FIRQ:
			if(flag) do_firq(true);
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

    
uint32 DISPLAY::read_data8(uint32 addr)
{
	uint32 raddr = addr;;
	uint32 mask = 0x3fff;
	uint32 offset;
	uint8 retval;
	uint32 dummy;
	//	addr = addr & 0x00ffffff;
	
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
	offset = offset_point &0x7fe0;
#endif
    
	if(addr < 0xc000) {
		uint32 pagemod;
		uint32 page_offset;
		//if(!(is_cyclesteal | vram_accessflag)) return 0xff;
#if defined(_FM77L4)
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr < 0x8000) {
				if(workram) {
				  raddr = addr & 0x3fff;
				  if((multimode_accessmask & 0x04) == 0) {
				  	return gvram[0x8000 + (raddr + offset) & mask];
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
		}
#elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		page_offset = 0;
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32 color = vram_bank & 0x03;
			raddr = raddr & 0xbfff; 
			if((multimode_accessmask & (1 << color)) != 0) {
				return 0xff;
			} else {
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
		} else if(display_mode == DISPLAY_MODE_256k) {
			uint32 color = 0; // b
			mask = 0x1fff;
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
			if((multimode_accessmask & (1 << color)) != 0) {
				return 0xff;
			} else {
				pagemod = addr & 0xe000;
				return gvram[(((addr + offset) & mask) | pagemod) + page_offset];
			}
		} else	if(display_mode == DISPLAY_MODE_4096) {
			uint32 color = 0; // b
			mask = 0x1fff;
			pagemod = addr & 0xe000;
			if(vram_page) page_offset = 0x18000;
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
			if((multimode_accessmask & (1 << color)) != 0) {
				return 0xff;
			} else {
				return gvram[(((addr + offset) & mask) | pagemod) + page_offset];
			}
		} else { // 8Colors, 200LINE
			mask = 0x3fff;
			pagemod = (addr & 0xc000) >> 14;
			if(vram_page) page_offset = 0x18000;
			if((multimode_accessmask & (1 << pagemod)) != 0) {
				return 0xff;
			} else {
				return gvram[(((addr + offset) & mask) | (pagemod << 14)) + page_offset];
			}
		}
#elif defined(_FM77AV_VARIANTS)
		page_offset = 0;
		uint32 color = 0; // b
		uint32 dummy;
		color = (addr & 0x0c000) >> 14;
		if(active_page != 0) page_offset = 0xc000;
		if(mode320) {
			mask = 0x1fff;
			pagemod = addr >> 13;
			if((alu->read_data8(ALU_CMDREG) & 0x80) != 0) {
				dummy = alu->read_data8((addr & 0x3fff) + ALU_WRITE_PROXY);
			}
			if((multimode_accessmask & (1 << color)) != 0) {
				return 0xff;
			} else {
				retval = gvram[(((addr + offset) & mask) | (pagemod << 13)) + page_offset];
				return retval;
			}
		} else {
			mask = 0x3fff;
			pagemod = (addr & 0xc000) >> 14;
			if((alu->read_data8(ALU_CMDREG) & 0x80) != 0) {
				dummy = alu->read_data8((addr & 0x3fff) + ALU_WRITE_PROXY);
			}
			if((multimode_accessmask & (1 << pagemod)) != 0) {
				return 0xff;
			} else {
				retval = gvram[(((addr + offset) & mask) | (pagemod << 14)) + page_offset];
				return retval;
			}
		}
#else //_FM77L4
		pagemod = addr >> 14;
		if((multimode_accessmask & (1 << pagemod)) != 0) {
			return 0xff;
		} else {
			pagemod = addr & 0xc000;
			return gvram[((addr + offset) & mask) | pagemod];
		}
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
		retval = 0xff;
#if !defined(_FM77AV_VARIANTS)
		raddr = (addr - 0xd400) & 0x000f;
#else
		raddr = (addr - 0xd400) & 0x003f;
#endif
		//if(addr >= 0x02) printf("SUB: IOREAD PC=%04x, ADDR=%02x\n", subcpu->get_pc(), addr);
		switch(raddr) {
			case 0x00: // Read keyboard
				retval = (keyboard->read_data8(0x0) & 0x80) | 0x7f;
				break;
			case 0x01: // Read keyboard
				do_firq(false);
				mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 0, 1);
				retval = keyboard->read_data8(1);
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
				vram_wrote = true;
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
			case 0x10:
				retval = alu->read_data8(ALU_CMDREG);
				break;
			case 0x11:
				retval = alu->read_data8(ALU_LOGICAL_COLOR) & 0x07;
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
			case 0x30:
				retval = get_miscreg();
				break;
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
#if defined(_FM77AV_VARIANTS)
		if((raddr >= 0x14) && (raddr < 0x1b)) {
			retval = 0xff;
		} 
		if((raddr >= 0x1c) && (raddr < 0x20)) {
			retval = 0xff;
		} 
		if((raddr >= 0x20) && (raddr < 0x2c)) {
			retval = 0xff;
		} 
#endif
		return retval;
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
		uint32 tmp_offset = 0;
		uint32 rpage;
		uint32 rofset;
		if(active_page != 0) tmp_offset = 0xc000;
		addr = addr - DISPLAY_VRAM_DIRECT_ACCESS;
		rpage = (addr & 0xc000) >> 14;
		if(((1 << rpage) & multimode_accessmask) != 0) return 0xff;
		if(mode320) {
		  	raddr  = (addr + offset) & 0x1fff;
			rofset = addr & 0xe000;
		} else { // 640x200
		  	raddr = (addr + offset) & 0x3fff;
			rofset = addr & 0xc000;
		}
		//{
		//	uint32 rpc = subcpu->get_pc();
		//	printf("SUBCPU: %04x %02x %02x %02x\n",
		//	       rpc, this->read_data8(rpc),
		//	       this->read_data8((rpc + 1) & 0xffff), this->read_data8((rpc + 2) & 0xffff));
		//}
		return gvram[(raddr | rofset) + tmp_offset];
	}
#endif
	return 0xff;
}	



void DISPLAY::write_data8(uint32 addr, uint32 data)
{
	uint32 mask = 0x3fff;
	uint8 val8 = data & 0xff;
	uint32 rval;
	uint32 offset;
	uint8 dummy;
	//	addr = addr & 0x00ffffff;
	
#if defined(_FM77AV_VARIANTS)
	if(active_page != 0) {
		offset = offset_point_bank1 & 0x7fff;
	} else {
		offset = offset_point & 0x7fff; 
	}
#else
	offset = offset_point &0x7fe0;
#endif
    
#if defined(_FM77AV_VARIANTS)
	if(!offset_77av) {
		offset = offset & 0x7fe0;
	}
#endif
	if(addr < 0xc000) {
		uint32 pagemod;
		//if(!(is_cyclesteal | vram_accessflag)) return;
#if defined(_FM77L4)
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr < 0x8000) {
				if(workram) {
					raddr = addr & 0x3fff;
					if((multimode_accessmask & 0x04) == 0) {
						gvram[0x8000 + (raddr + offset) & mask] = val8;
					}
					return;
				}
				pagemod = addr & 0x4000;
				gvram[((addr + offset) & mask) | pagemod] = val8;
			} else if(addr < 0x9800) {
				textvram[addr & 0x0fff] = val8;
			} else { // $9800-$bfff
				//return subrom_l4[addr - 0x9800];
				return;
			}
			return;
		}
#elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		page_offset = 0;
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32 color = vram_bank & 0x03;
			raddr = raddr & 0xbfff; 
			if((multimode_accessmask & (1 << color)) != 0) {
				return;
			} else {
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
						return;
						break;
				}
				gvram[raddr] = val8;
				return;
			}
		} else if(display_mode == DISPLAY_MODE_256k) {
			uint32 color = 0; // b
			mask = 0x1fff;
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
				gvram[(((addr + offset) & mask) | pagemod) + page_offset] = val8;
			}
			return;
		} else	if(display_mode == DISPLAY_MODE_4096) {
			uint32 color = 0; // b
			mask = 0x1fff;
			pagemod = addr & 0xe000;
			if(vram_page) page_offset = 0x18000;
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
				gvram[(((addr + offset) & mask) | pagemod) + page_offset] = val8;
			}
			return;
		} else { // 8Colors, 200LINE
			mask = 0x3fff;
			pagemod = (addr & 0xc000) >> 14;
			if(vram_page) page_offset = 0x18000;
			if((multimode_accessmask & (1 << pagemod)) == 0) {
				gvram[(((addr + offset) & mask) | (pagemod << 14)) + page_offset] = val8;
			}
			return;
		}
#elif defined(_FM77AV_VARIANTS)
		uint32 page_offset;
		page_offset = 0;
		uint32 color = 0; // b
		color = (addr & 0x0c000) >> 14;
		if(active_page != 0) {
			page_offset = 0xc000;
		}
		if(mode320) {
			mask = 0x1fff;
			pagemod = (addr & 0xe000) >> 13;
			if((alu->read_data8(ALU_CMDREG) & 0x80) != 0) {
				dummy = alu->read_data8((addr  & 0x3fff) + ALU_WRITE_PROXY);
				return;
			}
			if((multimode_accessmask & (1 << color)) == 0) {
				gvram[(((addr + offset) & mask) | (addr & 0xe000)) + page_offset] = val8;
			}
			return;
		} else {
			mask = 0x3fff;
			pagemod = (addr & 0xc000) >> 14;
			if((alu->read_data8(ALU_CMDREG) & 0x80) != 0) {
				dummy = alu->read_data8((addr & 0x3fff) + ALU_WRITE_PROXY);
				return;
			}	
			if((multimode_accessmask & (1 << pagemod)) == 0) {
			  	gvram[(((addr + offset) & mask) | (pagemod << 14)) + page_offset] = val8;
			}
			return;
		}
#else //_FM77L4
		pagemod = addr >> 14;
		if((multimode_accessmask & (1 << pagemod)) == 0) {
			pagemod = addr & 0xc000;
			gvram[((addr + offset) & mask) | pagemod] = val8;
			if(crt_flag && ((multimode_dispmask & (1 << pagemod)) == 0)) vram_wrote = true;
		}
		return;
#endif
	} else if(addr < 0xd000) { 
		addr = addr - 0xc000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(submon_bank == 4) {
			if(cgram_bank >= 1) {
				submem_cgram[((cgram_bank - 1) << 12) | addr] = val8;
				return;
			}
		}
#endif
		console_ram[addr] = val8;
		return;
	} else if(addr < 0xd380) {
		addr -= 0xd000;
		work_ram[addr] = val8;
		return;
	} else if(addr < 0xd400) {
		addr -= 0xd380;
		shared_ram[addr] = val8;
		return;
	} else if(addr < 0xd800) {
#if !defined(_FM77AV_VARIANTS)
		addr = (addr - 0xd400) & 0x000f;
#else
		addr = (addr - 0xd400) & 0x003f;
#endif
		//if(addr >= 0x02) printf("SUB: IOWRITE PC=%04x, ADDR=%02x DATA=%02x\n", subcpu->get_pc(), addr, val8);
		switch(addr) {
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4)
			case 0x05:
				set_cyclesteal(val8);
				break;
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
			case 0x06:
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
				//if(!kanjisub) break;
				if(kanji_level2) {
					kanji2_addr.b.h = data;
					mainio->write_signal(FM7_MAINIO_KANJI2_ADDR_HIGH, data, 0xff);
				} else {
					kanji1_addr.b.h = data;
					mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_HIGH, data, 0xff);
				}
#else
				//if(!kanjisub) break;
				kanji1_addr.b.h = data;
				mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_HIGH, data, 0xff);
#endif				
				break;
			case 0x07:
			  //printf("KANJI LO=%02x\n", data);
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
				//if(!kanjisub) break;
				if(kanji_level2) {
					kanji2_addr.b.l = data;
					mainio->write_signal(FM7_MAINIO_KANJI2_ADDR_LOW, data, 0xff);
				} else {
					kanji1_addr.b.l = data;
					mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_LOW, data, 0xff);
				}
#else
				kanji1_addr.b.l = data;
				mainio->write_signal(FM7_MAINIO_KANJI1_ADDR_LOW, data, 0xff);
#endif
				break;
#endif	        
			case 0x08:
				vram_wrote = true;
				reset_crtflag();
				break;
			case 0x09:
				reset_vramaccess();
				break;
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
			case 0x0d:
				keyboard->write_signal(SIG_FM7KEY_SET_INSLED, 0x00, 0x01);
				break;
			case 0x0e:
#if defined(_FM77L4) || defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
				rval = (data & 0x7f) << 8;
				if(display_mode != DISPLAY_MODE_8_400L) rval = rval & 0x3fff;		  
#else
				rval = (data & 0x3f) << 8;
#endif
				tmp_offset_point = (tmp_offset_point & 0x00ff) | rval;
				offset_changed = !offset_changed;
				if(offset_changed) {
#if defined(_FM77AV_VARIANTS)
					if(active_page != 0) {
						offset_point_bank1 = tmp_offset_point;
					} else {
						offset_point = tmp_offset_point;
					}
#else
					offset_point = tmp_offset_point;
#endif				   
				}
 				break;
			case 0x0f:
#if defined(_FM77AV_VARIANTS)
				rval = data & 0x00ff;
				if(!offset_77av) rval = rval & 0xe0;		  
#else
				rval = data & 0x00e0;
#endif
				tmp_offset_point = (tmp_offset_point & 0x7f00) | rval;
				offset_changed = !offset_changed;
				if(offset_changed) {
#if defined(_FM77AV_VARIANTS)
					if(active_page != 0) {
						offset_point_bank1 = tmp_offset_point;
					} else {
						offset_point = tmp_offset_point;
					}
#else
					offset_point = tmp_offset_point;
#endif				   
   					vram_wrote = true;
				}
 				break;
#if defined(_FM77AV_VARIANTS)
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
			case 0x30:
				set_miscreg(data);
				break;
			case 0x31:
				put_key_encoder(data);
				break;
#endif				
			default:
#if defined(_FM77AV_VARIANTS)
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
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0x0c000))) {
		uint32 tmp_offset = 0;
		uint32 rpage;
		uint32 rofset;
		uint32 raddr;
		if(active_page != 0) tmp_offset = 0xc000;
		addr = addr - DISPLAY_VRAM_DIRECT_ACCESS;
		rpage = (addr & 0xc000) >> 14;
		if(((1 << rpage) & multimode_accessmask) != 0) return;
		if(mode320) {
		  	raddr  = (addr + offset) & 0x1fff;
			rofset = addr & 0xe000;
		} else { // 640x200
		  	raddr = (addr + offset) & 0x3fff;
			rofset = addr & 0xc000;
		}		  
		//{
		//	uint32 rpc = subcpu->get_pc();
		//	printf("SUBCPU: %04x %02x %02x %02x\n",
		//	       rpc, this->read_data8(rpc), this->read_data8((rpc + 1) & 0xffff),
		//	       this->read_data8((rpc + 2) & 0xffff));
		//}
		gvram[(raddr | rofset) + tmp_offset] = val8;
		return;
	}
# endif
#endif
	return;
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
	memset(apalette_pixel, 0x00, 4096 * sizeof(scrntype));
	apalette_index.d = 0;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = i & 0xf00;
		analog_palette_b[i] = i & 0x00f;
		calc_apalette(i);
	}
#endif
	for(i = 0; i < 8; i++) set_dpalette(i, i);
	vram_wrote = true;
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	offset_77av = false;
	display_mode = DISPLAY_MODE_8_200L;
	subcpu_resetreq = false;
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
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	mode400line = false;
	mode256k = false;
#endif	
#if defined(_FM77_VARIANTS)
	mode400line = false;
#endif
}

void DISPLAY::release()
{
	MEMORY::release();
}

