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

  extern void CreateVirtualVram8_Line(uint8 *src, uint32 *p, int ybegin, uint32 *pal);
  extern void CreateVirtualVram8_WindowedLine(uint8 *vram_1, uint8 *vram_w, Uint32 *p, int ybegin, int xbegin, int xend, uint32 *pal);
}

DISPLAY::DISPLAY()
{
	initvramtbl_4096_vec();
}

void DISPLAY::getvram(uint32 *pvram, uint32 pitch)
{
	int y;
	int height = (display_mode == DISPLAY_MODE_8_400L) ? 400 : 200;
	Uint32 *p;
	Uint32 planesize = 0x4000;
	uint32 offset;
	
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
	
	if(!crtflag) {
	  // Set blank
		PutBlank(p, height);
	} else if((display_mode == DISPLAY_MODE_8_200L) || (display_mode == DISPLAY_MODE_8_200L_TEXT) ||
		  (display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_8_400L_TEXT)) {
		for(y = 0; y < height; y++) {
			p = &pvram[y * pitch];
			if(((y < window_low) && (y > window_high)) || (!window_opened)) {
				CreateVirtualVram8_Line(p, y, digital_palette, planesize, offset,multimode_dispmask);
			} else {
				CreateVirtualVram8_WindowedLine(p, y, window_xbegin, window_xend,
								digital_palette, planesize, offset, multimode_dispmask);
			}
		}
	}
#if defined(_FM77AV_VARIANTS)
	else if(display_mode == DISPLAY_MODE_4096) {
		for(y = 0; y < height; y++) {
			p = &pvram[y * pitch];
			if(((y < window_low) && (y > window_high)) || (!window_opened)) {
				CreateVirtualVram4096_Line(p, y, offset, analog_palette, multimode_dispmask);
			} else {
				CreateVirtualVram4096_WindowedLine(p, y, window_xbegin, window_xend,
								   offset, analog_palette, multimode_dispmask);
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
	uint8 val = multimode_accessmask & 0x07;
	val |= ((multimode_dispmask << 4) & 0x70);
	return val;
}

uint8 DISPLAY::get_cpuaccessmask(void)
{
	return muitimode_accessmask & 0x07;
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
	
	data = dpalet_data[addr];
	return data;
}

void DISPLAY::calc_apalette(uint32 index)
{
#if defined(_FM77AV_VARIANTS)
	scrntype r, g, b;
	r = (apalette_r[index] == 0)? 0 : apalette_r[index] << 4; // + 0x0f?
	g = (apalette_g[index] == 0)? 0 : apalette_g[index] << 4; // + 0x0f?
	b = (apalette_b[index] == 0)? 0 : apalette_b[index] << 4; // + 0x0f?
	apalette_pixel[index] = RGB_COLOR(r, g, b);
#endif
}

// FD34
void DISPLAY::set_apalette_g(uint8 val)
{
#if defined(_FM77AV_VARIANTS)
	uint32 index;
	index = apalette_index & 0xfff;
	apalette_g[index] = val & 0x0f;
	calc_apalette(index);
#endif
}

// FD33
void DISPLAY::set_apalette_r(uint8 val)
{
#if defined(_FM77AV_VARIANTS)
	uint32 index;
	index = apalette_index & 0xfff;
	apalette_r[index] = val & 0x0f;
	calc_apalette(index);
#endif
}

// FD32
void DISPLAY::set_apalette_b(uint8 val)
{
#if defined(_FM77AV_VARIANTS)
	uint32 index;
	index = apalette_index & 0xfff;
	apalette_b[index] = val & 0x0f;
	calc_apalette(index);
#endif
}

// FD30
void DISPLAY::set_apalette_index_hi(uint8 val)
{
#if defined(_FM77AV_VARIANTS)
	uint32 index = (uint32)val;
	index &= 0x0f;
	index <<= 8; // right?

	apalette_index = (apalette_index & 0xff) | index;
#endif
}

// FD31
void DISPLAY::set_apalette_index_lo(uint8 val)
{
#if defined(_FM77AV_VARIANTS)
	uint32 index = (uint32)val;
	index &= 0xff;

	apalette_index = (apalette_index & 0xf00) | index;
#endif
}

//SUB:D409:R
uint8 DISPLAY::set_vramaccess(void)
{
	vram_accessflag = true;
	if(!is_cyclesteal) {
		vram_cpuaccess = true;
	} else {
		vram_cpuaccess = false;
		leave_display();
	}
	return 0xff;
}

//SUB:D409:W
void DISPLAY::reset_vramaccess(void)
{
	vram_accessflag = false;
	vram_cpuaccess = false;
	leave_display();
}

//SUB:D40A:R
uint8 DISPLAY::reset_subbusy(void)
{
  	maincpu->write_signal(FM7_SUB_BUSY, 0x00, 0x01);
	return 0xff;
}

//SUB:D40A:W
void DISPLAY::set_subbusy(void)
{
	maincpu->write_signal(FM7_SUB_BUSY, 0x01, 0x01);
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
#if defined(_FM77AV_VARIANTS)
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
#endif
}
void DISPLAY::enter_display(void)
{
	if(vram_cpuaccess) {
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
	maincpu->write_signal(FM7_SUB_BUSY, 0x01, 0x01); // BUSY
}

void DISPLAY::restart_subsystem(void)
{
	sub_run = true;
	if(!vram_wait) subcpu->write_signal(SIG_CPU_BUSREQ, 0x00, 0x01);
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
uint8 DISPLAY::cancel_irq(void)
{
	mainio->write_signal(SIG_FM7_SUB_CANCEL, 0x01, 0x01);
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
	val &= 0x01;
	if(val != 0) {
		is_cyclesteal = true;
	} else {
		is_cyclesteal = false;
	}
}



// Main: FD13
void DISPLAY::set_monitor_bank(uint8 var)
{
#if defined(_FM77AV_VARIANTS)
	if(is_77av40) {
		if((var & 0x04) != 0) {
			submem->write_signal(SIG_FM7SUBMEM_MONITOR_RAM, 0x01, 0x01);
		} else {
			submem->write_signal(SIG_FM7SUBMEM_MONITOR_RAM, 0x00, 0x01);
			submem->write_signal(SIG_FM7SUBMEM_MONITOR_ROM, (uint32)var, 0x03);
		}
		subrom_bank = var & 0x07;
	} else {
		mem->write_signal(SIG_FM7SUBMEM_MONITOR_RAM, 0x00, 0x01);
		submem->write_signal(SIG_FM7SUBMEM_MONITOR_ROM, (uint32)var, 0x03);
		subrom_bank = var & 0x03;
	}
	subcpu_resetreq = true;
	maincpu->write_signal(FM7_SUB_BUSY, 0x01, 0x01);
#endif
}

// Timing values from XM7 . Thanks Ryu.
void DISPLAY::event_callback(int event_id, int err)
{
	double usec;
	uint64 haltclocks;
	bool f;
	switch(event_id) {
		case EVENT_FM7SUB_DISPLAY_HALT:
			if(is_cyclesteal || !(vram_cpuaccess)) break;
			enter_display();
			haltclocks =  2; // is this right?
			register_event(this, EVENT_FM7SUB_DISPLAY_RUN, haltclocks, false, &halt_event_id); // NEXT CYCLE_
			break;
		case EVENT_FM7SUB_DISPLAY_RUN:
			if(is_cyclesteal || !(vram_cpuaccess)) break;
	  		leave_display();
			haltclocks =  1; // is this right?
			register_event(this, EVENT_FM7SUB_DISPLAY_RUN, haltclocks, false, &halt_event_id); // NEXT CYCLE_
			break;
  		case EVENT_FM7SUB_DISPLAY_NMI: // per 20.00ms
			subcpu->write_signal(SIG_CPU_NMI, 0x01, 0x01);
			break;
		case EVENT_FM7SUB_HDISP:
			hblank = false;
			usec = 39.5;
			if(is_400line) usec = 30.0;
			register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hblank_event_id); // NEXT CYCLE_
			break;
		case EVENT_FM7SUB_HBLANK:
			hblank = true;
			f = false;
			displine++;
			if(is_400line) {
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
				if(is_400line) {
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
			if(is_400line) {
				usec = 11.0;
			} else {
				usec = 24.0;
			}
			register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id); // NEXT CYCLE_
			break;
		case EVENT_FM7SUB_VSYNC:
			vblank = true;
			vsync = false;
			if(is_400line) {
				usec = 0.98 * 1000.0;
			} else {
				usec = 1.91 * 1000.0;
			}
			register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
			break;
	}
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	bool flag = ((data & mask) != 0);
	switch(id) {
		case SIG_FM7_SUB_HALT:
			if(flag) {
				if(sub_run) halt_subsystem();
			} else {
				if(!sub_run) {
					restart_subsytem();
					if(subcpu_resetreq) {
						subcpu->reset();
						subcpu_resetreq = false;
					}
				}
			}
			break;
		case SIG_FM7_SUB_CANCEL:
		  	if((flag) && (sub_run)) subcpu->write_signal(SIG_CPU_IRQ, 1, 1);
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_BANK: // Main: FD13
			set_monitor_bank(data & 0xff);
			break;
#endif // _FM77AV_VARIANTS
		case SIG_FM7_SUB_DPALETTE_ADDR: // Main: FD38-FD3F
			dpalette_addr = data & 0x07;
			break;
		case SIG_FM7_SUB_DPALETTE_DATA: // Main: FD38-FD3F
			set_dpalette(dpalette_addr, (uint8)data & 0x07);
			break;
		case SIG_FM7_SUB_MULTIPAGE:
	  		set_multimode(data & 0xff);
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_APALETTE_ADDR_HI:
			set_apalette_index_hi(data & 0xff);
			break;
		case SIG_FM7_SUB_APALETTE_ADDR_LO:
			set_apalette_index_lo(data & 0xff);
			break;
		case SIG_FM7_SUB_APALETTE_DATA_R:
			set_apalette_r(data & 0xff);
			break;
		case SIG_FM7_SUB_APALETTE_DATA_G:
			set_apalette_g(data & 0xff);
			break;
		case SIG_FM7_SUB_APALETTE_DATA_B:
			set_apalette_b(data & 0xff);
			break;
		case SIG_FM7_SET_SUBMEM_ADDR:
			submem_addr = data & 0xffff;
			break;
		case SIG_FM7_SET_SUBMEM_DATA:
			submem->write_signal(SIG_FM7_MEM_WRITE, data, 0xff);
			break;
#endif // _FM77AV_VARIANTS
		case SIG_FM7_SET_SHAREDMEM_ADDR:
			sharedmem_addr = (data >> 1) & 0x7f;
			break;
		case SIG_FM7_SET_SHAREDMEM_DATA:
			submem->write_signal(SIG_FM7_SHAREDMEM_WRITE, data, 0xff);
			break;
		case SIG_FM7_SUB_SET_OFFSET_HI:
			if(is_400line) { // 400line
				data = data & 0x7f;
			} else {
				data = data & 0x3f;
			}
			offset_point &= 0x00ff;
			offset_point = offset_point | (data << 8);
			break;
		case SIG_FM7_SUB_SET_OFFSET_LO:
			if(!offset_77av) { // 400line
				data = data & 0xe0;
			}
			offset_point &= 0xff00;
			offset_point = offset_point | data;
			break;
		default:
			break;
	}
}
		  
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
	submem->write_signal(SIG_FM7_SUBMEM_SELECT_KANJILEVEL2, val, 0x80);
	submem->write_signal(SIG_FM7_SUBMEM_SELECT_BANK val, 0x1f);
}

// D42F
void DISPLAY::select_vram_bank_av40(uint8 val)
{
	submem->write_signal(SIG_FM7_SUBMEM_AV40_SELECT_BANK val, 0x03);
}


  
