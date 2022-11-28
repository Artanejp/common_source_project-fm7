/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_DISP		0
#define SIG_MEMORY_VSYNC	1

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_dma;
	
	uint8_t* rbank[4096];	// 16MB / 4KB
	uint8_t* wbank[4096];
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	
#if defined(HAS_I86)
	uint8_t ram[0x100000];	// RAM 1MB
#elif defined(HAS_I286)
	uint8_t ram[0x400000];	// RAM 1+3MB
#endif
	uint8_t vram[0x20000];	// VRAM 32KB * 4pl
	uint8_t cvram[0x1000];
	uint8_t kvram[0x1000];
	
	uint8_t ipl[0x10000];	// IPL 64KB
	uint8_t kanji16[0x40000];	// KANJI(16x16) 256KB
	
	uint8_t mcr1, mcr2, a20;
	uint8_t lcdadr, lcdreg[32];
	uint16_t dcr1, dcr2;
	int kj_h, kj_l, kj_ofs, kj_row;
	
	int blinkcnt;
	uint8_t screen_txt[400][648];
	uint8_t screen_cg[400][640];
	
	void update_bank();
	void draw_text40();
	void draw_text80();
	void draw_cg();
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	uint8_t* get_cvram()
	{
		return cvram;
	}
	uint8_t* get_kvram()
	{
		return kvram;
	}
	void draw_screen();
};

#endif

