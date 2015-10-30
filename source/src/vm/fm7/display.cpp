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

DISPLAY::DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
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
	double usec;
	keyboard->write_signal(SIG_FM7KEY_SET_INSLED, 0x00, 0x01);
	mainio->write_signal(SIG_FM7_SUB_HALT, 0x00, 0xff);
	sub_busy = true;
	
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	//firq_mask = false;
	firq_mask = (mainio->read_signal(FM7_MAINIO_KEYBOARDIRQ_MASK) != 0) ? false : true;
	//cancel_request = false;
	switch(config.cpu_type){
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
	is_cyclesteal = ((config.dipswitch & FM7_DIPSW_CYCLESTEAL) != 0) ? true : false;
	enter_display();
   
	offset_point = 0;
	for(i = 0; i < 2; i++) {
		offset_changed[i] = true;
		tmp_offset_point[i].d = 0;
	}
	
#if defined(_FM77AV_VARIANTS)
	offset_77av = false;
	offset_point_bank1 = 0;
	offset_point_bak   = 0;
	offset_point_bank1_bak = 0;
	display_page = 0;
	active_page = 0;
	
	subcpu_resetreq = false;
	//offset_77av = false;
	subrom_bank_using = subrom_bank;
   
	nmi_enable = true;
	//nmi_enable = false;
	use_alu = false;
	vram_wrote_shadow = false;
	for(i = 0; i < 400; i++) vram_wrote_table[i] = true;
	for(i = 0; i < 400; i++) vram_draw_table[i] = true;
	
	vsync = true;
	vblank = true;
	hblank = false;
	vblank_count = 0;
	displine = 0;
	
	if(hblank_event_id >= 0) cancel_event(this, hblank_event_id);
	if(hdisp_event_id >= 0) cancel_event(this, hdisp_event_id);
	if(vsync_event_id >= 0) cancel_event(this, vsync_event_id);
	if(vstart_event_id >= 0) cancel_event(this, vstart_event_id);
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
	if(display_mode == DISPLAY_MODE_8_400L) {
		usec = 0.33 * 1000.0; 
	} else {
		usec = 0.51 * 1000.0;
	}
	//usec = 16.0;
	register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
	mainio->write_signal(SIG_DISPLAY_DISPLAY, 0x00, 0xff);
	mainio->write_signal(SIG_DISPLAY_VSYNC, 0xff, 0xff);
	
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	vram_bank = 0;
	vram_display_block = 0;
	vram_active_block = 0;
	
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	window_low = 0;
	window_high = 0;
	window_xbegin = 0;
	window_xend = 0;
	window_opened = false;
#  endif	
# endif
	
	alu->reset();
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	alu->write_signal(SIG_ALU_X_WIDTH, ((mode320 || mode256k) && !(mode400line)) ? 40 : 80, 0xffff);
	alu->write_signal(SIG_ALU_Y_HEIGHT, (mode400line) ? 400: 200, 0xffff);
	alu->write_signal(SIG_ALU_400LINE, (mode400line) ? 0xffffffff : 0, 0xffffffff);
# else
	alu->write_signal(SIG_ALU_X_WIDTH, (mode320) ? 40 : 80, 0xffff);
	alu->write_signal(SIG_ALU_Y_HEIGHT, 200, 0xffff);
	alu->write_signal(SIG_ALU_400LINE, 0, 0xffffffff);
# endif
	alu->write_signal(SIG_ALU_MULTIPAGE, multimode_accessmask, 0x07);
	alu->write_signal(SIG_ALU_PLANES, 3, 3);
	
	if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	nmi_event_id = -1;
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	
#endif	
	for(i = 0; i < 8; i++) set_dpalette(i, i);
	do_firq(!firq_mask && key_firq_req);

#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	kanjisub = false;
	kanjiaddr.d = 0x00000000;
# if defined(_FM77L4)
	mode400line = false;
	stat_400linecard = false;
# endif	
#endif
	vram_wrote = true;
	clr_count = 0;
	frame_skip_count = 3;
}


void DISPLAY::reset()
{
	int i;
	
	memset(io_w_latch, 0xff, sizeof(io_w_latch));
	halt_flag = false;
	vram_accessflag = true;
	display_mode = DISPLAY_MODE_8_200L;
	crt_flag = true;
	
	cancel_request = false;
#if defined(_FM77AV_VARIANTS)
	mode320 = false;
	apalette_index.d = 0;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = i & 0xf00;
		analog_palette_b[i] = i & 0x00f;
		calc_apalette(i);
	}
#endif
#if defined(_FM77AV_VARIANTS)
   	subrom_bank = 0;
	cgrom_bank = 0;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	monitor_ram = false;
	ram_protect = true;
	
	mode400line = false;
	mode256k = false;
# endif
#	
#elif defined(_FM77L4)
	mode400line = false;
#endif

	mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 0x00 , 0xff);
	keyboard->reset();
	keyboard->write_signal(SIG_FM7KEY_SET_INSLED, 0x00, 0x01);
	firq_mask = (mainio->read_signal(FM7_MAINIO_KEYBOARDIRQ_MASK) != 0) ? false : true;
   
	reset_cpuonly();
#if defined(_FM77AV_VARIANTS)
	power_on_reset = false;
	for(i = 0; i < 411; i++) vram_wrote_table[i] = false;
	nmi_enable = true;
   
	//if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	//nmi_event_id = -1;
	//register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	//for(i = 0; i < 8; i++) set_dpalette(i, i);
	subcpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
	do_firq(!firq_mask && key_firq_req);
#else
	if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	nmi_event_id = -1;
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_

#endif	
	subcpu->reset();
}

void DISPLAY::update_config()
{
	vram_wrote = true;
	switch(config.cpu_type) {
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
#if !defined(_FM8)
	is_cyclesteal = ((config.dipswitch & FM7_DIPSW_CYCLESTEAL) != 0) ? true : false;
#endif	
	enter_display();
}

/*
 * Vram accessing functions moved to vram.cpp .
 */

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
	if(!nmi_enable) flag = false;
#endif
	subcpu->write_signal(SIG_CPU_NMI, flag ? 1 : 0, 1);
}

void DISPLAY::set_multimode(uint8 val)
{
	multimode_accessmask = val & 0x07;
	multimode_dispmask = (val & 0x70) >> 4;
	vram_wrote = true;
#if defined(_FM77AV_VARIANTS)
	alu->write_signal(SIG_ALU_MULTIPAGE, multimode_accessmask, 0x07);
#endif
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
	if(prev_clock != subclock) p_vm->set_cpu_clock(subcpu, subclock);
	prev_clock = subclock;
}

void DISPLAY::leave_display(void)
{
}

void DISPLAY::halt_subsystem(void)
{
	halt_flag = false;
  	halt_subcpu();
}

void DISPLAY::restart_subsystem(void)
{
	halt_flag = false;
#if defined(_FM77AV_VARIANTS)
	if(subcpu_resetreq) {
		reset_cpuonly();
		power_on_reset = true;
		subcpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		subcpu->reset();
		do_firq(!firq_mask && key_firq_req);
	}
#endif
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
	mainio->write_signal(FM7_MAINIO_SUB_ATTENTION, 0x01, 0x01);
	return 0xff;
}

// SUB:D405:W
void DISPLAY::set_cyclesteal(uint8 val)
{
#if !defined(_FM8)
	vram_wrote = true;
	val &= 0x01;
# if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)	
	if(val == 0) {
		is_cyclesteal = true;
	} else {
		is_cyclesteal = false;
	}
# endif	
	enter_display();
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
	alu->write_data8(ALU_BANK_DISABLE, data);
}

// D41C - D41F
void DISPLAY::alu_write_tilepaint_data(uint32 addr, uint8 val)
{
	uint32 data = (uint32)val;
	switch(addr & 3) {
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
			//alu->write_data8(ALU_TILEPAINT_L, 0xff);
			break;
	}
}

