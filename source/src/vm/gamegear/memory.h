/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_SEL	0

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu;
	// memory
	uint8 *cart;
    uint32 size;
    uint8 pages;
	uint8 ram[0x10000];	
	uint8 rdmy[0x2000];
	uint8 sram[0x8000];
	uint8 *cpu_readmap[8];
	uint8 *cpu_writemap[8];
    uint8 fcr[4];
    uint8 save;
	
	bool inserted;
	void sms_mapper_w(uint32 addr, uint32 data);
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void bios();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
};

#endif
