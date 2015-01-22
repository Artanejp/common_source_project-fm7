/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	[ memory/crtc ]
*/

#include "memory80b.h"
#include "../i8255.h"
#include "../z80.h"
#include "../../fileio.h"

#define EVENT_HBLANK		0

#define MONITOR_TYPE_COLOR	0
#define MONITOR_TYPE_GREEN	1

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
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	vram_sel = vram_page = 0;
	
	// crtc
	back_color = 0;
	text_color = vram_mask = 7;
	width80 = reverse = false;
	
	update_palette();
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

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	if(!hblank && is_vram[addr >> 11]) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
{
	addr &= 0xffff;
	if(!hblank && is_vram[addr >> 11]) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
	return rbank[addr >> 11][addr & 0x7ff];
}

uint32 MEMORY::fetch_op(uint32 addr, int *wait)
{
	addr &= 0xffff;
	*wait = (ipl_selected && addr < 0x800);
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
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
	case 0xf4:
	case 0xf5:
	case 0xf6:
	case 0xf7:
		if(vram_page != (data & 7)) {
			uint8 prev_page = vram_page;
			vram_page = data & 7;
			if((prev_page & 1) != (vram_page & 1) && (vram_sel == 0x80 || vram_sel == 0xc0)) {
				update_vram_map();
			}
		}
		break;
#endif
	}
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
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
		if(config.monitor_type != MONITOR_TYPE_COLOR)
#endif
		update_palette();
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
void MEMORY::update_config()
{
	update_palette();
}
#endif

void MEMORY::update_palette()
{
#ifndef _MZ80B
	if(config.monitor_type == MONITOR_TYPE_COLOR) {
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
		}
	} else
#endif
	if(reverse) {
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = RGB_COLOR(0, i ? 0 : 255, 0);
		}
	} else {
		for(int i = 0; i < 8; i++) {
			palette_pc[i] = RGB_COLOR(0, i ? 255 : 0, 0);
		}
	}
}

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

void MEMORY::load_dat_image(_TCHAR* file_path)
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

bool MEMORY::load_mzt_image(_TCHAR* file_path)
{
	bool result = false;
	bool is_mtw = check_file_extension(file_path, _T(".mtw"));
	
	if(is_mtw || ipl_selected) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
			uint8 header[128];
			fio->Fread(header, sizeof(header), 1);
			uint16 size = header[0x12] | (header[0x13] << 8);
			uint16 offs = header[0x14] | (header[0x15] << 8);
			
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
	uint8 color = (text_color & 7) ? (text_color & 7) : 8;
#else
	#define color 1
#endif
	for(int y = 0, addr = 0; y < 200; y += 8) {
		for(int x = 0; x < (width80 ? 80 : 40); x++) {
			uint8 code = tvram[addr++];
			for(int l = 0; l < 8; l++) {
				uint8 pat = font[(code << 3) + l];
				uint8* d = &screen_txt[y + l][x << 3];
				
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
	if(config.monitor_type != MONITOR_TYPE_COLOR && (vram_mask & 8)) {
		memset(screen_gra, 0, sizeof(screen_gra));
	} else {
		// vram[0x0000-0x3fff] should be always blank
		uint8 *vram_b = vram + ((vram_mask & 1) ? 0x4000 : 0);
		uint8 *vram_r = vram + ((vram_mask & 2) ? 0x8000 : 0);
		uint8 *vram_g = vram + ((vram_mask & 4) ? 0xc000 : 0);
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0; x < 80; x++) {
				uint8 b = vram_b[addr];
				uint8 r = vram_r[addr];
				uint8 g = vram_g[addr];
				addr++;
				uint8* d = &screen_gra[y][x << 3];
				
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
	}
#else
	if(!(vram_page & 6)) {
		memset(screen_gra, 0, sizeof(screen_gra));
	} else {
		// vram[0x8000-0xbfff] should be always blank
		uint8 *vram_1 = vram + ((vram_page & 2) ? 0x0000 : 0x8000);
		uint8 *vram_2 = vram + ((vram_page & 4) ? 0x4000 : 0x8000);
		for(int y = 0, addr = 0; y < 200; y++) {
			for(int x = 0; x < 40; x++) {
				uint8 pat = vram_1[addr] | vram_2[addr];
				addr++;
				uint8* d = &screen_gra[y][x << 3];
				
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
#endif
	
	// copy to real screen
	for(int y = 0; y < 200; y++) {
		scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
		scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
		uint8* src_txt = screen_txt[y];
		uint8* src_gra = screen_gra[y];
#ifndef _MZ80B
		uint8 back = (config.monitor_type == MONITOR_TYPE_COLOR) ? back_color : 0;
		
		if(text_color & 8) {
			// graphics > text
			for(int x = 0; x < 640; x++) {
				uint8 txt = src_txt[width80 ? x : (x >> 1)], gra = src_gra[x];
				dest0[x] = palette_pc[gra ? gra : txt ? (txt & 7) : back];
			}
		} else {
			// text > graphics
			for(int x = 0; x < 640; x++) {
				uint8 txt = src_txt[width80 ? x : (x >> 1)], gra = src_gra[x];
				dest0[x] = palette_pc[txt ? (txt & 7) : gra ? gra : back];
			}
		}
#else
		if(width80) {
			for(int x = 0; x < 640; x++) {
				uint8 txt = src_txt[x], gra = src_gra[x >> 1];
				dest0[x] = palette_pc[txt | gra];
			}
		} else {
			for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
				uint8 txt = src_txt[x], gra = src_gra[x];
				dest0[x2] = dest0[x2 + 1] = palette_pc[txt | gra];
			}
		}
#endif
		if(config.scan_line) {
			memset(dest1, 0, 640 * sizeof(scrntype));
		} else {
			memcpy(dest1, dest0, 640 * sizeof(scrntype));
		}
	}
#ifndef _MZ80B
	emu->screen_skip_line = true;
#endif
}

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(vram, sizeof(vram), 1);
	state_fio->Fwrite(tvram, sizeof(tvram), 1);
	state_fio->FputBool(ipl_selected);
	state_fio->FputUint8(vram_sel);
	state_fio->FputUint8(vram_page);
	state_fio->FputUint8(back_color);
	state_fio->FputUint8(text_color);
	state_fio->FputUint8(vram_mask);
	state_fio->FputBool(width80);
	state_fio->FputBool(reverse);
	state_fio->FputBool(hblank);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	state_fio->Fread(vram, sizeof(vram), 1);
	state_fio->Fread(tvram, sizeof(tvram), 1);
	ipl_selected = state_fio->FgetBool();
	vram_sel = state_fio->FgetUint8();
	vram_page = state_fio->FgetUint8();
	back_color = state_fio->FgetUint8();
	text_color = state_fio->FgetUint8();
	vram_mask = state_fio->FgetUint8();
	width80 = state_fio->FgetBool();
	reverse = state_fio->FgetBool();
	hblank = state_fio->FgetBool();
	
	// post process
	if(ipl_selected) {
		SET_BANK(0x0000, 0x07ff, wdmy, ipl, false);
		SET_BANK(0x0800, 0x7fff, wdmy, rdmy, false);
		SET_BANK(0x8000, 0xffff, ram, ram, false);
	} else {
		SET_BANK(0x0000, 0xffff, ram, ram, false);
	}
	update_vram_map();
	update_palette();
	return true;
}