// D420
void DISPLAY::alu_write_offsetreg_hi(uint8 val)
{
	alu->write_data8(ALU_OFFSET_REG_HIGH, val & 0x7f);
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

//SUB:D430:R
uint8 DISPLAY::get_miscreg(void)
{
	uint8 ret;

	ret = 0x6a;
	if(!hblank) ret |= 0x80;
	if(vsync) ret |= 0x04;
	if(alu->read_signal(SIG_ALU_BUSYSTAT) == 0) ret |= 0x10;
	if(power_on_reset) ret |= 0x01;
	return ret;
}

//SUB:D430:W
void DISPLAY::set_miscreg(uint8 val)
{
	int old_display_page = display_page;

	nmi_enable = ((val & 0x80) == 0) ? true : false;
	if(!nmi_enable) do_nmi(false);

	if((val & 0x40) == 0) {
		display_page = 0;
	} else {
		display_page = 1;
	}
	if(display_page != old_display_page) {
			vram_wrote = true;
	}
	active_page = ((val & 0x20) == 0) ? 0 : 1;
	if((val & 0x04) == 0) {
		offset_77av = false;
	} else {
		offset_77av = true;
	}
	cgrom_bank = val & 0x03;
}

// Main: FD13
void DISPLAY::set_monitor_bank(uint8 var)
{
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if((var & 0x04) != 0){
		monitor_ram = true;
	} else {
		monitor_ram = false;
	}
# endif
	subrom_bank = var & 0x03;
	vram_wrote = true;
	if(!halt_flag) {
	  	subcpu_resetreq = false;
		power_on_reset = true;
		reset_cpuonly();
		subcpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		subcpu->reset();
		do_firq(!firq_mask && key_firq_req);
	} else {
	  	subcpu_resetreq = true;
	}
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

void DISPLAY::calc_apalette(uint16 idx)
{
	uint8 r, g, b;
	idx = idx & 4095;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	analog_palette_pixel[idx] = RGB_COLOR(r, g, b);
}

// FD32
void DISPLAY::set_apalette_b(uint8 val)
{
	uint16 index;
	uint8 tmp;
	index = apalette_index.w.l;
	tmp = (val & 0x0f) << 4;
	if(analog_palette_b[index] != tmp) {
		analog_palette_b[index] = tmp;
		calc_apalette(index);
		vram_wrote = true;
	}
}

// FD33
void DISPLAY::set_apalette_r(uint8 val)
{
	uint16 index;
	uint8 tmp;
	index = apalette_index.w.l;
	tmp = (val & 0x0f) << 4;
	if(analog_palette_r[index] != tmp) {
		analog_palette_r[index] = tmp;
		calc_apalette(index);
		vram_wrote = true;
	}
}

// FD34
void DISPLAY::set_apalette_g(uint8 val)
{
	uint16 index;
	uint8 tmp;
	index = apalette_index.w.l;
	tmp = (val & 0x0f) << 4;
	if(analog_palette_g[index] != tmp) {
		analog_palette_g[index] = tmp;
		calc_apalette(index);
		vram_wrote = true;
	}
}

#endif // _FM77AV_VARIANTS


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
				//do_nmi(false);
			}
#else
			do_nmi(true);
#endif
			break;
  		case EVENT_FM7SUB_DISPLAY_NMI_OFF: // per 20.00ms
			do_nmi(false);
			break;
#if defined(_FM77AV_VARIANTS)
		case EVENT_FM7SUB_HDISP:
			hblank = false;
			vblank = false;
			vsync = false;
			mainio->write_signal(SIG_DISPLAY_DISPLAY, 0x02, 0xff);
			usec = 39.5;
			if(display_mode == DISPLAY_MODE_8_400L) usec = 30.0;
			register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hblank_event_id); // NEXT CYCLE_
			
			break;
		case EVENT_FM7SUB_HBLANK:
			hblank = true;
			vblank = false;
			vsync = false;
			mainio->write_signal(SIG_DISPLAY_DISPLAY, 0x00, 0xff);
			f = false;
			if(display_mode == DISPLAY_MODE_8_400L) {
				if(displine < 400) f = true;
			} else {
				if(displine < 200) f = true;
			}
			if(f) {
				if(display_mode == DISPLAY_MODE_8_400L) {
					usec = 11.0;
				} else {
					usec = 24.0;
				}
				register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id);
				
				if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) != 0) {
					if((display_mode == DISPLAY_MODE_4096) || (display_mode == DISPLAY_MODE_256k)){
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
						int planes = 2;
# elif defined(_FM77AV40)
						int planes = 1;
# else
						int planes = 1;
#endif
						if(vram_wrote_table[displine] || vram_wrote) {
							uint32 baseaddr1 = (displine * 40) & 0x1fff;
							uint32 baseaddr2 = baseaddr1 + 0xc000;
							vram_wrote_table[displine] = false;
							for(int i = 0; i < planes; i++) {
								for(int j = 0; j < 6; j++) {
									memcpy(&gvram_shadow[j * 0x2000 + baseaddr1],
										   &gvram[j * 0x2000 + baseaddr1], 40);
									memcpy(&gvram_shadow[j * 0x2000 + baseaddr2],
										   &gvram[j * 0x2000 + baseaddr2], 40);
								}
								baseaddr1 += 0x18000;
								baseaddr2 += 0x18000;
							}
# if defined(_FM77AV40)
							for(int j = 0; j < 6; j++) {
								memcpy(&gvram_shadow[j * 0x2000 + baseaddr1],
									   &gvram[j * 0x2000 + baseaddr1], 40);
							}
# endif							
							vram_draw_table[displine] = true;
						}
					} else if(display_mode == DISPLAY_MODE_8_400L) {
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
						int planes = 2;
# elif defined(_FM77AV40)
						int planes = 1;
# else
						int planes = 0;
#endif
						if(vram_wrote_table[displine] || vram_wrote) {
							uint32 baseaddr1 = (displine * 80) & 0x7fff;
							vram_wrote_table[displine] = false;
							for(int i = 0; i < planes; i++) {
								for(int j = 0; j < 3; j++) {
									memcpy(&gvram_shadow[j * 0x8000 + baseaddr1],
										   &gvram[j * 0x8000 + baseaddr1], 80);
								}
								baseaddr1 += 0x18000;
							}
							
# if defined(_FM77AV40)
							for(int j = 0; j < 3; j++) {
								memcpy(&gvram_shadow[j * 0x4000 + baseaddr1],
									   &gvram[j * 0x4000 + baseaddr1], 80);
							}
# endif							
							vram_draw_table[displine] = true;
						}
					} else {
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
						int planes = 2;
# elif defined(_FM77AV40)
						int planes = 1;
# else
						int planes = 1;
#endif
						if(vram_wrote_table[displine] || vram_wrote) {
							uint32	baseaddr1 = (displine * 80) & 0x3fff;
							uint32	baseaddr2 = baseaddr1 + 0xc000;
							vram_wrote_table[displine] = false;
							for(int i = 0; i < planes; i++) {
								for(int j = 0; j < 3; j++) {
									memcpy(&gvram_shadow[j * 0x4000 + baseaddr1],
										   &gvram[j * 0x4000 + baseaddr1], 80);
									memcpy(&gvram_shadow[j * 0x4000 + baseaddr2],
										   &gvram[j * 0x4000 + baseaddr2], 80);
								}
								baseaddr1 += 0x18000;
								baseaddr2 += 0x18000;
							}
# if defined(_FM77AV40)
							for(int j = 0; j < 3; j++) {
								memcpy(&gvram_shadow[j * 0x4000 + baseaddr1],
								   &gvram[j * 0x4000 + baseaddr1], 80);
							}
# endif
							vram_draw_table[displine] = true;
						}
					}
				}
			}
			//vram_wrote_shadow = true;
			displine++;
			break;
		case EVENT_FM7SUB_VSTART: // Call first.
			vblank = true;
			vsync = false;
			hblank = false;
			displine = 0;
			//leave_display();
			// Parameter from XM7/VM/display.c , thanks, Ryu.
			mainio->write_signal(SIG_DISPLAY_DISPLAY, 0x00, 0xff);
			mainio->write_signal(SIG_DISPLAY_VSYNC, 0x00, 0xff);
			if(vblank_count != 0) {
				if(display_mode == DISPLAY_MODE_8_400L) {
					usec = (0.98 + 16.4) * 1000.0;
				} else {
					usec = (1.91 + 12.7) * 1000.0;
				}
				register_event(this, EVENT_FM7SUB_VSYNC, usec, false, &vsync_event_id);

				if(display_mode == DISPLAY_MODE_8_400L) {
					usec = 930.0; // 939.0
				} else {
					usec = 1840.0; // 1846.5
				}
				offset_point_bank1_bak = offset_point_bank1;
				offset_point_bak = offset_point;
				if(vram_wrote || ((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0)) {
					//for(int i = 0; i < 411; i++) vram_wrote_table[i] = false;
				}
				register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id); // NEXT CYCLE_
				vblank_count = 0;
			} else {
				if(display_mode == DISPLAY_MODE_8_400L) {
					usec = 0.34 * 1000.0;
				} else {
					usec = 1.52 * 1000.0;
				}
				register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
				vblank_count++;
			}
			break;
	case EVENT_FM7SUB_VSYNC:
		vblank = true;
		hblank = false;
		vsync = true;
		displine = 0;
		if(display_mode == DISPLAY_MODE_8_400L) {
			usec = 0.33 * 1000.0; 
		} else {
			usec = 0.51 * 1000.0;
		}
		mainio->write_signal(SIG_DISPLAY_VSYNC, 0x01, 0xff);
		register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
		if(vram_wrote || ((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0)) {
			//for(int i = 0; i < 411; i++) vram_wrote_table[i] = false;
			memcpy(gvram_shadow, gvram, sizeof(gvram_shadow));
			vram_wrote_shadow = true;
			vram_wrote = false;
			for(int i = 0; i < 411; i++) vram_draw_table[i] = true;
		} else {
			if((display_mode == DISPLAY_MODE_4096) || (display_mode == DISPLAY_MODE_256k)){
				int planes;
				bool ff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				planes = 24;
# elif defined(_FM77AV40)
				planes = 18;
# else
				planes = 12;
# endif
				ff = vram_wrote;
				if(!ff) {
					for(int j = 200; j < 206; j++) {
						if(vram_wrote_table[j]) {
							ff = true;
						}
						vram_wrote_table[j] = false;
					}
				}
				if(ff) {
					for(int j = 200; j < 205; j++) {
						vram_draw_table[(((j * 40) + offset_point_bak) & 0x1fff) / 40] = true;
						vram_draw_table[(((j * 40) + offset_point_bank1_bak) & 0x1fff) / 40] = true;
					}
					for(int i = 0; i < planes; i++) memcpy(&gvram_shadow[i * 0x2000 + 200 * 40],
														   &gvram[i * 0x2000 + 200 * 40], 0x2000 - 200 * 40);
				}
			} else if(display_mode == DISPLAY_MODE_8_400L) {
				int planes;
				bool ff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				planes = 6;
# elif defined(_FM77AV40)
				planes = 3;
# else
				planes = 0;
# endif
				ff = vram_wrote;
				if(!ff) {
					for(int j = 400; j < 411; j++) {
						if(vram_wrote_table[j]) {
							ff = true;
						}
						vram_wrote_table[j] = false;
					}
				}
				if(ff) {
					for(int j = 400; j < 410; j++) {
						vram_draw_table[(((j * 80) + offset_point_bak) & 0x7fff) / 80] = true;
						vram_draw_table[(((j * 80) + offset_point_bank1_bak) & 0x7fff) / 80] = true;
					}
					for(int i = 0; i < planes; i++) memcpy(&gvram_shadow[i * 0x8000 + 400 * 80],
														   &gvram[i * 0x8000 + 400 * 80], 0x8000 - 400 * 80);
				}
			} else {
				int planes;
				bool ff;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
				planes = 12;
# elif defined(_FM77AV40)
				planes = 9;
# else
				planes = 6;
# endif
				ff = vram_wrote;
				if(!ff) {
					for(int j = 200; j < 206; j++) {
						if(vram_wrote_table[j]) {
							ff = true;
						}
						vram_wrote_table[j] = false;
					}
				}
				if(ff) {
					for(int j = 200; j < 205; j++) {
						vram_draw_table[(((j * 80) + offset_point_bak) & 0x3fff) / 80] = true;
						vram_draw_table[(((j * 80) + offset_point_bank1_bak) & 0x3fff) / 80] = true;
					}
					for(int i = 0; i < planes; i++) memcpy(&gvram_shadow[i * 0x4000 + 200 * 80],
														   &gvram[i * 0x4000 + 200 * 80], 0x4000 - 200 * 80);
				}
			}
		}
		if(vram_wrote) {
			vram_wrote_shadow = true;
		} else {
			int fy = false;
			if(display_mode == DISPLAY_MODE_8_400L) {
				for(int yy = 0; yy < 400; yy++) {
					if(vram_draw_table[yy]) fy = true;
					vram_draw_table[yy] = false;
				}
			} else {
				for(int yy = 0; yy < 200; yy++) {
					if(vram_draw_table[yy]) fy = true;
				vram_draw_table[yy] = false;
				}
			}
			vram_wrote_shadow = fy;
		}
		vram_wrote = false;
		break;
#endif			
	case EVENT_FM7SUB_CLR_BUSY:
		set_subbusy();
		break;
	case EVENT_FM7SUB_CLR_CRTFLAG:
		reset_crtflag();
		break;
	}
}

