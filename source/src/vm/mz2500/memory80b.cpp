/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	I-O DATA PIO-3039 Emulator

	Author : Mr.Suga
	Data   : 2017.02.22-

	[ memory/crtc ]
*/

#include "memory80b.h"
#include "../i8255.h"
#include "../z80.h"

#define EVENT_HBLANK		0

#ifdef _MZ80B
#define MONITOR_TYPE_GREEN		0
#define MONITOR_TYPE_COLOR		1
#define MONITOR_TYPE_GREEN_COLOR	2
#define MONITOR_TYPE_COLOR_GREEN	3
#else
#define MONITOR_TYPE_COLOR		0
#define MONITOR_TYPE_GREEN		1
#define MONITOR_TYPE_COLOR_GREEN	2
#define MONITOR_TYPE_GREEN_COLOR	3
#endif

#define SET_BANK(s, e, w, r, v) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
		is_vram[i] = v; \
	} \
}

void MEMORY::initialize()
{
	// memory
	memset(rdmy, 0xff, sizeof(rdmy));
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(tvram, 0, sizeof(tvram));
	memset(ipl, 0xff, sizeof(ipl));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	vram_sel = vram_page = 0;
	
	// crtc
	back_color = 0;
	text_color = vram_mask = 7;
	width80 = reverse = false;
	
#ifndef _MZ80B
	for(int i = 0; i < 8; i++) {
		palette_color[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}
	update_green_palette();
#else
	for(int i = 0; i < 8; i++) {
		pio3039_color[i] = RGB_COLOR((i & 1) ? 255 : 0, (i & 2) ? 255 : 0, (i & 4) ? 255 : 0);	// BGR
		pio3039_palette[i] = (i > 3) ? 7 : 0;
	}
	pio3039_data = 0;
	pio3039_txt_sw = true;
	
	palette_green[0] = RGB_COLOR(0, 0, 0);
	palette_green[1] = RGB_COLOR(0, 255, 0);
#endif
	register_vline_event(this);
}

void MEMORY::reset()
{
	// ipl reset
	SET_BANK(0x0000, 0x07ff, wdmy, ipl, false);
	SET_BANK(0x0800, 0x7fff, wdmy, rdmy, false);
	SET_BANK(0x8000, 0xffff, ram, ram, false);
	
	ipl_selected = true;
	update_vram_map();
}

void MEMORY::special_reset()
{
	// reset
	SET_BANK(0x0000, 0xffff, ram, ram, false);
	
	ipl_selected = false;
	update_vram_map();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(!hblank && is_vram[addr >> 11]) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if(!hblank && is_vram[addr >> 11]) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	return rbank[addr >> 11][addr & 0x7ff];
}

uint32_t MEMORY::fetch_op(uint32_t addr, int *wait)
{
	addr &= 0xffff;
	*wait = (ipl_selected && addr < 0x800);
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
#ifndef _MZ80B
	case 0xf4:
		back_color = data & 7;
		break;
	case 0xf5:
		text_color = data;
		break;
	case 0xf6:
		vram_mask = data;
		break;
	case 0xf7:
		if(vram_page != (data & 3)) {
			vram_page = data & 3;
			if(vram_sel == 0x80) {
				update_vram_map();
			}
		}
		break;
#else
	case 0xb4:
		// PIO-3039
		pio3039_data = (uint8_t)data;
		if(pio3039_data & 0x40) {
			// 2^6bit equ 1
			if(pio3039_data & 0x80) {
				// 2^7bit equ 1
				// character color screen : display
				pio3039_txt_sw = true;
			} else {
				// 2^7bit equ 0
				// character color screen : hide
				pio3039_txt_sw = false;
			}
		} else {
			// 2^6bit equ 0
			// Look up table set
			pio3039_palette[pio3039_data & 0x07] = (pio3039_data >> 3) & 0x07;
		}
		break;
	case 0xf4:
	case 0xf5:
	case 0xf6:
	case 0xf7:
		if(vram_page != (data & 7)) {
			uint8_t prev_page = vram_page;
			vram_page = data & 7;
			if((prev_page & 1) != (vram_page & 1) && (vram_sel == 0x80 || vram_sel == 0xc0)) {
				update_vram_map();
			}
		}
		break;
#endif
	}
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_VRAM_SEL) {
		if(vram_sel != (data & mask)) {
			vram_sel = data & mask;
			update_vram_map();
		}
	} else if(id == SIG_CRTC_WIDTH80) {
		width80 = ((data & mask) != 0);
	} else if(id == SIG_CRTC_REVERSE) {
		reverse = ((data & mask) == 0);
#ifndef _MZ80B
		update_green_palette();
#endif
	} else if(id == SIG_CRTC_VGATE) {
		vgate = ((data & mask) != 0);
	}
}

