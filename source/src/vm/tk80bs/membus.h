/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

	Author : Takeda.Toshiya
	Date   : 2017.01.13-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

#if defined(_TK85)
#define SIG_MEMBUS_PC7	0
#endif

class MEMBUS : public MEMORY
{
private:
	DEVICE *d_cpu;
#if defined(_TK85)
	uint32_t pc7, count;
#endif
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void reset();
	uint32_t fetch_op(uint32_t addr, int *wait);
#if defined(_TK85)
	void write_signal(int id, uint32_t data, uint32_t mask);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
};

#endif
