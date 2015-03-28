/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#include "fm7_display.h"

extern "C" {

  extern void initvramtbl_4096_vec(void);
  extern void detachvramtbl_4096_vec(void);
  extern void PutBlank(uint32 *p, int height);
  extern void CreateVirtualVram8_Line(uint8 *src, uint32 *p, int ybegin, uint32 *pal);
  extern void CreateVirtualVram8_WindowedLine(uint8 *vram_1, uint8 *vram_w, Uint32 *p, int ybegin, int xbegin, int xend, uint32 *pal);
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
	for(i = 0; i < 8; i++) set_dpalette(i, i);
	
	offset_77av = false;
	sub_run = true;
	crt_flag = true;
	vram_accessflag = false;
	display_mode = DISPLAY_MODE_8_200L;
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
	
	set_cyclesteal(config.dipswitch & 0x01); // CYCLE STEAL = bit0.
	nmi_count = 0;
	irq_count = 0;
	firq_count = 0;
	vram_wrote = true;
	window_low = 0;
	window_high = 200;
	window_xbegin = 0;
	window_xend = 80;
	window_opened = false;
	subcpu_resetreq = false;
	offset_point = 0;
	tmp_offset_point = 0;
	offset_changed = true;
	halt_count = 0;

	switch(config.cpu_type){
		case 0:
			subclock = SUBCLOCK_NORMAL;
			break;
		case 1:
			subclock = SUBCLOCK_SLOW;
			break;
	}
   
	if(is_cyclesteal || !(vram_accessflag)) {
		//
	} else {
		if((config.dipswitch & 0x01) == 0) subclock = subclock / 3;
	}
	p_vm->set_cpu_clock(subcpu, subclock);
	prev_clock = subclock;

	register_event(this, EVENT_FM7SUB_VSTART, 1.0 * 1000.0, false, &vstart_event_id);   
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	sub_busy = true;
	subcpu->reset();
}

void DISPLAY::update_config(void)
{
	set_cyclesteal(config.dipswitch & 0x01); // CYCLE STEAL = bit0.
}

