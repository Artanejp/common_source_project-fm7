/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_HBLANK_TEXT	0
#define SIG_MEMORY_VBLANK_TEXT	1
#define SIG_MEMORY_HBLANK_GRAPH	2
#define SIG_MEMORY_VBLANK_GRAPH	3
#define SIG_MEMORY_VRAM_SEL	4

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_crtc;
	
	uint8_t* rbank[32];
	uint8_t* wbank[32];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ram[0x40000];	// Main RAM 256KB
	uint8_t vram[0x20000];	// VRAM 128KB
	uint8_t tvram[0x1800];	// Text VRAM 6KB
	uint8_t pcg[0x2000];	// PCG 0-3 8KB
	uint8_t ipl[0x8000];	// IPL 32KB
	uint8_t dic[0x40000];	// Dictionary ROM 256KB
	uint8_t kanji[0x40000];	// Kanji ROM (low) / Kanji ROM (high) 128KB + 128KB
	uint8_t phone[0x8000];	// Phone ROM 32KB
	
	uint8_t bank;
	uint8_t page[8];
	int page_type[16];
	int page_wait[16];
	uint8_t dic_bank;
	uint8_t kanji_bank;
	bool hblank_t, vblank_t, wait_t;
	bool hblank_g, vblank_g, wait_g;
	int extra_wait;
	
	void set_map(uint8_t data);
	void set_map(uint8_t bank, uint8_t data);
	
	// MZ-2000/80B
	bool is_4mhz;
	uint8_t mode;
	uint8_t vram_sel, vram_page;
	void update_vram_map();
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void special_reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	uint32_t fetch_op(uint32_t addr, int* wait);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	uint8_t* get_tvram()
	{
		return tvram;
	}
	uint8_t* get_kanji()
	{
		return kanji;
	}
	uint8_t* get_pcg()
	{
		return pcg;
	}
};

#endif

