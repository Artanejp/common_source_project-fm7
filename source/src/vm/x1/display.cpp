/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Origin : X1EMU by KM (kanji rom)
	         X-millenium by Yui (ank16 patch)
	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ display ]
*/

#include "display.h"
#include "../hd46505.h"
#include "../i8255.h"

#ifdef _X1TURBO_FEATURE
#define EVENT_AFTER_BLANK	0
#endif

#ifdef _X1TURBOZ
#define AEN	((zmode1 & 0x80) != 0)
#define C64	((zmode1 & 0x10) != 0)
#define APEN	((zmode2 & 0x80) != 0)
#define APRD	((zmode2 & 0x08) != 0)
#endif

void DISPLAY::initialize()
{
	// load rom images
	FILEIO* fio = new FILEIO();
	
	// ank8 (8x8)
	if(fio->Fopen(create_local_path(_T("ANK8.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("FNT0808.X1")), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	
	// ank16 (8x16)
	if(fio->Fopen(create_local_path(_T("ANK16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, 0x1000, 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("FNT0816.X1")), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(kanji, 0x1000, 1);
		fio->Fclose();
	}
	memcpy(kanji + 0x7f * 16, ANKFONT7f_9f, sizeof(ANKFONT7f_9f));
	memcpy(kanji + 0xe0 * 16, ANKFONTe0_ff, sizeof(ANKFONTe0_ff));
	
	// kanji (16x16)
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji + 0x1000, 0x4ac00, 1);
		fio->Fclose();
	} else if(fio->Fopen(create_local_path(_T("FNT1616.X1")), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(kanji + 0x1000, 0x4ac00, 1);
		fio->Fclose();
	}
	for(int ofs = 0x1000; ofs < 0x4bc00; ofs += 32) {
		// LRLR.. -> LL..RR..
		uint8_t buf[32];
		for(int i = 0; i < 16; i++) {
			buf[i     ] = kanji[ofs + i * 2    ];
			buf[i + 16] = kanji[ofs + i * 2 + 1];
		}
		memcpy(kanji + ofs, buf, 32);
	}
	delete fio;
	
	// create pc palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i    ] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// text
		palette_pc[i + 8] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// cg
	}
#ifdef _X1TURBOZ
	for(int i = 0; i < 8; i++) {
		ztpal[i] = ((i & 1) ? 0x03 : 0) | ((i & 2) ? 0x0c : 0) | ((i & 4) ? 0x30 : 0);
		zpalette_tmp[i    ] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// text
		zpalette_tmp[i + 8] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// digital
	}
	for(int g = 0; g < 16; g++) {
		for(int r = 0; r < 16; r++) {
			for(int b = 0; b < 16; b++) {
				int num = b + r * 16 + g * 256;
				zpal[num].b = b;
				zpal[num].r = r;
				zpal[num].g = g;
				zpalette_tmp[num + 16] = RGB_COLOR((r * 255) / 15, (g * 255) / 15, (b * 255) / 15);
			}
		}
	}
	zpalette_changed = true;
#endif
	
	// initialize regs
	pal[0] = 0xaa;
	pal[1] = 0xcc;
	pal[2] = 0xf0;
	priority = 0;
	update_pal();
	column40 = true;
	
	memset(vram_t, 0, sizeof(vram_t));
	memset(vram_a, 0, sizeof(vram_a));
#ifdef _X1TURBO_FEATURE
	memset(vram_k, 0, sizeof(vram_k));
#endif
	memset(pcg_b, 0, sizeof(pcg_b));
	memset(pcg_r, 0, sizeof(pcg_r));
	memset(pcg_g, 0, sizeof(pcg_g));
#ifdef _X1TURBO_FEATURE
	memset(gaiji_b, 0, sizeof(gaiji_b));
	memset(gaiji_r, 0, sizeof(gaiji_r));
	memset(gaiji_g, 0, sizeof(gaiji_g));
#endif
	
	// register event
	register_frame_event(this);
	register_vline_event(this);
}

void DISPLAY::reset()
{
#ifdef _X1TURBO_FEATURE
	mode1 = 0;//3;
	mode2 = 0;
	hireso = true;
#endif
#ifdef _X1TURBOZ
	zmode1 = 0;
	zpriority = 0;
	zadjust = 0;
	zmosaic = 0;
	zchromakey = 0;
	zscroll = 0;
	zmode2 = 0;
	zpal_num = 0;
#endif
	cur_line = cur_code = 0;
	vblank_clock = 0;
	cur_vline = 0;
	cur_blank = true;
	
	kaddr = kofs = kflag = 0;
	kanji_ptr = &kanji[0];
}

void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff00) {
	case 0x0e00:
		write_kanji(addr, data);
		break;
	case 0x1000:
#ifdef _X1TURBOZ
		if(AEN) {
			if(APEN && !APRD) {
				if(!cur_blank) {
					// wait next blank
					d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				}
				int num = get_zpal_num(addr, data);
				if(zpal[num].b != (data & 0x0f)) {
					zpal[num].b = data & 0x0f;
					zpalette_tmp[num + 16] = RGB_COLOR((zpal[num].r * 255) / 15, (zpal[num].g * 255) / 15, (zpal[num].b * 255) / 15);
					zpalette_changed = true;
				}
			} else if(APEN && APRD) {
				zpal_num = get_zpal_num(addr, data);
			}
		} else
#endif
		{
			pal[0] = data;
			update_pal();
		}
		break;
	case 0x1100:
#ifdef _X1TURBOZ
		if(AEN) {
			if(APEN && !APRD) {
				if(!cur_blank) {
					// wait next blank
					d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				}
				int num = get_zpal_num(addr, data);
				if(zpal[num].r != (data & 0x0f)) {
					zpal[num].r = data & 0x0f;
					zpalette_tmp[num + 16] = RGB_COLOR((zpal[num].r * 255) / 15, (zpal[num].g * 255) / 15, (zpal[num].b * 255) / 15);
					zpalette_changed = true;
				}
			} else if(APEN && APRD) {
//				zpal_num = get_zpal_num(addr, data);
			}
		} else
#endif
		{
			pal[1] = data;
			update_pal();
		}
		break;
	case 0x1200:
#ifdef _X1TURBOZ
		if(AEN) {
			if(APEN && !APRD) {
				if(!cur_blank) {
					// wait next blank
					d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				}
				int num = get_zpal_num(addr, data);
				if(zpal[num].g != (data & 0x0f)) {
					zpal[num].g = data & 0x0f;
					zpalette_tmp[num + 16] = RGB_COLOR((zpal[num].r * 255) / 15, (zpal[num].g * 255) / 15, (zpal[num].b * 255) / 15);
					zpalette_changed = true;
				}
			} else if(APEN && APRD) {
//				zpal_num = get_zpal_num(addr, data);
			}
		} else
#endif
		{
			pal[2] = data;
			update_pal();
		}
		break;
	case 0x1300:
		priority = data;
		update_pal();
		break;
	case 0x1500:
		get_cur_pcg(addr);
		pcg_b[cur_code][cur_line] = data;
#ifdef _X1TURBO_FEATURE
		gaiji_b[cur_code >> 1][(cur_line << 1) | (cur_code & 1)] = data;
#endif
		break;
	case 0x1600:
		get_cur_pcg(addr);
		pcg_r[cur_code][cur_line] = data;
#ifdef _X1TURBO_FEATURE
		gaiji_r[cur_code >> 1][(cur_line << 1) | (cur_code & 1)] = data;
#endif
		break;
	case 0x1700:
		get_cur_pcg(addr);
		pcg_g[cur_code][cur_line] = data;
#ifdef _X1TURBO_FEATURE
		gaiji_g[cur_code >> 1][(cur_line << 1) | (cur_code & 1)] = data;
#endif
		break;
#ifdef _X1TURBO_FEATURE
	case 0x1f00:
		switch(addr) {
#ifdef _X1TURBOZ
		case 0x1fb0:
			zmode1 = data;
			break;
//		case 0x1fb8:
		case 0x1fb9:
		case 0x1fba:
		case 0x1fbb:
		case 0x1fbc:
		case 0x1fbd:
		case 0x1fbe:
		case 0x1fbf:
			if(AEN) {
				if(ztpal[addr & 7] != data) {
					ztpal[addr & 7] = data;
					zpalette_tmp[addr & 7] = RGB_COLOR((((data >> 2) & 3) * 255) / 3, (((data >> 4) & 3) * 255) / 3, (((data >> 0) & 3) * 255) / 3);
					zpalette_changed = true;
				}
			}
			break;
		case 0x1fc0:
			if(AEN) {
				zpriority = data;
			}
			break;
		case 0x1fc1:
			if(AEN) {
				zadjust = data;
			}
			break;
		case 0x1fc2:
			if(AEN) {
				zmosaic = data;
			}
			break;
		case 0x1fc3:
			if(AEN) {
				zchromakey = data;
			}
			break;
		case 0x1fc4:
			if(AEN) {
				zscroll = data;
			}
			break;
		case 0x1fc5:
			if(AEN) {
				zmode2 = data;
			}
			break;
#endif
		case 0x1fd0:
			mode1 = data;
			update_crtc();
//			hireso = !((mode1 & 3) == 0 || (mode1 & 3) == 2);
			break;
		case 0x1fe0:
			mode2 = data;
			update_pal();
			break;
		}
		break;
#endif
	case 0x2000:
	case 0x2100:
	case 0x2200:
	case 0x2300:
	case 0x2400:
	case 0x2500:
	case 0x2600:
	case 0x2700:
		vram_a[addr & 0x7ff] = data;
		break;
	case 0x2800:
	case 0x2900:
	case 0x2a00:
	case 0x2b00:
	case 0x2c00:
	case 0x2d00:
	case 0x2e00:
	case 0x2f00:
		vram_a[addr & 0x7ff] = data; // mirror
		break;
	case 0x3000:
	case 0x3100:
	case 0x3200:
	case 0x3300:
	case 0x3400:
	case 0x3500:
	case 0x3600:
	case 0x3700:
		vram_t[addr & 0x7ff] = data;
		break;
	case 0x3800:
	case 0x3900:
	case 0x3a00:
	case 0x3b00:
	case 0x3c00:
	case 0x3d00:
	case 0x3e00:
	case 0x3f00:
#ifdef _X1TURBO_FEATURE
		vram_k[addr & 0x7ff] = data;
#else
		vram_t[addr & 0x7ff] = data; // mirror
#endif
		break;
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr & 0xff00) {
	case 0x0e00:
		return read_kanji(addr);
#ifdef _X1TURBOZ
	case 0x1000:
		if(AEN && APEN && APRD) {
			return zpal[zpal_num].b;
		}
		break;
	case 0x1100:
		if(AEN && APEN && APRD) {
			return zpal[zpal_num].r;
		}
		break;
	case 0x1200:
		if(AEN && APEN && APRD) {
			return zpal[zpal_num].g;
		}
		break;
#endif
//	case 0x1300:
//		return priority;
	case 0x1400:
		return get_cur_font(addr);
	case 0x1500:
		get_cur_pcg(addr);
		return pcg_b[cur_code][cur_line];
	case 0x1600:
		get_cur_pcg(addr);
		return pcg_r[cur_code][cur_line];
	case 0x1700:
		get_cur_pcg(addr);
		return pcg_g[cur_code][cur_line];
#ifdef _X1TURBOZ
	case 0x1f00:
		switch(addr) {
		case 0x1fb0:
			return zmode1;
//		case 0x1fb8:
		case 0x1fb9:
		case 0x1fba:
		case 0x1fbb:
		case 0x1fbc:
		case 0x1fbd:
		case 0x1fbe:
		case 0x1fbf:
			if(AEN) {
				return ztpal[addr & 7];
			}
			break;
		case 0x1fc0:
			if(AEN) {
				return zpriority;
			}
			break;
		case 0x1fc1:
			if(AEN) {
				return zadjust;
			}
			break;
		case 0x1fc2:
			if(AEN) {
				return zmosaic;
			}
			break;
		case 0x1fc3:
			if(AEN) {
				return zchromakey;
			}
			break;
		case 0x1fc4:
			if(AEN) {
				return zscroll;
			}
			break;
		case 0x1fc5:
			if(AEN) {
				return zmode2;
			}
			break;
		case 0x1fd0:
			return mode1;
		case 0x1fe0:
			return mode2;
		}
		break;
#endif
	case 0x2000:
	case 0x2100:
	case 0x2200:
	case 0x2300:
	case 0x2400:
	case 0x2500:
	case 0x2600:
	case 0x2700:
		return vram_a[addr & 0x7ff];
	case 0x2800:
	case 0x2900:
	case 0x2a00:
	case 0x2b00:
	case 0x2c00:
	case 0x2d00:
	case 0x2e00:
	case 0x2f00:
		return vram_a[addr & 0x7ff]; // mirror
	case 0x3000:
	case 0x3100:
	case 0x3200:
	case 0x3300:
	case 0x3400:
	case 0x3500:
	case 0x3600:
	case 0x3700:
		return vram_t[addr & 0x7ff];
	case 0x3800:
	case 0x3900:
	case 0x3a00:
	case 0x3b00:
	case 0x3c00:
	case 0x3d00:
	case 0x3e00:
	case 0x3f00:
#ifdef _X1TURBO_FEATURE
		return vram_k[addr & 0x7ff];
#else
		return vram_t[addr & 0x7ff]; // mirror
#endif
	}
	return 0xff;
}

void DISPLAY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_DISPLAY_VBLANK) {
		if(!(data & mask)) {
			// enter vblank
			vblank_clock = get_current_clock();
		}
	} else if(id == SIG_DISPLAY_COLUMN40) {
		column40 = ((data & mask) != 0);
		update_crtc();
	} else if(id == SIG_DISPLAY_DETECT_VBLANK) {
		// hack: cpu detects vblank
		vblank_clock = get_current_clock();
	} else if(id == SIG_DISPLAY_DISP) {
		bool prev = cur_blank;
		cur_blank = ((data & mask) == 0); // blank = not disp
		if(!prev && cur_blank) {
			// draw line at start of hblank
#ifdef _X1TURBO_FEATURE
			if(hireso) {
				if(cur_vline < 400) {
					draw_line(cur_vline);
				}
			} else {
#endif
				if(cur_vline < 200) {
					draw_line(cur_vline);
				}
#ifdef _X1TURBO_FEATURE
			}
			// restart cpu after pcg/cgrom/zpal is accessed
//			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
			register_event_by_clock(this, EVENT_AFTER_BLANK, 24, false, NULL);
#endif
		}
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x3f;
	
	// update crtc parameters
	ch_height = (regs[9] & 0x1f) + 1;
	hz_total = regs[0] + 1;
	hz_disp = regs[1];
	vt_disp = regs[6] & 0x7f;
	vt_ofs = regs[5] & 0x1f;
	st_addr = (regs[12] << 8) | regs[13];
	
#ifdef _X1TURBO_FEATURE
	int vt_total = ((regs[4] & 0x7f) + 1) * ch_height + (regs[5] & 0x1f);
	hireso = (vt_total > 400);
	if(!hireso) vt_ofs = max(vt_ofs - 2, 0);
#endif
	
	// initialize draw screen
	memset(text, 0, sizeof(text));
	memset(cg, 0, sizeof(cg));
	memset(pri_line, 0, sizeof(pri_line));
#ifdef _X1TURBOZ
	memset(zcg, 0, sizeof(zcg));
	memset(aen_line, 0, sizeof(aen_line));
#endif
	prev_vert_double = false;
	raster = 0;
}

void DISPLAY::event_vline(int v, int clock)
{
#ifdef _X1TURBOZ
//	if(zpalette_changed && v == (hireso ? 400 : 200)) {
	if(zpalette_changed && v == 0) {
		update_zpalette();
		zpalette_changed = false;
	}
#endif
	cur_vline = v;
}

#ifdef _X1TURBO_FEATURE
void DISPLAY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_AFTER_BLANK) {
		// restart cpu after pcg/cgrom/zpal is accessed
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}
#endif

void DISPLAY::update_crtc()
{
#ifdef _X1TURBO_FEATURE
	if(column40) {
		d_crtc->set_char_clock((mode1 & 1) ? VDP_CLOCK * 1.5 / 32.0 : VDP_CLOCK / 32.0);
	} else {
		d_crtc->set_char_clock((mode1 & 1) ? VDP_CLOCK * 1.5 / 16.0 : VDP_CLOCK / 16.0);
	}
#else
	if(column40) {
		d_crtc->set_char_clock(VDP_CLOCK / 32.0);
	} else {
		d_crtc->set_char_clock(VDP_CLOCK / 16.0);
	}
#endif
}

void DISPLAY::update_pal()
{
	uint8_t pal2[8];
	for(int i = 0; i < 8; i++) {
		uint8_t bit = 1 << i;
		pal2[i] = ((pal[0] & bit) ? 1 : 0) | ((pal[1] & bit) ? 2 : 0) | ((pal[2] & bit) ? 4 : 0) | 8;
	}
#ifdef _X1TURBO_FEATURE
	if(mode2 & 0x10) pal2[0] = 8;
	if(mode2 & 0x20) pal2[1] = 8;
#endif
	for(int c = 0; c < 8; c++) {
		for(int t = 0; t < 8; t++) {
			if(priority & (1 << c)) {
				pri[c][t] = pal2[c];
			} else if(t) {
#ifdef _X1TURBO_FEATURE
				pri[c][t] = ((mode2 & 8) && (mode2 & 7) == t) ? 0 : t;
#else
				pri[c][t] = t;
#endif
			} else {
				pri[c][t] = pal2[c];
			}
		}
	}
}

uint8_t DISPLAY::get_cur_font(uint32_t addr)
{
#ifdef _X1TURBO_FEATURE
	if(mode1 & 0x20) {
		// wait next blank
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		
		// from X1EMU
		uint16_t vaddr;
		if(!(vram_a[0x7ff] & 0x20)) {
			vaddr = 0x7ff;
		} else if(!(vram_a[0x3ff] & 0x20)) {
			vaddr = 0x3ff;
		} else if(!(vram_a[0x5ff] & 0x20)) {
			vaddr = 0x5ff;
		} else if(!(vram_a[0x1ff] & 0x20)) {
			vaddr = 0x1ff;
		} else {
			vaddr = 0x3ff;
		}
		uint16_t ank = vram_t[vaddr];
		uint16_t knj = vram_k[vaddr];
		
		if(knj & 0x80) {
			uint32_t ofs = adr2knj_x1t((knj << 8) | ank);
			if(knj & 0x40) {
				ofs += 16; // right
			}
			return kanji[ofs | (addr & 15)];
		} else if(mode1 & 0x40) {
			return kanji[(ank << 4) | (addr & 15)];
		} else {
			return font[(ank << 3) | ((addr >> 1) & 7)];
		}
	}
#endif
	get_cur_code_line();
	return font[(cur_code << 3) | (cur_line & 7)];
}

void DISPLAY::get_cur_pcg(uint32_t addr)
{
#ifdef _X1TURBO_FEATURE
	if(mode1 & 0x20) {
		// wait next blank
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		
		// from X1EMU
		uint16_t vaddr;
		if(vram_a[0x7ff] & 0x20) {
			vaddr = 0x7ff;
		} else if(vram_a[0x3ff] & 0x20) {
			vaddr = 0x3ff;
		} else if(vram_a[0x5ff] & 0x20) {
			vaddr = 0x5ff;
		} else if(vram_a[0x1ff] & 0x20) {
			vaddr = 0x1ff;
		} else {
			vaddr = 0x3ff;
		}
		cur_code = vram_t[vaddr];
		cur_line = (addr >> 1) & 7;
		
		if(vram_k[vaddr] & 0x90) {
			cur_code &= 0xfe;
			cur_code += addr & 1;
		}
	} else
#endif
	get_cur_code_line();
}

void DISPLAY::get_cur_code_line()
{
//#ifdef _X1TURBO_FEATURE
//	int ht_clock = hireso ? 161 : 250;
//#else
	#define ht_clock 250
//#endif
	int clock = get_passed_clock(vblank_clock);
	int vt_line = vt_disp * ch_height + (int)(clock / ht_clock);
	
	int addr = (hz_total * (clock % ht_clock)) / ht_clock;
	addr += hz_disp * (int)(vt_line / ch_height);
	addr &= 0x7ff; // thanks Mr.YAT
	addr += st_addr;
	
	cur_code = vram_t[addr & 0x7ff];
	cur_line = (vt_line % ch_height) & 7;
}

void DISPLAY::draw_line(int v)
{
	if((regs[8] & 0x30) != 0x30) {
		if((v % ch_height) == 0) {
			draw_text(v / ch_height);
		}
#ifdef _X1TURBOZ
		if(AEN && !hireso && column40) {
			draw_cg(v, 1);
		}
#endif
		draw_cg(v, 0);
		memcpy(&pri_line[v][0][0], &pri[0][0], sizeof(pri));
//	} else {
//		memset(&pri_line[v][0][0], 0, sizeof(pri));
	}
#ifdef _X1TURBOZ
	aen_line[v] = AEN;
#endif
}

void DISPLAY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
#ifdef _X1TURBO_FEATURE
		for(int v = 0; v < (hireso ? 400 : 200); v++) {
#else
		for(int v = 0; v < 200; v++) {
#endif
			draw_line(v);
		}
#ifdef _X1TURBOZ
		if(zpalette_changed && cur_vline < (hireso ? 400 : 200)) {
			update_zpalette();
			zpalette_changed = false;
		}
#endif
	}
	
	// total raster_adjust
#ifdef _X1TURBO_FEATURE
	for(int y = 0; y < vt_ofs * (hireso ? 1 : 2); y++) {
#else
	for(int y = 0; y < vt_ofs * 2; y++) {
#endif
		scrntype_t* dest = emu->get_screen_buffer(y);
		memset(dest, 0, 640 * sizeof(scrntype_t));
	}
	
	// copy to real screen
#ifdef _X1TURBO_FEATURE
	if(hireso) {
		// 400 lines
		emu->set_vm_screen_lines(400);
		
		if(column40) {
			// 40 columns
			for(int y = 0; y + vt_ofs < 400; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y + vt_ofs);
				uint8_t* src_text = text[y];
#ifdef _X1TURBOZ
				if(aen_line[y]) {
					uint16_t* src_cg0 = zcg[0][y];
					
					for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
						uint16_t cg00 = src_cg0[x] | (src_cg0[x] >> 2);
						
						dest[x2] = dest[x2 + 1] = get_zpriority(src_text[x], cg00, cg00);
					}
				} else {
#endif
					uint8_t* src_cg = cg[y];
					
					for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
#ifdef _X1TURBOZ
						dest[x2] = dest[x2 + 1] = zpalette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#else
						dest[x2] = dest[x2 + 1] =  palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#endif
					}
#ifdef _X1TURBOZ
				}
#endif
			}
		} else {
			// 80 columns
			for(int y = 0; y + vt_ofs < 400; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y + vt_ofs);
				uint8_t* src_text = text[y];
#ifdef _X1TURBOZ
				if(aen_line[y]) {
					uint16_t* src_cg0 = zcg[0][y];
					
					for(int x = 0; x < 640; x++) {
						uint16_t cg00 = src_cg0[x] | (src_cg0[x] >> 2);
						
						dest[x] = get_zpriority(src_text[x], cg00, cg00);
					}
				} else {
#endif
					uint8_t* src_cg = cg[y];
					
					for(int x = 0; x < 640; x++) {
#ifdef _X1TURBOZ
						dest[x] = zpalette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#else
						dest[x] =  palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#endif
					}
#ifdef _X1TURBOZ
				}
#endif
			}
		}
		emu->screen_skip_line(false);
	} else {
#endif
		// 200 lines
		emu->set_vm_screen_lines(200);
		
		if(column40) {
			// 40 columns
			for(int y = 0; y + vt_ofs < 200; y++) {
				scrntype_t* dest0 = emu->get_screen_buffer((y + vt_ofs) * 2 + 0);
				scrntype_t* dest1 = emu->get_screen_buffer((y + vt_ofs) * 2 + 1);
				uint8_t* src_text = text[y];
#ifdef _X1TURBOZ
				if(aen_line[y]) {
					uint16_t* src_cg0 = zcg[0][y];
					uint16_t* src_cg1 = zcg[1][y];
					
					if(C64) {
						for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
							uint16_t cg00 = src_cg0[x];// | (src_cg0[x] >> 2);
							uint16_t cg11 = src_cg1[x];// | (src_cg1[x] >> 2);
							
							dest0[x2] = dest0[x2 + 1] = get_zpriority(src_text[x], cg00, cg11);
						}
					} else {
						for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
							uint16_t cg01 = src_cg0[x] | (src_cg1[x] >> 2);
							
							dest0[x2] = dest0[x2 + 1] = get_zpriority(src_text[x], cg01, cg01);
						}
					}
				} else {
#endif
					uint8_t* src_cg = cg[y];
					
					for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
#ifdef _X1TURBOZ
						dest0[x2] = dest0[x2 + 1] = zpalette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#else
						dest0[x2] = dest0[x2 + 1] =  palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#endif
					}
#ifdef _X1TURBOZ
				}
