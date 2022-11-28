/*
	National JR-100 Emulator 'eJR-100'

	Author : Takeda.Toshiya
	Date   : 2015.08.27-

	[ memory bus ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_VIA_PORT_A	0
#define SIG_MEMORY_VIA_PORT_B	1

class MEMORY : public DEVICE
{
private:
	// contexts
	DEVICE *d_via;
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
	
	// memory
	uint8_t ram[0x8000];
	uint8_t vram[0x400];
	uint8_t rom[0x2000];
	
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	
	int key_column;
	bool cmode;
	scrntype_t palette_pc[2];
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_via(DEVICE* device)
	{
		d_via = device;
	}
	void draw_screen();
};

#endif

