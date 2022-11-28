/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ memory ]
*/

#include "memory.h"
#include "../i8253.h"
#include "../i8255.h"

#define EVENT_TEMPO	0
#define EVENT_BLINK	1
#define EVENT_HBLANK	2

#if defined(_MZ80K) || defined(_MZ1200)
#define MONITOR_TYPE_MONOCHROME		0
#define MONITOR_TYPE_COLOR		1
#define MONITOR_TYPE_MONOCHROME_COLOR	2
#define MONITOR_TYPE_COLOR_MONOCHROME	3
#endif

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 10, eb = (e) >> 10; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x400 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x400 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(ipl, 0xff, sizeof(ipl));
#if defined(_MZ1200) || defined(_MZ80A)
	memset(ext, 0xff, sizeof(ext));
#endif
	
#if defined(_MZ80K) || defined(_MZ1200)
	// COLOR GAL 5 - 2019.01.24 Suga
	memset(gal5_vram, 0x07, sizeof(gal5_vram));	// Color attribute RAM
	gal5_wdat = 0x07;					// Color palette data
#endif
	
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	memset(fdif, 0xff, sizeof(fdif));
#endif
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
#if defined(_MZ1200) || defined(_MZ80A)
	if(fio->Fopen(create_local_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
#endif
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	if(fio->Fopen(create_local_path(_T("FDIF.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(fdif, sizeof(fdif), 1);
		fio->Fclose();
	}
#endif
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	delete fio;
	
	// 0000-0FFF	IPL/RAM
	// 1000-BFFF	RAM
	// C000-CFFF	RAM/IPL
	SET_BANK(0x0000, 0x0fff, wdmy, ipl);
	SET_BANK(0x1000, 0xbfff, ram + 0x1000, ram + 0x1000);
	SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
#if defined(_MZ1200) || defined(_MZ80A)
	SET_BANK(0xd000, 0xd7ff, vram, vram);	// VRAM 2KB
	SET_BANK(0xd800, 0xdfff, wdmy, rdmy);
#else
	SET_BANK(0xd000, 0xd3ff, vram, vram);	// VRAM 1KB
	SET_BANK(0xd400, 0xd7ff, vram, vram);
	SET_BANK(0xd800, 0xdbff, vram, vram);
	SET_BANK(0xdc00, 0xdfff, vram, vram);
#endif
#if defined(_MZ1200) || defined(_MZ80A)
	SET_BANK(0xe000, 0xe7ff, wdmy, rdmy);
	SET_BANK(0xe800, 0xefff, wdmy, ext);
#else
	SET_BANK(0xe000, 0xefff, wdmy, rdmy);
#endif
#if defined(SUPPORT_MZ80FIO)
	SET_BANK(0xf000, 0xf3ff, wdmy, fdif);
	SET_BANK(0xf400, 0xffff, wdmy, rdmy);
#else
	SET_BANK(0xf000, 0xffff, wdmy, rdmy);
#endif
	
#if defined(_MZ80A)
	// init scroll register
	e200 = 0x00;	// scroll
#endif
	// init pcg
	memset(pcg, 0, sizeof(pcg));
	memcpy(pcg + 0x000, font, 0x400); // copy pattern of 00h-7fh
#if defined(_MZ1200)
	memcpy(pcg + 0x800, font, 0x400);
#endif
	
	// create pc palette
	palette_pc[0] = RGB_COLOR(0, 0, 0);
#if defined(_MZ1200) || defined(_MZ80A)
	palette_pc[1] = RGB_COLOR(0, 255, 0);
#else
	if(config.monitor_type & 4) {
		palette_pc[1] = RGB_COLOR(0, 255, 0);
	} else {
		palette_pc[1] = RGB_COLOR(255, 255, 255);
	}
#endif
	
#if defined(_MZ80K) || defined(_MZ1200)
	// COLOR GAL 5 - 2019.01.24 Suga
	for(int i = 0; i < 8; i++) {
		gal5_palette[i] = RGB_COLOR((i & 4) ? 255 : 0, (i & 2) ? 255 : 0, (i & 1) ? 255 : 0);	// RGB
	}
#endif
	
	// register event
	register_vline_event(this);
	register_event_by_clock(this, EVENT_TEMPO, CPU_CLOCKS / 64, true, NULL);	// 32hz * 2
	register_event_by_clock(this, EVENT_BLINK, CPU_CLOCKS / 3, true, NULL);		// 1.5hz * 2
}

void MEMORY::reset()
{
#if defined(_MZ1200) || defined(_MZ80A)
	// reset memory swap
	memory_swap = false;
	update_memory_swap();
#endif
#if defined(SUPPORT_MZ80AIF)
	// MB8866 IRQ,DRQ
	fdc_irq = fdc_drq = false;
	update_fdif_rom_bank();
#endif
	
	tempo = blink = false;
	vgate = true;
#if defined(_MZ1200) || defined(_MZ80A)
	hblank = reverse = false;
#endif
	pcg_data = pcg_addr = 0;
	pcg_ctrl = 0xff;
	
	// motor is always rotating...
	d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x10);
}

void MEMORY::event_vline(int v, int clock)
{
	// draw one line
	if(0 <= v && v < 200) {
		draw_line(v);
	}
	
	// vblank
	if(v == 0) {
		d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x80);
	} else if(v == 200) {
		d_pio->write_signal(SIG_I8255_PORT_C, 0, 0x80);
	}
	
#if defined(_MZ1200) || defined(_MZ80A)
	// hblank
	hblank = true;
	register_event_by_clock(this, EVENT_HBLANK, 92, false, NULL);
#endif
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TEMPO) {
		// 32khz
		tempo = !tempo;
	} else if(event_id == EVENT_BLINK) {
		// 1.5khz
		d_pio->write_signal(SIG_I8255_PORT_C, (blink = !blink) ? 0xff : 0, 0x40);
#if defined(_MZ1200) || defined(_MZ80A)
	} else if(event_id == EVENT_HBLANK) {
		hblank = false;
#endif
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(0xe000 <= addr && addr <= 0xe7ff) {
		// memory mapped i/o
		switch(addr) {
		case 0xe000: case 0xe001: case 0xe002: case 0xe003:
			d_pio->write_io8(addr & 3, data);
			break;
		case 0xe004: case 0xe005: case 0xe006: case 0xe007:
			d_ctc->write_io8(addr & 3, data);
			break;
		case 0xe008:
			// 8253 gate0
			d_ctc->write_signal(SIG_I8253_GATE_0, data, 1);
			break;
#if defined(_MZ80K) || defined(_MZ1200)
		case 0xe00c: case 0xe00d: case 0xe00e: case 0xe00f:
			// COLOR GAL 5 - 2019.01.24 Suga
			gal5_wdat = (uint8_t)(data & 0xff);		// Color palette data
			break;
#endif
		case 0xe010:
			pcg_data = data;
			break;
		case 0xe011:
			pcg_addr = data;
			break;
		case 0xe012:
			if(!(pcg_ctrl & 0x10) && (data & 0x10)) {
				int offset = pcg_addr | ((data & 3) << 8);
#if defined(_MZ1200)
				pcg[offset | ((data & 4) ? 0xc00 : 0x400)] = (data & 0x20) ? font[offset | 0x400] : pcg_data;
#else
				pcg[offset |                       0x400 ] = (data & 0x20) ? font[offset | 0x400] : pcg_data;
#endif
			}
			pcg_ctrl = data;
			break;
		}
		return;
	}
#if defined(_MZ80K) || defined(_MZ1200)
	// COLOR GAL 5 - 2019.01.24 Suga
	if(0xd000 <= addr && addr <= 0xdfff) {
		gal5_vram[addr & 0x3ff] = gal5_wdat;		// Color attribute RAM
	}
#endif
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	if(0xe000 <= addr && addr <= 0xe7ff) {
		// memory mapped i/o
		switch(addr) {
		case 0xe000: case 0xe001: case 0xe002: case 0xe003:
			return d_pio->read_io8(addr & 3);
		case 0xe004: case 0xe005: case 0xe006: case 0xe007:
			return d_ctc->read_io8(addr & 3);
		case 0xe008:
#if defined(_MZ1200) || defined(_MZ80A)
			return (hblank ? 0x80 : 0) | (tempo ? 1 : 0) | 0x7e;
#else
			return (tempo ? 1 : 0) | 0xfe;
#endif
#if defined(_MZ1200) || defined(_MZ80A)
		case 0xe00c:
			// memory swap
			if(!memory_swap) {
				memory_swap = true;
				update_memory_swap();
			}
			break;
		case 0xe010:
			// reset memory swap
			if(memory_swap) {
				memory_swap = false;
				update_memory_swap();
			}
			break;
		case 0xe014:
			// normal display
			reverse = false;
			break;
		case 0xe015:
			// reverse display
			reverse = true;
			break;
#endif
#if defined(_MZ80A)
		default:
			if(0xe200 <= addr && addr <= 0xe2ff) {
				e200 = addr & 0xff;	// scroll
			}
			break;
#endif
		}
		return 0xff;
	}
	return rbank[addr >> 10][addr & 0x3ff];
}

#if defined(_MZ1200) || defined(_MZ80A)
void MEMORY::update_memory_swap()
{
	if(memory_swap) {
		SET_BANK(0x0000, 0x0fff, ram + 0xc000, ram + 0xc000);
		SET_BANK(0xc000, 0xcfff, wdmy, ipl);
	} else {
		SET_BANK(0x0000, 0x0fff, wdmy, ipl);
		SET_BANK(0xc000, 0xcfff, ram + 0xc000, ram + 0xc000);
	}
}
#endif

#if defined(SUPPORT_MZ80AIF)
void MEMORY::update_fdif_rom_bank()
{
	// FD IF ROM BANK switching
	if(fdc_drq) {
		// F000-F7FF	FD IF (MZ-80AIF) ROM  offset 0x400
		SET_BANK(0xf000, 0xf3ff, wdmy, fdif + 0x400 );	// FD IF ROM 1KB (2KB / 2)
		SET_BANK(0xf400, 0xf7ff, wdmy, fdif + 0x400 );	// FD IF ROM ghost
	} else {
		// F000-F7FF	FD IF (MZ-80AIF) ROM  offset 0
		SET_BANK(0xf000, 0xf3ff, wdmy, fdif );		// FD IF ROM 1KB (2KB / 2)
		SET_BANK(0xf400, 0xf7ff, wdmy, fdif );		// FD IF ROM ghost
	}
}
#endif

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool signal = ((data & mask) != 0);
	
	if(id == SIG_MEMORY_VGATE) {
		vgate = signal;
#if defined(SUPPORT_MZ80AIF)
	} else if(id == SIG_MEMORY_FDC_IRQ) {
		if(fdc_irq != signal) {
			fdc_irq = signal;
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("MEM\tfdc_irq=%2x\n"), fdc_irq);
#endif
//			update_fdif_rom_bank();
		}
	} else if(id == SIG_MEMORY_FDC_DRQ) {
		if(fdc_drq != signal) {
			fdc_drq = signal;
#ifdef _FDC_DEBUG_LOG
			this->out_debug_log(_T("MEM\tfdc_drq=%2x\n"), fdc_drq);
#endif
			update_fdif_rom_bank();
		}
#endif
	}
}