void MEMORY::event_vline(int v, int clock)
{
	if(v == 0) {
		d_pio->write_signal(SIG_I8255_PORT_B, 1, 1);
	} else if(v == 200) {
		d_pio->write_signal(SIG_I8255_PORT_B, 0, 1);
	}
//	if(v < 200) {
		hblank = false;
		register_event_by_clock(this, EVENT_HBLANK, 184, false, NULL);
//	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_HBLANK) {
		hblank = true;
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
	}
}

#ifndef _MZ80B
void MEMORY::update_green_palette()
{
	palette_green[reverse ? 1 : 0] = RGB_COLOR(0, 0, 0);
	palette_green[reverse ? 0 : 1] = RGB_COLOR(0, 255, 0);
}
#endif

void MEMORY::update_vram_map()
{
#ifndef _MZ80B
	if(vram_sel == 0x80) {
		if(vram_page) {
			SET_BANK(0xc000, 0xffff, vram + 0x4000 * vram_page, vram + 0x4000 * vram_page, true);
		} else {
			SET_BANK(0xc000, 0xffff, wdmy, rdmy, false);
		}
	} else {
		if(ipl_selected) {
			SET_BANK(0xc000, 0xffff, ram + 0x4000, ram + 0x4000, false);
		} else {
			SET_BANK(0xc000, 0xffff, ram + 0xc000, ram + 0xc000, false);
		}
		if(vram_sel == 0xc0) {
			SET_BANK(0xd000, 0xdfff, tvram, tvram, true);
		}
	}
#else
	if(ipl_selected) {
		SET_BANK(0x5000, 0x7fff, wdmy, rdmy, false);
		SET_BANK(0xd000, 0xffff, ram + 0x5000, ram + 0x5000, false);
	} else {
		SET_BANK(0x5000, 0x7fff, ram + 0x5000, ram + 0x5000, false);
		SET_BANK(0xd000, 0xffff, ram + 0xd000, ram + 0xd000, false);
	}
	if(vram_sel == 0x80) {
		SET_BANK(0xd000, 0xdfff, tvram, tvram, true);
		SET_BANK(0xe000, 0xffff, vram + 0x4000 * (vram_page & 1), vram + 0x4000 * (vram_page & 1), true);
	} else if(vram_sel == 0xc0) {
		SET_BANK(0x5000, 0x5fff, tvram, tvram, true);
		SET_BANK(0x6000, 0x7fff, vram + 0x4000 * (vram_page & 1), vram + 0x4000 * (vram_page & 1), true);
	}
#endif
}

void MEMORY::load_dat_image(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(ram, 0, sizeof(ram));
		memset(vram, 0, sizeof(vram));
		memset(tvram, 0, sizeof(tvram));
		
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
		vm->special_reset();
	}
	delete fio;
}

bool MEMORY::load_mzt_image(const _TCHAR* file_path)
{
	bool result = false;
	bool is_mtw = check_file_extension(file_path, _T(".mtw"));
	
	if(is_mtw || ipl_selected) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
			uint8_t header[128];
			fio->Fread(header, sizeof(header), 1);
			uint16_t size = header[0x12] | (header[0x13] << 8);
			uint16_t offs = header[0x14] | (header[0x15] << 8);
			
//			if(header[0] == 0x01 && (is_mtw || size > 0x7e00)) {
			if(header[0] == 0x01 && offs == 0) {
				memset(ram, 0, sizeof(ram));
				memset(vram, 0, sizeof(vram));
				memset(tvram, 0, sizeof(tvram));
				
				fio->Fread(ram, size, 1);
				vm->special_reset();
				result = true;
			}
			fio->Fclose();
		}
		delete fio;
	}
	return result;
}

