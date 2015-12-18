/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.14-

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
	DEVICE *d_cpu;
	
	uint8 mon[0x800];
	uint8 prom1[0x400];
	uint8 prom2[0x400];
	uint8 ram[0x800];
	
	uint8 wdmy[0x400];
	uint8 rdmy[0x400];
	uint8* wbank[64];
	uint8* rbank[64];
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int *wait);
	uint32 intr_ack()
	{
		// RST 7
		return 0xff;
	}
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	uint8* get_ram()
	{
		return ram;
	}
	void load_binary(const _TCHAR* file_path);
	void save_binary(const _TCHAR* file_path);
};

#endif