inline int DISPLAY::GETVRAM_8_200L(int yoff, scrntype *p, uint32 rgbmask)
{
	register uint8 b, r, g;
	register uint32 dot;
	yoff = yoff & 0x3fff;
	b = gvram[yoff];
	r = gvram[yoff + 0x4000];
	g = gvram[yoff + 0x8000];
	
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

void DISPLAY::draw_screen(void)
{
	int y;
	int x;
	int i;
	int height = (display_mode == DISPLAY_MODE_8_400L) ? 400 : 200;
	scrntype *p, *pp, *q;
	register int yoff;
	Uint32 planesize = 0x4000;
	uint32 offset;
	register uint32 rgbmask;
	//if(!vram_wrote) return;
	
#if defined(_FM77AV_VARIANTS)
	if(offset_77av) {
	  offset = offset_point;
	} else {
	  offset = offset_point & 0x7fe0;
	}
#else
	offset = offset_point & 0x7fe0;
#endif
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_8_400L_TEXT)) {
		planesize = 0x8000;
	} else if((display_mode == DISPLAY_MODE_8_200L) || (display_mode == DISPLAY_MODE_8_200L_TEXT)) {
		offset = offset & 0x3fff;
	} else if((display_mode == DISPLAY_MODE_4096) || (display_mode == DISPLAY_MODE_256k)) {
		planesize = 0x2000;
		offset = offset & 0x1fff;
	} else {
		return;
	}
	  // Set blank
	if(!crt_flag) {
		vram_wrote = false;
		for(y = 0; y < 400; y++) {
			memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
	} else  if((display_mode == DISPLAY_MODE_8_200L) || (display_mode == DISPLAY_MODE_8_200L_TEXT)) {
		vram_wrote = false;
		yoff = offset & 0x3fff;
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
	}
	     
			
#if defined(_FM77AV_VARIANTS)
	else if(display_mode == DISPLAY_MODE_4096) {
		for(y = 0; y < height; y++) {
			p = &pvram[y * pitch];
			if(((y < window_low) && (y > window_high)) || (!window_opened)) {
				CreateVirtualVram4096_Line(p, y, offset, apalette_pixel, multimode_dispmask);
			} else {
				CreateVirtualVram4096_WindowedLine(p, y, window_xbegin, window_xend,
								   offset, apalette_pixel, multimode_dispmask);
			}
		}
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
	dpalette_data[addr] = val | 0b11111000;
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
	switch(config.cpu_type){
		case 0:
			subclock = SUBCLOCK_NORMAL;
			break;
		case 1:
			subclock = SUBCLOCK_SLOW;
			break;
	}
   
	if(is_cyclesteal || !(vram_accessflag)) {
		vram_wait = false;
	} else {
		vram_wait = true;
		if((config.dipswitch & 0x01) == 0) subclock = subclock / 3;
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
	//printf("DISPLAY: ACKNOWLEDGE\n");
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
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	alu->write_signal(SIG_FM7_ALU_CMDREG, data, 0x07);
	alu->write_signal(SIG_FM7_ALU_MBITS, (data & 0x60) >> 5, 0x03);
	alu->write_signal(SIG_FM7_ALU_EXEC, data, 0x80);
}

// D411
void DISPLAY::alu_write_logical_color(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	alu->write_signal(SIG_FM7_ALU_LOGICAL_COLOR, data, 0x03);
}

// D412
void DISPLAY::alu_write_mask_reg(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	alu->write_signal(SIG_FM7_ALU_WRITE_MASKREG, data, 0xff);
	}

// D413 - D41A
void DISPLAY::alu_write_mask_reg(int addr, uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	addr = addr & 7;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	alu->write_signal(SIG_FM7_ALU_WRITE_CMPREG_ADDR, data, 0x07);
}

// D41B
void DISPLAY::alu_write_disable_reg(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
  
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	data = data & 0x07 | 0x08;
	alu->write_signal(SIG_FM7_ALU_BANK_DISABLE, data, 0xff);
}

// D41C - D41F
void DISPLAY::alu_write_tilepaint_data(int addr, uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	switch(addr) {
		case 0: // $D41C
			alu->write_signal(SIG_FM7_ALU_TILEPAINT_B, data, 0xff);
			break;
		case 1: // $D41D
			alu->write_signal(SIG_FM7_ALU_TILEPAINT_R, data, 0xff);
			break;
		case 2: // $D41E
			alu->write_signal(SIG_FM7_ALU_TILEPAINT_G, data, 0xff);
			break;
		case 3: // xxxx
			//alu->write_signal(SIG_FM7_ALU_TILEPAINT_L, data, 0xff);
			alu->write_signal(SIG_FM7_ALU_TILEPAINT_L, 0xff, 0xff);
			break;
	}
}

// D420
void DISPLAY::alu_write_offsetreg_hi(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	uint32 addr;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	addr = alu->read_signal(SIG_FM7_ALU_OFFSET_REG); 
	addr = addr & 0x00ff;
	data <<= 8;
	addr = addr | data;
	alu->write_signal(SIG_FM7_ALU_OFFSET_REG, data, 0x1fff);
}
 
// D421
void DISPLAY::alu_write_offsetreg_lo(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	uint32 addr;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	addr = alu->read_signal(SIG_FM7_ALU_OFFSET_REG); 
	addr = addr & 0xff00;
	addr = addr | data;
	alu->write_signal(SIG_FM7_ALU_OFFSET_REG, data, 0x1fff);
}

// D422
void DISPLAY::alu_write_linepattern_hi(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	uint32 addr;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	addr = alu->read_signal(SIG_FM7_ALU_LINEPATTERN_REG); 
	addr = addr & 0x00ff;
	data = data << 8;
	addr = addr | data;
	alu->write_signal(SIG_FM7_ALU_LINEPATTERN_REG, data, 0xffff);
}

// D423
void DISPLAY::alu_write_linepattern_lo(uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	uint32 addr;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	addr = alu->read_signal(SIG_FM7_ALU_LINEPATTERN_REG); 
	addr = addr & 0xff00;
	addr = addr | data;
	alu->write_signal(SIG_FM7_ALU_LINEPATTERN_REG, data, 0xffff);
}

// D424-D42B
void DISPLAY::alu_write_line_position(int addr, uint8 val)
{
	bool busyflag;
	uint32 data = (uint32)val;
	busyflag = alu->read_signal(SIG_FM7_ALU_BUSYSTAT) ? true : false;
	if(busyflag) return; // DISCARD
	switch(addr) {
		case 0:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_START_X_HIGH, data, 0x03); 
			break;
		case 1:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_START_X_LOW, data, 0xff); 
			break;
		case 2:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_START_Y_HIGH, data, 0x01); 
			break;
		case 3:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_START_Y_LOW, data, 0xff); 
			break;
		case 4:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_END_X_HIGH, data, 0x03); 
			break;
		case 5:  
    			alu->write_signal(SIG_FM7_ALU_LINEPOS_END_X_LOW, data, 0xff); 
			break;
  		case 6:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_END_Y_HIGH, data, 0x01); 
			break;
		case 7:  
			alu->write_signal(SIG_FM7_ALU_LINEPOS_END_Y_LOW, data, 0xff); 
			break;
	}
}