void MEMORY::draw_screen()
{
	// render text
#ifndef _MZ80B
	uint8_t color = (text_color & 7) ? (text_color & 7) : 8;
#else
	#define color 1
#endif
	for(int y = 0, addr = 0; y < 200; y += 8) {
		for(int x = 0; x < (width80 ? 80 : 40); x++) {
			uint8_t code = tvram[addr++];
			for(int l = 0; l < 8; l++) {
				uint8_t pat = font[(code << 3) + l];
				uint8_t* d = &screen_txt[y + l][x << 3];
				
				d[0] = (pat & 0x80) ? color : 0;
				d[1] = (pat & 0x40) ? color : 0;
				d[2] = (pat & 0x20) ? color : 0;
				d[3] = (pat & 0x10) ? color : 0;
				d[4] = (pat & 0x08) ? color : 0;
				d[5] = (pat & 0x04) ? color : 0;
				d[6] = (pat & 0x02) ? color : 0;
				d[7] = (pat & 0x01) ? color : 0;
			}
		}
	}
	
	// render graphics
#ifndef _MZ80B
//	if(config.monitor_type != MONITOR_TYPE_COLOR && (vram_mask & 8)) {
//		memset(screen_gra, 0, sizeof(screen_gra));
//	} else {
		// vram[0x0000-0x3fff] should be always blank
		uint8_t *vram_b = vram + ((vram_mask & 1) ? 0x4000 : 0);
		uint8_t *vram_r = vram + ((vram_mask & 2) ? 0x8000 : 0);
		uint8_t *vram_g = vram + ((vram_mask & 4) ? 0xc000 : 0);
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0; x < 80; x++) {
				uint8_t b = vram_b[addr];
				uint8_t r = vram_r[addr];
				uint8_t g = vram_g[addr];
				addr++;
				uint8_t* d = &screen_gra[y][x << 3];
				
				d[0] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
				d[1] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
				d[2] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
				d[3] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
				d[4] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
				d[5] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
				d[6] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
				d[7] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
			}
		}
