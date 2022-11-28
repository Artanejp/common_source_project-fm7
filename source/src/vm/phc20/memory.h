/*
	SANYO PHC-20 Emulator 'ePHC-20'

	Author : Takeda.Toshiya
	Date   : 2010.09.03-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_SYSPORT	0

class MEMORY : public DEVICE
{
private:
	DEVICE *d_drec;
	
	uint8_t rom[0x2000];
	uint8_t ram[0x1000];
	uint8_t vram[0x400];
	
	uint8_t wdmy[0x400];
	uint8_t rdmy[0x400];
	uint8_t* wbank[64];
	uint8_t* rbank[64];
	
	const uint8_t* key_stat;
	uint8_t status[9];
	uint8_t sysport;
	
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
	void event_frame();
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
};

#endif