// D42E :  AV40
void DISPLAY::select_sub_bank(uint8 val)
{
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
	kanji_level2 = ((val & 0x80) == 0) ? false : true;
  //	submem->write_signal(SIG_FM7_SUBMEM_SELECT_BANK val, 0x1f);
#endif
}

// D42F
void DISPLAY::select_vram_bank_av40(uint8 val)
{
  //	submem->write_signal(SIG_FM7_SUBMEM_AV40_SELECT_BANK val, 0x03);
}


//SUB:D430:R
uint8 DISPLAY::get_miscreg(void)
{
	uint8 ret;
#if defined(_FM77AV_VARIANTS)
	ret = 0x00;
	if(!hblank && !vblank) ret |= 0x80;
	if(vsync) ret |= 0x40;
	if(alu->read_signal(SIG_ALU_BUSY) == 0) ret |= 0x10;
	if(subreset) ret |= 0x01;
#else // 77 or older.
	ret = 0xff;
#endif
	return ret;
}
//SUB:D430:W
void DISPLAY::set_miscreg(uint8 val)
{
  
#if defined(_FM77AV40) || defined(_FM77AV40EX) || || defined(_FM77AV40SX)
	nmi_enable = ((val & 0x80) == 0) ? true : false;
#endif
	if((val & 0x40) == 0) {
		if(display_page != 0) {
		  redrawreq();
		}
		display_page = 0;
	} else {
		if(display_page == 0) {
			redrawreq();
		}
		display_page = 1;
	}
	active_page = ((val & 0x20) == 0) ? 0 : 1;
	if((val & 0x04) == 0) {
		offset_77av = false;
	} else {
		offset_77av = true;
	}
	submem->write_signal(SIG_FM7SUBMEM_CGROM, val, 0x03);
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
	subcpu_resetreq = true;
}


// FD30
void DISPLAY::set_apalette_index_hi(uint8 val)
{
	uint32 index = (uint32)val;
	index &= 0x0f;
	index <<= 8; // right?

	apalette_index = (apalette_index & 0xff) | index;
}

// FD31
void DISPLAY::set_apalette_index_lo(uint8 val)
{
	uint32 index = (uint32)val;
	index &= 0xff;

	apalette_index = (apalette_index & 0xf00) | index;
}

