/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE* d_crtc;
	
	uint8_t* rbank[8192];	// 16MB / 2KB
	uint8_t* wbank[8192];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ram[0xc0000];	// Main RAM 768KB
	uint8_t ext[0x600000];	// Ext RAM 6MB
	uint8_t vram[0x80000];	// VRAM 512KB
	uint8_t tvram[0x2000];	// Text VRAM 6KB + dummy 2KB
	uint8_t pcg[0x4000];	// PCG 8KB (even addr only)
	uint8_t ipl[0x10000];	// IPL 64KB
	uint8_t dic[0x40000];	// Dictionary ROM 256KB
	uint8_t kanji[0x80000];	// Kanji ROM 256KB (even addr only)
	
	uint32_t mem_window;
	uint8_t vram_bank, dic_bank, kanji_bank;
	
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
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unitque functions
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

