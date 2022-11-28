/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.09-

	[ sub pcb ]
*/

#include "./sub.h"
#include "./main.h"
#include "../beep.h"
#include "../datarec.h"
#include "../hd46505.h"
#include "../upd7801.h"

#define EVENT_CLOCK	0

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 7, eb = (e) >> 7; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x80 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x80 * (i - sb); \
		} \
	} \
}

void SUB::initialize()
{
	// init memory
	memset(sub1, 0xff, sizeof(sub1));
	memset(sub2, 0xff, sizeof(sub2));
	memset(sub3, 0xff, sizeof(sub3));
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(ram, 0, sizeof(ram));
	memset(vram_b, 0, sizeof(vram_b));
	memset(vram_r, 0, sizeof(vram_r));
	memset(vram_g, 0, sizeof(vram_g));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("SUB1.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(sub1, sizeof(sub1), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("SUB2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(sub2, sizeof(sub2), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("SUB3.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(sub3, sizeof(sub3), 1);
		fio->Fclose();
	}
	delete fio;
	
	SET_BANK(0x0000, 0x0fff, wdmy, sub1);
	SET_BANK(0x1000, 0x1fff, wdmy, sub2);
	SET_BANK(0x2000, 0x5fff, vram_b, vram_b);
	SET_BANK(0x6000, 0x9fff, vram_r, vram_r);
	SET_BANK(0xa000, 0xdfff, vram_g, vram_g);
	SET_BANK(0xe000, 0xefff, wdmy, rdmy);	// I/O
	SET_BANK(0xf000, 0xff7f, wdmy, sub3);	// 0xf400-
	SET_BANK(0xff80, 0xffff, ram, ram);
	
	// create palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	
	key_stat = emu->get_key_buffer();
	register_frame_event(this);
	
	// serial i/o
	so = true;
	clock = 0;
	register_event(this, EVENT_CLOCK, 1000000.0 / 76800.0, true, NULL);
	
	// int cmt
	memset(&b16_1, 0, sizeof(b16_1));
	memset(&b16_2, 0, sizeof(b16_2));
	memset(&g21_1, 0, sizeof(g21_1));
	memset(&g21_2, 0, sizeof(g21_2));
	memset(&c15, 0, sizeof(c15));
	memset(&c16, 0, sizeof(c16));
	memset(&f21, 0, sizeof(f21));
	
	b16_1.in_s = b16_1.in_r = true;
	b16_2.in_s = b16_2.in_r = true;
	g21_1.in_s = true;
	g21_2.in_s = g21_2.in_r = true;
	g21_1.in_d = true; // f21:q5 and f21:q6 are low
	c15.in_s = false;
}

void SUB::reset()
{
	pa = pc = 0;
	key_sel = key_data = 0;
	color_reg = 0x70;
	hsync = wait = false;
	cblink = 0;
	
	c15.in_b = ((pa & 0x40) != 0);
	c15.in_c = ((pa & 0x80) != 0);
	update_cmt();
}

void SUB::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	switch(addr & 0xfc00) {
	case 0xe000:
		d_crtc->write_io8(addr, data);
		break;
	case 0xe400:
		key_sel = data;
		key_update();
		// bit4: buzzer
		d_beep->write_signal(SIG_BEEP_ON, data, 0x10);
		break;
	case 0xe800:
		d_main->write_signal(SIG_MAIN_COMM, data, 0xff);
		break;
	case 0xec00:
		break;
	case 0xf000:
		color_reg = data;
		break;
	default:
		if(0x2000 <= addr && addr < 0xe000) {
			if(!wait && hsync) {
				d_cpu->write_signal(SIG_UPD7801_WAIT, 1, 1);
				wait = true;
			}
			data = ~data;
		}
		wbank[addr >> 7][addr & 0x7f] = data;
		break;
	}
}

uint32_t SUB::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	switch(addr & 0xfc00) {
	case 0xe000:
		return d_crtc->read_io8(addr);
	case 0xe400:
		// dipswitch
		// bit0: 1 = 80char,  0 = 40char
		// bit1: 1 = Screen1, 0 = Screen0
		// bit2: 1 = FP-1100, 0 = FP-1000
		// bit3: 1 = 300baud, 0 = 1200baud
		// bit4: 1 = FP-1012PR
		// bit5: 1 = Always ON
#ifdef _FP1000
		return config.baud_high[0] ? 0xf0 : 0xf8;
#else
		return config.baud_high[0] ? 0xf4 : 0xfc;
#endif
	case 0xe800:
		return comm_data;
	case 0xec00:
	case 0xf000:
		return 0xff;
	default:
		if(0x2000 <= addr && addr < 0xe000) {
			if(!wait && hsync) {
				d_cpu->write_signal(SIG_UPD7801_WAIT, 1, 1);
				wait = true;
			}
		}
		return rbank[addr >> 7][addr & 0x7f];
	}
}

