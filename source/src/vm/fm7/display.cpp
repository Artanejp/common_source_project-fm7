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
	if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	if(hblank_event_id >= 0) cancel_event(this, hblank_event_id);
	if(hdisp_event_id >= 0) cancel_event(this, hdisp_event_id);
	if(vsync_event_id >= 0) cancel_event(this, vsync_event_id);
	if(vstart_event_id >= 0) cancel_event(this, vstart_event_id);
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	register_event(this, EVENT_FM7SUB_VSTART, 1.0 * 1000.0, false, &vstart_event_id);   
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	for(i = 0; i < 8; i++) set_dpalette(i, i);

//	subcpu->reset();
}

void DISPLAY::draw_screen(void)
{
	int y;
	int x;
	int i;
	int height = (display_mode == DISPLAY_MODE_8_400L) ? 400 : 200;
	scrntype *p, *pp, *q;
	int yoff;
	Uint32 planesize = 0x4000;
	uint32 offset;
	uint8 r, g, b;
	uint8 rgbmask;
	uint16 dot;
	
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
		for(y = 0; y < 400; y++) {
			memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
		}
	} else  if((display_mode == DISPLAY_MODE_8_200L) || (display_mode == DISPLAY_MODE_8_200L_TEXT)) {
	  
		yoff = offset & 0x3fff;
		rgbmask = ~multimode_dispmask & 0x07;
		for(y = 0; y < 400; y += 2) {
			p = emu->screen_buffer(y);
			pp = p;
			for(x = 0; x < 80; x++) {
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
				*p++ = dpalette_pixel[dot & rgbmask];
				yoff++;
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

void DISPLAY::set_multimode(uint8 val)
{
	multimode_accessmask = val & 0x07;
	multimode_dispmask = (val & 0x70) >> 4;
}

uint8 DISPLAY::get_multimode(void)
{
  uint8 val;
	val = multimode_accessmask & 0x07;
	val |= ((multimode_dispmask << 4) & 0x70);
	val |= 0x88;
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

void DISPLAY::enter_display(void)
{
	if(vram_accessflag) {
		vram_wait = true;
		subcpu->write_signal(SIG_CPU_BUSREQ, 0x01, 0x01);
	}
}

void DISPLAY::leave_display(void)
{
	vram_wait = false;
	if(sub_run) {
		subcpu->write_signal(SIG_CPU_BUSREQ, 0x00, 0x01);
	}
}

void DISPLAY::halt_subsystem(void)
{
	sub_run = false;
	subcpu->write_signal(SIG_CPU_BUSREQ, 0x01, 0x01);
	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0x01, 0x01); // BUSY
}

void DISPLAY::restart_subsystem(void)
{
	sub_run = true;
	subcpu->write_signal(SIG_CPU_BUSREQ, 0x00, 0x01);
}

//SUB:D408:R
void DISPLAY::set_crtflag(void)
{
	crt_flag = true;
}

//SUB:D408:W
void DISPLAY::reset_crtflag(void)
{
	crt_flag = false;
}

//SUB:D402:R
uint8 DISPLAY::acknowledge_irq(void)
{
	subcpu->write_signal(SIG_CPU_IRQ, 0x00, 0x01);
	mainio->write_signal(FM7_MAINIO_SUB_CANCEL, 0x00, 0x01);
	//	mainio->write_signal(SIG_FM7_SUB_CANCEL, 0x01, 0x01);
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
	return 0xff;
}

// SUB:D405:W
void DISPLAY::set_cyclesteal(uint8 val)
{
#if !defined(_FM7) && !defined(_FMNEW7) && !defined(_FM8)
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
	if(is_cyclesteal) {
		vram_accessflag = false;
		leave_display();
	}
	return 0xff;
}

//SUB:D409:W
void DISPLAY::reset_vramaccess(void)
{
	vram_accessflag = false;
	leave_display();
}

//SUB:D40A:R
uint8 DISPLAY::reset_subbusy(void)
{
  	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0x00, 0x01);
	return 0xff;
}

//SUB:D40A:W
void DISPLAY::set_subbusy(void)
{
	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0x01, 0x01);
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
	mainio->write_signal(FM7_MAINIO_SUB_BUSY, 0x01, 0x01);
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
			subcpu->write_signal(SIG_CPU_NMI, 0x01, 0x01);
			break;
		case EVENT_FM7SUB_HDISP:
			hblank = false;
			usec = 39.5;
			if(display_mode == DISPLAY_MODE_8_400L) usec = 30.0;
			register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hblank_event_id); // NEXT CYCLE_
			if((!is_cyclesteal) && vram_accessflag)  enter_display();
			break;
		case EVENT_FM7SUB_HBLANK:
			hblank = true;
			f = false;
			displine++;
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
			return (!sub_run) ? 0x00000000 : 0xffffffff;
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
		case SIG_DISPLAY_HALT:
		case SIG_FM7_SUB_HALT:
			if(flag) {
				if(sub_run) halt_subsystem();
			} else {
				if(!sub_run) {
					restart_subsystem();
					if(subcpu_resetreq) {
						subcpu->reset();
						subcpu_resetreq = false;
					}
				}
			}
			break;
		case SIG_FM7_SUB_CANCEL:
		  	if(flag) subcpu->write_signal(SIG_CPU_IRQ, 1, 1);
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_BANK: // Main: FD13
			set_monitor_bank(data & 0xff);
			break;
#endif // _FM77AV_VARIANTS
		case SIG_FM7_SUB_MULTIPAGE:
	  		set_multimode(data & 0xff);
			break;
		case SIG_FM7_SUB_KEY_FIRQ:
			subcpu->write_signal(SIG_CPU_FIRQ, flag ? 1 : 0, 1);
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
		// Still not implement offset.
		if(!is_cyclesteal && !vram_accessflag) return 0xff;
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
#elif defined(_FM77AV_VARIANTS)
      
# if defined(_FM77AV40) || if defined(_FM77AV40EX) || if defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr < 0x8000) {
			  // Fixme:Read via ALU
			  //
			  pagemod = addr >> 14;
			  mask = 0x7fff;
			  addr = (addr + offset) & mask;
			  if((multimode_accessmask & (1 << vram_bank)) == 0) {
			  	switch(vram_bank) {
				case 0:
					return gvram[addr];
					break;
				case 1:
					return gvram_1[addr];
					break;
				case 2:
					return gvram_2[addr];
					break;
				default:
					return 0xff;
					break;
				}
			  }
			}
			return 0xff;
		} else if(display_mode == DISPLAY_MODE_256k) {
			if(vram_ac);
		}
