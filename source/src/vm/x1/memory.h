/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

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
	
	uint8* wbank[16];
	uint8* rbank[16];
	
	uint8 rom[0x8000];
	uint8 ram[0x10000];
	uint8 romsel;
#ifdef _X1TURBO_FEATURE
	uint8 extram[0x90000]; // 32kb*16bank
	uint8 bank;
#else
	int m1_cycle;
#endif
	void update_map();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
#ifndef _X1TURBO_FEATURE
	uint32 fetch_op(uint32 addr, int *wait);
#endif
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
#ifdef _X1TURBO_FEATURE
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
#endif
};

#endif