void SUB::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case P_A:
		// clear vram
		if((pa & 0x20) && !(data & 0x20)) {
			uint8_t fore_color = (color_reg & 0x80) ? ((color_reg >> 4) & 7) : 0;
			memset(vram_b, (fore_color & 1) ? 0xff : 0, sizeof(vram_b));
			memset(vram_r, (fore_color & 2) ? 0xff : 0, sizeof(vram_r));
			memset(vram_g, (fore_color & 4) ? 0xff : 0, sizeof(vram_g));
		}
		// update crtc character clock
		if(data & 8) {
			d_crtc->set_char_clock(998400);		// 40 column
		} else {
			d_crtc->set_char_clock(1996800);	// 80 column
		}
		pa = data;
		c15.in_b = ((pa & 0x40) != 0);
		c15.in_c = ((pa & 0x80) != 0);
		update_cmt();
		break;
	case P_B:
		// printer data
		pb = data;
		break;
	case P_C:
		//if((pc & 8) != (data & 8)) {
		if(!(pc & 8) && (data & 8)) {
			d_main->write_signal(SIG_MAIN_INTS, data, 8);
		}
		d_drec->write_signal(SIG_DATAREC_REMOTE, data, 0x20);
		pc = data;
		break;
	}
}

uint32_t SUB::read_io8(uint32_t addr)
{
	switch(addr) {
	case P_A:
		return pa;
	case P_B:
		if(key_sel & 0x20) {
			return key_data;
		}
		return 0;//xff;
	case P_C:
		return pc;
	}
	return 0xff;
}

void SUB::write_signal(int id, uint32_t data, uint32_t mask)
{
	switch(id) {
	case SIG_SUB_INT2:
		// from main pcb
		d_cpu->write_signal(SIG_UPD7801_INTF2, data, mask);
		// FIXME: ugly patch for boot
		if(data & mask) {
			d_main->write_signal(SIG_MAIN_COMM, 0, 0xff);
		}
		break;
	case SIG_SUB_COMM:
		// from main pcb
		comm_data = data & 0xff;
		// FIXME: ugly patch for command
		if(get_cpu_pc(1) == 0x10e || get_cpu_pc(1) == 0x110) {
			d_cpu->write_signal(SIG_UPD7801_INTF2, 1, 1);
		}
		break;
	case SIG_SUB_HSYNC:
		// from crtc
		hsync = ((data & mask) != 0);
		if(wait && !hsync) {
			d_cpu->write_signal(SIG_UPD7801_WAIT, 0, 0);
			wait = false;
		}
		break;
	case SIG_SUB_SO:
		so = ((data & mask) != 0);
		break;
	case SIG_SUB_EAR:
		b16_1.in_d = g21_1.in_ck = c16.in_a = ((data & mask) != 0);
		update_cmt();
		break;
	}
}

void SUB::event_frame()
{
	cblink = (cblink + 1) & 0x1f;
}

#define BIT_2400HZ	0x10
#define BIT_1200HZ	0x20
#define BIT_300HZ	0x80

