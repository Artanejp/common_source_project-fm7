/*
 * Common source code project -> FM-7 -> Display
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *  Feb 10, 2015 : Initial.
 */

#include "../vm.h"
#include "../../emu.h"
#include "../../fileio.h"
#include "fm7_display.h"
#if defined(_FM77AV_VARIANTS)
# include "mb61vh010.h"
#endif
#if defined(_FM77L4)
#include "../hd46505.h"
#endif

#include "fm7_mainio.h"
#include "./fm7_keyboard.h"
#include "./kanjirom.h"

DISPLAY::DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	ins_led = NULL;
	kana_led = NULL;
	caps_led = NULL;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	kanjiclass1 = NULL;
	kanjiclass2 = NULL;
#elif defined(_FM77_VARIANTS)
	kanjiclass1 = NULL;
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	kanjisub = false;	// fix by Ryu Takegami
#endif	
#if defined(_FM77AV_VARIANTS)
	alu = NULL;
#endif
	mainio = NULL;
	subcpu = NULL;
	keyboard = NULL;
	for(int i = 0; i < 256; i++) {
		uint16_t n = (uint16_t)i;
		for(int j = 0; j < 8; j++) {
			bit_trans_table_0[i][j] = n & 0x80;
			bit_trans_table_1[i][j] = ((n & 0x80) != 0) ? 0x40 : 0;
			bit_trans_table_2[i][j] = ((n & 0x80) != 0) ? 0x20 : 0;
			bit_trans_table_3[i][j] = ((n & 0x80) != 0) ? 0x10 : 0;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			bit_trans_table_4[i][j] = ((n & 0x80) != 0) ? 0x08 : 0;
			bit_trans_table_5[i][j] = ((n & 0x80) != 0) ? 0x04 : 0;
#endif			
			n <<= 1;
		}
	}
	displine = 0;
	active_page = 0;
#if defined(USE_GREEN_DISPLAY)
	use_green_monitor = false;
#endif
	force_update = true;
	set_device_name(_T("DISPLAY SUBSYSTEM"));
}

DISPLAY::~DISPLAY()
{

}

void DISPLAY::reset_some_devices()
{
	int i;
	double usec;
	call_write_signal(keyboard, SIG_FM7KEY_SET_INSLED, 0x00, 0x01);
	call_write_signal(mainio, SIG_FM7_SUB_HALT, 0x00, 0xff);
	sub_busy = true;
	
	palette_changed = true;
	multimode_accessmask = 0;
	multimode_dispmask = 0;
	for(i = 0; i < 4; i++) {
		multimode_accessflags[i] = ((multimode_accessmask & (1 << i)) != 0) ? true : false;
		multimode_dispflags[i] = ((multimode_dispmask & (1 << i)) != 0) ? true : false;
	}
	//firq_mask = false; // 20180215 Thanks to Ryu takegami.
	//cancel_request = false;
	is_cyclesteal = ((config.dipswitch & FM7_DIPSW_CYCLESTEAL) != 0) ? true : false;
	switch(config.cpu_type){
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
	if(clock_fast) {
		prev_clock = SUBCLOCK_NORMAL;
	} else {
		prev_clock = SUBCLOCK_SLOW;
	}
	enter_display();
   
	offset_point = 0;
	for(i = 0; i < 2; i++) {
		offset_changed[i] = true;
		tmp_offset_point[i].d = 0;
	}

	vram_wrote_shadow = true;
	for(i = 0; i < 411 * 5; i++) vram_wrote_table[i] = true;
	for(i = 0; i < 411; i++) vram_draw_table[i] = true;
	displine = 0;
	active_page = 0;
	
//#if defined(_FM77AV_VARIANTS) || defined(_FM77L4)
#if 1
	vsync = true;
	vblank = true;
	hblank = false;
	vblank_count = 0;

	if(hblank_event_id >= 0) cancel_event(this, hblank_event_id);
	if(hdisp_event_id >= 0) cancel_event(this, hdisp_event_id);

	if(vsync_event_id >= 0) cancel_event(this, vsync_event_id);
	if(vstart_event_id >= 0) cancel_event(this, vstart_event_id);
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
#endif
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
		usec = 0.33 * 1000.0; 
		vm->set_vm_frame_rate(55.40);
	} else {
		usec = 0.51 * 1000.0;
		vm->set_vm_frame_rate(FRAMES_PER_SEC);
	}
	//usec = 16.0;
	//register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
	call_write_signal(mainio, SIG_DISPLAY_DISPLAY, 0x00, 0xff);
	call_write_signal(mainio, SIG_DISPLAY_VSYNC, 0xff, 0xff);
//#endif
	display_page = 0;
	display_page_bak = 0;

#if defined(_FM77AV_VARIANTS)
	offset_77av = false;
	offset_point_bank1 = 0;
	
	subcpu_resetreq = false;
	subrom_bank_using = subrom_bank;
   
	nmi_enable = true;
	use_alu = false;

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
	call_write_signal(alu, SIG_ALU_X_WIDTH, (mode320 || mode256k) ? 40 : 80, 0xffff);
	call_write_signal(alu, SIG_ALU_Y_HEIGHT, (display_mode == DISPLAY_MODE_8_400L) ? 400: 200, 0xffff);
	call_write_signal(alu, SIG_ALU_400LINE, (display_mode == DISPLAY_MODE_8_400L) ? 0xffffffff : 0, 0xffffffff);
# else
	call_write_signal(alu, SIG_ALU_X_WIDTH, (mode320) ? 40 : 80, 0xffff);
	call_write_signal(alu, SIG_ALU_Y_HEIGHT, 200, 0xffff);
	call_write_signal(alu, SIG_ALU_400LINE, 0, 0xffffffff);
# endif
	call_write_signal(alu, SIG_ALU_MULTIPAGE, multimode_accessmask, 0x07);
	call_write_signal(alu, SIG_ALU_PLANES, 3, 3);
#endif
	for(i = 0; i < 8; i++) set_dpalette(i, i);
#if defined(USE_GREEN_DISPLAY)
	memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
#endif
	memcpy(dpalette_pixel, dpalette_pixel_tmp, sizeof(dpalette_pixel));
	//do_firq(!firq_mask && key_firq_req);

#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	//kanjisub = false; // Fixed by Ryu takegami
	kanjiaddr.d = 0x00000000;
# if defined(_FM77L4)
	mode400line = false;
	stat_400linecard = false;
# endif	
#endif
	vram_wrote = true;
	delay_busy = false;
	frame_skip_count_draw = 3;
	frame_skip_count_transfer = 3;
	need_transfer_line = true;
	setup_display_mode();

}


void DISPLAY::reset()
{
	int i;
	//printf("RESET\n");
	halt_flag = false;
	vram_accessflag = true;
	display_mode = DISPLAY_MODE_8_200L;
	//crt_flag = true;
	crt_flag = false; // Fixed by Ryu Takegami
	screen_update_flag = true;
	crt_flag_bak = false;
	cancel_request = false;
#if defined(_FM77AV_VARIANTS)
	mode320 = false;
	apalette_index.d = 0;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = (i & 0xf00) >> 4;
		analog_palette_b[i] = (i & 0x00f) << 4;
		calc_apalette(i);
		memcpy(analog_palette_pixel, analog_palette_pixel_tmp, sizeof(analog_palette_pixel));
	}
   	subrom_bank = 0;
	cgrom_bank = 0;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	monitor_ram = false;
	ram_protect = true;
	
	mode400line = false;
	mode256k = false;
# endif
#elif defined(_FM77L4)
	mode400line = false;
	stat_400linecard = false;
	
	workram_l4 = false;
	cursor_lsb = false;
    text_width40 = false;
	
	text_blink = true;
	cursor_blink = true;
	
	text_start_addr.d = 0x0000;
	text_lines = 64;
	text_xmax = 80;
	
	cursor_addr.d = 0;
	cursor_start = 0;
	cursor_end = 0;
	cursor_type = 0;
	text_scroll_count = 0;

	cursor_blink = true;
	{
		// OK?
		double usec;
		uint8_t *regs = l4crtc->get_regs();
		display_mode = DISPLAY_MODE_1_400L;
		if(event_id_l4_cursor_blink >= 0) {
			cancel_event(this, event_id_l4_cursor_blink);
		}
		if(event_id_l4_text_blink >= 0) {
			cancel_event(this, event_id_l4_text_blink);
		}
		event_id_l4_cursor_blink = -1;
		event_id_l4_text_blink = -1;
		if(regs != NULL) {
			usec = ((regs[10] & 0x20) == 0) ? 160.0 : 320.0;
			usec = usec * 1000.0;
			register_event(this, EVENT_FM7SUB_CURSOR_BLINK, true, usec, &event_id_l4_cursor_blink);
			usec = 160.0 * 1000.0;
			register_event(this, EVENT_FM7SUB_TEXT_BLINK, true, usec, &event_id_l4_cursor_blink);
		}						
	}
	//memset(crtc_regs, 0x00, sizeof(crtc_regs));
#endif

#if !defined(FIXED_FRAMEBUFFER_SIZE)
	emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
#else
	emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
#endif	
	emu->set_vm_screen_lines(200);
	
	reset_some_devices();
	
#if defined(_FM77AV_VARIANTS)
	power_on_reset = false;
	for(i = 0; i < 411 * 5; i++) vram_wrote_table[i] = false;
	nmi_enable = true;
#else
# if defined(_FM8)
	for(i = 0; i < 8; i++) set_dpalette(i, i);
# endif
#endif	
#if defined(USE_GREEN_DISPLAY) && defined(USE_MONITOR_TYPE)
	memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
	switch(config.monitor_type) {
	case FM7_MONITOR_GREEN:
		use_green_monitor = true;
		break;
	case FM7_MONITOR_STANDARD:
	default:
		use_green_monitor = false;
		break;
	}
#else
	//use_green_monitor = false;
#endif
	force_update = true;
	
	memcpy(dpalette_pixel, dpalette_pixel_tmp, sizeof(dpalette_pixel));
	//enter_display();
	
	if(nmi_event_id >= 0) cancel_event(this, nmi_event_id);
	register_event(this, EVENT_FM7SUB_DISPLAY_NMI, 20000.0, true, &nmi_event_id); // NEXT CYCLE_
	
	firq_mask = false;
	key_firq_req = false;
	do_firq(false);
	reset_subcpu(true);
}

void DISPLAY::reset_subcpu(bool _check_firq)
{
	call_write_signal(subcpu, SIG_CPU_HALTREQ, 0, 1);
	call_write_signal(subcpu, SIG_CPU_BUSREQ, 0, 1);
	subcpu->reset();
	if(_check_firq) {
		do_firq(!firq_mask && key_firq_req);
	}
}
void DISPLAY::setup_display_mode(void)
{
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(display_mode == DISPLAY_MODE_8_400L) {
		page_offset = 0x0000;
		pagemod_mask = 0x18000;
		page_mask = 0x7fff;
	} else if(display_mode == DISPLAY_MODE_256k) {
		if(active_page != 0) {
			page_offset = 0xc000;
		} else {
			page_offset = 0x0000;
		}
		pagemod_mask = 0xe000;
		page_mask = 0x1fff;
	} else if(display_mode == DISPLAY_MODE_4096) {
		if(active_page != 0) {
			page_offset = 0xc000;
		} else {
			page_offset = 0x0000;
		}
		pagemod_mask = 0xe000;
		page_mask = 0x1fff;
	} else { // 200Line
		if(active_page != 0) {
			page_offset = 0xc000;
		} else {
			page_offset = 0x0000;
		}
		pagemod_mask = 0xc000;
		page_mask = 0x3fff;
	}		
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) page_offset += 0x18000;
# endif		
#elif defined(_FM77AV_VARIANTS)
	if(mode320) {
		page_mask = 0x1fff;
		pagemod_mask = 0xe000;
	} else { // 640x200, 8colors
		page_mask = 0x3fff;
		pagemod_mask = 0xc000;
	}
	if(active_page != 0) {
		page_offset = 0xc000;
	} else {
		page_offset = 0x0000;
	}
#elif defined(_FM77L4)
	if(display_mode == DISPLAY_MODE_1_400L) {
		page_mask = 0x7fff;
		pagemod_mask = 0x0000;
		page_offset = 0x0000;
	} else { // 640x200, 8colors
		page_mask = 0x3fff;
		pagemod_mask = 0xc000;
		page_offset = 0x0000;
	}
	page_offset = 0x0000;
#else
	page_offset = 0x0000;
	pagemod_mask = 0xc000;
	page_mask = 0x3fff;
#endif
}

void DISPLAY::update_config()
{
	vram_wrote = true;

		
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
	call_write_signal(subcpu, SIG_CPU_IRQ, flag ? 1: 0, 1);
}

void DISPLAY::do_firq(bool flag)
{
	call_write_signal(subcpu, SIG_CPU_FIRQ, flag ? 1: 0, 1);
}

void DISPLAY::do_nmi(bool flag)
{
#if defined(_FM77AV_VARIANTS)
	if(!nmi_enable) flag = false;
#endif
	call_write_signal(subcpu, SIG_CPU_NMI, flag ? 1 : 0, 1);
}

