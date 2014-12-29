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
	
	uint8 rom[0x2000];
	uint8 ram[0x1000];
	uint8 vram[0x400];
	
	uint8 wdmy[0x400];
	uint8 rdmy[0x400];
	uint8* wbank[64];
	uint8* rbank[64];
	
	uint8* key_stat;
	uint8 status[9];
	uint8 sysport;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	uint8* get_vram()
	{
		return vram;
	}
};

#endif