#if defined(_MZ80K)
void MEMORY::update_config()
{
	if(config.monitor_type & 4) {
		palette_pc[1] = RGB_COLOR(0, 255, 0);
	} else {
		palette_pc[1] = RGB_COLOR(255, 255, 255);
	}
}
#endif

void MEMORY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
		for(int v = 0; v < 200; v++) {
			draw_line(v);
		}
	}
	
#if defined(_MZ80K) || defined(_MZ1200)
	// COLOR GAL 5 - 2019.01.24 Suga
	if((config.monitor_type & 3) == MONITOR_TYPE_MONOCHROME_COLOR || (config.monitor_type & 3) == MONITOR_TYPE_COLOR_MONOCHROME) {
		emu->set_vm_screen_size(640, 200, 640, 200, 640, 240);
	} else {
		emu->set_vm_screen_size(320, 200, 320, 200, 320, 240);
	}
#endif
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
#if defined(_MZ80K) || defined(_MZ1200)
	int offset_monochrome = 0;
	int offset_color = 320;
	if((config.monitor_type & 3) == MONITOR_TYPE_COLOR || (config.monitor_type & 3) == MONITOR_TYPE_COLOR_MONOCHROME) {
		offset_monochrome = 320;
		offset_color = 0;
	}
#else
	int offset_monochrome = 0;