void DISPLAY::set_multimode(uint8_t val)
{
#if !defined(_FM8)	
	multimode_accessmask = val & 0x07;
	multimode_dispmask = (val & 0x70) >> 4;
	for(int i = 0; i < 4; i++) {
		multimode_accessflags[i] = ((multimode_accessmask & (1 << i)) != 0) ? true : false;
		multimode_dispflags[i] = ((multimode_dispmask & (1 << i)) != 0) ? true : false;
	}
	vram_wrote = true;
# if defined(_FM77AV_VARIANTS)
	call_write_signal(alu, SIG_ALU_MULTIPAGE, multimode_accessmask, 0x07);
# endif
#endif	
}

uint8_t DISPLAY::get_multimode(void)
{
#if defined(_FM8)
	return 0xff;
#else
	uint8_t val;
	val = multimode_accessmask & 0x07;
	val |= ((multimode_dispmask << 4) & 0x70);
	val |= 0x80;
	return val;
#endif	
}

uint8_t DISPLAY::get_cpuaccessmask(void)
{
	return multimode_accessmask & 0x07;
}

void DISPLAY::set_dpalette(uint32_t addr, uint8_t val)
{
	scrntype_t r, g, b;
	addr &= 7;
	dpalette_data[addr] = val | 0xf8; //0b11111000;
	b =  ((val & 0x01) != 0x00)? 255 : 0x00;
	r =  ((val & 0x02) != 0x00)? 255 : 0x00;
	g =  ((val & 0x04) != 0x00)? 255 : 0x00;
	
	dpalette_pixel_tmp[addr] = RGB_COLOR(r, g, b);
#if defined(USE_GREEN_DISPLAY)
	static const scrntype_t colortable[8] = {0, 48, 70, 100, 140, 175, 202, 255};
	g = colortable[val & 0x07];
	b = r = ((val & 0x07) > 4) ? 48 : 0;
	dpalette_green_tmp[addr] = RGB_COLOR(r, g, b);
#endif	
	palette_changed = true;
}

uint8_t DISPLAY::get_dpalette(uint32_t addr)
{
#if defined(_FM8)
	return 0xff;
#else
	uint8_t data;
	addr = addr & 7;
	
	data = dpalette_data[addr];
	return data;
#endif
}

void DISPLAY::halt_subcpu(void)
{
	//call_write_signal(subcpu, SIG_CPU_BUSREQ, 0x01, 0x01);
	call_write_signal(subcpu, SIG_CPU_HALTREQ, 0x01, 0x01);
}

void DISPLAY::go_subcpu(void)
{
	call_write_signal(subcpu, SIG_CPU_HALTREQ, 0x00, 0x01);
}

void DISPLAY::enter_display(void)
{
	uint32_t subclock;
	
	if(clock_fast) {
		subclock = SUBCLOCK_NORMAL;
	} else {
		subclock = SUBCLOCK_SLOW;
	}
	if(!(is_cyclesteal) && (vram_accessflag)) {
		subclock = subclock / 3;
	}
	if(prev_clock != subclock) {
		vm->set_cpu_clock(subcpu, subclock);
	}
	prev_clock = subclock;
}


void DISPLAY::leave_display(void)
{
}

void DISPLAY::halt_subsystem(void)
{
	//halt_flag = false;
  	halt_subcpu();
}

void DISPLAY::restart_subsystem(void)
{
	//halt_flag = false;
#if defined(_FM77AV_VARIANTS)
	if(subcpu_resetreq) {
		//firq_mask = (mainio->read_signal(FM7_MAINIO_KEYBOARDIRQ_MASK) != 0) ? false : true;
		reset_some_devices();
		power_on_reset = true;
		reset_subcpu(true);
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
uint8_t DISPLAY::acknowledge_irq(void)
{
	cancel_request = false;
	do_irq(false);
	return 0xff;
}

//SUB:D403:R
uint8_t DISPLAY::beep(void)
{
	call_write_signal(mainio, FM7_MAINIO_BEEP, 0x01, 0x01);
	return 0xff; // True?
}


// SUB:D404 : R 
uint8_t DISPLAY::attention_irq(void)
{
	call_write_signal(mainio, FM7_MAINIO_SUB_ATTENTION, 0x01, 0x01);
	return 0xff;
}

// SUB:D405:W
void DISPLAY::set_cyclesteal(uint8_t val)
{
#if !defined(_FM8)
# if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)	
	vram_wrote = true;
	val &= 0x01;
	if(val == 0) {
		is_cyclesteal = true;
	} else {
		is_cyclesteal = false;
	}
   enter_display();
# endif
#endif
}

void DISPLAY::setup_400linemode(uint8_t val)
{
#if defined(_FM77L4)
	// 400Line board.
	cursor_lsb = ((val & 0x10) != 0);
	text_width40 = ((val & 0x08) != 0);
	workram_l4 = ((val & 0x04) != 0);
	bool tmpmode = ((val & 0x02) != 0);
	if(tmpmode != mode400line) {
		int oldmode = display_mode;
		mode400line = tmpmode;
		if(mode400line && stat_400linecard) {
			display_mode = DISPLAY_MODE_1_400L;
		} else {
			display_mode = DISPLAY_MODE_8_200L;
		}
		if(oldmode != display_mode) {
			scrntype_t *pp;
			if(display_mode == DISPLAY_MODE_1_400L) {
				emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
				for(int y = 0; y < 400; y++) {
					pp = emu->get_screen_buffer(y);
					if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
				}
			} else { // 200Line
				
				vm->set_vm_frame_rate(FRAMES_PER_SEC);
#if !defined(FIXED_FRAMEBUFFER_SIZE)
				emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
				for(int y = 0; y < 200; y++) {
					pp = emu->get_screen_buffer(y);
					if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
				}
#else
				for(int y = 0; y < 400; y++) {
					pp = emu->get_screen_buffer(y);
					if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
				}
#endif
			}
			vram_wrote = true;
			setup_display_mode();
			
		}
	}
#endif	
}

//SUB:D409:R
uint8_t DISPLAY::set_vramaccess(void)
{
	vram_accessflag = true;
	//enter_display();
	return 0xff;
}

//SUB:D409:W
void DISPLAY::reset_vramaccess(void)
{
	vram_accessflag = false;
	//enter_display();
}

//SUB:D40A:R
uint8_t DISPLAY::reset_subbusy(void)
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
void DISPLAY::alu_write_cmdreg(uint32_t val)
{
	call_write_data8(alu, ALU_CMDREG, val);
	if((val & 0x80) != 0) {
		use_alu = true;
	} else {
		use_alu = false;
	}
}

// D411
void DISPLAY::alu_write_logical_color(uint8_t val)
{
	uint32_t data = (uint32_t)val;
	call_write_data8(alu, ALU_LOGICAL_COLOR, data);
}

// D412
void DISPLAY::alu_write_mask_reg(uint8_t val)
{
	uint32_t data = (uint32_t)val;
	call_write_data8(alu, ALU_WRITE_MASKREG, data);
}

// D413 - D41A
void DISPLAY::alu_write_cmpdata_reg(int addr, uint8_t val)
{
	uint32_t data = (uint32_t)val;
	addr = addr & 7;
	call_write_data8(alu, ALU_CMPDATA_REG + addr, data);
}

// D41B
void DISPLAY::alu_write_disable_reg(uint8_t val)
{
	uint32_t data = (uint32_t)val;
	call_write_data8(alu, ALU_BANK_DISABLE, data);
}

// D41C - D41F
void DISPLAY::alu_write_tilepaint_data(uint32_t addr, uint8_t val)
{
	uint32_t data = (uint32_t)val;
	switch(addr & 3) {
		case 0: // $D41C
			call_write_data8(alu, ALU_TILEPAINT_B, data);
			break;
		case 1: // $D41D
			call_write_data8(alu, ALU_TILEPAINT_R, data);
			break;
		case 2: // $D41E
			call_write_data8(alu, ALU_TILEPAINT_G, data);
			break;
		case 3: // xxxx
			//call_write_data8(alu, ALU_TILEPAINT_L, 0xff);
			break;
	}
}

// D420
void DISPLAY::alu_write_offsetreg_hi(uint8_t val)
{
	call_write_data8(alu, ALU_OFFSET_REG_HIGH, val & 0x7f);
}
 
// D421
void DISPLAY::alu_write_offsetreg_lo(uint8_t val)
{
	call_write_data8(alu, ALU_OFFSET_REG_LO, val);
}

// D422
void DISPLAY::alu_write_linepattern_hi(uint8_t val)
{
	call_write_data8(alu, ALU_LINEPATTERN_REG_HIGH, val);
}

// D423
void DISPLAY::alu_write_linepattern_lo(uint8_t val)
{
	call_write_data8(alu, ALU_LINEPATTERN_REG_LO, val);
}

// D424-D42B
void DISPLAY::alu_write_line_position(int addr, uint8_t val)
{
	uint32_t data = (uint32_t)val;
	switch(addr) {
		case 0:  
			call_write_data8(alu, ALU_LINEPOS_START_X_HIGH, data & 0x03); 
			break;
		case 1:  
			call_write_data8(alu, ALU_LINEPOS_START_X_LOW, data); 
			break;
		case 2:  
			call_write_data8(alu, ALU_LINEPOS_START_Y_HIGH, data & 0x01); 
			break;
		case 3:  
			call_write_data8(alu, ALU_LINEPOS_START_Y_LOW, data); 
			break;
		case 4:  
			call_write_data8(alu, ALU_LINEPOS_END_X_HIGH, data & 0x03); 
			break;
		case 5:  
			call_write_data8(alu, ALU_LINEPOS_END_X_LOW, data); 
			break;
  		case 6:  
			call_write_data8(alu, ALU_LINEPOS_END_Y_HIGH, data & 0x01); 
			break;
		case 7:  
			call_write_data8(alu, ALU_LINEPOS_END_Y_LOW, data);
			break;
	}
}

//SUB:D430:R
uint8_t DISPLAY::get_miscreg(void)
{
	uint8_t ret;

	ret = 0x6a;
	if(!hblank) ret |= 0x80;
	if(vsync) ret |= 0x04;
	if(alu->read_signal(SIG_ALU_BUSYSTAT) == 0) ret |= 0x10;
	if(power_on_reset) ret |= 0x01;
	return ret;
}

//SUB:D430:W
void DISPLAY::set_miscreg(uint8_t val)
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
	setup_display_mode();
}

// Main: FD13
void DISPLAY::set_monitor_bank(uint8_t var)
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
		//firq_mask = (mainio->read_signal(FM7_MAINIO_KEYBOARDIRQ_MASK) != 0) ? false : true;
		reset_some_devices();
		reset_subcpu(true);
	} else {
	  	subcpu_resetreq = true;
	}
}


// FD30
void DISPLAY::set_apalette_index_hi(uint8_t val)
{
	apalette_index.b.h = val & 0x0f;
}

// FD31
void DISPLAY::set_apalette_index_lo(uint8_t val)
{
	apalette_index.b.l = val;
}

void DISPLAY::calc_apalette(uint16_t idx)
{
	uint8_t r, g, b;
	idx = idx & 4095;
	g = analog_palette_g[idx];
	r = analog_palette_r[idx];
	b = analog_palette_b[idx];
	if(g != 0) g |= 0x0f; 
	if(r != 0) r |= 0x0f; 
	if(b != 0) b |= 0x0f; 
	analog_palette_pixel_tmp[idx] = RGB_COLOR(r, g, b);
}

// FD32
void DISPLAY::set_apalette_b(uint8_t val)
{
	uint16_t index;
	uint8_t tmp;
	index = apalette_index.w.l;
	tmp = (val & 0x0f) << 4;
	if(analog_palette_b[index] != tmp) {
		analog_palette_b[index] = tmp;
		calc_apalette(index);
		palette_changed = true;
	}
}

// FD33
void DISPLAY::set_apalette_r(uint8_t val)
{
	uint16_t index;
	uint8_t tmp;
	index = apalette_index.w.l;
	tmp = (val & 0x0f) << 4;
	if(analog_palette_r[index] != tmp) {
		analog_palette_r[index] = tmp;
		calc_apalette(index);
		palette_changed = true;
	}
}

// FD34
void DISPLAY::set_apalette_g(uint8_t val)
{
	uint16_t index;
	uint8_t tmp;
	index = apalette_index.w.l;
	tmp = (val & 0x0f) << 4;
	if(analog_palette_g[index] != tmp) {
		analog_palette_g[index] = tmp;
		calc_apalette(index);
		palette_changed = true;
	}
}

#endif // _FM77AV_VARIANTS


void DISPLAY::copy_vram_blank_area(void)
{
}

void DISPLAY::copy_vram_per_line(int begin, int end)
{
	uint32_t src_offset;
	uint32_t yoff_d1 = 0;
	uint32_t yoff_d2 = 0;
	uint32_t yoff_d;
	uint32_t poff = 0;
	uint32_t src_base;
	int pages = 1;
	uint32_t src_offset_d1;
	uint32_t src_offset_d2;
	uint32_t src_offset_d;
	uint32_t bytes_d1;
	uint32_t bytes_d2;
	uint32_t bytes_d;

	uint32_t addr_d1, addr_d2;
	int sectors;
	
	int i, j, k;
	//int dline = (int)displine - 1;
	int dline = (int)displine;

	if((begin  < 0) || (begin > 4)) return;
	if((end  < 0) || (end > 4)) return;
	if(begin > end) return;
	if(dline < 0) return;
	
	sectors = end - begin + 1;
	
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)){
		if(dline >= 400) return;
	} else {
		if(dline >= 200) return;
	}
