/*
	NEC PC-9801VX Emulator 'ePC-9801VX'

	Author : Takeda.Toshiya
	Date   : 2017.06.25-

	[ cpu regs ]
*/

#ifndef _CPUREG_H_
#define _CPUREG_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#if defined(SUPPORT_32BIT_ADDRESS)
class I386;
#else
class I286;
#endif

class CPUREG : public DEVICE
{
private:
#if defined(SUPPORT_32BIT_ADDRESS)
	I386 *d_cpu;
#else
	I286 *d_cpu;
#endif
	
public:
	CPUREG(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CPU I/O"));
	}
	~CPUREG() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
//	void save_state(FILEIO* state_fio);
//	bool load_state(FILEIO* state_fio);
	
	// unique function
#if defined(SUPPORT_32BIT_ADDRESS)
	void set_context_cpu(I386* device)
#else
	void set_context_cpu(I286* device)
#endif
	{
		d_cpu = device;
	}
};

#endif