#endif
		if(display_mode == DISPLAY_MODE_4096) {
			mask = 0x1fff;
		} else {
			mask = 0x3fff;
		}
		if(vram_bank) {
			pagemod = addr >> 14;
			if((multimode_accessmask & (1 << pagemod)) != 0) {
				return 0xff;
			} else {
			  	return gvram_1[((addr + offset) & mask) | (pagemod << 14)];
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
		raddr = addr & 0x0fff;
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
		switch(raddr) {
			case 0x00: // Read keyboard
				retval = keyboard->read_data8(0x0) & 0x80;
				break;
			case 0x01: // Read keyboard
				this->write_signal(SIG_FM7_SUB_KEY_FIRQ, 0, 1);
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
				set_crtflag();
				break;
			case 0x09:
				set_vramaccess();
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
		if(!is_cyclesteal && !vram_accessflag) return;
#if defined(_FM77L4)
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr < 0x8000) {
				if(workram) {
				  addr = addr & 0x3fff;
				  if((multimode_accessmask & 0x04) == 0) {
				  	gvram[0x8000 + (addr + offset) & mask] = val8;
				  }
				  return;
				}
				pagemod = addr & 0x4000;
				gvram[((addr + offset) & mask) | pagemod] = val8;
			} else if(addr < 0x9800) {
				textvram[addr & 0x0fff] = val8;
			} else { // $9800-$bfff
				subrom_l4[addr - 0x9800] = val8;
			}
			return;
		}
#elif defined(_FM77AV_VARIANTS)
      
# if defined(_FM77AV40) || if defined(_FM77AV40EX) || if defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr < 0x8000) {
			  // Fixme:Read via ALU
			  //
			  pagemod = addr >> 14;
			  mask = 0x7fff;
			  addr = (addr + offset) & mask;
			  if((multimode_accessmask & (1 << vram_bank)) == 0) {
			  	switch(vram_bank) {
					case 0:
						gvram[addr] = val8;
						break;
					case 1:
						gvram_1[addr] = val8;
						break;
					case 2:
						gvram_2[addr] = val8;
						break;
					default:
						break;
				}
			  }
			  return;
		} else if(display_mode == DISPLAY_MODE_256k) {
			return;
		} else 
#endif
		if(display_mode == DISPLAY_MODE_4096) {
			mask = 0x1fff;
		} else {
			mask = 0x3fff;
		}
		if(vram_bank) {
			pagemod = addr >> 14;
			if((multimode_accessmask & (1 << pagemod)) == 0) {
				gvram_1[((addr + offset) & mask) | (pagemod << 14)] = val8;
			}
			return;
		}
#endif //_FM77L4
		pagemod = addr >> 14;
		if((multimode_accessmask & (1 << pagemod)) == 0) {
			pagemod <<= 14;
			gvram[((addr + offset) & mask) | pagemod] = val8;
			//gvram[addr] = val8;
		}
		return;
	} else if(addr < 0xd000) { 
		addr = addr & 0x0fff;
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
				reset_crtflag();
				break;
			case 0x09:
				reset_vramaccess();
				break;
			case 0x0a:
				set_subbusy();
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
				offset_point = (offset_point & 0x00ff) | rval;
				break;
			case 0x0f:
#if defined(_FM77AV_VARIANTS)
				rval = data & 0x00ff;
				if(!offset_77av) rval = rval & 0xe0;		  
#else
				rval = data & 0x00e0;
#endif
				offset_point = (offset_point & 0x7f00) | rval;
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
	memset(shared_ram, 0x00, sizeof(shared_ram));
	memset(subsys_c, 0x00, sizeof(subsys_c));

	read_bios("SUBSYS_C.ROM", subsys_c, 0x2800);

	for(i = 0; i < 8; i++) set_dpalette(i, i);
	nmi_event_id = -1;
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
	offset_77av = false;
	sub_run = true;
	crt_flag = true;
	vram_accessflag = true;
	display_mode = DISPLAY_MODE_8_200L;
	vram_wait = false;
	
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	
	vblank = false;
	vsync = false;
	hblank = true;
	displine = 0;
	
	window_low = 0;
	window_high = 200;
	window_xbegin = 0;
	window_xend = 80;
	window_opened = false;
	subcpu_resetreq = false;
	
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4) || defined(_FM77AV_VARIANTS)
	is_cyclesteal = true;
#else
	is_cyclesteal = false;
#endif
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	register_event(this, EVENT_FM7SUB_VSTART, 1.0 * 1000.0, false, &vstart_event_id);   
//	subcpu->reset();
#if defined(_FM77AV_VARIANTS)
#endif
}