void DISPLAY::calc_apalette(uint32 index)
{
	scrntype r, g, b;
	vram_wrote = true;
	r = (apalette_r[index] == 0)? 0 : apalette_r[index] << 4; // + 0x0f?
	g = (apalette_g[index] == 0)? 0 : apalette_g[index] << 4; // + 0x0f?
	b = (apalette_b[index] == 0)? 0 : apalette_b[index] << 4; // + 0x0f?
	apalette_pixel[index] = RGB_COLOR(r, g, b);
}

// FD32
void DISPLAY::set_apalette_b(uint8 val)
{
	uint32 index;
	index = apalette_index & 0xfff;
	analog_palette_b[index] = val & 0x0f;
	calc_apalette(index);
}

// FD33
void DISPLAY::set_apalette_r(uint8 val)
{
	uint32 index;
	index = apalette_index & 0xfff;
	analog_palette_r[index] = val & 0x0f;
	calc_apalette(index);
}

// FD34
void DISPLAY::set_apalette_g(uint8 val)
{
	uint32 index;
	index = apalette_index & 0xfff;
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
			register_event(this, EVENT_FM7SUB_DISPLAY_NMI_OFF, 1.0 * 1000.0, false, NULL); // NEXT CYCLE_
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
		default:
			return 0;
			break;
	}
   
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	bool flag = ((data & mask) != 0);
	switch(id) {
		case SIG_FM7_SUB_HALT:
			if(cancel_request && flag) {
				sub_run = true;
				subcpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				//printf("SUB: HALT : CANCEL\n");
				return;
			}
			halt_flag = flag;
			//printf("SUB: HALT : DID STAT=%d\n", flag);   
			break;
       		case SIG_DISPLAY_HALT:
			if(flag) {
				//if(cancel_request) {
				//	return;
				//}
				//if(sub_run) halt_subsystem();
				halt_subsystem();
			} else {
				//if((sub_run) && !(cancel_request)) return;   
				restart_subsystem();
				if(subcpu_resetreq) {
					vram_wrote = true;
					subcpu->reset();
					subcpu_resetreq = false;
				}
			}
			break;
		case SIG_FM7_SUB_CANCEL:
			if(flag) {
				cancel_request = true;
				//printf("MAIN: CANCEL REQUEST TO SUB\n");
				do_irq(true);
			}
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_BANK: // Main: FD13
			set_monitor_bank(data & 0xff);
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
	//	addr = addr & 0x00ffffff;
	
#if defined(_FM77AV_VARIANTS)
	if(offset_77av) {
	  offset = offset_point;
	} else {
	  offset = offset_point & 0x7fe0;
	}
#else
	offset = offset_point & 0x7fe0;
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
		if(vram_bank) page_offset = 0xc000;
		if(display_mode == DISPLAY_MODE_4096) {
			uint32 color = 0; // b
			mask = 0x1fff;
			pagemod = addr & 0xe000;
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
		} else {
			mask = 0x3fff;
			pagemod = (addr & 0xc000) >> 14;
			if((multimode_accessmask & (1 << pagemod)) != 0) {
				return 0xff;
			} else {
				return gvram[(((addr + offset) & mask) | (pagemod << 14)) + page_offset];
			}
		}
#endif //_FM77L4
		pagemod = addr >> 14;
		if((multimode_accessmask & (1 << pagemod)) != 0) {
			return 0xff;
		} else {
			pagemod = addr & 0xc000;
			return gvram[((addr + offset) & mask) | pagemod];
		}
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
		uint32 retval = 0xff;
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
	        
#if defined(_FM77L4) || defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			case 0x06:
 #if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
				if(kanji_level2) {
					retval = kanjiclass2->read_data8(kanji2_addr);
				} else {
					retval = kanjiclass1->read_data8(kanji1_addr);
				}
 #else // _FM77L4
				retval = kanjiclass1->read_data8(kanji1_addr);
#endif				
				break;
			case 0x07:
 #if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)	
				if(kanji_level2) {
					retval = kanjiclass2->read_data8(kanji2_addr + 1);
				} else {
					retval = kanjiclass1->read_data8(kanji1_addr + 1);
				}
 #else // _FM77L4
				retval = kanjiclass1->read_data8(kanji1_addr + 1);
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
			default:
				break;
		}
		return retval;
	} else if(addr < 0x10000) {
#if !defined(_FM77AV_VARIANTS)
		return subsys_c[addr - 0xd800];
#endif
	} else if((addr >= FM7_SUBMEM_OFFSET_DPALETTE) && (addr < (FM7_SUBMEM_OFFSET_DPALETTE + 8))) {
		return dpalette_data[addr - FM7_SUBMEM_OFFSET_DPALETTE];
	}
