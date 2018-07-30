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
	uint8_t *cart;
	uint32_t size;
	uint8_t pages;
	uint8_t ram[0x10000];
	uint8_t rdmy[0x2000];
	uint8_t sram[0x8000];
	uint8_t *cpu_readmap[8];
	uint8_t *cpu_writemap[8];
	uint8_t fcr[4];
	uint8_t save;
	
	bool inserted;
	void sms_mapper_w(uint32_t addr, uint32_t data);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void bios();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	
	// unique functions
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
};

#endif
