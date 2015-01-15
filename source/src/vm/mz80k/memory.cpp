/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ memory ]
*/

#include "memory.h"
#if defined(_MZ1200) || defined(_MZ80A)
#include "display.h"
#endif
#include "../i8253.h"
#include "../i8255.h"
#include "../../fileio.h"

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
#if defined(SUPPORT_MZ80AIF)
	memset(fdif, 0xff, sizeof(fdif));
#endif
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
#if defined(_MZ1200) || defined(_MZ80A)
	if(fio->Fopen(emu->bios_path(_T("EXT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
#endif
#if defined(SUPPORT_MZ80AIF)
	if(fio->Fopen(emu->bios_path(_T("FDIF.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(fdif, sizeof(fdif), 1);
		fio->Fclose();
	}
#endif
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
	SET_BANK(0xf000, 0xffff, wdmy, rdmy);
#else
	SET_BANK(0xe000, 0xffff, wdmy, rdmy);
#endif
	
#if defined(_MZ80A)
	// init scroll register
	e200 = 0x00;	// scroll
#endif
	
	// register event
	register_vline_event(this);
	register_event_by_clock(this, EVENT_TEMPO, CPU_CLOCKS / 64, true, NULL);	// 32hz * 2
	register_event_by_clock(this, EVENT_BLINK, CPU_CLOCKS / 3, true, NULL);	// 1.5hz * 2
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
#if defined(_MZ1200) || defined(_MZ80A)
	hblank = false;
#endif
	
	// motor is always rotating...
	d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x10);
}

void MEMORY::event_vline(int v, int clock)
{
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

void MEMORY::write_data8(uint32 addr, uint32 data)
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
		}
		return;
	}
	wbank[addr >> 10][addr & 0x3ff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
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
			d_disp->write_signal(SIG_DISPLAY_REVERSE, 0, 0);
			break;
		case 0xe015:
			// reverse display
			d_disp->write_signal(SIG_DISPLAY_REVERSE, 1, 1);
			break;
#endif
#if defined(_MZ80A)
		default:
			if(0xe200 <= addr && addr <= 0xe2ff) {
				e200 = (uint8)(addr & 0xff);	// scroll
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
void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	bool signal = ((data & mask) != 0);
	
	if(id == SIG_MEMORY_FDC_IRQ) {
		if(fdc_irq != signal) {
			fdc_irq = signal;
#ifdef _FDC_DEBUG_LOG
			emu->out_debug_log(_T("MEM\tfdc_irq=%2x\n"), fdc_irq);
#endif
//			update_fdif_rom_bank();
		}
	} else if(id == SIG_MEMORY_FDC_DRQ) {
		if(fdc_drq != signal) {
			fdc_drq = signal;
#ifdef _FDC_DEBUG_LOG
			emu->out_debug_log(_T("MEM\tfdc_drq=%2x\n"), fdc_drq);
#endif
			update_fdif_rom_bank();
		}
	}
}

void MEMORY::update_fdif_rom_bank()
{
	// FD IF ROM BANK switching
	if(fdc_drq) {
		// F000-F7FF	FD IF (MZ-80AIF) ROM  offset 0x400
		SET_BANK(0xf000, 0xf3ff, wdmy, fdif + 0x400 );	// FD IF ROM 1KB (2KB / 2)
		SET_BANK(0xf400, 0xf7ff, wdmy, fdif + 0x400 );	// FD IF ROM ghost
	} else {
		// F000-F7FF	FD IF (MZ-80AIF) ROM  offset 0
		SET_BANK(0xf000, 0xf3ff, wdmy, fdif );	// FD IF ROM 1KB (2KB / 2)
		SET_BANK(0xf400, 0xf7ff, wdmy, fdif );	// FD IF ROM ghost
	}
}
#endif

#define STATE_VERSION	2

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(vram, sizeof(vram), 1);
	state_fio->FputBool(tempo);
	state_fio->FputBool(blink);
#if defined(_MZ1200) || defined(_MZ80A)
	state_fio->FputBool(hblank);
	state_fio->FputBool(memory_swap);
#endif
#if defined(_MZ80A)
	state_fio->FputUint8(e200);
#endif
#if defined(SUPPORT_MZ80AIF)
	state_fio->FputBool(fdc_irq);
	state_fio->FputBool(fdc_drq);
#endif
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
	tempo = state_fio->FgetBool();
	blink = state_fio->FgetBool();
#if defined(_MZ1200) || defined(_MZ80A)
	hblank = state_fio->FgetBool();
	memory_swap = state_fio->FgetBool();
#endif
#if defined(_MZ80A)
	e200 = state_fio->FgetUint8();
#endif
#if defined(SUPPORT_MZ80AIF)
	fdc_irq = state_fio->FgetBool();
	fdc_drq = state_fio->FgetBool();
#endif
	
	// post process
#if defined(_MZ1200) || defined(_MZ80A)
	update_memory_swap();
#endif
#if defined(SUPPORT_MZ80AIF)
	update_fdif_rom_bank();
#endif
	return true;
}