void SUB::event_callback(int event_id, int err)
{
	if(event_id == EVENT_CLOCK) {
		c15.in_d4 = c15.in_d5 = ((clock & BIT_1200HZ) != 0);
		c15.in_d6 = c15.in_d7 = ((clock & BIT_300HZ ) != 0);
		
		// 76.8KHz: L->H
		b16_1.in_ck = b16_2.in_ck = g21_2.in_ck = false;
		f21.in_ck = !(g21_1.in_d && false);
		update_cmt();
		b16_1.in_ck = b16_2.in_ck = g21_2.in_ck = true;
		f21.in_ck = !(g21_1.in_d && true);
		update_cmt();
		
		d_drec->write_signal(SIG_DATAREC_MIC, clock, so ? BIT_2400HZ : BIT_1200HZ);
		clock++;
	}
}

void SUB::update_cmt()
{
	b16_1.update();
	b16_2.update();
	f21.update();
	g21_1.update();
	g21_2.update();
	c16.update();
	
	b16_2.in_d = b16_1.out_q;
	f21.in_clr = (b16_1.out_q && b16_2.out_nq);
	g21_1.in_d = g21_1.in_r = c15.in_d0 = !(f21.out_q5 && f21.out_q6);
	g21_2.in_d = g21_1.out_q;
	c16.in_rc1 = c16.in_rc2 = (g21_1.out_q != g21_2.out_q); // xor
	c15.in_a = (g21_1.out_q && g21_2.out_q);
	c16.in_b = c16.out_qa;
	c15.in_d1 = !c16.out_qa;
	c15.in_d2 = c16.out_qb;
	c15.in_d3 = c16.out_qc;
	
	c15.update();
	
	pc &= ~(0x04 | 0x80);
	if(c15.out_y) pc |= 0x04;
	if(c15.in_a ) pc |= 0x80;
	d_cpu->write_signal(SIG_UPD7801_SCK, pc, 0x04);
	d_cpu->write_signal(SIG_UPD7801_SI,  pc, 0x80);
}

void SUB::key_down(int code)
{
	key_update();
}

void SUB::key_up(int code)
{
	key_update();
}

// BREAK -> PAUSE
// STOP  -> END
// CLS   -> HOME
// KANA  -> ALT
// PF0   -> F10

static const int key_map[16][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x10, 0x11, 0x12, 0x14, 0x15, 0x00, 0x00, 0x13},
	{0x41, 0x1b, 0x6d, 0x51, 0x5a, 0x6a, 0x00, 0x70},
	{0x53, 0x31, 0x6b, 0x57, 0x58, 0x6f, 0x00, 0x71},
	{0x44, 0x32, 0x63, 0x45, 0x43, 0x2e, 0x6e, 0x72},
	{0x46, 0x33, 0x66, 0x52, 0x56, 0x27, 0x00, 0x73},
	{0x47, 0x34, 0x69, 0x54, 0x42, 0x2d, 0x20, 0x74},
	{0x48, 0x35, 0x68, 0x59, 0x4e, 0x28, 0x60, 0x75},
	{0x4a, 0x36, 0x65, 0x55, 0x4d, 0x26, 0x62, 0x76},
	{0x4b, 0x37, 0x64, 0x49, 0xbc, 0x24, 0x61, 0x77},
	{0x4c, 0x38, 0x67, 0x4f, 0xbe, 0x25, 0xdd, 0x78},
	{0xbb, 0x39, 0x0d, 0x50, 0xbf, 0x00, 0xdb, 0x79},
	{0xba, 0x30, 0xde, 0xc0, 0xe2, 0xdc, 0xbd, 0x23},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

void SUB::key_update()
{
	uint8_t prev = key_data;
	key_data = 0;
	
	for(int i = 0; i < 8; i++) {
		if(key_stat[key_map[key_sel & 0xf][i]]) {
			key_data |= (1 << i);
		}
	}
	if((key_data & 0x80) != (prev & 0x80)) {
		d_cpu->write_signal(SIG_UPD7801_INTF0, key_data, 0x80);
	}
}

