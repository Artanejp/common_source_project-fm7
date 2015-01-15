/*
	TOMY PyuTa Emulator 'ePyuTa'

	Author : Takeda.Toshiya
	Date   : 2007.07.15 -

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
	DEVICE *d_cmt, *d_cpu, *d_psg, *d_vdp;
	
	uint8 ipl[0x8000];	// ipl rom (32k)
	uint8 basic[0x4000];	// basic rom (16k)
	uint8 cart[0x8000];	// cartridge (32k)
	
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	uint8* wbank[16];
	uint8* rbank[16];
	
	bool cmt_signal, cmt_remote;
	bool has_extrom, cart_enabled;
	int ctype;
	
	uint8 *key;
	uint32 *joy;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cmt(DEVICE* device)
	{
		d_cmt = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void set_context_vdp(DEVICE* device)
	{
		d_vdp = device;
	}
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return (ctype != 0);
	}
};

#endif