//	}
#else
	if(!(vram_page & 6)) {
		memset(screen_gra, 0, sizeof(screen_gra));
	} else {
		// vram[0x8000-0xbfff] should be always blank
		uint8_t *vram_1 = vram + ((vram_page & 2) ? 0x0000 : 0x8000);
		uint8_t *vram_2 = vram + ((vram_page & 4) ? 0x4000 : 0x8000);
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0; x < 40; x++) {
				uint8_t pat = vram_1[addr] | vram_2[addr];
				addr++;
				uint8_t* d = &screen_gra[y][x << 3];
				
				d[0] = (pat & 0x01) >> 0;
				d[1] = (pat & 0x02) >> 1;
				d[2] = (pat & 0x04) >> 2;
				d[3] = (pat & 0x08) >> 3;
				d[4] = (pat & 0x10) >> 4;
				d[5] = (pat & 0x20) >> 5;
				d[6] = (pat & 0x40) >> 6;
				d[7] = (pat & 0x80) >> 7;
			}
		}
	}
	
	// 80B built-in monitor (PIO-3039 character)
	for(int y = 0; y < 200; y++) {
		uint8_t* dest0 = screen_80b_green[y];
		uint8_t* src_txt = screen_txt[y];
		uint8_t* src_gra = screen_gra[y];
		
		// VGATE (Forces display to be blank)
		if(vgate) {
			for(int x = 0; x < 640; x++) {
				dest0[x] =  reverse ? 1: 0;
			}
		} else {
			if(width80) {
				for(int x = 0; x < 640; x++) {
					uint8_t txt = src_txt[x], gra = src_gra[x >> 1];
					if(txt | gra) {
						dest0[x] = reverse ? 0: 1;
					} else {
						dest0[x] = reverse ? 1: 0;
					}
				}
			} else {
				for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
					uint8_t txt = src_txt[x], gra = src_gra[x];
					if(txt | gra) {
						dest0[x2] = dest0[x2 + 1] = reverse ? 0: 1;
					} else {
						dest0[x2] = dest0[x2 + 1] = reverse ? 1: 0;
					}
				}
			}
		}
	}
	
	// PIO-3039 GRAPHIC 1,2
	uint8_t *gvram_1 = vram + 0x0000;
	uint8_t *gvram_2 = vram + 0x4000;
	
	for(int y = 0, addr = 0; y < 200; y++) {
		for(int x = 0; x < 40; x++) {
			uint8_t pat1 = gvram_1[addr];
			uint8_t pat2 = gvram_2[addr];
			addr++;
			
			uint8_t* d1 = &screen_80b_vram1[y][x << 4];
			uint8_t* d2 = &screen_80b_vram2[y][x << 4];
			
			d1[0]  = d1[1]  = (pat1 & 0x01) >> 0;
			d1[2]  = d1[3]  = (pat1 & 0x02) >> 1;
			d1[4]  = d1[5]  = (pat1 & 0x04) >> 2;
			d1[6]  = d1[7]  = (pat1 & 0x08) >> 3;
			d1[8]  = d1[9]  = (pat1 & 0x10) >> 4;
			d1[10] = d1[11] = (pat1 & 0x20) >> 5;
			d1[12] = d1[13] = (pat1 & 0x40) >> 6;
			d1[14] = d1[15] = (pat1 & 0x80) >> 7;
			
			d2[0]  = d2[1]  = (pat2 & 0x01) >> 0;
			d2[2]  = d2[3]  = (pat2 & 0x02) >> 1;
			d2[4]  = d2[5]  = (pat2 & 0x04) >> 2;
			d2[6]  = d2[7]  = (pat2 & 0x08) >> 3;
			d2[8]  = d2[9]  = (pat2 & 0x10) >> 4;
			d2[10] = d2[11] = (pat2 & 0x20) >> 5;
			d2[12] = d2[13] = (pat2 & 0x40) >> 6;
			d2[14] = d2[15] = (pat2 & 0x80) >> 7;
		}
	}
#endif
	
	// copy to real screen
	if(config.monitor_type == MONITOR_TYPE_COLOR_GREEN || config.monitor_type == MONITOR_TYPE_GREEN_COLOR) {
		emu->set_vm_screen_size(1280, 400, 1280, 400, 1280, 480);
	} else {
		emu->set_vm_screen_size(640, 400, 640, 400, 640, 480);
	}
	
#ifndef _MZ80B
	if(config.monitor_type != MONITOR_TYPE_GREEN) {
		// color monitor
		int offset = (config.monitor_type == MONITOR_TYPE_GREEN_COLOR) ? 640 : 0;
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0) + offset;
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1) + offset;
			uint8_t* src_txt = screen_txt[y];
			uint8_t* src_gra = screen_gra[y];
			
			// VGATE (Forces display to be blank) or Reverse
			if(vgate || reverse) {
				for(int x = 0; x < 640; x++) {
					dest0[x] = 0;
				}
			} else {
				if(text_color & 8) {
					// graphics > text
					for(int x = 0; x < 640; x++) {
						uint8_t txt = src_txt[width80 ? x : (x >> 1)], gra = src_gra[x];
						dest0[x] = palette_color[gra ? gra : txt ? (txt & 7) : back_color];
					}
				} else {
					// text > graphics
					for(int x = 0; x < 640; x++) {
						uint8_t txt = src_txt[width80 ? x : (x >> 1)], gra = src_gra[x];
						dest0[x] = palette_color[txt ? (txt & 7) : gra ? gra : back_color];
					}
				}
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype_t));
			} else {
				my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
			}
		}
	}
	if(config.monitor_type != MONITOR_TYPE_COLOR) {
		// green monitor
		int offset = (config.monitor_type == MONITOR_TYPE_COLOR_GREEN) ? 640 : 0;
		
		// VGATE (Forces display to be blank)
		if(vgate) {
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0) + offset;
				scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1) + offset;
				for(int x = 0; x < 640; x++) {
					dest0[x] = palette_green[0];
				}
				if(config.scan_line) {
					memset(dest1, 0, 640 * sizeof(scrntype_t));
				} else {
					my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
				}
			}
		} else {
			if(vram_mask & 8) {
				// text only
				for(int y = 0; y < 200; y++) {
					scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0) + offset;
					scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1) + offset;
					uint8_t* src_txt = screen_txt[y];
					
					for(int x = 0; x < 640; x++) {
						uint8_t txt = src_txt[width80 ? x : (x >> 1)];
						dest0[x] = palette_green[txt ? 1 : 0];
					}
					if(config.scan_line) {
						memset(dest1, 0, 640 * sizeof(scrntype_t));
					} else {
						my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
					}
				}
			} else {
				// both text and graphic
				for(int y = 0; y < 200; y++) {
					scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0) + offset;
					scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1) + offset;
					uint8_t* src_txt = screen_txt[y];
					uint8_t* src_gra = screen_gra[y];
					
					for(int x = 0; x < 640; x++) {
						uint8_t txt = src_txt[width80 ? x : (x >> 1)], gra = src_gra[x];
						dest0[x] = palette_green[(txt || gra) ? 1 : 0];
					}
					if(config.scan_line) {
						memset(dest1, 0, 640 * sizeof(scrntype_t));
					} else {
						my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
					}
				}
			}
		}
	}
	emu->screen_skip_line(true);
	emu->set_vm_screen_lines(200);
