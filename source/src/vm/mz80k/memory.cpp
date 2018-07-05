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
	if(config.monitor_type) {
		palette_pc[1] = RGB_COLOR(0, 255, 0);
	} else {
		palette_pc[1] = RGB_COLOR(255, 255, 255);
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
			int code = vram[(ptr++) & 0x7ff] << 3;
			uint8_t pat = pcg_active ? pcg_ptr[code | (v & 7)] : font[code | (v & 7)];
			
			// 8255(PIO) PC0 is /V-GATE 2016.11.21 by Suga
			if((d_pio->read_io8(2) & 0x01) == 0x00) {
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
//#ifdef _FDC_DEBUG_LOG
			if(config.special_debug_fdc) this->out_debug_log(_T("MEM\tfdc_irq=%2x\n"), fdc_irq);
//#endif
//			update_fdif_rom_bank();
		}
	} else if(id == SIG_MEMORY_FDC_DRQ) {
		if(fdc_drq != signal) {
			fdc_drq = signal;
//#ifdef _FDC_DEBUG_LOG
			if(config.special_debug_fdc) this->out_debug_log(_T("MEM\tfdc_drq=%2x\n"), fdc_drq);
//#endif
			update_fdif_rom_bank();
		}
#endif
	}
}

#if defined(_MZ80K)
void MEMORY::update_config()
{
	if(config.monitor_type) {
		palette_pc[1] = RGB_COLOR(0, 255, 0);
	} else {
		palette_pc[1] = RGB_COLOR(255, 255, 255);
	}
}
#endif

void MEMORY::draw_screen()
{
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	if(true || vgate) {
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			uint8_t* src = screen[y];
			
			for(int x = 0; x < 320; x++) {
				dest[x] = palette_pc[src[x] & 1];
			}
		}
	} else {
		for(int y = 0; y < 200; y++) {
			scrntype_t* dest = emu->get_screen_buffer(y);
			memset(dest, 0, sizeof(scrntype_t) * 320);
		}
	}
}

#define STATE_VERSION	3

#include "../../statesub.h"

void MEMORY::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_1D_ARRAY(ram, sizeof(ram));
	DECL_STATE_ENTRY_1D_ARRAY(vram, sizeof(vram));
	DECL_STATE_ENTRY_BOOL(tempo);
	DECL_STATE_ENTRY_BOOL(blink);
#if defined(_MZ1200) || defined(_MZ80A)
	DECL_STATE_ENTRY_BOOL(hblank);
	DECL_STATE_ENTRY_BOOL(memory_swap);
#endif
#if defined(SUPPORT_MZ80AIF)
	DECL_STATE_ENTRY_BOOL(fdc_irq);
	DECL_STATE_ENTRY_BOOL(fdc_drq);
#endif
	DECL_STATE_ENTRY_BOOL(vgate);
#if defined(_MZ1200) || defined(_MZ80A)
	DECL_STATE_ENTRY_BOOL(reverse);
#endif
#if defined(_MZ80A)
	DECL_STATE_ENTRY_UINT32(e200);
#endif
	DECL_STATE_ENTRY_1D_ARRAY(&(pcg[0x400]), 0x400);
#if defined(_MZ1200)
	DECL_STATE_ENTRY_1D_ARRAY(&(pcg[0xc00]), 0x400);
#endif
	DECL_STATE_ENTRY_UINT8(pcg_data);
	DECL_STATE_ENTRY_UINT8(pcg_addr);
	DECL_STATE_ENTRY_UINT8(pcg_ctrl);
	
	leave_decl_state();
}

void MEMORY::save_state(FILEIO* state_fio)
{

	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->Fwrite(ram, sizeof(ram), 1);
//	state_fio->Fwrite(vram, sizeof(vram), 1);
//	state_fio->FputBool(tempo);
//	state_fio->FputBool(blink);
//#if defined(_MZ1200) || defined(_MZ80A)
//	state_fio->FputBool(hblank);
//	state_fio->FputBool(memory_swap);
//#endif
//#if defined(SUPPORT_MZ80AIF)
//	state_fio->FputBool(fdc_irq);
//	state_fio->FputBool(fdc_drq);
//#endif
//	vgate = state_fio->FgetBool();
//#if defined(_MZ1200) || defined(_MZ80A)
//	reverse = state_fio->FgetBool();
//#endif
//#if defined(_MZ80A)
//	state_fio->FputUint32(e200);
//#endif
//	state_fio->Fwrite(pcg + 0x400, 0x400, 1);
//#if defined(_MZ1200)
//	state_fio->Fwrite(pcg + 0xc00, 0x400, 1);
//#endif
//	state_fio->FputUint8(pcg_data);
//	state_fio->FputUint8(pcg_addr);
//	state_fio->FputUint8(pcg_ctrl);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	state_fio->Fread(ram, sizeof(ram), 1);
//	state_fio->Fread(vram, sizeof(vram), 1);
//	tempo = state_fio->FgetBool();
//	blink = state_fio->FgetBool();
//#if defined(_MZ1200) || defined(_MZ80A)
//	hblank = state_fio->FgetBool();
//	memory_swap = state_fio->FgetBool();
//#endif
//#if defined(SUPPORT_MZ80AIF)
//	fdc_irq = state_fio->FgetBool();
//	fdc_drq = state_fio->FgetBool();
//#endif
//	state_fio->FputBool(vgate);
//#if defined(_MZ1200) || defined(_MZ80A)
//	state_fio->FputBool(reverse);
//#endif
//#if defined(_MZ80A)
//	e200 = state_fio->FgetUint32();
//#endif
//	state_fio->Fread(pcg + 0x400, 0x400, 1);
//#if defined(_MZ1200)
//	state_fio->Fread(pcg + 0xc00, 0x400, 1);
//#endif
//	pcg_data = state_fio->FgetUint8();
//	pcg_addr = state_fio->FgetUint8();
//	pcg_ctrl = state_fio->FgetUint8();
	
	// post process
#if defined(_MZ1200) || defined(_MZ80A)
	update_memory_swap();
#endif
#if defined(SUPPORT_MZ80AIF)
	update_fdif_rom_bank();
#endif
	return true;
}