#endif
	
#if defined(_MZ80K) || defined(_MZ1200)
	if((config.monitor_type & 3) != MONITOR_TYPE_COLOR) {
#endif
	if(true || vgate) {
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y);
				uint8_t* src = screen[y];
				
				for(int x = 0; x < 320; x++) {
					dest[x + offset_monochrome] = palette_pc[src[x] & 1];
				}
			}
		} else {
			for(int y = 0; y < 200; y++) {
				scrntype_t* dest = emu->get_screen_buffer(y);
				memset(dest, 0, sizeof(scrntype_t) * 320);
				}
			}
#if defined(_MZ80K) || defined(_MZ1200)
		}
#endif
	
#if defined(_MZ80K) || defined(_MZ1200)
	// COLOR GAL 5 - 2019.01.24 Suga
	if((config.monitor_type & 3) != MONITOR_TYPE_MONOCHROME) {
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t* srcg = gal5_screen[y];
			for(int x = 0; x < 320; x++) {
				dest[x + offset_color] = gal5_palette[ srcg[x] ];
			}
		}
	}
#endif
}

void MEMORY::draw_line(int v)
{
#if defined(_MZ80A)
	int ptr = (v >> 3) * 40 + (int)(e200 << 3);	// scroll
#else
	int ptr = (v >> 3) * 40;
#endif
	bool pcg_active = ((config.dipswitch & 1) && !(pcg_ctrl & 8));
#if defined(_MZ1200)
	uint8_t *pcg_ptr = pcg + ((pcg_ctrl & 4) ? 0x800 : 0);
#else
	#define pcg_ptr pcg
#endif
	
	for(int x = 0; x < 320; x += 8) {
#if defined(_MZ80K) || defined(_MZ1200)
		// COLOR GAL 5 - 2019.01.24 Suga
		int cc = gal5_vram[ptr & 0x7ff];
		uint8_t bk = (cc >> 4) & 0x07;
		uint8_t fw = cc & 0x07;
		uint8_t* dtc = &gal5_screen[v][x];
#endif
		int code = vram[(ptr++) & 0x7ff] << 3;
		uint8_t pat = pcg_active ? pcg_ptr[code | (v & 7)] : font[code | (v & 7)];
		
#if defined(_MZ80K) || defined(_MZ1200)
		// COLOR GAL 5 - 2019.01.24 Suga
		dtc[0] = (pat & 0x80) ? fw : bk;
		dtc[1] = (pat & 0x40) ? fw : bk;
		dtc[2] = (pat & 0x20) ? fw : bk;
		dtc[3] = (pat & 0x10) ? fw : bk;
		dtc[4] = (pat & 0x08) ? fw : bk;
		dtc[5] = (pat & 0x04) ? fw : bk;
		dtc[6] = (pat & 0x02) ? fw : bk;
		dtc[7] = (pat & 0x01) ? fw : bk;
#endif
		
		// 8255(PIO) PC0 is /V-GATE 2016.11.21 by Suga
		if((d_pio->read_signal(2) & 0x01) == 0x00) {
			pat = 0x00;
		}
#if defined(_MZ1200) || defined(_MZ80A)
		if(reverse) {
			pat = ~pat;
		}
#endif
		uint8_t* dest = &screen[v][x];
		
		dest[0] = (pat & 0x80) >> 7;
		dest[1] = (pat & 0x40) >> 6;
		dest[2] = (pat & 0x20) >> 5;
		dest[3] = (pat & 0x10) >> 4;
		dest[4] = (pat & 0x08) >> 3;
		dest[5] = (pat & 0x04) >> 2;
		dest[6] = (pat & 0x02) >> 1;
		dest[7] = (pat & 0x01) >> 0;
	}
}