void SUB::draw_screen()
{
	// render screen
//	int lmax = (regs[9] & 0x1f) + 1;
	int lmax = (regs[9] & 0x1f) < 8 ? 8 : 16;
	int ymax = (regs[6] & 0x7f) * lmax;
	uint16_t src = ((regs[12] << 11) | (regs[13] << 3)) & 0x3fff;
	uint16_t cursor = ((regs[14] << 11) | (regs[15] << 3)) & 0x3fff;
	
	memset(screen, 0, sizeof(screen));
	
//	emu->set_vm_screen_lines((ymax > 400) ? 400 : ymax);
	
	if((regs[8] & 0x30) != 0x30 && (pa & 7) != 0) {
		if(pa & 8) {
			// 40 column
			for(int y = 0; y < ymax && y < 400; y += lmax) {
				for(int x = 0; x < 640; x += 16) {
					for(int l = 0; l < lmax; l++) {
						uint16_t src2 = src | (l & 7);
						uint8_t b, r, g;
						if(lmax > 8) {
							b = vram_b[src2];
							r = vram_r[src2];
							g = vram_g[src2];
							if(l < 8) {
								r = g = b;
							} else if(l < 16) {
								g = b = r;
							} else {
								b = r = g;
							}
						} else {
							b = (pa & 4) ? vram_b[src2] : 0;
							r = (pa & 2) ? vram_r[src2] : 0;
							g = (pa & 1) ? vram_g[src2] : 0;
							if(pa & 0x10) {
								b = r = g = b | r | g;
							}
						}
						uint8_t* d = &screen[y + l][x];
						
						d[ 0] = d[ 1] = ((b & 0x01) << 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
						d[ 2] = d[ 3] = ((b & 0x02) >> 1) | ((r & 0x02) << 0) | ((g & 0x02) << 1);
						d[ 4] = d[ 5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) << 0);
						d[ 6] = d[ 7] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
						d[ 8] = d[ 9] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
						d[10] = d[11] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
						d[12] = d[13] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
						d[14] = d[15] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
					}
					if(src == cursor && (regs[8] & 0xc0) != 0xc0) {
						uint8_t bp = regs[10] & 0x60;
						if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
							uint8_t cursor_color = (color_reg & 0x80) ? 7 : ((color_reg >> 4) & 7);
							for(int l = (regs[10] & 0x1f); l < lmax; l++) {
								memset(&screen[y + l][x], cursor_color, 16);
							}
						}
					}
					src = (src + 8) & 0x3fff;
				}
			}
		} else {
			// 80 column
			for(int y = 0; y < ymax && y < 400; y += lmax) {
				for(int x = 0; x < 640; x += 8) {
					for(int l = 0; l < lmax; l++) {
						uint16_t src2 = src | (l & 7);
						uint8_t b, r, g;
						if(lmax > 8) {
							b = vram_b[src2];
							r = vram_r[src2];
							g = vram_g[src2];
							if(l < 8) {
								r = g = b;
							} else if(l < 16) {
								g = b = r;
							} else {
								b = r = g;
							}
						} else {
							b = (pa & 4) ? vram_b[src2] : 0;
							r = (pa & 2) ? vram_r[src2] : 0;
							g = (pa & 1) ? vram_g[src2] : 0;
							if(pa & 0x10) {
								b = r = g = b | r | g;
							}
						}
						uint8_t* d = &screen[y + l][x];
						
						d[0] = ((b & 0x01) << 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
						d[1] = ((b & 0x02) >> 1) | ((r & 0x02) << 0) | ((g & 0x02) << 1);
						d[2] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) << 0);
						d[3] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
						d[4] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
						d[5] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
						d[6] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
						d[7] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
					}
					if(src == cursor && (regs[8] & 0xc0) != 0xc0) {
						uint8_t bp = regs[10] & 0x60;
						if(bp == 0 || (bp == 0x40 && (cblink & 8)) || (bp == 0x60 && (cblink & 0x10))) {
							uint8_t cursor_color = (color_reg & 0x80) ? 7 : ((color_reg >> 4) & 7);
							for(int l = (regs[10] & 0x1f); l < lmax; l++) {
								memset(&screen[y + l][x], cursor_color, 8);
							}
						}
					}
					src = (src + 8) & 0x3fff;
				}
			}
		}
	}
	
	// copy to real screen
	if(ymax > 200) {
		// 400 line
		for(int y = 0; y < 400; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest[x] = palette_pc[src[x] & 7];
			}
		}
		emu->screen_skip_line(false);
	} else {
		// 200 line
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0);
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1);
			uint8_t* src = screen[y];
			
			for(int x = 0; x < 640; x++) {
				dest0[x] = palette_pc[src[x] & 7];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype_t));
			} else {
				memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
			}
		}
		emu->screen_skip_line(true);
	}
}

