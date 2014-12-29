/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.13-

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
	uint8* rbank[16];
	uint8* wbank[16];
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	uint8 ram[0x10000];	// Main RAM 64KB
	uint8 bios[0x3000];	// IPL 12KB
	uint8 basic[0x1000];	// BASIC 4KB
	uint32 amask;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
};

#endif

