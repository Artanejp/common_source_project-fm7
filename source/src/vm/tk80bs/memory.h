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
	
	uint8 mon[0x800];
	uint8 ext[0x7000];
	uint8 basic[0x2000];
	uint8 bsmon[0x1000];
	uint8 ram[0x5000];	// with TK-M20K
	uint8 vram[0x200];
	
	uint8 wdmy[0x200];
	uint8 rdmy[0x200];
	uint8* wbank[128];
	uint8* rbank[128];
	
	int boot_mode;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int *wait);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
	uint8* get_vram()
	{
		return vram;
	}
	void load_binary(const _TCHAR* file_path);
	void save_binary(const _TCHAR* file_path);
};

#endif

