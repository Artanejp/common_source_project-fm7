/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'

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
	DEVICE *d_cpu, *d_pio, *d_sio;
	
	uint8_t mon[0x800];
	uint8_t ext[0x7000];
	uint8_t basic[0x2000];
	uint8_t bsmon[0x1000];
	uint8_t ram[0x5000];	// with TK-M20K
	uint8_t vram[0x200];
	
	uint8_t wdmy[0x200];
	uint8_t rdmy[0x200];
	uint8_t* wbank[128];
	uint8_t* rbank[128];
	
	int boot_mode;
	
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
	uint32_t fetch_op(uint32_t addr, int *wait);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	void load_binary(const _TCHAR* file_path);
	void save_binary(const _TCHAR* file_path);
};

#endif