void DISPLAY::event_frame()
{
	enter_display();
}

void DISPLAY::event_vline(int v, int clock)
{
}


uint32 DISPLAY::read_signal(int id)
{
	uint32 retval = 0;
	switch(id) {
		case SIG_FM7_SUB_HALT:
		case SIG_DISPLAY_HALT:
			retval = (halt_flag) ? 0xffffffff : 0;
			break;
		case SIG_DISPLAY_BUSY:
			retval =  (sub_busy) ? 0x80 : 0;
		 	break;
		case SIG_DISPLAY_MULTIPAGE:
			retval =  multimode_accessmask;
			break;
		case SIG_DISPLAY_PLANES:
			retval = 3;
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_DISPLAY_VSYNC:
			retval = (vsync) ? 0x01 : 0x00;
			break;
		case SIG_DISPLAY_DISPLAY:
			retval = (!hblank) ? 0x02: 0x00;
			break;
		case SIG_FM7_SUB_BANK: // Main: FD13
			retval = subrom_bank & 0x03;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(monitor_ram) retval |= 0x04;
#endif
			break;
#endif			
#if defined(_FM77AV_VARIANTS)
		case SIG_DISPLAY_MODE320:
			retval = (mode320) ? 0x40: 0x00;
			break;
#endif
		case SIG_DISPLAY_Y_HEIGHT:
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			retval = (mode400line) ? 400 : 200;
#else
		  	retval = 200;
#endif		  
			break;
		case SIG_DISPLAY_X_WIDTH:
#if defined(_FM77AV_VARIANTS)
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			retval = (mode320 || mode256k) ? 40 : 80;
# else
			retval = (mode320) ? 40 : 80;
# endif	   
#else
		  	retval = 640;
#endif		  
			break;
		default:
			break;
	}
	return retval;
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	bool flag = ((data & mask) != 0);
	int y;
	switch(id) {
		case SIG_FM7_SUB_HALT:
			if(flag) {
				sub_busy = true;
			}
			halt_flag = flag;
			//mainio->write_signal(SIG_FM7_SUB_HALT, data, mask);
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
				if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) == 0) clk = clk / 3;
				if(clk != prev_clock) p_vm->set_cpu_clock(subcpu, clk);
				clock_fast = flag;
				prev_clock = clk;
			}
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_BANK: // Main: FD13
			set_monitor_bank(data & 0xff);
			break;
