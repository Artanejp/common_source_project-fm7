/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

#define SIG_MEMORY_BANK	0

class MEMBUS : public MEMORY
{
private:
	DEVICE *d_cpu;
	DEVICE *d_dma;
	
#ifdef _MZ6550
	uint8_t ipl[0x8000];	// IPL 32KB
#else
	uint8_t ipl[0x4000];	// IPL 16KB
#endif
#if defined(_MZ6500) || defined(_MZ6550)
	uint8_t ram[0x90000];	// Main RAM 640KB
#else
	uint8_t ram[0x80000];	// Main RAM 512KB
#endif
	uint8_t vram[0x80000];	// VRAM 192KB + 1024B + padding
	uint8_t kanji[0x40000];	// Kanji ROM 256KB
	uint8_t dic[0x40000];	// Dictionary ROM 256KB
#ifdef _MZ6550
	uint8_t dic2[0x100000];	// New Dictionary ROM 1MB
#endif
#if defined(_MZ6500) || defined(_MZ6550)
	uint8_t mz1r32[0x100000];	// MZ-1R32 512KB * 2
#endif
	uint8_t bank1, bank2;
	
	void update_bank();
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common function
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
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
};

#endif