#if defined(_FM77AV_VARIANTS)
	yoff_d1 = offset_point;
	yoff_d2 = offset_point_bank1;
	if(display_mode == DISPLAY_MODE_4096) {
		src_offset = dline * 40 + begin * 8;
		sectors = sectors * 8;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		pages = 2;
#endif
		addr_d1 = (src_offset + yoff_d1) & 0x1fff;
		addr_d2 = (src_offset + yoff_d2) & 0x1fff;
		bytes_d1 = 0x2000 - addr_d1;
		bytes_d2 = 0x2000 - addr_d2;
		for(k = 0; k < pages; k++) {
			src_base = 0;
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 2; j++) {
					uint32_t _addr_base = src_base + src_offset + poff;
					if(bytes_d1 < sectors) {
						my_memcpy(&gvram_shadow[_addr_base],
							   &gvram[addr_d1 + src_base + poff],
							   bytes_d1);
						my_memcpy(&gvram_shadow[_addr_base + bytes_d1],
							   &gvram[src_base + poff],
							   sectors - bytes_d1);
					} else {
						my_memcpy(&gvram_shadow[_addr_base],
							   &gvram[addr_d1 + src_base + poff],
							   sectors);
					}
					_addr_base += 0xc000;
					if(bytes_d2 < sectors) {
						my_memcpy(&gvram_shadow[_addr_base],
							   &gvram[addr_d2 + src_base + poff + 0xc000],
							   bytes_d2);
						my_memcpy(&gvram_shadow[_addr_base + bytes_d2],
							   &gvram[src_base + poff + 0xc000],
							   sectors - bytes_d2);
					} else {
						my_memcpy(&gvram_shadow[_addr_base],
							   &gvram[addr_d2 + src_base + poff + 0xc000],
							   sectors);
					}
					src_base += 0x2000;
				}
				src_base = (i + 1) * 0x4000;
			}
			poff += 0x18000;
		}
		vram_draw_table[dline] = true;
		for(int ii = begin; ii <= end; ii++) vram_wrote_table[(dline * 5) + ii] = false;
	}
# if defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77AV40)
	else if(display_mode == DISPLAY_MODE_256k) {
		src_offset = dline * 40 + begin * 8;
		sectors = sectors * 8;
		
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		pages = 4;
#elif defined(_FM77AV40)
		pages = 3;
#else
		pages = 0;
#endif
		src_offset_d1 = (src_offset + yoff_d1) & 0x1fff;
		src_offset_d2 = (src_offset + yoff_d2) & 0x1fff;
		bytes_d1 = 0x2000 - ((src_offset + yoff_d1) & 0x1fff);
		bytes_d2 = 0x2000 - ((src_offset + yoff_d2) & 0x1fff);
		for(k = 0; k < pages; k++) {
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 2; j++) {
					if((k & 1) == 0) {
						src_base = i * 0x4000 + j * 0x2000 + k *  0xc000;
						src_offset_d = src_offset_d1;
						bytes_d = bytes_d1;
					} else {
						src_base = i * 0x4000 + j * 0x2000 + k *  0xc000;
						src_offset_d = src_offset_d2;
						bytes_d = bytes_d2;
					}						
					if(bytes_d < sectors) {
						my_memcpy(&gvram_shadow[src_offset + src_base],
							   &gvram[src_offset_d + src_base],
							   bytes_d);
						my_memcpy(&gvram_shadow[src_offset + bytes_d + src_base],
							   &gvram[src_base],
							   sectors - bytes_d);
					} else {
						my_memcpy(&gvram_shadow[src_offset + src_base],
							   &gvram[src_offset_d + src_base],
							   sectors);
					}
				}
			}
		}
		vram_draw_table[dline] = true;
		for(int ii = begin; ii <= end; ii++) vram_wrote_table[(dline * 5) + ii] = false;
	}
	else if(display_mode == DISPLAY_MODE_8_400L) {
		src_offset = dline * 80 + begin * 16;
		sectors = sectors * 16;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		pages = 2;
#endif
		if(display_page_bak == 1) { // Is this dirty?
			yoff_d = yoff_d2;
		} else {
			yoff_d = yoff_d1;
		}
		yoff_d = (yoff_d << 1) & 0x7fff;
		src_offset_d = (src_offset + yoff_d) & 0x7fff;
		bytes_d = 0x8000 - ((src_offset + yoff_d) & 0x7fff);
		for(i = 0; i < pages; i++) {
			for(j = 0; j < 3; j++) {
				src_base = i * 0x18000 + j * 0x8000;
				if(bytes_d < sectors) {
					if(bytes_d > 0) {
						my_memcpy(&gvram_shadow[src_offset + src_base],
							   &gvram[src_offset_d + src_base],
							   bytes_d);
					}
					my_memcpy(&gvram_shadow[src_offset + bytes_d + src_base],
						   &gvram[src_base],
						   sectors - bytes_d);
				} else {
					my_memcpy(&gvram_shadow[src_offset + src_base + poff],
						   &gvram[src_offset_d + src_base],
						   sectors);
				}
			}
		}
		vram_draw_table[dline] = true;
		for(int ii = begin; ii <= end; ii++) vram_wrote_table[(dline * 5) + ii] = false;
	}
#endif	
	else { // 200line
		src_offset = dline * 80 + begin * 16;
		sectors = sectors * 16;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		pages = 4;
#elif defined(_FM77AV40)
		pages = 3;
#elif defined(_FM77AV_VARIANTS)
		pages = 2;
#else
		pages = 1;
#endif
		poff = 0;
		src_offset_d1 = (src_offset + yoff_d1) & 0x3fff;
		src_offset_d2 = (src_offset + yoff_d2) & 0x3fff;
		bytes_d1 = 0x4000 - ((src_offset + yoff_d1) & 0x3fff);
		bytes_d2 = 0x4000 - ((src_offset + yoff_d2) & 0x3fff);
		for(i = 0; i < pages; i++) {
			if((i & 1) == 0) {
				src_offset_d = src_offset_d1;
				bytes_d = bytes_d1;
			} else {
				src_offset_d = src_offset_d2;
				bytes_d = bytes_d2;
			}
			src_base = 0;
			for(j = 0; j < 3; j++) {
				if(bytes_d < sectors) {
					my_memcpy(&gvram_shadow[src_offset + src_base + poff],
						   &gvram[src_offset_d + src_base + poff],
						   bytes_d);
					my_memcpy(&gvram_shadow[src_offset + bytes_d + src_base + poff],
						   &gvram[src_base + poff],
						   sectors - bytes_d);
				} else {
					my_memcpy(&gvram_shadow[src_offset + src_base + poff],
						   &gvram[src_offset_d + src_base + poff],
						   sectors);
				}
				src_base += 0x4000;
			}
			poff += 0xc000;
		}
		vram_draw_table[dline] = true;
		for(int ii = begin; ii <= end; ii++) vram_wrote_table[(dline * 5) + ii] = false;
		//vram_wrote_table[dline] = false;
	}
#else // FM-8/7/77
#if defined(_FM77L4)
	if(display_mode == DISPLAY_MODE_1_400L) {
		src_offset = dline * 80 + begin * 16;
		sectors = sectors * 16;
		yoff_d = (yoff_d1 << 1) & 0x7fff;
		src_offset_d = (src_offset + yoff_d) & 0x7fff;
		bytes_d = 0x8000 - ((src_offset + yoff_d) & 0x7fff);
		if(bytes_d < sectors) {
			if(bytes_d > 0) {
				my_memcpy(&gvram_shadow[src_offset],
						  &gvram[src_offset_d],
						  bytes_d);
			}
			my_memcpy(&gvram_shadow[src_offset + bytes_d + src_base],
					  &gvram[0],
					  sectors - bytes_d);
		} else {
			my_memcpy(&gvram_shadow[src_offset +  poff],
					  &gvram[src_offset_d ],
					  sectors);
		}
		vram_draw_table[dline] = true;
		for(int ii = begin; ii <= end; ii++) vram_wrote_table[(dline * 5) + ii] = false;
		return;
	}
#endif
	{ // 200line
		src_offset = dline * 80 + begin * 16;
		sectors = sectors * 16;
		pages = 1;
		poff = 0;
		yoff_d = offset_point;
		src_offset_d = (src_offset + yoff_d) & 0x3fff;
		bytes_d = 0x4000 - ((src_offset + yoff_d) & 0x3fff);
		for(j = 0; j < 3; j++) {
			src_base = j * 0x4000;
			if(bytes_d < sectors) {
				my_memcpy(&gvram_shadow[src_offset + src_base + poff],
					   &gvram[src_offset_d + src_base + poff],
					   bytes_d);
				my_memcpy(&gvram_shadow[src_offset + bytes_d + src_base + poff],
					   &gvram[src_base + poff],
					   sectors - bytes_d);
			} else {
				my_memcpy(&gvram_shadow[src_offset + src_base + poff],
					   &gvram[src_offset_d + src_base + poff],
					   sectors);
			}
		}
		vram_draw_table[dline] = true;
		for(int ii = begin; ii <= end; ii++) vram_wrote_table[(dline * 5) + ii] = false;
		//vram_wrote_table[dline] = false;
	}

#endif
}

void DISPLAY::copy_vram_all()
{
#if defined(_FM77AV_VARIANTS)	
	uint32_t yoff_d1 = offset_point;
	uint32_t yoff_d2 = offset_point_bank1;
	uint32_t src_offset_1, src_offset_2;
	uint32_t poff = 0;
	if(display_mode == DISPLAY_MODE_4096) {
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		int pages = 2;
#else
		int pages = 1;
#endif
		uint32_t bytes_d1 = 0x2000 - (yoff_d1 & 0x1fff);
		uint32_t bytes_d2 = 0x2000 - (yoff_d2 & 0x1fff);
		for(int k = 0; k < pages; k++) {
			for(int i = 0; i < 3; i++) {
				for(int j = 0; j < 2; j++) {
					src_offset_1 = i * 0x4000 + j * 0x2000;
					src_offset_2 = src_offset_1 + 0xc000;
					my_memcpy(&gvram_shadow[src_offset_1 + poff], &gvram[src_offset_1 + (yoff_d1 & 0x1fff) + poff], bytes_d1);
					my_memcpy(&gvram_shadow[src_offset_1 + bytes_d1 + poff], &gvram[src_offset_1 + poff], 0x2000 - bytes_d1);
					my_memcpy(&gvram_shadow[src_offset_2 + poff], &gvram[src_offset_2 + (yoff_d2 & 0x1fff) + poff], bytes_d2);
					my_memcpy(&gvram_shadow[src_offset_2 + bytes_d2 + poff], &gvram[src_offset_2 + poff], 0x2000 - bytes_d2);
				}
			}
			poff += 0x18000;
		}
	}
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	else if(display_mode == DISPLAY_MODE_256k) {
		uint32_t bytes_d1 = 0x2000 - (yoff_d1 & 0x1fff);
		uint32_t bytes_d2 = 0x2000 - (yoff_d2 & 0x1fff);
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 2; j++) {
				src_offset_1 = i * 0x4000 + j * 0x2000;
				src_offset_2 = src_offset_1 + 0xc000;
				my_memcpy(&gvram_shadow[src_offset_1 + poff], &gvram[src_offset_1 + (yoff_d1 & 0x1fff) + poff], bytes_d1);
				my_memcpy(&gvram_shadow[src_offset_1 + bytes_d1 + poff], &gvram[src_offset_1 + poff], 0x2000 - bytes_d1);
				my_memcpy(&gvram_shadow[src_offset_2 + poff], &gvram[src_offset_2 + (yoff_d2 & 0x1fff) + poff], bytes_d2);
				my_memcpy(&gvram_shadow[src_offset_2 + bytes_d2 + poff], &gvram[src_offset_2 + poff], 0x2000 - bytes_d2);
			}
		}
		poff += 0x18000;
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 2; j++) {
				src_offset_1 = i * 0x4000 + j * 0x2000;
				my_memcpy(&gvram_shadow[src_offset_1 + poff], &gvram[src_offset_1 + (yoff_d1 & 0x1fff) + poff], bytes_d1);
				my_memcpy(&gvram_shadow[src_offset_1 + bytes_d1 + poff], &gvram[src_offset_1 + poff], 0x2000 - bytes_d1);
			}
		}
	} else if(display_mode == DISPLAY_MODE_8_400L) {
		int pages = 1;
		uint32_t yoff_d, bytes_d;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		pages = 2;
#endif
		if(display_page_bak == 1) { // Is this dirty?
			yoff_d = yoff_d2;
		} else {
			yoff_d = yoff_d1;
		}
		yoff_d = (yoff_d << 1) & 0x7fff;
		bytes_d = 0x8000 - yoff_d;
		for(int i = 0; i < pages; i++) {
			for(int j = 0; j < 3; j++) {
				uint32_t src_base = i * 0x18000 + j * 0x8000;
				my_memcpy(&gvram_shadow[src_base],
					   &gvram[yoff_d + src_base],
					   bytes_d);
				if(bytes_d < 0x8000) {
					my_memcpy(&gvram_shadow[bytes_d + src_base],
						   &gvram[src_base],
						   0x8000 - bytes_d);
				}
			}
		}
	}