#endif			
		case SIG_DISPLAY_EXTRA_MODE: // FD04 bit 4, 3
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			{
				int oldmode = display_mode;
				kanjisub = ((data & 0x20) == 0) ? true : false;
				mode256k = ((data & 0x10) != 0) ? true : false;
				mode400line = ((data & 0x08) != 0) ? false : true;
				ram_protect = ((data & 0x04) != 0) ? false : true;
				if(mode400line) {
					display_mode = DISPLAY_MODE_8_400L;
				} else if(mode256k) {
					display_mode = DISPLAY_MODE_256k;
				} else {
					display_mode = (mode320) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
				}
				if(oldmode != display_mode) {
					for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
					vram_wrote = true;
					alu->write_signal(SIG_ALU_X_WIDTH, ((mode320 || mode256k) && !(mode400line)) ? 40 :  80, 0xffff);
					alu->write_signal(SIG_ALU_Y_HEIGHT, (mode400line) ? 400 : 200, 0xffff);
					alu->write_signal(SIG_ALU_400LINE, (mode400line) ? 0xff : 0x00, 0xff);
					frame_skip_count = 3;
				}
			}
#elif defined(_FM77_VARIANTS)
			{
				int oldmode = display_mode;
				kanjisub = ((data & 0x20) == 0) ? true : false;
# if defined(_FM77L4)				
				stat_400linecard = ((data & 0x20) != 0) ? true : false;
				mode400line = ((data & 0x08) != 0) ? false : true;
				if(mode400line && stat_400linecard) {
					display_mode = DISPLAY_MODE_8_400L_TEXT;
				} else if(stat_400linecard) {
					display_mode = DISPLAY_MODE_8_200L_TEXT;
				} else {
					display_mode = DISPLAY_MODE_8_200L;
				}
# endif				
			}			
#endif
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_DISPLAY_MODE320: // FD12 bit 6
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			{
				int oldmode = display_mode;
				mode320 = flag;
				if((!mode400line) && (!mode256k)){
					display_mode = (mode320) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
				}
				if(oldmode != display_mode) {
					for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
					vram_wrote = true;
					alu->write_signal(SIG_ALU_X_WIDTH, ((mode320 || mode256k) && !(mode400line)) ? 40 :  80, 0xffff);
					alu->write_signal(SIG_ALU_Y_HEIGHT, (mode400line) ? 400 : 200, 0xffff);
					alu->write_signal(SIG_ALU_400LINE, (mode400line) ? 0xff : 0x00, 0xff);
					frame_skip_count = 3;
				}
			}
# else
			if(mode320 != flag) {
				for(y = 0; y < 400; y++) memset(emu->screen_buffer(y), 0x00, 640 * sizeof(scrntype));
			}
			mode320 = flag;
			display_mode = (mode320 == true) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
			alu->write_signal(SIG_ALU_X_WIDTH, (mode320) ? 40 :  80, 0xffff);
			alu->write_signal(SIG_ALU_Y_HEIGHT, 200, 0xffff);
			alu->write_signal(SIG_ALU_400LINE, 0, 0xffffffff);
			vram_wrote = true;
# endif
#endif			
			break;
		case SIG_DISPLAY_MULTIPAGE:
	  		set_multimode(data);
			break;
		case SIG_FM7_SUB_KEY_MASK:
			if(firq_mask == flag) {
				do_firq(!flag && key_firq_req);
			}
			firq_mask = !flag;
			break;
		case SIG_FM7_SUB_KEY_FIRQ:
			do_firq(flag & !(firq_mask));
			key_firq_req = flag;
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
   
/*
 * Vram accessing functions moved to vram.cpp .
 */

uint8 DISPLAY::read_mmio(uint32 addr)
{
	uint32 retval = 0xff;
	uint32 raddr;	
	if(addr < 0xd400) return 0xff;
	
#if !defined(_FM77AV_VARIANTS)
	raddr = (addr - 0xd400) & 0x000f;
#elif !defined(_FM77AV40SX) && !defined(_FM77AV40EX)
	raddr = (addr - 0xd400) & 0x003f;
#else // FM77AV40EX || FM77AV40SX
	raddr = (addr - 0xd400) & 0x00ff;
#endif
	switch(raddr) {
		case 0x00: // Read keyboard
			retval = (keyboard->read_data8(0x00) != 0) ? 0xff : 0x7f;
			break;
		case 0x01: // Read keyboard
			retval = keyboard->read_data8(0x01) & 0xff;
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
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
     defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) || defined(_FM77_VARIANTS) // _FM77L4
		case 0x06:
			if(!kanjisub) return 0xff;
# if !defined(_FM77_VARIANTS)
			if(kanji_level2) {
				return (uint8)kanjiclass2->read_data8(KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff));
			}
# endif
			retval = kanjiclass1->read_data8(KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff));
			break;
		case 0x07:
			if(!kanjisub) return 0xff;
# if !defined(_FM77_VARIANTS)
			if(kanji_level2) {
				return (uint8)kanjiclass2->read_data8(KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff) + 1);
			}
# endif
			retval = kanjiclass1->read_data8(KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff) + 1);
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
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		case 0x2f: // VRAM BANK
			retval = 0xfc | (vram_bank & 0x03);
			break;
# endif			
		// MISC
		case 0x30:
			retval = get_miscreg();
			break;
		// KEY ENCODER.
		case 0x31:
			retval = keyboard->read_data8(0x31);
			break;
		case 0x32:
			retval = keyboard->read_data8(0x32);
			break;
#endif				
		default:
			break;
	}
	return (uint8)retval;
}

uint32 DISPLAY::read_vram_data8(uint32 addr)
{
	uint32 offset;
	uint32 page_offset = 0;
	uint32 page_mask = 0x3fff;
	uint32 color = (addr >> 14) & 0x03;
	uint32 pagemod;
	
#if defined(_FM77AV_VARIANTS)
	if (active_page != 0) {
		offset = offset_point_bank1;
	} else {
		offset = offset_point;
	}
#else
	offset = offset_point;
#endif
		
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(vram_active_block != 0) page_offset = 0x18000;
# endif		
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr >= 0x8000) return 0xff;
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
			offset <<= 1;
			pagemod = 0x8000 * color;
			return gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset];
		} else {
			if(mode256k) {
#if defined(_FM77AV40)
				if(vram_bank < 3) {
					page_offset = 0xc000 * (vram_bank & 0x03);
				} else {
					page_offset = 0; // right?
				}
#else			
				page_offset = 0xc000 * (vram_bank & 0x03);
#endif			
				page_mask = 0x1fff;
				pagemod = addr & 0xe000;
			} else {
				if(mode320) {
					page_mask = 0x1fff;
					pagemod = addr & 0xe000;
				} else {
					pagemod = addr & 0xc000;
				}
				if(active_page != 0) {
					page_offset += 0xc000;
				}
			}				
			return gvram[(((addr + offset) & page_mask) | pagemod) + page_offset];
		}
#elif defined(_FM77AV_VARIANTS)
		{		
			if(active_page != 0) {
				page_offset += 0xc000;
			}
			if(mode320) {
				page_mask = 0x1fff;
				pagemod = addr & 0xe000;
			} else {
				pagemod = addr & 0xc000;
			}
			return gvram[(((addr + offset) & page_mask) | pagemod) + page_offset];
		}
#elif defined(_FM77L4) //_FM77L4
		{
			if(display_mode == DISPLAY_MODE_8_400L) {
				return (uint32)read_vram_l4_400l(addr, offset);
			} else {
				pagemod = addr & 0xc000;
				return gvram[((addr + offset) & 0x3fff) | pagemod];
			}
			return 0xff;
		}
#else // Others (77/7/8)
		pagemod = addr & 0xc000;
		return gvram[((addr + offset) & 0x3fff) | pagemod];
#endif
}

void DISPLAY::write_dma_data8(uint32 addr, uint32 data)
{
	uint32 raddr = addr & 0xffff;
	uint32 color;
	if(addr < 0xc000) {
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
		} else {
			color = (addr >> 14) & 0x03;
		}
# endif
		if((multimode_accessmask & (1 << color)) != 0) return;
		return write_vram_data8(raddr, (uint8)data);
	} else {
		return write_data8_main(raddr, (uint8)data);
	}
}


