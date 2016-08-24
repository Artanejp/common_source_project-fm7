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

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_VGATE	0
#if defined(SUPPORT_MZ80AIF)
#define SIG_MEMORY_FDC_IRQ	1
#define SIG_MEMORY_FDC_DRQ	2
#endif

class MEMORY : public DEVICE
{
private:
	DEVICE *d_ctc, *d_pio;
	
	uint8_t* rbank[64];
	uint8_t* wbank[64];
	uint8_t wdmy[0x400];
	uint8_t rdmy[0x400];
	
	uint8_t ram[0xd000];	// RAM 48KB + swap 4KB
#if defined(_MZ1200) || defined(_MZ80A)
	uint8_t vram[0x800];	// VRAM 2KB
#else
	uint8_t vram[0x400];	// VRAM 1KB
#endif
	uint8_t ipl[0x1000];	// IPL 4KB
#if defined(_MZ1200) || defined(_MZ80A)
	uint8_t ext[0x800];	// EXT 2KB
#endif
	
	bool tempo, blink;
#if defined(_MZ1200) || defined(_MZ80A)
	bool hblank;
	bool memory_swap;
	void update_memory_swap();
#endif
	
#if defined(SUPPORT_MZ80AIF)
	uint8_t fdif[0x800];	// FD IF ROM 2KB
	bool fdc_irq, fdc_drq;
	void update_fdif_rom_bank();
#elif defined(SUPPORT_MZ80FIO)
	uint8_t fdif[0x400];	// FD IF ROM 1KB
#endif
	
	uint8_t screen[200][320];
	uint8_t font[0x800];
#if defined(_MZ1200)
	uint8_t pcg[0x1000];	// PCG-1200
#else
	uint8_t pcg[0x800];	// PCG-8000
#endif
	scrntype_t palette_pc[2];
	bool vgate;
#if defined(_MZ1200) || defined(_MZ80A)
	bool reverse;
#endif
#if defined(_MZ80A)
	uint32_t e200;		// scroll
#endif
	uint8_t pcg_data;
	uint8_t pcg_addr;
	uint8_t pcg_ctrl;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("MEMORY"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
#if defined(_MZ80K)
	void update_config();
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_ctc(DEVICE* device)
	{
		d_ctc = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void draw_screen();
};

#endif

