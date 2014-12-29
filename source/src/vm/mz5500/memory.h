/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_BANK	0

class MEMORY : public DEVICE
{
private:
	DEVICE* d_cpu;
	
	uint8* rbank[64];	// 1MB / 16KB
	uint8* wbank[64];
	uint8 wdmy[0x4000];
	uint8 rdmy[0x4000];
#ifdef _MZ6550
	uint8 ipl[0x8000];	// IPL 32KB
#else
	uint8 ipl[0x4000];	// IPL 16KB
#endif
#if defined(_MZ6500) || defined(_MZ6550)
	uint8 ram[0x90000];	// Main RAM 640KB
#else
	uint8 ram[0x80000];	// Main RAM 512KB
#endif
	uint8 vram[0x80000];	// VRAM 192KB + 1024B + padding
	uint8 kanji[0x40000];	// Kanji ROM 256KB
	uint8 dic[0x40000];	// Dictionary ROM 256KB
#ifdef _MZ6550
	uint8 dic2[0x100000];	// New Dictionary ROM 1MB
#endif
#if defined(_MZ6500) || defined(_MZ6550)
	uint8 mz1r32[0x100000];	// MZ-1R32 512KB * 2
#endif
	uint8 bank1, bank2;
	uint32 haddr;		// DMAC high-order address latch
	
	void update_bank();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_dma_data8(uint32 addr, uint32 data);
	uint32 read_dma_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unitque function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	uint8* get_vram()
	{
		return vram;
	}
};

#endif