void DISPLAY::write_vram_data8(uint32 addr, uint8 data)
{
	uint32 offset;
	uint32 page_offset = 0;
	uint32 page_mask = 0x3fff;
	uint32 color = (addr >> 14) & 0x03;
	uint32 pagemod;
	
#if defined(_FM77AV_VARIANTS)
	if (active_page != 0) {
		offset = offset_point_bank1;
	} else {
		offset = offset_point;
	}
#else
	offset = offset_point;
#endif

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(vram_active_block != 0) page_offset = 0x18000;
# endif		
		if(display_mode == DISPLAY_MODE_8_400L) {
			if(addr >= 0x8000) return;
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
			offset <<= 1;
			pagemod = 0x8000 * color;
			gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x8000) / 80] = true;
		} else 	if(display_mode == DISPLAY_MODE_256k) {
#if defined(_FM77AV40)
			if(vram_bank < 3) {
				page_offset = 0xc000 * (vram_bank & 0x03);
			} else {
				page_offset = 0; // right?
			}
#else			
			page_offset = 0xc000 * (vram_bank & 0x03);
#endif			
			page_mask = 0x1fff;
			pagemod = addr & 0xe000;
			gvram[(((addr + offset) & page_mask) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x2000) / 40] = true;
			return;
		} else if(display_mode == DISPLAY_MODE_4096) {
			if(active_page != 0) {
				page_offset += 0xc000;
			}
			page_mask = 0x1fff;
			pagemod = addr & 0xe000;
			gvram[(((addr + offset) & page_mask) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x2000) / 40] = true;
		} else { // 200line
			if(active_page != 0) {
				page_offset += 0xc000;
			}
			page_mask = 0x3fff;
			pagemod = addr & 0xc000;
			gvram[(((addr + offset) & page_mask) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x4000) / 80] = true;
		}				

#elif defined(_FM77AV_VARIANTS)
		if(display_mode == DISPLAY_MODE_4096) {
			if(active_page != 0) {
				page_offset = 0xc000;
			}
			page_mask = 0x1fff;
			pagemod = addr & 0xe000;
			gvram[(((addr + offset) & page_mask) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x2000) / 40] = true;
		} else { // 200line
			if(active_page != 0) {
				page_offset = 0xc000;
			}
			page_mask = 0x3fff;
			pagemod = addr & 0xc000;
			gvram[(((addr + offset) & page_mask) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x4000) / 80] = true;
		}
#elif defined(_FM77L4) //_FM77L4
		{
			if(display_mode == DISPLAY_MODE_8_400L) {
				write_vram_l4_400l(addr, data, offset);
			} else {
				pagemod = addr & 0xc000;
				gvram[((addr + offset) & 0x3fff) | pagemod] = data;
			}
			//vram_wrote_table[((addr + offset) % 0x4000) / 80] = true;
		}
#else // Others (77/7/8)
		pagemod = addr & 0xc000;
		gvram[((addr + offset) & 0x3fff) | pagemod] = data;
		//vram_wrote_table[((addr + offset) % 0x4000) / 80] = true;
#endif

#if defined(_FM77AV_VARIANTS)	
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
#else
	vram_wrote = true;
#endif	
}

uint32 DISPLAY::read_data8_main(uint32 addr)
{
	uint32 raddr;
	if(addr < 0xc000) return 0xff;
	if(addr < 0xd000) { 
		raddr = addr - 0xc000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(monitor_ram) {
			if(console_ram_bank >= 1) {
				return submem_console_av40[((console_ram_bank - 1) << 12) + raddr];
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
#if defined(_FM77AV_VARIANTS)
		if(addr >= 0xd500) {
			return submem_hidden[addr - 0xd500];
		}
#endif	   
		return read_mmio(addr);
	} else if(addr < 0x10000) {
#if !defined(_FM77AV_VARIANTS)
		return subsys_c[addr - 0xd800];
#else
		if(addr < 0xe000) {
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
			if(monitor_ram) {
				return submem_cgram[cgram_bank * 0x0800 + (addr - 0xd800)]; //FIXME
			}
#endif		
			return subsys_cg[(addr - 0xd800) + cgrom_bank * 0x800];
		} else if(addr < 0x10000) {
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
			if(monitor_ram) {
				return subsys_ram[addr - 0xe000];
			}
#endif		
			switch(subrom_bank_using & 3) {
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
	}
	return 0xff;
}

uint32 DISPLAY::read_dma_data8(uint32 addr)
{
	uint32 raddr = addr & 0xffff;
	uint32 color;
	if(addr < 0xc000) {
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
		} else {
			color = (addr >> 14) & 0x03;
		}
# endif
		if((multimode_accessmask & (1 << color)) != 0) return 0xff;
		return read_vram_data8(raddr);
	} else {
		return read_data8_main(raddr);
	}
}


uint32 DISPLAY::read_data8(uint32 addr)
{
	uint32 raddr = addr;
	uint32 offset;
	uint32 color = (addr & 0x0c000) >> 14;
	if(addr < 0xc000) {
#if defined(_FM77AV_VARIANTS)
		if(use_alu) {
			alu->read_data8(addr + ALU_WRITE_PROXY);
		}
#endif
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
		} else {
			color = (addr >> 14) & 0x03;
		}
# endif
		if((multimode_accessmask & (1 << color)) != 0) return 0xff;
		return read_vram_data8(addr);
	} else if(addr < 0x10000) {
		return read_data8_main(addr);
	} else if((addr >= FM7_SUBMEM_OFFSET_DPALETTE) && (addr < (FM7_SUBMEM_OFFSET_DPALETTE + 8))) {
		return dpalette_data[addr - FM7_SUBMEM_OFFSET_DPALETTE];
	}
#if defined(_FM77AV_VARIANTS)
	// ACCESS VIA ALU.
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0x18000))) {
		addr = addr - DISPLAY_VRAM_DIRECT_ACCESS;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)		
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32 page_offset = 0;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(vram_active_block != 0) page_offset = 0x18000;
# endif		
			color = (addr & 0x18000) >> 15;
			if(color > 2) color = 0;
			
			if (active_page != 0) {
				offset = offset_point_bank1 << 1;
			} else {
				offset = offset_point << 1;
			}
			if(color > 2) color = 0;
			uint32 pagemod = 0x8000 * color;
			return gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset];
		}
# endif		
		return read_vram_data8(addr);
	}
#endif
	return 0xff;
}	

/*
 * Vram accessing functions moved to vram.cpp .
 */

