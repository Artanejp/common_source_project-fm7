/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#ifndef _HUC6280_H_ 
#define _HUC6280_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class HUC6280 : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
	void *opaque;
	
public:
	HUC6280(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~HUC6280() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 get_pc();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	uint8 irq_status_r(uint16 offset);
	void irq_status_w(uint16 offset, uint8 data);
	uint8 timer_r(uint16 offset);
	void timer_w(uint16 offset, uint8 data);
};

#endif