#define STATE_VERSION	5

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
	state_fio->StateValue(tempo);
	state_fio->StateValue(blink);
#if defined(_MZ1200) || defined(_MZ80A)
	state_fio->StateValue(hblank);
	state_fio->StateValue(memory_swap);
#endif
#if defined(SUPPORT_MZ80AIF)
	state_fio->StateValue(fdc_irq);
	state_fio->StateValue(fdc_drq);
#endif
	state_fio->StateValue(vgate);
#if defined(_MZ1200) || defined(_MZ80A)
	state_fio->StateValue(reverse);
#endif
#if defined(_MZ80A)
	state_fio->StateValue(e200);
#endif
	state_fio->StateArray(pcg + 0x400, 0x400, 1);
#if defined(_MZ1200)
	state_fio->StateArray(pcg + 0xc00, 0x400, 1);
#endif
	state_fio->StateValue(pcg_data);
	state_fio->StateValue(pcg_addr);
	state_fio->StateValue(pcg_ctrl);
#if defined(_MZ80K) || defined(_MZ1200)
	// COLOR GAL 5 - 2019.01.24 Suga
	state_fio->StateArray(gal5_vram, sizeof(gal5_vram), 1);
	state_fio->StateValue(gal5_wdat);
#endif
	
	// post process
	if(loading) {
#if defined(_MZ1200) || defined(_MZ80A)
		update_memory_swap();
#endif
#if defined(SUPPORT_MZ80AIF)
		update_fdif_rom_bank();
#endif
	}
	return true;
}