void DISPLAY::write_mmio(uint32 addr, uint32 data)
{
	uint8 rval = 0;
	pair tmpvar;
	if(addr < 0xd400) return;
	
#if !defined(_FM77AV_VARIANTS)
	addr = (addr - 0xd400) & 0x000f;
#elif !defined(_FM77AV40SX) && !defined(_FM77AV40EX)
	addr = (addr - 0xd400) & 0x003f;
#else // FM77AV40EX || FM77AV40SX
	addr = (addr - 0xd400) & 0x00ff;
#endif
	io_w_latch[addr] = (uint8)data;
	switch(addr) {
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4)
		// FM77 SPECIFIED
		case 0x05:
			set_cyclesteal((uint8)data);
			break;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
     defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) || defined(_FM77_VARIANTS) // _FM77L4
		// KANJI
		case 0x06:
			if(!kanjisub) return;
			kanjiaddr.w.h = 0x0000;
			kanjiaddr.b.h = (uint8) data;
			break;
		case 0x07:
			if(!kanjisub) return;
			kanjiaddr.w.h = 0x0000;
			kanjiaddr.b.l = (uint8)data;
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
				double usec;
				if(clock_fast) {
					usec = (1000.0 * 1000.0) / 2000000.0;
				} else {
					usec = (1000.0 * 1000.0) / 999000.0;
				}
			 	if(!is_cyclesteal) usec = usec * 3.0;
				usec = (double)clr_count * usec;
				register_event(this, EVENT_FM7SUB_CLR_BUSY, usec, false, NULL); // NEXT CYCLE_
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
		case 0x0f:
			rval = (uint8)data;
			if(offset_changed[active_page]) {
#if defined(_FM77AV_VARIANTS)
				if(active_page != 0) {
					tmp_offset_point[active_page].d = offset_point_bank1;
				} else {
					tmp_offset_point[active_page].d = offset_point;
				}
#else
				tmp_offset_point[active_page].d = offset_point;
#endif
			}
			tmp_offset_point[active_page].w.h = 0x0000;
			if(addr == 0x0e) {
				tmp_offset_point[active_page].b.h = rval;
			} else {
				tmp_offset_point[active_page].b.l = rval;
			}
			offset_changed[active_page] = !offset_changed[active_page];
			if(offset_changed[active_page]) {
				vram_wrote = true;
#if defined(_FM77AV_VARIANTS)
				if(active_page != 0) {
					if(offset_77av) {
						offset_point_bank1 = tmp_offset_point[active_page].d & 0x007fff;
					} else {
						offset_point_bank1 = tmp_offset_point[active_page].d & 0x007fe0;
					}				   
				} else {
					if(offset_77av) {
						offset_point = tmp_offset_point[active_page].d & 0x007fff;
					} else {
						offset_point = tmp_offset_point[active_page].d & 0x007fe0;
					}				   
				}
#else
				offset_point = tmp_offset_point[active_page].d & 0x7fe0;
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
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		case 0x2e: //
			console_ram_bank = (data & 0x18) >> 3;
			if(console_ram_bank > 2) console_ram_bank = 0;
			cgram_bank = data & 0x07;
			kanji_level2 = ((data & 0x80) == 0) ? false : true;
			break;
		case 0x2f: // VRAM BANK
			vram_bank = data &  0x03;
			if(vram_bank > 2) vram_bank = 0;
			vram_wrote = true;
			break;
# endif			
		// MISC
		case 0x30:
			set_miscreg(data);
			break;
		// KEYBOARD ENCODER
		case 0x31:
			keyboard->write_data8(0x31, data);
			break;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		case 0x33: //
			vram_active_block = data & 0x01;
			if(vram_display_block != (((data & 0x10) != 0) ? 1 : 0)) vram_wrote = true;
			vram_display_block = ((data & 0x10) != 0) ? 1 : 0;
			break;
			// Window
		case 0x38: //
		case 0x39: //
			tmpvar.d = window_xbegin * 8;
			tmpvar.w.h = 0;
			if(addr == 0x38) {
				tmpvar.b.h = data & 0x03;
			} else {
				tmpvar.b.l = data & 0xf8;
			}
			if(mode320 || mode256k) {
			   if(tmpvar.d > 320) tmpvar.d = 320;
			} else {
			   if(tmpvar.d > 640) tmpvar.d = 640;
			}
			window_xbegin = tmpvar.d / 8;
			vram_wrote = true;
			break;
		case 0x3a: //
		case 0x3b: //
			tmpvar.d = window_xend * 8;
			tmpvar.w.h = 0;
			if(addr == 0x3a) {
				tmpvar.b.h = data & 0x03;
			} else {
				tmpvar.b.l = data & 0xf8;
			}
			if(mode320 || mode256k) {
			   if(tmpvar.d > 320) tmpvar.d = 320;
			} else {
			   if(tmpvar.d > 640) tmpvar.d = 640;
			}
			window_xend = tmpvar.d / 8;
			vram_wrote = true;
			break;
		case 0x3c: //
		case 0x3d: //
			tmpvar.d = window_low;
			tmpvar.w.h = 0;
			if(addr == 0x3c) {
				tmpvar.b.h = data & 0x03;
			} else {
				tmpvar.b.l = data & 0xff;
			}
			if(mode400line) {
				if(tmpvar.d > 400) tmpvar.d = 400;
			} else {
				tmpvar.d <<= 1;
				if(tmpvar.d > 400) tmpvar.d = 400;
			}
			window_low = tmpvar.d;
			vram_wrote = true;
			break;
		case 0x3e: //
		case 0x3f: //
			tmpvar.d = window_high;
			tmpvar.w.h = 0;
			if(addr == 0x3e) {
				tmpvar.b.h = data & 0x03;
			} else {
				tmpvar.b.l = data & 0xff;
			}
			if(mode400line) {
				if(tmpvar.d > 400) tmpvar.d = 400;
			} else {
				tmpvar.d <<= 1;
				if(tmpvar.d > 400) tmpvar.d = 400;
			}
			window_high = tmpvar.d;
			vram_wrote = true;
			break;
# endif
#endif				
		default:
#if defined(_FM77AV_VARIANTS)
			//ALU
			if((addr >= 0x13) && (addr <= 0x1a)) {
				alu_write_cmpdata_reg(addr - 0x13, data);
			} else if((addr >= 0x1c) && (addr <= 0x1e)) {
				alu_write_tilepaint_data(addr, data);
			} else if((addr >= 0x24) && (addr <= 0x2b)) {
				alu_write_line_position(addr - 0x24, data);
			}
#endif				
			break;
	}
}

void DISPLAY::write_data8_main(uint32 addr, uint8 data)
{
	uint32 offset;
	uint32 raddr;
	uint32 page_offset = 0x0000;

	if(addr < 0xc000) return;
	
	if(addr < 0xd000) { 
		raddr = addr - 0xc000;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(monitor_ram) {
			if(console_ram_bank >= 1) {
				submem_console_av40[((console_ram_bank - 1) << 12) + raddr] = data;
				return;
			}
		}
#endif
		console_ram[raddr] = data;
		return;
	} else if(addr < 0xd380) {
		raddr = addr - 0xd000;
		work_ram[raddr] = data;
		return;
	} else if(addr < 0xd400) {
		raddr = addr - 0xd380;
		shared_ram[raddr] = data;
		return;
	} else if(addr < 0xd800) {
#if defined(_FM77AV_VARIANTS)
		if(addr >= 0xd500) {
			submem_hidden[addr - 0xd500] = data;
			return;
		}
#endif
		write_mmio(addr, data);
		return;
	} else if(addr < 0x10000) {
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
		if(ram_protect) return;
		if(monitor_ram) {
			if(addr < 0xe000) {
				submem_cgram[cgram_bank * 0x0800 + (addr - 0xd800)] = data; //FIXME
			} else {
				subsys_ram[addr - 0xe000] = data;
			}
		}
#endif		
		return;
	}
}

void DISPLAY::write_data8(uint32 addr, uint32 data)
{
	uint32 offset;
	uint32 raddr;
	uint32 page_offset = 0x0000;
	uint8 val8 = data & 0xff;
	uint32 pagemod;
	uint32 color = (addr & 0xc000) >> 14;

	if(addr < 0xc000) {
#if defined(_FM77AV_VARIANTS)
		if(use_alu) {
			alu->read_data8(addr + ALU_WRITE_PROXY);
			return;
		}
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
		}
# endif
#endif
		if((multimode_accessmask & (1 << color)) != 0) return;
		write_vram_data8(addr, val8);
		return;
	} else if(addr < 0x10000) {
		write_data8_main(addr, val8);
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
	// ACCESS VIA ALU.
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0x18000))) {
		addr = addr - DISPLAY_VRAM_DIRECT_ACCESS; 
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			color = (addr & 0x18000) >> 15;
			if(color > 2) color = 0;
			page_offset = 0;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(vram_active_block != 0) page_offset = 0x18000;
# endif		
			if(color > 2) color = 0;
			if (active_page != 0) {
				offset = offset_point_bank1 << 1;
			} else {
				offset = offset_point << 1;
			}
			pagemod = 0x8000 * color;
			gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset] = data;
			vram_wrote_table[((addr + offset) % 0x8000) / 80] = true;
			return;
		}
		write_vram_data8(addr, data);
# else
		write_vram_data8(addr, data);
# endif
		if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
	}
#endif
	return;
}	