#define STATE_VERSION	3

void process_state_ls74(ls74_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_d);
	state_fio->StateValue(val->in_ck);
	state_fio->StateValue(val->in_s);
	state_fio->StateValue(val->in_r);
	state_fio->StateValue(val->out_q);
	state_fio->StateValue(val->out_nq);
	state_fio->StateValue(val->tmp_ck);
}

void process_state_ls151(ls151_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_d0);
	state_fio->StateValue(val->in_d1);
	state_fio->StateValue(val->in_d2);
	state_fio->StateValue(val->in_d3);
	state_fio->StateValue(val->in_d4);
	state_fio->StateValue(val->in_d5);
	state_fio->StateValue(val->in_d6);
	state_fio->StateValue(val->in_d7);
	state_fio->StateValue(val->in_a);
	state_fio->StateValue(val->in_b);
	state_fio->StateValue(val->in_c);
	state_fio->StateValue(val->in_s);
	state_fio->StateValue(val->out_y);
	state_fio->StateValue(val->out_ny);
}

void process_state_ls93(ls93_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_a);
	state_fio->StateValue(val->in_b);
	state_fio->StateValue(val->in_rc1);
	state_fio->StateValue(val->in_rc2);
	state_fio->StateValue(val->out_qa);
	state_fio->StateValue(val->out_qb);
	state_fio->StateValue(val->out_qc);
	state_fio->StateValue(val->tmp_a);
	state_fio->StateValue(val->tmp_b);
	state_fio->StateValue(val->counter_a);
	state_fio->StateValue(val->counter_b);
}

void process_state_tc4024bp(tc4024bp_t* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->in_ck);
	state_fio->StateValue(val->in_clr);
	state_fio->StateValue(val->out_q5);
	state_fio->StateValue(val->out_q6);
	state_fio->StateValue(val->tmp_ck);
	state_fio->StateValue(val->counter);
}

bool SUB::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram_b, sizeof(vram_b), 1);
	state_fio->StateArray(vram_r, sizeof(vram_r), 1);
	state_fio->StateArray(vram_g, sizeof(vram_g), 1);
	state_fio->StateValue(pa);
	state_fio->StateValue(pb);
	state_fio->StateValue(pc);
	state_fio->StateValue(comm_data);
	state_fio->StateValue(so);
	state_fio->StateValue(clock);
	process_state_ls74(&b16_1, state_fio);
	process_state_ls74(&b16_2, state_fio);
	process_state_ls74(&g21_1, state_fio);
	process_state_ls74(&g21_2, state_fio);
	process_state_ls151(&c15, state_fio);
	process_state_ls93(&c16, state_fio);
	process_state_tc4024bp(&f21, state_fio);
	state_fio->StateValue(key_sel);
	state_fio->StateValue(key_data);
	state_fio->StateValue(color_reg);
	state_fio->StateValue(hsync);
	state_fio->StateValue(wait);
	state_fio->StateValue(cblink);
	return true;
}