#endif
				if(!config.scan_line) {
					my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
				} else {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				}
			}
		} else {
			// 80 columns
			for(int y = 0; y + vt_ofs < 200; y++) {
				scrntype_t* dest0 = emu->get_screen_buffer((y + vt_ofs) * 2 + 0);
				scrntype_t* dest1 = emu->get_screen_buffer((y + vt_ofs) * 2 + 1);
				uint8_t* src_text = text[y];
#ifdef _X1TURBOZ
				if(aen_line[y]) {
					uint16_t* src_cg0 = zcg[0][y];
					
					for(int x = 0; x < 640; x++) {
						uint16_t cg00 = src_cg0[x];// | (src_cg0[x] >> 2);
						
						dest0[x] = get_zpriority(src_text[x], cg00, cg00);
					}
				} else {
#endif
					uint8_t* src_cg = cg[y];
					
					for(int x = 0; x < 640; x++) {
#ifdef _X1TURBOZ
						dest0[x] = zpalette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#else
						dest0[x] =  palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
#endif
					}
#ifdef _X1TURBOZ
				}
#endif
				if(!config.scan_line) {
					my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
				} else {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				}
			}
		}
		emu->screen_skip_line(true);
#ifdef _X1TURBO_FEATURE
	}
#endif
}

void DISPLAY::draw_text(int y)
{
	int width = column40 ? 40 : 80;
	uint16_t src = st_addr + hz_disp * y;
	
	bool cur_vert_double = true;
	uint8_t prev_attr = 0, prev_pattern_b[32], prev_pattern_r[32], prev_pattern_g[32];
	
	for(int x = 0; x < hz_disp && x < width; x++) {
		src &= 0x7ff;
		uint8_t code = vram_t[src];
#ifdef _X1TURBO_FEATURE
		uint8_t knj = vram_k[src];
#endif
		uint8_t attr = vram_a[src];
		src++;
		
		uint8_t col = attr & 7;
		bool reverse = ((attr & 8) != 0);
		bool blink = ((attr & 0x10) && (cblink & 0x20));
		reverse = (reverse != blink);
		
		// select pcg or ank
		const uint8_t *pattern_b, *pattern_r, *pattern_g;
#ifdef _X1TURBO_FEATURE
		int shift = 0;
		int max_line = 8;
#else
		#define max_line 8
#endif
		if(attr & 0x20) {
			// pcg
#ifdef _X1TURBO_FEATURE
			if(knj & 0x90) {
				pattern_b = gaiji_b[code >> 1];
				pattern_r = gaiji_r[code >> 1];
				pattern_g = gaiji_g[code >> 1];
				max_line = 16;
			} else {
#endif
				pattern_b = pcg_b[code];
				pattern_r = pcg_r[code];
				pattern_g = pcg_g[code];
#ifdef _X1TURBO_FEATURE
				shift = (mode1 & 1) ? 1 : 0;
			}
#endif
#ifdef _X1TURBO_FEATURE
		} else if(knj & 0x80) {
			uint32_t ofs = adr2knj_x1t((knj << 8) | code);
			if(knj & 0x40) {
				ofs += 16; // right
			}
			pattern_b = pattern_r = pattern_g = &kanji[ofs];
			max_line = 16;
		} else if((mode1 & 5) != 0) {
			// ank 8x16 or kanji
			pattern_b = pattern_r = pattern_g = &kanji[code << 4];
			max_line = 16;
#endif
		} else {
			// ank 8x8
			pattern_b = pattern_r = pattern_g = &font[code << 3];
		}
#ifdef _X1TURBO_FEATURE
		if(max_line == 16) {
			shift = ((mode1 & 5) == 5) ? 1 : ((mode1 & 5) != 0) ? 0 : -1;
		}
#endif
		
		// check vertical doubled char
		if(!(attr & 0x40)) {
			cur_vert_double = false;
		}
		
		// render character
		for(int l = 0; l < ch_height; l++) {
			uint8_t b, r, g;
			int line = cur_vert_double ? raster + (l >> 1) : l;
#ifdef _X1TURBO_FEATURE
			if(shift == 1) {
				line >>= 1;
			} else if(shift == -1) {
				line <<= 1;
				if(cur_vert_double) {
					line |= l & 1;
				}
			}
#endif
			if((x & 1) && (prev_attr & 0x80)) {
				b = prev_pattern_b[line] << 4;
				r = prev_pattern_r[line] << 4;
				g = prev_pattern_g[line] << 4;
			} else {
				b = prev_pattern_b[line] = pattern_b[line % max_line];
				r = prev_pattern_r[line] = pattern_r[line % max_line];
				g = prev_pattern_g[line] = pattern_g[line % max_line];
			}
			if(reverse) {
				b = (!(col & 1)) ? 0xff : ~b;
				r = (!(col & 2)) ? 0xff : ~r;
				g = (!(col & 4)) ? 0xff : ~g;
			} else {
				b = (!(col & 1)) ? 0 : b;
				r = (!(col & 2)) ? 0 : r;
				g = (!(col & 4)) ? 0 : g;
			}
			
			int yy = y * ch_height + l;
#ifdef _X1TURBO_FEATURE
			if(yy >= 400) {
#else
			if(yy >= 200) {
#endif
				break;
			}
			uint8_t* d = &text[yy][x << 3];
			
			if(attr & 0x80) {
				// horizontal doubled char
				d[ 0] = d[ 1] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
				d[ 2] = d[ 3] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
				d[ 4] = d[ 5] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
				d[ 6] = d[ 7] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			} else {
				d[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
				d[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
				d[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
				d[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
				d[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
				d[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
				d[6] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
				d[7] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
			}
		}
		prev_attr = attr;
	}
	if(cur_vert_double && !prev_vert_double) {
		prev_vert_double = true;
		raster = ch_height >> 1;
	} else {
		prev_vert_double = false;
		raster = 0;
	}
}

void DISPLAY::draw_cg(int line, int plane)
{
	int width = column40 ? 40 : 80;
	
	int y = line / ch_height;
	int l = line % ch_height;
	if(y >= vt_disp) {
		return;
	}
	int ofs, src = st_addr + hz_disp * y;
#ifdef _X1TURBO_FEATURE
	int page = (hireso && !(mode1 & 2)) ? (l & 1) : (mode1 & 8);
	int ll = hireso ? (l >> 1) : l;
	
	if(mode1 & 4) {
		ofs = (0x400 * (ll & 15));// + (page ? 0xc000 : 0);
	} else {
		ofs = (0x800 * (ll &  7));// + (page ? 0xc000 : 0);
	}
#else
	ofs = 0x800 * (l & 7);
#endif
#ifdef _X1TURBOZ
	if(AEN) {
/*
		HIRESO=0, WIDTH=40, C64=0: 320x200, 4096	PAGE0:(ADDR 000h-3FFh) + PAGE0:(ADDR 400h-7FFh) + PAGE1:(ADDR 000h-3FFh) + PAGE1:(ADDR 400h-7FFh)
		HIRESO=0, WIDTH=40, C64=1: 320x200, 64x2	PAGE0:(ADDR 000h-3FFh) + PAGE0:(ADDR 400h-7FFh) / PAGE1:(ADDR 000h-3FFh) + PAGE1:(ADDR 400h-7FFh)
		HIRESO=1, WIDTH=40, C64=*: 320x200, 64		PAGE*:(ADDR 000h-3FFh) + PAGE*:(ADDR 400h-7FFh)
		HIRESO=0, WIDTH=80, C64=*: 640x200, 64		PAGE0:(ADDR 000h-7FFh) + PAGE1:(ADDR 000h-7FFh)
		HIRESO=1, WIDTH=80, C64=*: 640x200, 8		PAGE0:(ADDR 000h-7FFh) + PAGE0:(ADDR 000h-7FFh)

		HIRESO=0, WIDTH=40, C64=1: 320x200, 64x2
				mode1	zpriority	
		SCREEN 0	00	00		PAGE0
		SCREEM 2	18	08		PAGE1
		SCREEN 4	00	10		PAGE0 > PAGE1
		SCREEN 6	18	18		PAGE0 < PAGE1
*/
		if(!hireso) {
			if(column40) {
				if(C64 && !(zpriority & 0x10)) {
					if(plane) {
						my_memcpy(zcg[plane][1], zcg[plane][0], sizeof(uint16_t) * 640);
						return;
					}
				} else {
					page = plane;
				}
			} else {
				page = 0;
			}
		}
		if(page) {
			ofs += 0xc000;
		}
		int ofs_b0 = ofs + 0x0000;
		int ofs_r0 = ofs + 0x4000;
		int ofs_g0 = ofs + 0x8000;
		int ofs_b1 = column40 ? (ofs_b0 ^ 0x400) : hireso ? ofs_b0 : (ofs_b0 + 0xc000);
		int ofs_r1 = column40 ? (ofs_r0 ^ 0x400) : hireso ? ofs_r0 : (ofs_r0 + 0xc000);
		int ofs_g1 = column40 ? (ofs_g0 ^ 0x400) : hireso ? ofs_g0 : (ofs_g0 + 0xc000);
		
		for(int x = 0; x < hz_disp && x < width; x++) {
			src &= column40 ? 0x3ff : 0x7ff;
			uint16_t b0 = vram_ptr[ofs_b0 | src];
			uint16_t r0 = vram_ptr[ofs_r0 | src];
			uint16_t g0 = vram_ptr[ofs_g0 | src];
			uint16_t b1 = vram_ptr[ofs_b1 | src];
			uint16_t r1 = vram_ptr[ofs_r1 | src];
			uint16_t g1 = vram_ptr[ofs_g1 | src++];
			uint16_t* d = &zcg[plane][line][x << 3];
			
			// MSB <- G0,G1,0,0, R0,R1,0,0, B0,B1,0,0 -> LSB
			d[0] = ((b0 & 0x80) >> 4) | ((b1 & 0x80) >> 5) | ((r0 & 0x80) >> 0) | ((r1 & 0x80) >> 1) | ((g0 & 0x80) <<  4) | ((g1 & 0x80) <<  3);
			d[1] = ((b0 & 0x40) >> 3) | ((b1 & 0x40) >> 4) | ((r0 & 0x40) << 1) | ((r1 & 0x40) >> 0) | ((g0 & 0x40) <<  5) | ((g1 & 0x40) <<  4);
			d[2] = ((b0 & 0x20) >> 2) | ((b1 & 0x20) >> 3) | ((r0 & 0x20) << 2) | ((r1 & 0x20) << 1) | ((g0 & 0x20) <<  6) | ((g1 & 0x20) <<  5);
			d[3] = ((b0 & 0x10) >> 1) | ((b1 & 0x10) >> 2) | ((r0 & 0x10) << 3) | ((r1 & 0x10) << 2) | ((g0 & 0x10) <<  7) | ((g1 & 0x10) <<  6);
			d[4] = ((b0 & 0x08) >> 0) | ((b1 & 0x08) >> 1) | ((r0 & 0x08) << 4) | ((r1 & 0x08) << 3) | ((g0 & 0x08) <<  8) | ((g1 & 0x08) <<  7);
			d[5] = ((b0 & 0x04) << 1) | ((b1 & 0x04) >> 0) | ((r0 & 0x04) << 5) | ((r1 & 0x04) << 4) | ((g0 & 0x04) <<  9) | ((g1 & 0x04) <<  8);
			d[6] = ((b0 & 0x02) << 2) | ((b1 & 0x02) << 1) | ((r0 & 0x02) << 6) | ((r1 & 0x02) << 5) | ((g0 & 0x02) << 10) | ((g1 & 0x02) <<  9);
			d[7] = ((b0 & 0x01) << 3) | ((b1 & 0x01) << 2) | ((r0 & 0x01) << 7) | ((r1 & 0x01) << 6) | ((g0 & 0x01) << 11) | ((g1 & 0x01) << 10);
		}
#if 1
		// zpriorityで処理すると、プレーンの取得に遅延がでるので
		// ここで２ビット下へ移動してしまおう
		if(!hireso && column40 && C64 && page) {
			for(int x = 0; x < hz_disp && x < width; x++) {
				uint16_t* d = &zcg[plane][line][x << 3];
				
				// MSB <- G0,G1,0,0, R0,R1,0,0, B0,B1,0,0 -> LSB
				d[0] >>= 2;
				d[1] >>= 2;
				d[2] >>= 2;
				d[3] >>= 2;
				d[4] >>= 2;
				d[5] >>= 2;
				d[6] >>= 2;
				d[7] >>= 2;
			}
		}
#endif
	} else
#endif
	{
#ifdef _X1TURBO_FEATURE
		if(page) {
			ofs += 0xc000;
		}
#endif
		int ofs_b = ofs + 0x0000;
		int ofs_r = ofs + 0x4000;
		int ofs_g = ofs + 0x8000;
		
		for(int x = 0; x < hz_disp && x < width; x++) {
			src &= 0x7ff;
			uint8_t b = vram_ptr[ofs_b | src];
			uint8_t r = vram_ptr[ofs_r | src];
			uint8_t g = vram_ptr[ofs_g | src++];
			uint8_t* d = &cg[line][x << 3];
			
			d[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			d[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
			d[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
			d[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			d[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
			d[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
			d[6] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
			d[7] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
		}
	}
}

#ifdef _X1TURBOZ
int DISPLAY::get_zpal_num(uint32_t addr, uint32_t data)
{
	int num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
	
#if 1
	if(hireso) {
		if(!column40) {
			// 8 colors (use asic palette ram)
			num &= 0x888;
			num |= num >> 1;
			num |= num >> 2;
		} else {
			// 64 colors (single set)
			num &= 0xccc;
			num |= num >> 2;
		}
	} else {
		if(!column40 || C64) {
			// 64 colors (dual set)
			num &= 0xccc;  // BANK0 GRAM
			if(C64 && (mode1 & 0x10)) {
				num >>=2;  // BANK1 GRAM
			}
		}
	}
#else
	if(hireso && !column40) {
		// 8 colors
		num &= 0x888;
		num |= num >> 1;
		num |= num >> 2;
	} else if(!(!hireso && column40 && !C64)) {
		// 64 colors
		num &= 0xccc;
		num |= num >> 2;
	}
#endif
	return num;
}

void DISPLAY::update_zpalette()
{
	memcpy(zpalette_pc, zpalette_tmp, sizeof(zpalette_pc));
	zpalette_pc[8 + 0] = zpalette_pc[16 + 0x000];
	zpalette_pc[8 + 1] = zpalette_pc[16 + 0x00f];
	zpalette_pc[8 + 2] = zpalette_pc[16 + 0x0f0];
	zpalette_pc[8 + 3] = zpalette_pc[16 + 0x0ff];
	zpalette_pc[8 + 4] = zpalette_pc[16 + 0xf00];
	zpalette_pc[8 + 5] = zpalette_pc[16 + 0xf0f];
	zpalette_pc[8 + 6] = zpalette_pc[16 + 0xff0];
	zpalette_pc[8 + 7] = zpalette_pc[16 + 0xfff];
}

scrntype_t DISPLAY::get_zpriority(uint8_t text, uint16_t cg0, uint16_t cg1)
{
	if((mode2 & 8) && (text == (mode2 & 7))) {
		int digital = ((cg0 >> 9) & 4) | ((cg0 >> 6) & 2) | ((cg0 >> 3) & 1);
		if(!(priority & (1 << digital))) {
			return 0;
		}
	}
	uint16_t fore = ((zpriority & 0x18) != 0x18) ? cg0 : cg1;
	uint16_t back = ((zpriority & 0x18) != 0x18) ? cg1 : cg0;
	uint16_t disp;
	
	switch(zpriority & 0x13) {
	case 0x00:
	case 0x02:
		disp = text ? text : (fore + 16);
		break;
	case 0x01:
	case 0x03:
		disp = fore ? (fore + 16) : text;
		break;
	case 0x10:
		disp = text ? text : fore ? (fore + 16) : (back + 16);
		break;
	case 0x11:
		disp = fore ? (fore + 16) : back ? (back + 16) : text;
		break;
	case 0x12:
		disp = fore ? (fore + 16) : text ? text : (back + 16);
		break;
	default: // undefined case :-(
		disp = text ? text : fore ? (fore + 16) : (back + 16);
		break;
	}
//	if((mode2 & 0x10) && disp == (0x000 + 16)) {
//		return 0;
//	}
//	if((mode2 & 0x20) && disp == (0x00f + 16)) {
//		return 0;
//	}
	return zpalette_pc[disp];
}
#endif

// kanji rom (from X1EMU by KM)

void DISPLAY::write_kanji(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0xe80:
		kaddr = (kaddr & 0xff00) | data;
		break;
	case 0xe81:
		kaddr = (kaddr & 0xff) | (data << 8);
		break;
	case 0xe82:
		// TODO: bit0 L->H: Latch
		kanji_ptr = &kanji[adr2knj_x1(kaddr & 0xfff0)];
		break;
	}
}

uint32_t DISPLAY::read_kanji(uint32_t addr)
{
	switch(addr) {
	case 0xe80:
		if(kaddr & 0xff00) {
			uint32_t val = kanji_ptr[kofs];
			kflag |= 1;
			if(kflag == 3) {
				kofs = (kofs + 1) & 15;
				kflag = 0;
			}
			return val;
		}
		return jis2adr_x1(kaddr << 8) >> 8;
	case 0xe81:
		if(kaddr & 0xff00) {
			uint32_t val = kanji_ptr[kofs + 16];
			kflag |= 2;
			if(kflag == 3) {
				kofs = (kofs + 1) & 15;
				kflag = 0;
			}
			return val;
		}
		return 0;
	}
	return 0xff;
}

uint16_t DISPLAY::jis2adr_x1(uint16_t jis)
{
	uint16_t jh, jl, adr;
	
	jh = jis >> 8;
	jl = jis & 0xff;
	if(jh > 0x28) {
		adr = 0x4000 + (jh - 0x30) * 0x600;
	} else {
		adr = 0x0100 + (jh - 0x21) * 0x600;
	}
	if(jl >= 0x20) {
		adr += (jl - 0x20) * 0x10;
	}
	return adr;
}

uint32_t DISPLAY::adr2knj_x1(uint16_t adr)
{
	uint16_t jh, jl, jis;
	
	if(adr < 0x4000) {
		jh = adr - 0x0100;
		jh = 0x21 + jh / 0x600;
	} else {
		jh = adr - 0x4000;
		jh = 0x30 + jh / 0x600;
	}
	if(jh > 0x28) {
		adr -= 0x4000 + (jh - 0x30) * 0x600;
	} else {
		adr -= 0x0100 + (jh - 0x21) * 0x600;
	}
	jl = 0x20;
	if(adr) {
		jl += adr / 0x10;
	}
	
	jis = (jh << 8) | jl;
	return jis2knj(jis);
}

#ifdef _X1TURBO_FEATURE
uint32_t DISPLAY::adr2knj_x1t(uint16_t adr)
{
	uint16_t j1, j2;
	uint16_t rl, rh;
	uint16_t jis;
	
	rh = adr >> 8;
	rl = adr & 0xff;
	
	rh &= 0x1f;
	if(!rl && !rh) {
		return jis2knj(0);
	}
	j2 = rl & 0x1f;		// rl4,3,2,1,0
	j1 = (rl / 0x20) & 7;	// rl7,6,5
	
	if(rh < 0x04) {
		// 2121-277e
		j1 |= 0x20;
		switch(rh & 3) {
		case 0: j2 |= 0x20; break;
		case 1: j2 |= 0x60; break;
		case 2: j2 |= 0x40; break;
		default: j1 = j2 = 0; break;
		}
	} else if(rh > 0x1c) {
		// 7021-777e
		j1 |= 0x70;
		switch(rh & 3) {
		case 0: j2 |= 0x20; break;
		case 1: j2 |= 0x60; break;
		case 2: j2 |= 0x40; break;
		default: j1 = j2 = 0; break;
		}
	} else {
		j1 |= (((rh >> 1) + 7) / 3) * 0x10;
		j1 |= (rh & 1) * 8;
		j2 |= ((((rh >> 1) + 1) % 3) + 1) * 0x20;
	}
	
	jis = (j1 << 8) | j2;
	return jis2knj(jis);
}
#endif

uint32_t DISPLAY::jis2knj(uint16_t jis)
{
	uint32_t sjis = jis2sjis(jis);
	
	if(sjis < 0x100) {
		return sjis * 16;
	} else if(sjis >= 0x8140 && sjis < 0x84c0) {
		return 0x01000 + (sjis - 0x8140) * 32;
	} else if(sjis >= 0x8890 && sjis < 0xa000) {
		return 0x08000 + (sjis - 0x8890) * 32;
	} else if(sjis >= 0xe040 && sjis < 0xeab0) {
		return 0x36e00 + (sjis - 0xe040) * 32;
	} else {
		return 0;
	}
}

uint16_t DISPLAY::jis2sjis(uint16_t jis)
{
	uint16_t c1, c2;
	
	if(!jis) {
		return 0;
	}
	c1 = jis >> 8;
	c2 = jis & 0xff;
	
	if(c1 & 1) {
		c2 += 0x1f;
		if(c2 >= 0x7f) {
			c2++;
		}
	} else {
		c2 += 0x7e;
	}
	c1 = (c1 - 0x20 - 1) / 2 + 0x81;
	if(c1 >= 0xa0) {
		c1 += 0x40;
	}
	return (c1 << 8) | c2;
}

#define STATE_VERSION	5

bool DISPLAY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(vram_t, sizeof(vram_t), 1);
	state_fio->StateArray(vram_a, sizeof(vram_a), 1);
#ifdef _X1TURBO_FEATURE
	state_fio->StateArray(vram_k, sizeof(vram_k), 1);
#endif
	state_fio->StateArray(&pcg_b[0][0], sizeof(pcg_b), 1);
	state_fio->StateArray(&pcg_r[0][0], sizeof(pcg_r), 1);
	state_fio->StateArray(&pcg_g[0][0], sizeof(pcg_g), 1);
#ifdef _X1TURBO_FEATURE
	state_fio->StateArray(&gaiji_b[0][0], sizeof(gaiji_b), 1);
	state_fio->StateArray(&gaiji_r[0][0], sizeof(gaiji_r), 1);
	state_fio->StateArray(&gaiji_g[0][0], sizeof(gaiji_g), 1);
#endif
	state_fio->StateValue(cur_code);
	state_fio->StateValue(cur_line);
	state_fio->StateValue(kaddr);
	state_fio->StateValue(kofs);
	state_fio->StateValue(kflag);
	if(loading) {
		kanji_ptr = &kanji[0] + state_fio->FgetInt32_LE();
	} else {
		state_fio->FputInt32_LE((int)(kanji_ptr - &kanji[0]));
	}
	state_fio->StateArray(pal, sizeof(pal), 1);
	state_fio->StateValue(priority);
	state_fio->StateArray(&pri[0][0], sizeof(pri), 1);
	state_fio->StateValue(column40);
#ifdef _X1TURBO_FEATURE
	state_fio->StateValue(mode1);
	state_fio->StateValue(mode2);
	state_fio->StateValue(hireso);
#endif
#ifdef _X1TURBOZ
	state_fio->StateValue(zmode1);
	state_fio->StateValue(zpriority);
	state_fio->StateValue(zadjust);
	state_fio->StateValue(zmosaic);
	state_fio->StateValue(zchromakey);
	state_fio->StateValue(zscroll);
	state_fio->StateValue(zmode2);
	state_fio->StateArray(ztpal, sizeof(ztpal), 1);
	for(int i = 0; i < array_length(zpal); i++){
		state_fio->StateValue(zpal[i].r);
		state_fio->StateValue(zpal[i].g);
		state_fio->StateValue(zpal[i].b);
	}
	state_fio->StateValue(zpal_num);
	state_fio->StateArray(zpalette_tmp, sizeof(zpalette_tmp), 1);
#endif
	state_fio->StateValue(prev_vert_double);
	state_fio->StateValue(raster);
	state_fio->StateValue(cblink);
	state_fio->StateValue(ch_height);
	state_fio->StateValue(hz_total);
	state_fio->StateValue(hz_disp);
	state_fio->StateValue(vt_disp);
	state_fio->StateValue(st_addr);
	state_fio->StateValue(vblank_clock);
	state_fio->StateValue(cur_blank);
	
	// post process
	if(loading) {
#ifdef _X1TURBOZ
		zpalette_changed = true;
#endif
		update_crtc(); // force update timing
	}
	return true;
}

