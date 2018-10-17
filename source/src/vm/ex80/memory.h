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
	
	uint8_t mon[0x800];
	uint8_t prom1[0x400];
	uint8_t prom2[0x400];
	uint8_t ram[0x800];
	
	uint8_t wdmy[0x400];
	uint8_t rdmy[0x400];
	uint8_t* wbank[64];
	uint8_t* rbank[64];
	
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
	uint32_t fetch_op(uint32_t addr, int *wait);
	uint32_t get_intr_ack()
	{
		// RST 7
		return 0xff;
	}
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	uint8_t* get_ram()
	{
		return ram;
	}
	void load_binary(const _TCHAR* file_path);
	void save_binary(const _TCHAR* file_path);
};

#endif

