/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

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
	DEVICE *d_kbd;
	
	uint8_t rom[0x6000];
#ifdef _MAP1010
	uint8_t ram[0x8000];
#else
	uint8_t ram[0x4000];
#endif
	uint8_t vram[0x1800];
	
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t* wbank[32];
	uint8_t* rbank[32];
	
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
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_keyboard(DEVICE* device)
	{
		d_kbd = device;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
};

#endif