#endif	
    else { // 200line
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		int pages = 4;
#elif defined(_FM77AV40)
		int pages = 3;
#else
		int pages = 2;
#endif
		uint32_t bytes_d1 = 0x4000 - (yoff_d1 & 0x3fff);
		uint32_t bytes_d2 = 0x4000 - (yoff_d2 & 0x3fff);
		uint32_t yoff_d, bytes_d;
		for(int k = 0; k < pages; k++) {
			yoff_d = ((k & 1) == 0) ? (yoff_d1 & 0x3fff) : (yoff_d2 & 0x3fff);
			bytes_d = ((k & 1) == 0) ? bytes_d1 : bytes_d2;
			for(int j = 0; j < 3; j++) {
				src_offset_1 = k * 0xc000 + j * 0x4000;
				my_memcpy(&gvram_shadow[src_offset_1], &gvram[src_offset_1 + yoff_d], bytes_d);
				my_memcpy(&gvram_shadow[src_offset_1 + bytes_d], &gvram[src_offset_1], 0x4000 - bytes_d);
			}
		}
	}
#else // FM-8/7/77
#  if defined(_FM77L4)
	if(display_mode == DISPLAY_MODE_1_400L) {
		uint32_t yoff_d = offset_point & 0x7fff;
		uint32_t bytes_d = 0x8000 - (offset_point & 0x7fff);
		my_memcpy(&gvram_shadow[0], &gvram[0 + yoff_d], bytes_d);
		my_memcpy(&gvram_shadow[0 + bytes_d], &gvram[0], 0x8000 - bytes_d);
		return;
	}
#  endif
    { // 200line
		uint32_t yoff_d = offset_point & 0x3fff;
		uint32_t bytes_d = 0x4000 - (offset_point & 0x3fff);
		uint32_t src_offset_1;
		for(int j = 0; j < 3; j++) {
			src_offset_1 = j * 0x4000;
			my_memcpy(&gvram_shadow[src_offset_1], &gvram[src_offset_1 + yoff_d], bytes_d);
			my_memcpy(&gvram_shadow[src_offset_1 + bytes_d], &gvram[src_offset_1], 0x4000 - bytes_d);
		}
	}
#endif
}

// Timing values from XM7 . Thanks Ryu.
//#if defined(_FM77AV_VARIANTS) || defined(_FM77L4)
void DISPLAY::event_callback_hdisp(void)
{
	bool f = false;
	double usec;
	hblank = false;
	call_write_signal(mainio, SIG_DISPLAY_DISPLAY, 0x02, 0xff);
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
		if(displine < 400) f = true;
	} else {
		if(displine < 200) f = true;
	}

	hdisp_event_id = -1;
	if(f) {
		// DO ONLY WHEN SYNC-TO-HSYNC.
		if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) != 0) {
			if(vram_wrote) {
				//copy_vram_per_line(0, 4);
			} else if(need_transfer_line) { // Not frame skip.
				int begin = -1;
				int end = -1;
				for(int iii = 0; iii < 5 ; iii++) {
					if(vram_wrote_table[iii + displine * 5]) {
						if(begin < 0) begin = iii; // Check first.
					} else {
						// Check end.
						if(begin >= 0) {
							end = iii - 1;
							if(end < begin) end = begin;
							// Do transfer.
							copy_vram_per_line(begin, end);
							// Prepare to next block.
							begin = -1;
							end = -1;
						}
					}
				}
				// Tail of this line.
				if(begin >= 0) {
					end = 4;
					copy_vram_per_line(begin, end);
				}
			}
		}
		if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
			register_event(this, EVENT_FM7SUB_HBLANK, 30.0, false, &hblank_event_id); // NEXT CYCLE_
		} else {
			register_event(this, EVENT_FM7SUB_HBLANK, 39.5, false, &hblank_event_id); // NEXT CYCLE_
		}
		vsync = false;
		vblank = false;
		enter_display();
	}
	f = false;	
}
void DISPLAY::event_callback_hblank(void)
{
	bool f = false;
	bool ff = false;
	double usec;
	
	hblank = true;
	hblank_event_id = -1;
	
	call_write_signal(mainio, SIG_DISPLAY_DISPLAY, 0x00, 0xff);
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
		if((displine < 400)) f = true;
		usec = 11.0;
	} else {
		if((displine < 200)) f = true;
		usec = 24.0;
	}
	if(f) {
		register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id);
	}
	displine++;
}

void DISPLAY::event_callback_vstart(void)
{
	double usec; 
	vblank = true;
	vsync = false;
	hblank = false;
	displine = 0;
	display_page_bak = display_page;
	
	// Parameter from XM7/VM/display.c , thanks, Ryu.
	call_write_signal(mainio, SIG_DISPLAY_DISPLAY, 0x00, 0xff);
	call_write_signal(mainio, SIG_DISPLAY_VSYNC, 0x00, 0xff);
	
	if(vblank_count != 0) {
		if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
			usec = (0.98 + 16.4) * 1000.0;
		} else {
			usec = (1.91 + 12.7) * 1000.0;
		}
		register_event(this, EVENT_FM7SUB_VSYNC, usec, false, &vsync_event_id);

		if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
			usec = 930.0; // 939.0
		} else {
			usec = 1840.0; // 1846.5
		}
		vstart_event_id = -1;
		register_event(this, EVENT_FM7SUB_HDISP, usec, false, &hdisp_event_id); // NEXT CYCLE_
		//register_event(this, EVENT_FM7SUB_HBLANK, usec, false, &hdisp_event_id); // NEXT CYCLE_
		vblank_count = 0;
	} else {
		if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
			usec = 0.34 * 1000.0;
		} else {
			usec = 1.52 * 1000.0;
		}
		vsync_event_id = -1;
		hblank_event_id = -1;
		hdisp_event_id = -1;
		register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
		vblank_count++;
	}
}
void DISPLAY::event_callback_vsync(void)
{
	double usec; 
	vblank = true;
	hblank = false;
	vsync = true;
	//write_access_page = (write_access_page + 1) & 1;
	//displine = 0;
	
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
		usec = 0.33 * 1000.0; 
	} else {
		usec = 0.51 * 1000.0;
	}
	call_write_signal(mainio, SIG_DISPLAY_VSYNC, 0x01, 0xff);
	call_write_signal(mainio, SIG_DISPLAY_DISPLAY, 0x00, 0xff);
	//register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_

	if(palette_changed) {
#if defined(_FM77AV_VARIANTS)
		memcpy(analog_palette_pixel, analog_palette_pixel_tmp, sizeof(analog_palette_pixel));
#endif
#if defined(USE_GREEN_DISPLAY)
		memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
#endif
		memcpy(dpalette_pixel, dpalette_pixel_tmp, sizeof(dpalette_pixel));
		vram_wrote_shadow = true;
		for(int yy = 0; yy < 400; yy++) {
			vram_draw_table[yy] = true;
		}
		palette_changed = false;
	}
	// Transfer on VSYNC
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) {
		bool ff = false;
		int lines = 200;
		
		if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L))lines = 400;
		if(need_transfer_line) {
			if(vram_wrote) { // transfer all line
				for(displine = 0; displine < lines; displine++) {
					//if(!vram_draw_table[displine]) {
					copy_vram_per_line(0, 4);
						//}
				}
				vram_wrote = false;
			} else { // transfer wrote line
				int begin = -1;
				int end = -1;
				for(displine = 0; displine < lines; displine++) {
					//if(!vram_draw_table[displine]) {
					for(int iii = 0; iii < 5 ; iii++) {
						if(vram_wrote_table[iii + displine * 5]) {
							if(begin < 0) {
								begin = iii;
							}
						} else {
							if(begin >= 0) {
								end = iii - 1;
								if(end < begin) end = begin;
								copy_vram_per_line(begin, end);
								begin = -1;
								end = -1;
							}
						}
					}
					if(begin >= 0) {
						if(end < 0) end = 4;
						copy_vram_per_line(begin, end);
					}
					begin = -1;
					end = -1;
				//}
				}
			}
		}
		for(int yy = 0; yy < lines; yy++) {
			if(vram_draw_table[yy]) {
				vram_wrote_shadow = true;
				screen_update_flag = true;
				break;
			}
		}
	} else {
		// TRANSFER per HSYNC a.k.a SYNC-TO-HSYNC.
		int lines = 200;
		if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) lines = 400;
		
		if(need_transfer_line) {
			if(vram_wrote) { // Transfer all line.
				for(int yy = 0; yy < lines; yy++) {
					displine = yy;
					copy_vram_per_line(0, 4);
				}
				//displine = 0;
				vram_wrote = false;
			}
		}
		for(int yy = 0; yy < lines; yy++) {
			if(vram_draw_table[yy]) {
				vram_wrote_shadow = true;
				screen_update_flag = true;
				break;
			}
		}
		//vram_wrote = false;
	}
	frame_skip_count_transfer++;
	{
		// Check frame skip for next frame.
		uint32_t factor = ((config.dipswitch & FM7_DIPSW_FRAMESKIP) >> 28) & 3;
		if((frame_skip_count_transfer > factor) /* || (vram_wrote) */) {
			frame_skip_count_transfer = 0;
			need_transfer_line = true;
		} else {
			need_transfer_line = false;
		}
	}
}

//#endif
#if defined(_FM77L4)
void DISPLAY::cursor_blink_77l4()
{
	if(!(mode400line && stat_400linecard)) return;
	uint8_t *regs = l4crtc->get_regs();
	uint32_t naddr;
	if(regs != NULL) {
		int x, y;
		if((regs[10] & 0x40) != 0) {
			cursor_blink = !cursor_blink;
			uint16_t addr = cursor_addr.w.l;
			if(text_width40) {
				x = ((addr / 2) % 40) / 8;
				y = (addr / 2) / 40;
			} else { // Width 80
				x = ((addr / 2) % 80) / 8;
				y = (addr / 2) / 80;
			}
			for(int yy = 0; yy < 8; yy++) {
				naddr = (y + yy) * 5 + x;
				vram_wrote_table[naddr] = true;
			}
		}
	}
}

void DISPLAY::text_blink_77l4()
{
	uint16_t addr;
	uint16_t offset = text_start_addr.w.l;
	uint32_t naddr;
	int x, y;
	if(!(mode400line && stat_400linecard)) return;
	text_blink = !text_blink;
	for(addr = 0; addr < (80 * 50); addr++) {
		naddr = ((addr + offset) & 0x0ffe) + 1;
		if((text_vram[naddr] & 0x10) != 0) { // ATTR BLINK
			if(text_width40) {
				x = ((naddr / 2) % 40) / 8;
				y = (naddr / 2) / 40;
			} else { // Width 80
				x = ((naddr / 2) % 80) / 8;
				y = (naddr / 2) / 80;
			}
			for(int yy = 0; yy < 8; yy++) {
				naddr = (y + yy) * 5 + x;
				vram_wrote_table[naddr] = true;
			}
		}
	}
}
#endif //#if defined(_FM77L4)

void DISPLAY::event_callback(int event_id, int err)
{
	double usec;
	bool f;
	switch(event_id) {
  		case EVENT_FM7SUB_DISPLAY_NMI: // per 20.00ms
#if defined(_FM77AV_VARIANTS)
			if(nmi_enable) {
				do_nmi(true);
			}
#else
			do_nmi(true);
#endif
			break;
  		case EVENT_FM7SUB_DISPLAY_NMI_OFF: // per 20.00ms
			do_nmi(false);
			break;
//#if defined(_FM77AV_VARIANTS) || defined(_FM77L4)
		case EVENT_FM7SUB_HDISP:
			event_callback_hdisp();
			break;
		case EVENT_FM7SUB_HBLANK:
			event_callback_hblank();
			break;
		case EVENT_FM7SUB_VSTART: // Call first.
			event_callback_vstart();
			break;
		case EVENT_FM7SUB_VSYNC:
			event_callback_vsync();
			break;
//#endif			
		case EVENT_FM7SUB_DELAY_BUSY:
			delay_busy = false;
			set_subbusy();
			break;
		case EVENT_FM7SUB_CLEAR_DELAY:
			delay_busy = false;
			break;
		case EVENT_FM7SUB_CLR_CRTFLAG:
			reset_crtflag();
			break;
#if defined(_FM77L4)
		case EVENT_FM7SUB_CURSOR_BLINK:
			cursor_blink_77l4();
			break;
		case EVENT_FM7SUB_TEXT_BLINK:
			text_blink_77l4();
			break;
#endif			
	}
}

void DISPLAY::event_frame()
{
#if 1
	double usec; 
	vblank = true;
	hblank = false;
	vsync = true;
	//write_access_page = (write_access_page + 1) & 1;
	//out_debug_log(_T("DISPLINE=%d"), displine);
	displine = 0;
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) {
		usec = 0.34 * 1000.0; 
	} else {
		usec = 1.52 * 1000.0;
	}
	call_write_signal(mainio, SIG_DISPLAY_VSYNC, 0x01, 0xff);
	call_write_signal(mainio, SIG_DISPLAY_DISPLAY, 0x00, 0xff);
	register_event(this, EVENT_FM7SUB_VSTART, usec, false, &vstart_event_id); // NEXT CYCLE_
	vblank_count = 1;
#endif
#if 0
#if !defined(_FM77AV_VARIANTS) && !defined(_FM77L4)
	int yy;
	bool f = false;
	int lines = 200;
	if((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) lines = 400;
#if 0	
	if(need_transfer_line && vram_wrote) {
		for(yy = 0; yy < lines; yy++) {
			//if(!vram_draw_table[yy]) {
				displine = yy;
				copy_vram_per_line(0, 4);
			//}
		}
		vram_wrote = false;
		displine = 0;
	}
#endif
	{
		for(yy = 0; yy < lines; yy++) {
			if(vram_draw_table[yy]) {
				f = true;
				break;
			}
		}
		if(f) {
			screen_update_flag = true;
			vram_wrote_shadow = true;
		}
	}
	enter_display();
	frame_skip_count_transfer++;
	{
		uint32_t factor = (config.dipswitch & FM7_DIPSW_FRAMESKIP) >> 28;
		if(frame_skip_count_transfer > factor) {
			frame_skip_count_transfer = 0;
			need_transfer_line = true;
		} else {
			need_transfer_line = false;
			displine = 0;
			//vram_wrote = false;
		}
	}
	
#endif
#endif
}