#else
	if(config.monitor_type != MONITOR_TYPE_GREEN) {
		// PIO-3039 color monitor
		int offset = (config.monitor_type == MONITOR_TYPE_GREEN_COLOR) ? 640 : 0;
		emu->set_vm_screen_lines(200);
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0) + offset;
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1) + offset;
			uint8_t* v1 = screen_80b_vram1[y];
			uint8_t* v2 = screen_80b_vram2[y];
			uint8_t* gr = screen_80b_green[y];
			
			for(int x = 0; x < 640; x++) {
				dest0[x] = pio3039_color[pio3039_palette[(pio3039_txt_sw ? (gr[x] ? 4 : 0) : 0) + (v2[x] ? 2 : 0) + (v1[x] ? 1 : 0)]];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype_t));
			} else {
				my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
			}
		}
	}
	if(config.monitor_type != MONITOR_TYPE_COLOR) {
		// green monitor
		int offset = (config.monitor_type == MONITOR_TYPE_COLOR_GREEN) ? 640 : 0;
		emu->set_vm_screen_lines(200);
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest0 = emu->get_screen_buffer(y * 2 + 0) + offset;
			scrntype_t* dest1 = emu->get_screen_buffer(y * 2 + 1) + offset;
			uint8_t* src_80b = screen_80b_green[y];
			
			for(int x = 0; x < 640; x++) {
				dest0[x] = palette_green[src_80b[x] ? 1 : 0];
			}
			if(config.scan_line) {
				memset(dest1, 0, 640 * sizeof(scrntype_t));
			} else {
				my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
			}
		}
	}
#endif
}

#define STATE_VERSION	3

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateArray(tvram, sizeof(tvram), 1);
	state_fio->StateValue(ipl_selected);
	state_fio->StateValue(vram_sel);
	state_fio->StateValue(vram_page);
	state_fio->StateValue(back_color);
	state_fio->StateValue(text_color);
	state_fio->StateValue(vram_mask);
	state_fio->StateValue(width80);
	state_fio->StateValue(reverse);
	state_fio->StateValue(vgate);
	state_fio->StateValue(hblank);
#ifdef _MZ80B
	state_fio->StateArray(pio3039_palette, sizeof(pio3039_palette), 1);
	state_fio->StateValue(pio3039_txt_sw);
	state_fio->StateValue(pio3039_data);
#endif
	
	// post process
	if(loading) {
		if(ipl_selected) {
			SET_BANK(0x0000, 0x07ff, wdmy, ipl, false);
			SET_BANK(0x0800, 0x7fff, wdmy, rdmy, false);
			SET_BANK(0x8000, 0xffff, ram, ram, false);
		} else {
			SET_BANK(0x0000, 0xffff, ram, ram, false);
		}
		update_vram_map();
#ifndef _MZ80B
		update_green_palette();
#endif
	}
	return true;
}

