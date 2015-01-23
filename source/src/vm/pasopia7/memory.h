/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_I8255_1_A	0
#define SIG_MEMORY_I8255_1_B	1
#define SIG_MEMORY_I8255_1_C	2

class MEMORY : public DEVICE
{
private:
	DEVICE *d_iobus, *d_pio0, *d_pio2;
	
	uint8 bios[0x4000];
	uint8 basic[0x8000];
	uint8 ram[0x10000];
	uint8 vram[0x10000];	// blue, red, green + text, attribute
	uint8 pal[0x10];
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	uint8* wbank[16];
	uint8* rbank[16];
	
	uint8 mem_map, plane, attr_data, attr_latch;
	bool vram_sel, pal_sel, attr_wrap;
	
	void update_memory_map();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_iobus(DEVICE* device)
	{
		d_iobus = device;
	}
	void set_context_pio0(DEVICE* device)
	{
		d_pio0 = device;
	}
	void set_context_pio2(DEVICE* device)
	{
		d_pio2 = device;
	}
	uint8* get_ram()
	{
		return ram;
	}
	uint8* get_vram()
	{
		return vram;
	}
	uint8* get_pal()
	{
		return pal;
	}
};

#endif

