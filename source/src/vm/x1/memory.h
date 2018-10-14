/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

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
#ifdef _X1TURBO_FEATURE
	DEVICE *d_pio;
#endif
	
	uint8_t* wbank[16];
	uint8_t* rbank[16];
	
	uint8_t rom[0x8000];
	uint8_t ram[0x10000];
	uint8_t romsel;
#ifdef _X1TURBO_FEATURE
	uint8_t extram[0x90000]; // 32kb*16bank
	uint8_t bank;
#else
	int m1_cycle;
#endif
	void update_map();
	
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
#ifndef _X1TURBO_FEATURE
	uint32_t fetch_op(uint32_t addr, int *wait);
#endif
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
#ifdef _X1TURBO_FEATURE
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
#endif
};

#endif

