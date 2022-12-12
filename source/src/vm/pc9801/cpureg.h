/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

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
#if defined(HAS_SUB_V30)
class I86;
#endif

class CPUREG : public DEVICE
{
private:
#if defined(SUPPORT_32BIT_ADDRESS)
	I386 *d_cpu;
#else
	I286 *d_cpu;
#endif
#if defined(HAS_SUB_V30)
	I86 *d_v30;
	DEVICE *d_pio;
#endif
	bool nmi_enabled;
	
public:
	CPUREG(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CPU I/O"));
	}
	~CPUREG() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#if defined(HAS_SUB_V30)
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int clock);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
#if defined(SUPPORT_32BIT_ADDRESS)
	void set_context_cpu(I386* device)
#else
	void set_context_cpu(I286* device)
#endif
	{
		d_cpu = device;
	}
#if defined(HAS_SUB_V30)
	void set_context_v30(I86* device)
	{
		d_v30 = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	bool cpu_mode;
#endif
};

#endif

