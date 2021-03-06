/*
	MITEC MP-85 Emulator 'eMP-85'

	Author : Takeda.Toshiya
	Date   : 2021.01.19-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

#define SIG_MEMBUS_PC	0

namespace MP85 {
class MEMBUS : public MEMORY
{
private:
	DEVICE *d_cpu, *d_pio;
	uint8_t pc;
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	virtual void reset();
	virtual uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait);
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	virtual void __FASTCALL key_down(int code);
	virtual void __FASTCALL key_up(int code);
};
}
#endif
