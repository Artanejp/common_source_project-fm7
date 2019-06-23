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

#define SIG_CPUREG_RESET 1
#define SIG_CPUREG_HALT  2

#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
#include "../i386.h"
#else
#include "../i286.h"
#endif

class I286;

namespace PC9801 {

class CPUREG : public DEVICE
{
private:
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	I386 *d_cpu;
#else
	I286 *d_cpu;
#endif
	I286 *d_v30;
	DEVICE* d_mem;
	DEVICE* d_pio;
	uint8_t reg_0f0;
	bool nmi_enabled;
	int event_wait;
	bool stat_wait;
	bool stat_exthalt;
	uint64_t init_clock;
	
	outputs_t outputs_nmi; // NMI must route via CPUREG::
	
public:
	CPUREG(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		event_wait = -1;
		initialize_output_signals(&outputs_nmi);
		set_device_name(_T("CPU I/O"));
	}
	~CPUREG() {}
	
	// common functions
	void reset();
	void  __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t  __FASTCALL read_io8(uint32_t addr);
	// NOTE: NMI must route CPUREG::, should not connect directly.20190502 K.O 
	void  __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	void event_callback(int id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
#if defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	void set_context_cpu(I386* device)
#else
	void set_context_cpu(I286* device)
#endif
	{
		d_cpu = device;
		register_output_signal(&outputs_nmi, device, SIG_CPU_NMI, 0xffffffff, 0);
	}
	// This will be feature developing, still not implement V30 feature.20190502 K.O
#if 0
	void set_context_v30(I286* device)
	{
		d_v30 = device;
		register_output_signal(&outputs_nmi, device, SIG_CPU_NMI, 0xffffffff);
	}
#endif
	void set_context_membus(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_piosys(DEVICE* device)
	{
		d_pio = device;
	}
};

}
#endif