void DISPLAY::event_vline(int v, int clock)
{
#if !defined(_FM77AV_VARIANTS) && !defined(_FM77L4)
	bool ff = false;
	if(need_transfer_line == false) return;
	displine = v;
	if(vram_wrote) {
		// Not transfer, will transfer at event_frame.
		copy_vram_per_line(0, 4);
	} else {
		int begin = -1;
		int end = -1;
		for(int iii = 0; iii < 5 ; iii++) {
			if(vram_wrote_table[displine * 5 + iii]) {
				if(begin < 0) begin = iii;
			} else {
				if(begin >= 0) {
					end = iii - 1;
					if(end < begin) end = begin;
					copy_vram_per_line(begin, end);
					begin = -1;
					end = -1;
				}
			}
		}
		if(begin >= 0) {
			end = 4;
			copy_vram_per_line(begin, end);
		}
	}
	enter_display();
#endif	
}


uint32_t DISPLAY::read_signal(int id)
{
	uint32_t retval = 0;
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
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM77L4)
			retval = ((display_mode == DISPLAY_MODE_8_400L) || (display_mode == DISPLAY_MODE_1_400L)) ? 400 : 200;
#else
		  	retval = 200;
#endif		  
			break;
		case SIG_DISPLAY_EXTRA_MODE: // FD04 bit 4, 3
			retval = 0;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			retval |= (kanjisub) ? 0x00 : 0x20;
			retval |= (mode256k) ? 0x10 : 0x00;
			retval |= (mode400line) ? 0x00 : 0x08;
			retval |= (ram_protect) ? 0x00 : 0x04;
#elif defined(_FM77_VARIANTS)
			retval = 0x04;
			retval |= (kanjisub) ? 0x00 : 0x20;
#  if defined(_FM77L4)
			retval |= (stat_400linecard) ? 0x00 : 0x08;
#  else
			retval |= 0x18;
#  endif
#else
			retval = 0x2c;
#endif
			break;
		case SIG_DISPLAY_X_WIDTH:
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			retval = (mode320 || mode256k) ? 320 : 640;
#elif defined(_FM77AV_VARIANTS)
			retval = (mode320) ? 320 : 640;
#else
		  	retval = 640;