uint32 DISPLAY::read_bios(const char *name, uint8 *ptr, uint32 size)
{
	FILEIO fio;
	uint32 blocks;
	_TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = emu->bios_path((const _TCHAR *)name);
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
#if defined(_FM77AV_VARIANTS)
	memset(gvram_shadow, 0x00, sizeof(gvram_shadow));
	vram_wrote_shadow = false;
	for(i = 0; i < 411; i++) vram_wrote_table[i] = false;
	for(i = 0; i < 411; i++) vram_draw_table[i] = false;
#endif	
	memset(console_ram, 0x00, sizeof(console_ram));
	memset(work_ram, 0x00, sizeof(work_ram));
	memset(shared_ram, 0x00, sizeof(shared_ram));
	memset(subsys_c, 0xff, sizeof(subsys_c));
   
	diag_load_subrom_c = false;
	if(read_bios(_T("SUBSYS_C.ROM"), subsys_c, 0x2800) >= 0x2800) diag_load_subrom_c = true;
	emu->out_debug_log("SUBSYSTEM ROM Type C READING : %s", diag_load_subrom_c ? "OK" : "NG");
 
#if defined(_FM77AV_VARIANTS)
	memset(subsys_a, 0xff, sizeof(subsys_a));
	memset(subsys_b, 0xff, sizeof(subsys_b));
	memset(subsys_cg, 0xff, sizeof(subsys_cg));
	memset(submem_hidden, 0x00, sizeof(submem_hidden));
   
	diag_load_subrom_a = false;
   	if(read_bios(_T("SUBSYS_A.ROM"), subsys_a, 0x2000) >= 0x2000) diag_load_subrom_a = true;
	emu->out_debug_log("SUBSYSTEM ROM Type A READING : %s", diag_load_subrom_a ? "OK" : "NG");

	diag_load_subrom_b = false;
   	if(read_bios(_T("SUBSYS_B.ROM"), subsys_b, 0x2000) >= 0x2000) diag_load_subrom_b = true;
	emu->out_debug_log("SUBSYSTEM ROM Type B READING : %s", diag_load_subrom_b ? "OK" : "NG");

	diag_load_subrom_cg = false;
   	if(read_bios(_T("SUBSYSCG.ROM"), subsys_cg, 0x2000) >= 0x2000) diag_load_subrom_cg = true;
	emu->out_debug_log("SUBSYSTEM CG ROM READING : %s", diag_load_subrom_cg ? "OK" : "NG");
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	memset(subsys_ram, 0x00, sizeof(subsys_ram));
	memset(submem_cgram, 0x00, sizeof(submem_cgram));
	memset(submem_console_av40, 0x00, sizeof(submem_console_av40));
	ram_protect = true;
# endif
#endif
#if defined(_FM77AV_VARIANTS)
	mode320 = false;
	apalette_index.d = 0;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = i & 0xf00;
		analog_palette_b[i] = i & 0x00f;
		calc_apalette(i);
	}
#endif
#if defined(_FM77AV_VARIANTS)
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
#endif	
	switch(config.cpu_type){
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
	is_cyclesteal = ((config.dipswitch & FM7_DIPSW_CYCLESTEAL) != 0) ? true : false;
	enter_display();
	nmi_event_id = -1;
	firq_mask = false;
	key_firq_req = false;	//firq_mask = true;
	frame_skip_count = 3;
}

void DISPLAY::release()
{
}

#define STATE_VERSION 2
void DISPLAY::save_state(FILEIO *state_fio)
{
  	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);

	{
		int i;
		state_fio->FputInt32_BE(clr_count);
		state_fio->FputBool(halt_flag);
		state_fio->FputInt32_BE(active_page);
		state_fio->FputBool(sub_busy);
		state_fio->FputBool(crt_flag);
		state_fio->FputBool(vram_wrote);
#if defined(_FM77AV_VARIANTS)
		state_fio->FputBool(vram_wrote_shadow);
		for(i = 0; i < 411; i++) state_fio->FputBool(vram_wrote_table[i]);
		for(i = 0; i < 411; i++) state_fio->FputBool(vram_draw_table[i]);
#endif		
		state_fio->FputBool(is_cyclesteal);
		
		state_fio->FputBool(clock_fast);
		
#if defined(_FM77AV_VARIANTS)
		state_fio->FputBool(subcpu_resetreq);
		state_fio->FputBool(power_on_reset);
#endif	
		state_fio->FputBool(cancel_request);
		state_fio->FputBool(key_firq_req);

		state_fio->FputInt32_BE(display_mode);
		state_fio->FputUint32_BE(prev_clock);

		state_fio->Fwrite(dpalette_data, sizeof(dpalette_data), 1);
		state_fio->FputUint8(multimode_accessmask);
		state_fio->FputUint8(multimode_dispmask);
		state_fio->FputUint32_BE(offset_point);
#if defined(_FM77AV_VARIANTS)
		state_fio->FputUint32_BE(offset_point_bank1);
		state_fio->FputUint32_BE(offset_point_bak);
		state_fio->FputUint32_BE(offset_point_bank1_bak);
#endif		
		for(i = 0; i < 2; i++) {
			state_fio->FputUint32_BE(tmp_offset_point[i].d);
			state_fio->FputBool(offset_changed[i]);
		}
		state_fio->FputBool(offset_77av);
		state_fio->FputBool(diag_load_subrom_c);
		
	
		state_fio->Fwrite(io_w_latch, sizeof(io_w_latch), 1);
		state_fio->Fwrite(console_ram, sizeof(console_ram), 1);
		state_fio->Fwrite(work_ram, sizeof(work_ram), 1);
		state_fio->Fwrite(shared_ram, sizeof(shared_ram), 1);
		state_fio->Fwrite(subsys_c, sizeof(subsys_c), 1);
		state_fio->Fwrite(gvram, sizeof(gvram), 1);
#if defined(_FM77AV_VARIANTS)
		state_fio->Fwrite(gvram_shadow, sizeof(gvram_shadow), 1);
#endif	
	
#if defined(_FM77_VARIANTS)
		state_fio->FputBool(kanjisub);
		state_fio->FputUint16_BE(kanjiaddr.w.l);
# if defined(_FM77L4)
		state_fio->FputBool(mode400line);
		state_fio->FputBool(stat_400linecard);
# endif
#elif defined(_FM77AV_VARIANTS)
		state_fio->FputBool(kanjisub);
		state_fio->FputUint16_BE(kanjiaddr.w.l);

		state_fio->FputBool(vblank);
		state_fio->FputBool(vsync);
		state_fio->FputBool(hblank);
		state_fio->FputInt32_BE(vblank_count);
		state_fio->FputUint32_BE(displine);
		
		state_fio->FputBool(mode320);
		state_fio->FputInt32_BE(display_page);
		state_fio->FputInt32_BE(cgrom_bank);
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
		state_fio->FputInt32_BE(vram_bank);
#endif	
	
		state_fio->FputUint8(subrom_bank);
		state_fio->FputUint8(subrom_bank_using);
	
		state_fio->FputBool(nmi_enable);
		state_fio->FputBool(use_alu);
		
		state_fio->FputUint8(apalette_index.b.l);
		state_fio->FputUint8(apalette_index.b.h);
		state_fio->Fwrite(analog_palette_r, sizeof(analog_palette_r), 1);
		state_fio->Fwrite(analog_palette_g, sizeof(analog_palette_g), 1);
		state_fio->Fwrite(analog_palette_b, sizeof(analog_palette_b), 1);
		

		state_fio->FputBool(diag_load_subrom_a);
		state_fio->FputBool(diag_load_subrom_b);
		state_fio->FputBool(diag_load_subrom_cg);
	
		state_fio->Fwrite(subsys_a, sizeof(subsys_a), 1);
		state_fio->Fwrite(subsys_b, sizeof(subsys_b), 1);
		state_fio->Fwrite(subsys_cg, sizeof(subsys_cg), 1);
		state_fio->Fwrite(submem_hidden, sizeof(submem_hidden), 1);
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		state_fio->FputBool(mode400line);
		state_fio->FputBool(mode256k);
		
		state_fio->FputBool(monitor_ram);
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		state_fio->FputUint16_BE(window_low);
		state_fio->FputUint16_BE(window_high);
		state_fio->FputUint16_BE(window_xbegin);
		state_fio->FputUint16_BE(window_xend);
		state_fio->FputBool(window_opened);
#  endif	
		state_fio->FputBool(kanji_level2);

		state_fio->FputUint8(vram_active_block);
		state_fio->FputUint8(vram_display_block);
		state_fio->FputUint8(console_ram_bank);
		state_fio->FputBool(ram_protect);
		
		state_fio->FputUint32_BE(cgram_bank);
		state_fio->Fwrite(subsys_ram, sizeof(subsys_ram), 1);
		state_fio->Fwrite(submem_cgram, sizeof(submem_cgram), 1);
		state_fio->Fwrite(submem_console_av40, sizeof(submem_console_av40), 1);
# endif
#endif
	}
	// V2
	{
		state_fio->FputInt32_BE(nmi_event_id);
#if defined(_FM77AV_VARIANTS)
		state_fio->FputInt32_BE(hblank_event_id);
		state_fio->FputInt32_BE(hdisp_event_id);
		state_fio->FputInt32_BE(vsync_event_id);
		state_fio->FputInt32_BE(vstart_event_id);
#endif
		state_fio->FputBool(firq_mask);
		state_fio->FputBool(vram_accessflag);
		state_fio->FputUint32_BE(frame_skip_count);
	}			
}

