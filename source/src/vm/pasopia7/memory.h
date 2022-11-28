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
	
	uint8_t bios[0x4000];
	uint8_t basic[0x8000];
	uint8_t ram[0x10000];
	uint8_t vram[0x10000];	// blue, red, green + text, attribute
	uint8_t pal[0x10];
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t* wbank[16];
	uint8_t* rbank[16];
	
	uint8_t mem_map, plane, attr_data, attr_latch;
	bool vram_sel, pal_sel, attr_wrap;
	
	void update_memory_map();
	
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
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
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
	uint8_t* get_ram()
	{
		return ram;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	uint8_t* get_pal()
	{
		return pal;
	}
};

#endif