#endif		  
			break;
		default:
			break;
	}
	return retval;
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool flag = ((data & mask) != 0);
	bool oldflag;
	int y;
	switch(id) {
		case SIG_FM7_SUB_HALT:
			if(flag) {
				sub_busy = true;
			}
			halt_flag = flag;
			//call_write_signal(mainio, SIG_FM7_SUB_HALT, data, mask);
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
			clock_fast = flag;
			enter_display();
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_FM7_SUB_BANK: // Main: FD13
			set_monitor_bank(data & 0xff);
			break;
#endif			
		case SIG_DISPLAY_EXTRA_MODE: // FD04 bit 4, 3
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			//printf("Wrote $FD04: %02x\n", data);
			{
				int oldmode = display_mode;
				int mode;
				kanjisub = ((data & 0x20) == 0) ? true : false;
				mode256k = (((data & 0x10) != 0) && ((data & 0x08) != 0) ) ? true : false;
				mode400line = ((data & 0x08) == 0) ? true : false;
				ram_protect = ((data & 0x04) == 0) ? true : false;
				if((mode400line) && !(mode320)) {
					display_mode = DISPLAY_MODE_8_400L;
				} else if(mode256k) {
					display_mode = DISPLAY_MODE_256k;
				} else {
					display_mode = (mode320) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
				}
				if(oldmode != display_mode) {
					scrntype_t *pp;
					if(mode320 || mode256k) {
						if(oldmode == DISPLAY_MODE_8_400L) vm->set_vm_frame_rate(FRAMES_PER_SEC);
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						emu->set_vm_screen_size(320, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(y = 0; y < 200; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 320 * sizeof(scrntype_t));
						}
#else
						for(y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#endif	
						//emu->set_vm_screen_lines(200);
					} else if(display_mode == DISPLAY_MODE_8_400L) {
						emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						if(oldmode != DISPLAY_MODE_8_400L) vm->set_vm_frame_rate(55.40);
						for(y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
					} else {
						if(oldmode == DISPLAY_MODE_8_400L) vm->set_vm_frame_rate(FRAMES_PER_SEC);
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(y = 0; y < 200; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#else
						for(y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#endif	
						//emu->set_vm_screen_lines(200);
					}
					vram_wrote = true;
					call_write_signal(alu, SIG_ALU_X_WIDTH, (mode320 || mode256k) ? 40 :  80, 0xffff);
					call_write_signal(alu, SIG_ALU_Y_HEIGHT, (display_mode == DISPLAY_MODE_8_400L) ? 400 : 200, 0xffff);
					call_write_signal(alu, SIG_ALU_400LINE, (display_mode == DISPLAY_MODE_8_400L) ? 0xff : 0x00, 0xff);
					frame_skip_count_draw = 3;
					frame_skip_count_transfer = 3;
					setup_display_mode();
				}
			}
#elif defined(_FM77_VARIANTS)
			{
				int oldmode = display_mode;
				kanjisub = ((data & 0x20) == 0) ? true : false;
# if defined(_FM77L4)				
				stat_400linecard = ((data & 0x08) != 0) ? false : true;
				if(mode400line && stat_400linecard) {
					double usec;
					uint8_t *regs = l4crtc->get_regs();
					display_mode = DISPLAY_MODE_1_400L;
					if(event_id_l4_cursor_blink >= 0) {
						cancel_event(this, event_id_l4_cursor_blink);
					}
					if(event_id_l4_text_blink >= 0) {
						cancel_event(this, event_id_l4_text_blink);
					}
					event_id_l4_cursor_blink = -1;
					event_id_l4_text_blink = -1;
					if(regs != NULL) {
						usec = ((regs[10] & 0x20) == 0) ? 160.0 : 320.0;
						usec = usec * 1000.0;
						register_event(this, EVENT_FM7SUB_CURSOR_BLINK, true, usec, &event_id_l4_cursor_blink);
						usec = 160.0 * 1000.0;
						register_event(this, EVENT_FM7SUB_TEXT_BLINK, true, usec, &event_id_l4_cursor_blink);
					}						
				} else {
					display_mode = DISPLAY_MODE_8_200L;
				}
				if(oldmode != display_mode) {
					scrntype_t *pp;
					if(display_mode == DISPLAY_MODE_1_400L) {
						emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(int y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
					} else { // 200Line
				
						vm->set_vm_frame_rate(FRAMES_PER_SEC);
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(int y = 0; y < 200; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#else
						for(int y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#endif
					}
					vram_wrote = true;
					setup_display_mode();
					
				}
# endif				
			}
#endif
			break;
#if defined(_FM77AV_VARIANTS)
		case SIG_DISPLAY_MODE320: // FD12 bit 6
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			{
				//printf("Wrote $FD12: %02x\n", data);
				int oldmode = display_mode;
				mode320 = flag;
				if(mode400line) {
					display_mode = DISPLAY_MODE_8_400L;
				} else if(mode256k) {
					display_mode = DISPLAY_MODE_256k;
				} else 	if(!(mode320) && !(mode256k)) {
					//display_mode = (mode400line) ? DISPLAY_MODE_8_400L : DISPLAY_MODE_8_200L;
					display_mode = DISPLAY_MODE_8_200L;
				} else {
					display_mode = (mode256k) ? DISPLAY_MODE_256k : DISPLAY_MODE_4096;
				}
				if(oldmode != display_mode) {
					scrntype_t *pp;
					if(mode320 || mode256k) {
						if(oldmode == DISPLAY_MODE_8_400L) vm->set_vm_frame_rate(FRAMES_PER_SEC);
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						emu->set_vm_screen_size(320, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(y = 0; y < 200; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 320 * sizeof(scrntype_t));
						}
#else
						for(y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#endif	
						//emu->set_vm_screen_lines(200);
					} else { // 200 lines, 8 colors.
						if(display_mode == DISPLAY_MODE_8_400L) {
							if(oldmode != DISPLAY_MODE_8_400L) vm->set_vm_frame_rate(55.40);
						} else {
							if(oldmode == DISPLAY_MODE_8_400L) vm->set_vm_frame_rate(FRAMES_PER_SEC);
						}
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						int ymax = 	(display_mode == DISPLAY_MODE_8_400L) ? 400 : 200;

						emu->set_vm_screen_size(640, ymax, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(y = 0; y < ymax; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#else
						for(y = 0; y < 400; y++) {
							pp = emu->get_screen_buffer(y);
							if(pp != NULL) memset(pp, 0x00, 640 * sizeof(scrntype_t));
						}
#endif	
						//emu->set_vm_screen_lines(200);
					}
					vram_wrote = true;
					call_write_signal(alu, SIG_ALU_X_WIDTH, (mode320) ? 40 :  80, 0xffff);
					call_write_signal(alu, SIG_ALU_Y_HEIGHT, 200, 0xffff);
					call_write_signal(alu, SIG_ALU_400LINE, 0x00, 0xff);
					setup_display_mode();
					//frame_skip_count = 3;
				}
			}
# else /* FM77AV/20/20EX */
			oldflag = mode320;
			mode320 = flag;
			if(oldflag != mode320) {
		
				if(mode320) {
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						emu->set_vm_screen_size(320, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(y = 0; y < 200; y++) memset(emu->get_screen_buffer(y), 0x00, 320 * sizeof(scrntype_t));
#else
						for(y = 0; y < 400; y++) memset(emu->get_screen_buffer(y), 0x00, 640 * sizeof(scrntype_t));
#endif	
						//emu->set_vm_screen_lines(200);
				} else {
#if !defined(FIXED_FRAMEBUFFER_SIZE)
						emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
						for(y = 0; y < 200; y++) memset(emu->get_screen_buffer(y), 0x00, 640 * sizeof(scrntype_t));
#else
						for(y = 0; y < 400; y++) memset(emu->get_screen_buffer(y), 0x00, 640 * sizeof(scrntype_t));
#endif	
						//emu->set_vm_screen_lines(200);

				}
				display_mode = (mode320 == true) ? DISPLAY_MODE_4096 : DISPLAY_MODE_8_200L;
				call_write_signal(alu, SIG_ALU_X_WIDTH, (mode320) ? 40 :  80, 0xffff);
				call_write_signal(alu, SIG_ALU_Y_HEIGHT, 200, 0xffff);
				call_write_signal(alu, SIG_ALU_400LINE, 0, 0xffffffff);
				vram_wrote = true;
				setup_display_mode();
			}
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
		default:
			break;
	}
}
   
/*
 * Vram accessing functions moved to vram.cpp .
 */

uint32_t DISPLAY::read_mmio(uint32_t addr)
{
	uint32_t retval = 0xff;
	uint32_t raddr;	
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
			retval = (call_read_data8(keyboard, 0x00) != 0) ? 0xff : 0x7f;
			break;
		case 0x01: // Read keyboard
			retval = call_read_data8(keyboard, 0x01) & 0xff;
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
				return (uint8_t)call_read_data8(kanjiclass2, KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff));
			}
# endif
			if(kanjiclass1 != NULL) retval = call_read_data8(kanjiclass1, KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff));
			break;
		case 0x07:
			if(!kanjisub) return 0xff;
# if !defined(_FM77_VARIANTS)
			if(kanji_level2) {
				return (uint8_t)call_read_data8(kanjiclass2, KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff) + 1);
			}
# endif
			if(kanjiclass1 != NULL) retval = call_read_data8(kanjiclass1, KANJIROM_DIRECTADDR + ((kanjiaddr.d << 1) & 0x1ffff) + 1);
			break;
#endif
		case 0x08:
			set_crtflag();
			break;
		case 0x09:
			retval = set_vramaccess();
			break;
		case 0x0a:
			{ // If use CLR insn to set busy flag, clear flag at first, then delay and set flag. 
				double usec;
				// Delay at least 3+ clocks (CLR <$0A )
				delay_busy = true;
				if(clock_fast) {
					usec = (4000.0 * 1000.0) / 2000000.0;
				} else {
					usec = (4000.0 * 1000.0) / 999000.0;
				}
				register_event(this, EVENT_FM7SUB_CLEAR_DELAY, usec, false, NULL); // NEXT CYCLE_
			}
			reset_subbusy();
			break;
#if defined(_FM77L4)
		case 0x0b:
			if(stat_400linecard) {
				retval = l4crtc->read_io8(0);
			}
			break;
		case 0x0c:
			if(stat_400linecard) {
				retval = l4crtc->read_io8(1);
				// Update parameters.
			}
			break;
#endif			
		case 0x0d:
			call_write_signal(keyboard, SIG_FM7KEY_SET_INSLED, 0x01, 0x01);
			break;
#if defined(_FM77AV_VARIANTS)
		// ALU
		case 0x10:
			retval = call_read_data8(alu, ALU_CMDREG);
			break;
		case 0x11:
			retval = call_read_data8(alu, ALU_LOGICAL_COLOR);
			break;
		case 0x12:
			retval = call_read_data8(alu, ALU_WRITE_MASKREG);
			break;
		case 0x13:
			retval = call_read_data8(alu, ALU_CMP_STATUS_REG);
			break;
		case 0x1b:
			retval = call_read_data8(alu, ALU_BANK_DISABLE);
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
			retval = call_read_data8(keyboard, 0x31);
			break;
		case 0x32:
			retval = call_read_data8(keyboard, 0x32);
			break;
#endif				
		default:
			break;
	}
	return (uint8_t)retval;
}

uint32_t DISPLAY::read_vram_data8(uint32_t addr)
{
	uint32_t offset;
	uint32_t vramaddr;
	uint32_t color = (addr >> 14) & 0x03;
#if defined(_FM77L4)
	if(mode400line) {
		if(addr < 0x8000) {
			offset = offset_point;
			if(workram_l4) {
				if(multimode_accessflags[2]) return 0xff;
				vramaddr = ((addr + offset) & 0x3fff) + 0x8000; 
				return gvram[vramaddr];
			}
		} else {
			if(addr < 0x9800) {
				return text_vram[addr & 0x0fff];
			} else if(addr < 0xc000) {
				return subsys_l4[addr - 0x9800];
			}
			return 0xff;
		}
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
# if !defined(_FM8)		
	//if((multimode_accessmask & (1 << color)) != 0) return 0xff;
	if(multimode_accessflags[color]) return 0xff;
# endif		
#if defined(_FM77AV_VARIANTS)
	if (active_page != 0) {
		offset = offset_point_bank1;
	} else {
		offset = offset_point;
	}
#else
	offset = offset_point;
#endif
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) {
		if(display_mode != DISPLAY_MODE_256k) offset = 0; // Don't scroll at BLOCK 1.
	}
# endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32_t pagemod;
			uint32_t page_offset_alt = 0;
			if(addr >= 0x8000) return 0xff;
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
			offset <<= 1;
			pagemod = 0x8000 * color;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(vram_active_block != 0) page_offset = 0x18000;
# endif		
			vramaddr = (((addr + offset) & 0x7fff) | pagemod) + page_offset_alt;
			return gvram[vramaddr];
		} else {
			if(mode256k) {
				uint32_t page_offset_alt;
#if defined(_FM77AV40)
				if(vram_bank < 3) {
					page_offset_alt = 0xc000 * (vram_bank & 0x03);
				} else {
					page_offset_alt = 0; // right?
				}
#else			
				page_offset_alt = 0xc000 * (vram_bank & 0x03);
#endif			
				vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset_alt;
				//page_mask = 0x1fff;
				//pagemod = addr & 0xe000;
			} else {
				vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			}				
			return gvram[vramaddr];
		}
#elif defined(_FM77AV_VARIANTS)
		{		
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			return gvram[vramaddr];
		}
#elif defined(_FM77L4) //_FM77L4
		{
			if(mode400line) {
				vramaddr = (addr + offset) & 0x7fff;
				return gvram[vramaddr];
			} else {
				vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
				return gvram[vramaddr];
			}
			return 0xff;
		}
#else // Others (77/7/8)
		vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
		return gvram[vramaddr];
#endif
}

void DISPLAY::write_dma_data8(uint32_t addr, uint32_t data)
{
	uint32_t raddr = (addr & 0xffff) >> 7;
	if(write_dma_func_table[raddr] != NULL) {
		(this->*write_dma_func_table[raddr])(addr, (uint8_t) data);
	}
}

void DISPLAY::write_vram_data8(uint32_t addr, uint8_t data)
{
	uint32_t offset;
	uint32_t color = (addr >> 14) & 0x03;
	uint32_t vramaddr;
	uint8_t tdata;
	
#if defined(_FM77AV_VARIANTS)
	if (active_page != 0) {
		offset = offset_point_bank1;
	} else {
		offset = offset_point;
	}
#else
	offset = offset_point;
#endif
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(vram_active_block != 0) offset = 0; // Don't scroll at BLOCK 1.
# endif

#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32_t pagemod;
			uint32_t page_offset_alt = 0;
			uint32_t naddr;
			if(addr >= 0x8000) {
				return;
			}
			color = vram_bank & 0x03;
			if(color > 2) color = 0;
			offset <<= 1;
			pagemod = 0x8000 * color;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(vram_active_block != 0) page_offset_alt = 0x18000;
# endif		
			vramaddr = (((addr + offset) & 0x7fff) | pagemod) + page_offset_alt;
			// Reduce data transfer.
			tdata = gvram[vramaddr];			
			if(tdata != data) {
				naddr = (addr & 0x7fff) >> 4;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
		} else 	if(display_mode == DISPLAY_MODE_256k) {
			uint32_t page_offset_alt;
			uint32_t naddr;
#if defined(_FM77AV40)
			if(vram_bank < 3) {
				page_offset_alt = 0xc000 * (vram_bank & 0x03);
			} else {
				page_offset_alt = 0; // right?
			}
#else			
			page_offset_alt = 0xc000 * (vram_bank & 0x03);
#endif
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset_alt;
			tdata = gvram[vramaddr];
			if(tdata != data) {
				naddr = (addr & page_mask) >> 3;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
			return;
		} else if(display_mode == DISPLAY_MODE_4096) {
			uint32_t naddr;
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			tdata = gvram[vramaddr];
			if(tdata != data) {
				naddr = (addr & page_mask) >> 3;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
		} else { // 200line
			uint32_t naddr;
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			tdata = gvram[vramaddr];
			if(tdata != data) {
				naddr = (addr & page_mask) >> 4;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
		}
#elif defined(_FM77AV_VARIANTS)
		if(display_mode == DISPLAY_MODE_4096) {
			uint32_t naddr;
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			tdata = gvram[vramaddr];
			if(tdata != data) {
				naddr = (addr & page_mask) >> 3;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
		} else { // 200line
			uint32_t naddr;
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			tdata = gvram[vramaddr];
			if(tdata != data) {
				naddr = (addr & page_mask) >> 4;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
		}
#elif defined(_FM77L4) //_FM77L4
		if(display_mode == DISPLAY_MODE_1_400L) {
			if(addr < 0x8000) {
				if(workram_l4) {
					//if(multimode_accessflags[2]) return;
					vramaddr = ((addr + offset) & 0x3fff) + 0x8000; 
					gvram[vramaddr] = data;
				} else {
					uint32_t naddr;
					vramaddr = (addr + offset) & 0x7fff;
					tdata = gvram[vramaddr];
					if(tdata != data) {
						naddr = (addr & 0x7fff) >> 4;
						gvram[vramaddr] = data;
						vram_wrote_table[naddr] = true;
					}
				}
				return;
			}
		} else {
			uint32_t naddr;
			vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
			tdata = gvram[vramaddr];
			if(tdata != data) {
				naddr = (addr & 0x3fff) >> 4;
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
		}
#else // Others (77/7/8)
	{
		uint32_t naddr;
		vramaddr = (((addr + offset) & page_mask) | (pagemod_mask & addr)) + page_offset;
		tdata = gvram[vramaddr];
		if(tdata != data) {
			naddr = (addr & 0x3fff) >> 4;
			gvram[vramaddr] = data;
			vram_wrote_table[naddr] = true;
		}
	}
#endif
}

uint32_t DISPLAY::read_cpu_vram_data8(uint32_t addr)
{
	uint8_t color;
#if defined(_FM77AV_VARIANTS)
	if(use_alu) {
		call_read_data8(alu, addr);
	}
#endif
	return read_vram_data8(addr);
}

uint32_t DISPLAY::read_dma_vram_data8(uint32_t addr)
{
	return read_vram_data8(addr);
}

void DISPLAY::init_read_table(void)
{
	uint32_t _at;
	for(_at = 0 ; _at < 0xc000; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_cpu_vram_data8;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_dma_vram_data8;
	}
	for(_at = 0xc000; _at < 0xd000; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_console_ram;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_console_ram;
	}
	for(_at = 0xd000; _at < 0xd380; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_work_ram;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_work_ram;
	}
	for(_at = 0xd380; _at < 0xd400; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_shared_ram;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_shared_ram;
	}
#if defined(_FM77AV_VARIANTS)
	for(_at = 0xd400; _at < 0xd500; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_mmio;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_mmio;
	}
	for(_at = 0xd500; _at < 0xd800; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_hidden_ram;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_hidden_ram;
	}
#else
	for(_at = 0xd400; _at < 0xd800; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_mmio;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_mmio;
	}
#endif
	for(_at = 0xd800; _at < 0xe000; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_cgrom;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_cgrom;
	}
	for(_at = 0xe000; _at < 0x10000; _at += 0x80) {
		read_cpu_func_table[_at >> 7] = &DISPLAY::read_subsys_monitor;
		read_dma_func_table[_at >> 7] = &DISPLAY::read_subsys_monitor;
	}
}

uint32_t DISPLAY::read_console_ram(uint32_t addr)
{
	uint32_t raddr = addr & 0xfff;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(monitor_ram) {
		if(console_ram_bank >= 1) {
			return submem_console_av40[((console_ram_bank - 1) << 12) + raddr];
		}
	}
#endif
	return console_ram[raddr];
}

uint32_t DISPLAY::read_work_ram(uint32_t addr)
{
	addr = addr & 0x3ff;
	return work_ram[addr];
}


uint32_t DISPLAY::read_shared_ram(uint32_t addr)
{
	addr = addr - 0xd380;
	return shared_ram[addr];
}

#if defined(_FM77AV_VARIANTS)
uint32_t DISPLAY::read_hidden_ram(uint32_t addr)
{
	if(addr >= 0xd500) {
		return submem_hidden[addr - 0xd500];
	}
	return 0xff;
}
#endif

uint32_t DISPLAY::read_cgrom(uint32_t addr)
{
#if defined(_FM77AV_VARIANTS)
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	if(monitor_ram) {
		return submem_cgram[cgram_bank * 0x0800 + (addr - 0xd800)]; //FIXME
	}
# endif		
	return subsys_cg[(addr - 0xd800) + cgrom_bank * 0x800];
#else
	return subsys_c[addr - 0xd800];
#endif
}

uint32_t DISPLAY::read_subsys_monitor(uint32_t addr)
{
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	if(monitor_ram) {
		return subsys_ram[addr - 0xe000];
	}
#endif
#if defined(_FM77AV_VARIANTS)
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
#elif defined(_FM77L4)
	if(mode400line) {
		return subsys_l4[addr - 0xb800];
	}
	return subsys_c[addr - 0xd800];
#else
	return subsys_c[addr - 0xd800];
#endif
}

uint32_t DISPLAY::read_dma_data8(uint32_t addr)
{
	uint32_t raddr = (addr & 0xffff) >> 7;
	if(read_dma_func_table[raddr] != NULL) {
		return (this->*read_dma_func_table[raddr])(addr);
	}
	return 0xff;
}

uint32_t DISPLAY::read_data8(uint32_t addr)
{
	uint32_t raddr = addr;
	uint32_t offset;
	if(addr < 0x10000) {
		raddr = (addr & 0xffff) >> 7;
		if(read_cpu_func_table[raddr] != NULL) {
			return (this->*read_cpu_func_table[raddr])(addr);
		}
		return 0xff;
	}
#if !defined(_FM8)	
	else if((addr >= FM7_SUBMEM_OFFSET_DPALETTE) && (addr < (FM7_SUBMEM_OFFSET_DPALETTE + 8))) {
		return dpalette_data[addr - FM7_SUBMEM_OFFSET_DPALETTE];
	}
#endif	
#if defined(_FM77AV_VARIANTS)
	// ACCESS VIA ALU.
	else if((addr >= DISPLAY_VRAM_DIRECT_ACCESS) && (addr < (DISPLAY_VRAM_DIRECT_ACCESS + 0x18000))) {
		addr = addr - DISPLAY_VRAM_DIRECT_ACCESS;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(vram_active_block != 0) {
			offset = 0;
		}
# endif		
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)		
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32_t page_offset_alt = 0;
			uint32_t color;
			uint32_t pagemod;
			color = (addr & 0x18000) >> 15;
			if(color > 2) color = 0;
			pagemod = 0x8000 * color;
			if (active_page != 0) {
				offset = offset_point_bank1 << 1;
			} else {
				offset = offset_point << 1;
			}
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(vram_active_block != 0) {
				page_offset_alt = 0x18000;
				offset = 0;
			}
# endif		
			return gvram[(((addr + offset) & 0x7fff) | pagemod) + page_offset_alt];
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

void DISPLAY::write_mmio(uint32_t addr, uint8_t data)
{
	uint8_t rval = 0;
	uint8_t active_block_old;
	pair32_t tmpvar;
	if(addr < 0xd400) return;
	
#if !defined(_FM77AV_VARIANTS)
	addr = (addr - 0xd400) & 0x000f;
#elif !defined(_FM77AV40SX) && !defined(_FM77AV40EX)
	addr = (addr - 0xd400) & 0x003f;
#else // FM77AV40EX || FM77AV40SX
	addr = (addr - 0xd400) & 0x00ff;
#endif
	io_w_latch[addr] = (uint8_t)data;
	switch(addr) {
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4)
		// FM77 SPECIFIED
		case 0x05:
			set_cyclesteal((uint8_t)data);
#  if defined(_FM77L4)
			setup_400linemode((uint8_t)data);
#  endif
			break;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
     defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) || defined(_FM77_VARIANTS) // _FM77L4
		// KANJI
		case 0x06:
			if(!kanjisub) return;
			kanjiaddr.w.h = 0x0000;
			kanjiaddr.b.h = (uint8_t) data;
			break;
		case 0x07:
			if(!kanjisub) return;
			kanjiaddr.w.h = 0x0000;
			kanjiaddr.b.l = (uint8_t)data;
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
			if(delay_busy) { // If WRITE after READ immediately.Perhaps use CLR insn.
				double usec;
				// Delay at least 5+ clocks (CLR $D40A or CLR (INDEX = $D40A))
				if(clock_fast) {
					usec = (6000.0 * 1000.0) / 2000000.0;
				} else {
					usec = (6000.0 * 1000.0) / 999000.0;
				}
				register_event(this, EVENT_FM7SUB_DELAY_BUSY, usec, false, NULL); // NEXT CYCLE_
				reset_subbusy();
				delay_busy = false;
			} else {
				set_subbusy();
			}
			break;
		// LED
#if defined(_FM77L4)
		case 0x0b:
			if(stat_400linecard) {
				l4crtc->write_io8(0, data & 0x1f);
			}
			break;
		case 0x0c:
			if(stat_400linecard) {
				l4crtc->write_io8(1, data);
				// Update parameters.
				uint8_t crtc_addr = l4crtc->read_io8(0);
				const uint8_t *regs = l4crtc->get_regs();
				switch(crtc_addr & 0x1f) {
				case 10:
				case 11:
					cursor_addr.w.l &= 0x0ffc;
					cursor_addr.w.l |= (((uint16_t)regs[14]) << 10);
					cursor_addr.w.l |= (((uint16_t)regs[15]) << 2);
					if(cursor_lsb) {
						cursor_addr.w.l += 2;
					}
					cursor_addr.w.l &= 0xfff;
					// Redraw request
					if((crtc_addr & 0x1f) == 10) {
						double usec;
						if(event_id_l4_cursor_blink >= 0) {
							cancel_event(this, event_id_l4_cursor_blink);
						}
						usec = ((data & 0x20) == 0) ? 160.0 : 320.0;
						usec = usec * 1000.0;
						register_event(this, EVENT_FM7SUB_CURSOR_BLINK, true, usec, &event_id_l4_cursor_blink);
					}
					break;
				case 12:
					text_start_addr.w.l &= 0x03fc;
					text_start_addr.w.l |= (((uint16_t)data & 3) << 10);
					text_scroll_count++;
					if((text_scroll_count & 1) == 0) {
						// Redraw request
					}
					break;
				case 13:
					text_start_addr.w.l &= 0xfc00;
					text_start_addr.w.l |= ((uint16_t)data << 2);
					text_scroll_count++;
					if((text_scroll_count & 1) == 0) {
						// Redraw request
					}
				case 14:
				case 15:
					text_scroll_count++;
					if((text_scroll_count & 1) == 0) {
						// Redraw request
						cursor_addr.w.l &= 0x0ffc;
						cursor_addr.w.l |= (((uint16_t)regs[14]) << 10);
						cursor_addr.w.l |= (((uint16_t)regs[15]) << 2);
						if(cursor_lsb) {
							cursor_addr.w.l += 2;
						}
						cursor_addr.w.l &= 0xfff;
					}
					break;
				default:
					break;
				}
					
			}
			break;
#endif			
		case 0x0d:
			call_write_signal(keyboard, SIG_FM7KEY_SET_INSLED, 0x00, 0x01);
			break;
		// OFFSET
		case 0x0e:
		case 0x0f:
			rval = (uint8_t)data;
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
			call_write_data8(keyboard, 0x31, data);
			break;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		case 0x33: //
			active_block_old = vram_active_block;
			vram_active_block = data & 0x01;
			if(vram_display_block != (((data & 0x10) != 0) ? 1 : 0)) vram_wrote = true;
			vram_display_block = ((data & 0x10) != 0) ? 1 : 0;
			if(vram_active_block != active_block_old) setup_display_mode();
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
			if(display_mode == DISPLAY_MODE_8_400L) {
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
			if(display_mode == DISPLAY_MODE_8_400L) {
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

void DISPLAY::init_write_table(void)
{
	uint32_t _at;
	for(_at = 0 ; _at < 0xc000; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_cpu_vram_data8;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_dma_vram_data8;
	}
	for(_at = 0xc000; _at < 0xd000; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_console_ram;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_console_ram;
	}
	for(_at = 0xd000; _at < 0xd380; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_work_ram;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_work_ram;
	}
	for(_at = 0xd380; _at < 0xd400; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_shared_ram;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_shared_ram;
	}
#if defined(_FM77AV_VARIANTS)
	for(_at = 0xd400; _at < 0xd500; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_mmio;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_mmio;
	}
	for(_at = 0xd500; _at < 0xd800; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_hidden_ram;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_hidden_ram;
	}
#else
	for(_at = 0xd400; _at < 0xd800; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_mmio;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_mmio;
	}
#endif
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	for(_at = 0xd800; _at < 0xe000; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_subsys_cgram;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_subsys_cgram;
	}
	for(_at = 0xe000; _at < 0x10000; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_subsys_ram;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_subsys_ram;
	}
#else
	for(_at = 0xd800; _at < 0x10000; _at += 0x80) {
		write_cpu_func_table[_at >> 7] = &DISPLAY::write_dummy;
		write_dma_func_table[_at >> 7] = &DISPLAY::write_dummy;
	}
#endif
}

void DISPLAY::write_console_ram(uint32_t addr, uint8_t data)
{
	uint32_t raddr = addr & 0xfff;
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
}

void DISPLAY::write_work_ram(uint32_t addr, uint8_t data)
{
	uint32_t raddr = addr & 0xfff;
	work_ram[raddr] = data;
	return;
}

void DISPLAY::write_shared_ram(uint32_t addr, uint8_t data)
{
	uint32_t raddr = addr & 0x7f;
	shared_ram[raddr] = data;
	return;
}

#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
void DISPLAY::write_subsys_cgram(uint32_t addr, uint8_t data)
{
	uint32_t raddr = addr - 0xd800;
	if(ram_protect) return;
	if(!monitor_ram) return;
	submem_cgram[cgram_bank * 0x0800 + raddr] = data; //FIXME
}

void DISPLAY::write_subsys_ram(uint32_t addr, uint8_t data)
{
	if(ram_protect) return;
	if(!monitor_ram) return;
	subsys_ram[addr - 0xe000] = data; //FIXME
}
#endif

#if defined(_FM77AV_VARIANTS)
void DISPLAY::write_hidden_ram(uint32_t addr, uint8_t data)
{
	submem_hidden[addr - 0xd500] = data;
	return;
}
#endif

void DISPLAY::write_cpu_vram_data8(uint32_t addr, uint8_t data)
{
	uint32_t color = (addr & 0xc000) >> 14;
#if defined(_FM77AV_VARIANTS)
	if(use_alu) {
		call_read_data8(alu, addr);
		return;
	}
#endif	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(display_mode == DISPLAY_MODE_8_400L) {
		color = vram_bank & 0x03;
		if(color > 2) color = 0;
	}
#endif
#if defined(_FM77L4)
	if(mode400line) {
		if(addr < 0x8000) {
			if(workram_l4) {
				if(!(multimode_accessflags[2])) write_vram_data8(addr & 0x7fff, data);
				return;
			}
		} else if(addr < 0x9800) {
			uint32_t naddr;
			uint32_t x, y;
			addr = addr & 0x0fff;
			if(text_vram[addr] != data) {
				text_vram[addr] = data;
				if(text_width40) {
					x = ((addr / 2) % 40) / 8;
					y = (addr / 2) / 40;
				} else { // Width 80
					x = ((addr / 2) % 80) / 8;
					y = (addr / 2) / 80;
				}
				for(int yy = 0; yy < 8; yy++) {
					naddr = (y + yy) * 5 + x;
					vram_wrote_table[naddr] = true;
				}
			}
			return;
		} else {
			return;
		}
	}
#endif
#if !defined(_FM8)
	//if((multimode_accessmask & (1 << color)) != 0) return;
	if(multimode_accessflags[color]) return;
#endif		
	write_vram_data8(addr & 0xffff, data);
}

void DISPLAY::write_dma_vram_data8(uint32_t addr, uint8_t data)
{
	uint32_t color = (addr & 0xc000) >> 14;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(display_mode == DISPLAY_MODE_8_400L) {
		color = vram_bank & 0x03;
		if(color > 2) color = 0;
	}
#endif
#if !defined(_FM8)
	//if((multimode_accessmask & (1 << color)) != 0) return;
	if(multimode_accessflags[color]) return;
#endif		
	write_vram_data8(addr & 0xffff, data);
}

void DISPLAY::write_dummy(uint32_t addr, uint8_t data)
{
}

void DISPLAY::write_data8(uint32_t addr, uint32_t data)
{
	uint32_t offset;
	uint32_t raddr;
	//uint32_t page_offset = 0x0000;
	uint8_t val8 = data & 0xff;
	uint32_t color = (addr & 0xc000) >> 14;
	uint8_t tdata;

	if(addr < 0x10000) {
		void (*_write_func)(uint32_t, uint32_t);
		raddr = (addr & 0xffff) >> 7;
		if(write_cpu_func_table[raddr] != NULL) {
			(this->*write_cpu_func_table[raddr])(addr, (uint8_t)data);
			return;
		}
		return;
	}
#if !defined(_FM8)	
	else if((addr >= FM7_SUBMEM_OFFSET_DPALETTE) && (addr < (FM7_SUBMEM_OFFSET_DPALETTE + 8))) {
		set_dpalette(addr - FM7_SUBMEM_OFFSET_DPALETTE, val8);
		return;
	}
#endif	
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
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		if(display_mode == DISPLAY_MODE_8_400L) {
			uint32_t vramaddr;
			uint32_t naddr;
			uint32_t page_offset_alt = 0;
			uint32_t pagemod;
			color = (addr & 0x18000) >> 15;
			if(color > 2) color = 0;
			if (active_page != 0) {
				offset = offset_point_bank1 << 1;
			} else {
				offset = offset_point << 1;
			}
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(vram_active_block != 0) {
				page_offset_alt = 0x18000;
				offset = 0;
			}
# endif		
			naddr = (addr & 0x7fff) >> 4;
			pagemod = 0x8000 * color;
			vramaddr = (((addr + offset) & 0x7fff) | pagemod) + page_offset_alt;
			tdata = gvram[vramaddr];
			if(tdata != (uint8_t)data) {
				gvram[vramaddr] = data;
				vram_wrote_table[naddr] = true;
			}
			return;
		}
		write_vram_data8(addr, data);
#else
		write_vram_data8(addr, data);
#endif
		//if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) == 0) vram_wrote = true;
	}
#endif
	return;
}	


uint32_t DISPLAY::read_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size)
{
	FILEIO fio;
	uint32_t blocks;
	const _TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = create_local_path(name);
	if(s == NULL) return 0;
  
	if(!fio.Fopen(s, FILEIO_READ_BINARY)) return 0;
	blocks = fio.Fread(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}


void DISPLAY::initialize()
{
	int i;

	memset(io_w_latch, 0xff, sizeof(io_w_latch));
	screen_update_flag = true;
	memset(gvram, 0x00, sizeof(gvram));
	vram_wrote_shadow = false;
	memset(gvram_shadow, 0x00, sizeof(gvram_shadow));
	for(i = 0; i < 411 * 5; i++) vram_wrote_table[i] = false;
	for(i = 0; i < 411; i++) vram_draw_table[i] = false;
	force_update = false;

	memset(console_ram, 0x00, sizeof(console_ram));
	memset(work_ram, 0x00, sizeof(work_ram));
	memset(shared_ram, 0x00, sizeof(shared_ram));
	memset(subsys_c, 0xff, sizeof(subsys_c));
	need_transfer_line = true;
	frame_skip_count_draw = 3;
	frame_skip_count_transfer = 3;
	
	diag_load_subrom_c = false;
#if defined(_FM8)	
	if(read_bios(_T(ROM_FM8_SUBSYSTEM), subsys_c, 0x2800) >= 0x2800) diag_load_subrom_c = true;
	this->out_debug_log(_T("SUBSYSTEM ROM READING : %s"), diag_load_subrom_c ? "OK" : "NG");
#else
	if(read_bios(_T(ROM_FM7_SUBSYSTEM_TYPE_C), subsys_c, 0x2800) >= 0x2800) diag_load_subrom_c = true;
	this->out_debug_log(_T("SUBSYSTEM ROM Type C READING : %s"), diag_load_subrom_c ? "OK" : "NG");
#endif
#if defined(_FM77AV_VARIANTS)
	memset(subsys_a, 0xff, sizeof(subsys_a));
	memset(subsys_b, 0xff, sizeof(subsys_b));
	memset(subsys_cg, 0xff, sizeof(subsys_cg));
	memset(submem_hidden, 0x00, sizeof(submem_hidden));
   
	diag_load_subrom_a = false;
   	if(read_bios(_T(ROM_FM7_SUBSYSTEM_TYPE_A), subsys_a, 0x2000) >= 0x2000) diag_load_subrom_a = true;
	this->out_debug_log(_T("SUBSYSTEM ROM Type A READING : %s"), diag_load_subrom_a ? "OK" : "NG");

	diag_load_subrom_b = false;
   	if(read_bios(_T(ROM_FM7_SUBSYSTEM_TYPE_B), subsys_b, 0x2000) >= 0x2000) diag_load_subrom_b = true;
	this->out_debug_log(_T("SUBSYSTEM ROM Type B READING : %s"), diag_load_subrom_b ? "OK" : "NG");

	diag_load_subrom_cg = false;
   	if(read_bios(_T(ROM_FM7_SUBSYSTEM_CG), subsys_cg, 0x2000) >= 0x2000) diag_load_subrom_cg = true;
	this->out_debug_log(_T("SUBSYSTEM CG ROM READING : %s"), diag_load_subrom_cg ? "OK" : "NG");
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	memset(subsys_ram, 0x00, sizeof(subsys_ram));
	memset(submem_cgram, 0x00, sizeof(submem_cgram));
	memset(submem_console_av40, 0x00, sizeof(submem_console_av40));
	ram_protect = true;
# endif
#endif
#if defined(_FM77L4)
	memset(subsys_cg_l4, 0xff, sizeof(subsys_cg_l4));
	memset(subsys_l4, 0xff, sizeof(subsys_l4));
	
	read_bios(_T(ROM_FM77_400LINE_ANKCG), subsys_cg_l4, sizeof(subsys_cg_l4));
	read_bios(_T(ROM_FM77_400LINE_SUBSYS), subsys_l4, sizeof(subsys_l4));
	memset(text_vram, 0x00, sizeof(text_vram));
	//memset(crtc_regs, 0x00, sizeof(crtc_regs));
	
	workram_l4 = false;
	cursor_lsb = false;
    text_width40 = false;
	
	text_blink = true;
	cursor_blink = true;
	
	text_start_addr.d = 0x0000;
	text_lines = 64;
	text_xmax = 80;
	
	cursor_addr.d = 0;
	cursor_start = 0;
	cursor_end = 15;
	cursor_type = 0;
	text_scroll_count = 0;
	
	event_id_l4_cursor_blink = -1;
	event_id_l4_text_blink = -1;
#endif
	
	init_read_table();
	init_write_table();
	
#if defined(_FM77AV_VARIANTS)
	mode320 = false;
	apalette_index.d = 0;
	for(i = 0; i < 4096; i++) {
		analog_palette_r[i] = i & 0x0f0;
		analog_palette_g[i] = (i & 0xf00) >> 4;
		analog_palette_b[i] = (i & 0x00f) << 4;
		calc_apalette(i);
		memcpy(analog_palette_pixel, analog_palette_pixel_tmp, sizeof(analog_palette_pixel));
	}
#endif
	for(i = 0; i < 8; i++) set_dpalette(i, i);
#if defined(USE_GREEN_DISPLAY)
	memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
	use_green_monitor = false;
#endif
	
	memcpy(dpalette_pixel, dpalette_pixel_tmp, sizeof(dpalette_pixel));
//#if defined(_FM77AV_VARIANTS)
	hblank_event_id = -1;
	hdisp_event_id = -1;
	vsync_event_id = -1;
	vstart_event_id = -1;
//#endif
#if defined(_FM8)
	clock_fast = false;
#else
	clock_fast = true;
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	is_cyclesteal = true;
#else
	is_cyclesteal = false;
#endif
	multimode_accessmask = multimode_dispmask = 0;
	for(i = 0; i < 4; i++) {
		multimode_accessflags[i] = ((multimode_accessmask & (1 << i)) != 0) ? true : false;
		multimode_dispflags[i] = ((multimode_dispmask & (1 << i)) != 0) ? true : false;
	}

	prev_clock = SUBCLOCK_NORMAL;
	//enter_display();
	nmi_event_id = -1;
	firq_mask = false;
	key_firq_req = false;	//firq_mask = true;
	frame_skip_count_transfer = 3;
	frame_skip_count_draw = 3;
#if !defined(FIXED_FRAMEBUFFER_SIZE)
	emu->set_vm_screen_size(640, 200, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
#else
	emu->set_vm_screen_size(640, 400, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH_ASPECT, WINDOW_HEIGHT_ASPECT);
#endif
	emu->set_vm_screen_lines(200);
#if defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	mode400line = false;
	mode256k = false;
#elif defined(_FM77L4)
	mode400line = false;
#endif
	
	palette_changed = true;
//#if !defined(_FM77AV_VARIANTS) && !defined(_FM77L4)
	//register_vline_event(this);
	register_frame_event(this);
//#endif	
	setup_display_mode();
}

void DISPLAY::release()
{
}

#define STATE_VERSION 12
bool DISPLAY::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	{
		int i;
		state_fio->StateValue(delay_busy);
		state_fio->StateValue(halt_flag);
		state_fio->StateValue(active_page);
		state_fio->StateValue(sub_busy);
		state_fio->StateValue(crt_flag);
		state_fio->StateValue(vram_wrote);
		state_fio->StateValue(is_cyclesteal);
		
		state_fio->StateValue(clock_fast);
		
#if defined(_FM77AV_VARIANTS)
		state_fio->StateValue(subcpu_resetreq);
		state_fio->StateValue(power_on_reset);
#endif	
		state_fio->StateValue(cancel_request);
		state_fio->StateValue(key_firq_req);

		state_fio->StateValue(display_mode);
		state_fio->StateValue(prev_clock);

#if !defined(_FM8)	
		state_fio->StateArray(dpalette_data, sizeof(dpalette_data), 1);
		state_fio->StateValue(multimode_accessmask);
		state_fio->StateValue(multimode_dispmask);
#endif		
		state_fio->StateValue(offset_point);
#if defined(_FM77AV_VARIANTS)
		state_fio->StateValue(offset_point_bank1);
#endif		
		//for(i = 0; i < 2; i++) {
		state_fio->StateArray(tmp_offset_point, sizeof(tmp_offset_point), 1);
		state_fio->StateArray(offset_changed, sizeof(offset_changed), 1);
			//}
		state_fio->StateValue(offset_77av);
		state_fio->StateValue(diag_load_subrom_c);
		
	
		state_fio->StateArray(io_w_latch, sizeof(io_w_latch), 1);
		state_fio->StateArray(console_ram, sizeof(console_ram), 1);
		state_fio->StateArray(work_ram, sizeof(work_ram), 1);
		state_fio->StateArray(shared_ram, sizeof(shared_ram), 1);
		state_fio->StateArray(subsys_c, sizeof(subsys_c), 1);
		state_fio->StateArray(gvram, sizeof(gvram), 1);
		state_fio->StateArray(gvram_shadow, sizeof(gvram_shadow), 1);
	
#if defined(_FM77_VARIANTS)
		state_fio->StateValue(kanjisub);
		state_fio->StateValue(kanjiaddr.d);
# if defined(_FM77L4)
		state_fio->StateValue(mode400line);
		state_fio->StateValue(stat_400linecard);
# endif
#elif defined(_FM77AV_VARIANTS)
		state_fio->StateValue(kanjisub);
		state_fio->StateValue(kanjiaddr.d);

		state_fio->StateValue(mode320);
		state_fio->StateValue(cgrom_bank);
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
		state_fio->StateValue(vram_bank);
#endif	
	
		state_fio->StateValue(displine);
		state_fio->StateValue(subrom_bank);
		state_fio->StateValue(subrom_bank_using);
	
		state_fio->StateValue(nmi_enable);
		state_fio->StateValue(use_alu);
		
		state_fio->StateValue(apalette_index.d);
		state_fio->StateArray(analog_palette_r, sizeof(analog_palette_r), 1);
		state_fio->StateArray(analog_palette_g, sizeof(analog_palette_g), 1);
		state_fio->StateArray(analog_palette_b, sizeof(analog_palette_b), 1);
		

		state_fio->StateValue(diag_load_subrom_a);
		state_fio->StateValue(diag_load_subrom_b);
		state_fio->StateValue(diag_load_subrom_cg);
	
		state_fio->StateArray(subsys_a, sizeof(subsys_a), 1);
		state_fio->StateArray(subsys_b, sizeof(subsys_b), 1);
		state_fio->StateArray(subsys_cg, sizeof(subsys_cg), 1);
		state_fio->StateArray(submem_hidden, sizeof(submem_hidden), 1);
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		state_fio->StateValue(mode400line);
		state_fio->StateValue(mode256k);
		
		state_fio->StateValue(monitor_ram);
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		state_fio->StateValue(window_low);
		state_fio->StateValue(window_high);
		state_fio->StateValue(window_xbegin);
		state_fio->StateValue(window_xend);
		state_fio->StateValue(window_opened);
#  endif	
		state_fio->StateValue(kanji_level2);

		state_fio->StateValue(vram_active_block);
		state_fio->StateValue(vram_display_block);
		state_fio->StateValue(console_ram_bank);
		state_fio->StateValue(ram_protect);
		
		state_fio->StateValue(cgram_bank);
		state_fio->StateArray(subsys_ram, sizeof(subsys_ram), 1);
		state_fio->StateArray(submem_cgram, sizeof(submem_cgram), 1);
		state_fio->StateArray(submem_console_av40, sizeof(submem_console_av40), 1);
# endif
#endif
	}
	// V2
	{
		state_fio->StateValue(nmi_event_id);
//#if defined(_FM77AV_VARIANTS)
		state_fio->StateValue(hblank_event_id);
		state_fio->StateValue(hdisp_event_id);
		state_fio->StateValue(vsync_event_id);
		state_fio->StateValue(vstart_event_id);
//#endif
		state_fio->StateValue(firq_mask);
		state_fio->StateValue(vram_accessflag);
		
		state_fio->StateValue(display_page);
		state_fio->StateValue(display_page_bak);
		
		state_fio->StateValue(vblank);
		state_fio->StateValue(vsync);
		state_fio->StateValue(hblank);
		state_fio->StateValue(vblank_count);
	}			
#if defined(_FM77L4)
	state_fio->StateArray(subsys_cg_l4, sizeof(subsys_cg_l4), 1);
	state_fio->StateArray(subsys_l4, sizeof(subsys_l4), 1);
	state_fio->StateArray(text_vram, sizeof(text_vram), 1);
	//state_fio->Fwrite(crtc_regs, sizeof(crtc_regs), 1);
	
	state_fio->StateValue(workram_l4);
	state_fio->StateValue(cursor_lsb);
	state_fio->StateValue(text_width40);
	
	state_fio->StateValue(text_blink);
	state_fio->StateValue(cursor_blink);
	
	state_fio->StateValue(text_start_addr.d);
	state_fio->StateValue(text_lines);
	state_fio->StateValue(text_xmax);
	
	state_fio->StateValue(cursor_addr.d);
	state_fio->StateValue(cursor_start);
	state_fio->StateValue(cursor_end);
	state_fio->StateValue(cursor_type);
	state_fio->StateValue(text_scroll_count);

	state_fio->StateValue(event_id_l4_cursor_blink);
	state_fio->StateValue(event_id_l4_text_blink);
#endif
	return true;
}	

void DISPLAY::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
}

bool DISPLAY::load_state(FILEIO *state_fio)
{
	bool mb = decl_state(state_fio, true);
	this->out_debug_log(_T("Load State: DISPLAY : id=%d stat=%s"), this_device_id, (mb) ? _T("OK") : _T("NG"));
	if(!mb) return false;
	
	{
		int addr;
		int i;
		crt_flag_bak = true;
		for(i = 0; i < 411 * 5; i++) vram_wrote_table[i] = true;
		for(i = 0; i < 411; i++) vram_draw_table[i] = true;
#if defined(_FM8)
		for(addr = 0; addr < 8; addr++) set_dpalette(addr, addr);
		memcpy(dpalette_pixel, dpalette_pixel_tmp, sizeof(dpalette_pixel));
  #if defined(USE_GREEN_DISPLAY)
		memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
  #endif
#else

		for(addr = 0; addr < 8; addr++) set_dpalette(addr, dpalette_data[addr]);
		memcpy(dpalette_pixel, dpalette_pixel_tmp, sizeof(dpalette_pixel));
#if defined(USE_GREEN_DISPLAY)
		memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
#endif
		for(i = 0; i < 4; i++) {
			multimode_accessflags[i] = ((multimode_accessmask & (1 << i)) != 0) ? true : false;
			multimode_dispflags[i] = ((multimode_dispmask & (1 << i)) != 0) ? true : false;
		}
#endif
#if defined(_FM77_VARIANTS)
# if defined(_FM77L4)		
# endif		
#elif defined(_FM77AV_VARIANTS)
#if defined(_FM77AV40) || defined(_FM77AV40SX)|| defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
#endif		
		screen_update_flag = true;
		for(i = 0; i < 4096; i++) calc_apalette(i);
		memcpy(analog_palette_pixel, analog_palette_pixel_tmp, sizeof(analog_palette_pixel));
		
#endif
		palette_changed = true;
		vram_wrote_shadow = true; // Force Draw
		this->draw_screen();
	}
		frame_skip_count_draw = 3;
		frame_skip_count_transfer = 3;
		need_transfer_line = true;
#if defined(USE_GREEN_DISPLAY) && defined(USE_MONITOR_TYPE)
	memcpy(dpalette_pixel_green, dpalette_green_tmp, sizeof(dpalette_pixel_green));
	switch(config.monitor_type) {
	case FM7_MONITOR_GREEN:
		use_green_monitor = true;
		break;
	case FM7_MONITOR_STANDARD:
	default:
		use_green_monitor = false;
		break;
	}
#else
	//use_green_monitor = false;
#endif
	force_update = true;
	setup_display_mode();
	return mb;
}

	