#if 0
	else if((addr >= FM7_SUBMEM_VRAM_BANK0) && (addr < (FM7_SUBMEM_VRAM_BANK4 + 0x10000))) {
		uint32 rbank = ((addr - FM7_SUBMEM_VRAM_BANK0) / FM7_SUBMEM_VRAM_BANK0) & 0x03;
		uint32 pagemod;
#if !defined(_FM77AV_VARIANTS)
		if(display_mode == DISPLAY_MODE_8_400L) {
			raddr = addr & 0x7fff;
			return gvram[(raddr + offset) & 0x7fff];
		} else {
			raddr = addr & 0x3fff;
			if(rbank > 2) rbank = 2; // OK?
			pagemod = rbank << 14;
			return gvram[((raddr + offset) & 0x3fff) | pagemod];
		}
#elif defined(_FM77AV_VARIANTS)
		if(display_mode == DISPLAY_MODE_8_200L) {
			raddr = addr & 0x3fff;
			if(rbank > 2) rbank = 2; // OK?
			pagemod = rbank << 14;
			if(vram_bank1) {
				return gvram_1[((raddr + offset) & 0x3fff) | pagemod];
			} else {
				return gvram[((raddr + offset) & 0x3fff) | pagemod];
			}
		} else if(display_mode == DISPLAY_MODE_4096) {
			raddr = addr & 0xbfff;
			if(rbank > 2) rbank = 2; // OK?
			switch(rbank) {
			case 0: // B
				pagemod = raddr & 0x6000;
				raddr = ((raddr + offset) & 0x1fff) | pagemod;
				return gvram[raddr];
				break;
			case 1: // R
				if(raddr <= 0x3fff) {
					pagemod = (raddr & 0x2000) << 2;
					raddr = ((raddr + offset) & 0x1fff) | pagemod;
					return gvram[raddr];
				} else {
					pagemod = ((raddr - 0x3fff) & 0x2000) >> 2;
					raddr = ((raddr + offset) & 0x1fff) | pagemod;
					return gvram_1[raddr];
				}
				break;
			case 1: // G
				pagemod = raddr & 0x6000;
				raddr = ((raddr + offset) & 0x1fff) | pagemod;
				return gvram_1[raddr];
				break;
			default:
				return 0xff;
				break;
			}
		} else {
			return 0xff;
		}
		  
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
		else if(display_mode == DISPLAY_MODE_8_400L) {
			raddr = addr & 0x7fff;
			raddr = (raddr + offset) & 0x7fff;
			switch(rbank) {
			case 0:
				return gvram[raddr];
				break;
			case 1:
				return gvram_1[raddr];
				break;
			case 2:
				return gvram_2[raddr];
				break;
		        default:
				return 0xff;
				break;
			}
		} else if(display_mode == DISPLAY_MODE_256k) {
			raddr = addr & 0xbfff;
			pagemod = raddr & 0xe000;
			raddr = ((raddr + offset) & 0x1fff) | pagemod;
			switch(rbank) {
			case 0:
				return gvram[raddr];
				break;
			case 1:
				return gvram_1[raddr];
				break;
			case 2:
				return gvram_2[raddr];
				break;
		        default:
				return 0xff;
				break;
			}
#endif		  
#endif
#endif // if 0
	return 0xff;
}	



void DISPLAY::write_data8(uint32 addr, uint32 data)
{
	uint32 mask = 0x3fff;
	uint8 val8 = data & 0xff;
	uint32 rval;
	uint32 offset;
	//	addr = addr & 0x00ffffff;
	
#if defined(_FM77AV_VARIANTS)
	if(offset_77av) {
	  offset = offset_point;
	} else {
	  offset = offset_point & 0x7fe0;
	}
#else
	offset = offset_point & 0x7fe0;
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
		page_offset = 0;
		if(vram_bank) page_offset = 0xc000;
		if(display_mode == DISPLAY_MODE_4096) {
			uint32 color = 0; // b
			mask = 0x1fff;
			pagemod = addr & 0xe000;
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
		} else {
			mask = 0x3fff;
			pagemod = (addr & 0xc000) >> 14;
			if((multimode_accessmask & (1 << pagemod)) == 0) {
				gvram[(((addr + offset) & mask) | (pagemod << 14)) + page_offset] = val8;
			}
			return;
		}
#endif //_FM77L4
		pagemod = addr >> 14;
		if((multimode_accessmask & (1 << pagemod)) == 0) {
			pagemod = addr & 0xc000;
			gvram[((addr + offset) & mask) | pagemod] = val8;
			if(crt_flag && ((multimode_dispmask & (1 << pagemod)) == 0)) vram_wrote = true;
		}
		return;
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
		uint32 retval = 0xff;
#if !defined(_FM77AV_VARIANTS)
		addr = (addr - 0xd400) & 0x000f;
#else
		addr = (addr - 0xd400) & 0x003f;
#endif
		//if(addr >= 0x02) printf("SUB: IOWRITE PC=%04x, ADDR=%02x DATA=%02x\n", subcpu->get_pc(), addr, val8);
		switch(addr) {
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4) || defined(_FM77AV_VARIANTS)
			case 0x05:
				set_cyclesteal(val8);
				break;
#endif
			case 0x06:
				data <<= 9;
				data = data & 0x1fe00;
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
				if(kanji_level2) {
					kanji2_addr = data | (kanji2_addr & 0x001fe);
				} else {
					kanji1_addr = data | (kanji1_addr & 0x001fe);
				}
#elif defined(_FM77L4)
				kanji1_addr = data | (kanji1_addr & 0x001fe);
#endif				
				break;
			case 0x07:
				data <<= 1;
				data = data & 0x001fe;
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX)
				if(kanji_level2) {
					kanji2_addr = data | (kanji2_addr & 0x1fe00);
				} else {
					kanji1_addr = data | (kanji1_addr & 0x1fe00);
				}
#elif defined(_FM77L4)
				kanji1_addr = data | (kanji1_addr & 0x1fe00);
#endif
				break;
	        
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
					offset_point = tmp_offset_point;
   					vram_wrote = true;
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
					offset_point = tmp_offset_point;
   					vram_wrote = true;
				}
 				break;
			default:
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
#endif	
	return;
}	




void DISPLAY::initialize()
{
	int i;

	memset(gvram, 0xff, sizeof(gvram));
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
   	if(read_bios(_T("SUBSYS_B.ROM"), subsys_a, 0x2000) >= 0x2000) diag_load_subrom_b = true;
	emu->out_debug_log("SUBSYSTEM ROM Type B READING : %s\n", diag_load_subrom_b ? "OK" : "NG");

	diag_load_subrom_cg = false;
   	if(read_bios(_T("SUBSYS_B.ROM"), subsys_cg, 0x2000) >= 0x2000) diag_load_subrom_cg = true;
	emu->out_debug_log("SUBSYSTEM CG ROM READING : %s\n", diag_load_subrom_cg ? "OK" : "NG");
#endif

	vram_wrote = true;

}