bool DISPLAY::load_state(FILEIO *state_fio)
{

  	uint32 version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) {
		return false;
	}
   
	if(version >= 1) {
		int addr;
		int i;
		clr_count = state_fio->FgetInt32_BE();
		halt_flag = state_fio->FgetBool();
		active_page = state_fio->FgetInt32_BE();
		sub_busy = state_fio->FgetBool();
		crt_flag = state_fio->FgetBool();
		vram_wrote = state_fio->FgetBool();
#if defined(_FM77AV_VARIANTS)
		vram_wrote_shadow = state_fio->FgetBool();
		for(i = 0; i < 411; i++) vram_wrote_table[i] = state_fio->FgetBool();
		for(i = 0; i < 411; i++) vram_draw_table[i] = state_fio->FgetBool();
#endif		
		is_cyclesteal = state_fio->FgetBool();
	
		clock_fast = state_fio->FgetBool();

#if defined(_FM77AV_VARIANTS)
		subcpu_resetreq = state_fio->FgetBool();
		power_on_reset = state_fio->FgetBool();
#endif		
		cancel_request = state_fio->FgetBool();
		key_firq_req = state_fio->FgetBool();

		display_mode = state_fio->FgetInt32_BE();
		prev_clock = state_fio->FgetUint32_BE();
	
		state_fio->Fread(dpalette_data, sizeof(dpalette_data), 1);
		for(addr = 0; addr < 8; addr++) set_dpalette(addr, dpalette_data[addr]);

		multimode_accessmask = state_fio->FgetUint8();
		multimode_dispmask = state_fio->FgetUint8();
		offset_point = state_fio->FgetUint32_BE();
#if defined(_FM77AV_VARIANTS)
		offset_point_bank1     = state_fio->FgetUint32_BE();
		offset_point_bak       = state_fio->FgetUint32_BE();
		offset_point_bank1_bak = state_fio->FgetUint32_BE();
#endif		
		for(i = 0; i < 2; i++) {
			tmp_offset_point[i].d = state_fio->FgetUint32_BE();
			offset_changed[i] = state_fio->FgetBool();
		}
		offset_77av = state_fio->FgetBool();
		diag_load_subrom_c = state_fio->FgetBool();
		
		state_fio->Fread(io_w_latch, sizeof(io_w_latch), 1);
		state_fio->Fread(console_ram, sizeof(console_ram), 1);
		state_fio->Fread(work_ram, sizeof(work_ram), 1);
		state_fio->Fread(shared_ram, sizeof(shared_ram), 1);
		state_fio->Fread(subsys_c, sizeof(subsys_c), 1);
		state_fio->Fread(gvram, sizeof(gvram), 1);
#if defined(_FM77AV_VARIANTS)
		state_fio->Fread(gvram_shadow, sizeof(gvram_shadow), 1);
#endif	
#if defined(_FM77_VARIANTS)
		kanjisub = state_fio->FgetBool();
		kanjiaddr.d = 0;
		kanjiaddr.w.l = state_fio->FgetUint16_BE();
# if defined(_FM77L4)		
		mode400line = state_fio->FgetBool();
		stat_400linecard = state_fio->FgetBool();
# endif		
#elif defined(_FM77AV_VARIANTS)
		kanjisub = state_fio->FgetBool();
		kanjiaddr.d = 0;
		kanjiaddr.w.l = state_fio->FgetUint16_BE();
		
		vblank = state_fio->FgetBool();
		vsync = state_fio->FgetBool();
		hblank = state_fio->FgetBool();
		vblank_count = state_fio->FgetInt32_BE();
		displine = state_fio->FgetUint32_BE();

		mode320 = state_fio->FgetBool();
		display_page = state_fio->FgetInt32_BE();
		cgrom_bank = state_fio->FgetInt32_BE();
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
		vram_bank = state_fio->FgetInt32_BE();
#endif		
	
		subrom_bank = state_fio->FgetUint8();
		subrom_bank_using = state_fio->FgetUint8();
	
		nmi_enable = state_fio->FgetBool();
		use_alu = state_fio->FgetBool();

		apalette_index.b.l = state_fio->FgetUint8();
		apalette_index.b.h = state_fio->FgetUint8();
	
		state_fio->Fread(analog_palette_r, sizeof(analog_palette_r), 1);
		state_fio->Fread(analog_palette_g, sizeof(analog_palette_g), 1);
		state_fio->Fread(analog_palette_b, sizeof(analog_palette_b), 1);
		for(i = 0; i < 4096; i++) calc_apalette(i);
		
		diag_load_subrom_a = state_fio->FgetBool();
		diag_load_subrom_b = state_fio->FgetBool();
		diag_load_subrom_cg = state_fio->FgetBool();
	
		state_fio->Fread(subsys_a, sizeof(subsys_a), 1);
		state_fio->Fread(subsys_b, sizeof(subsys_b), 1);
		state_fio->Fread(subsys_cg, sizeof(subsys_cg), 1);
		state_fio->Fread(submem_hidden, sizeof(submem_hidden), 1);
	   
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		mode400line = state_fio->FgetBool();
		mode256k = state_fio->FgetBool();

		monitor_ram = state_fio->FgetBool();
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		window_low = state_fio->FgetUint16_BE();
		window_high = state_fio->FgetUint16_BE();
		window_xbegin = state_fio->FgetUint16_BE();
		window_xend = state_fio->FgetUint16_BE();
		window_opened = state_fio->FgetBool();
# endif	
		kanji_level2 = state_fio->FgetBool();
		
		vram_active_block = state_fio->FgetUint8();
		vram_display_block = state_fio->FgetUint8();
		console_ram_bank = state_fio->FgetUint8();
		ram_protect = state_fio->FgetBool();

		cgram_bank = state_fio->FgetUint32_BE();
		state_fio->Fread(subsys_ram, sizeof(subsys_ram), 1);
		state_fio->Fread(submem_cgram, sizeof(submem_cgram), 1);
		state_fio->Fread(submem_console_av40, sizeof(submem_console_av40), 1);
# endif
#endif
		if(version == 1) return true;
	}
	if(version >= 2) {	//V2
		nmi_event_id = state_fio->FgetInt32_BE();
#if defined(_FM77AV_VARIANTS)
		hblank_event_id = state_fio->FgetInt32_BE();
		hdisp_event_id = state_fio->FgetInt32_BE();
		vsync_event_id = state_fio->FgetInt32_BE();
		vstart_event_id = state_fio->FgetInt32_BE();
#endif
		firq_mask = state_fio->FgetBool();
		vram_accessflag = state_fio->FgetBool();
		frame_skip_count = state_fio->FgetUint32_BE();
	}			
	return true;
}

	
