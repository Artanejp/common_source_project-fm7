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

#define SIG_CPUREG_RESET	1
#define SIG_CPUREG_HALT		2

#if defined(UPPER_I386)
#include "../i386_np21.h"
#elif defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88) || defined(HAS_V30)
#include "../i86.h"
#else
#include "../i286.h"
#endif
#if defined(HAS_V30_SUB_CPU)
#include "../i86.h"
#endif

namespace PC9801 {

class CPUREG : public DEVICE
{
private:
#if defined(UPPER_I386)
	I386 *d_cpu;
#elif defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88)
	I86 *d_cpu;
#elif defined(HAS_V30)
	I86  *d_cpu;
#else
	I286 *d_cpu;
#endif
#if defined(HAS_V30_SUB_CPU)
	I86  *d_v30cpu;
#endif
	DEVICE* d_mem;
	DEVICE* d_pio;
	DEVICE* d_pic;
	uint8_t reg_0f0;
	bool nmi_enabled;
	int event_wait;
	bool stat_wait;
	bool stat_exthalt;
	uint64_t init_clock;
	
	outputs_t outputs_nmi; // NMI must route via CPUREG::
	outputs_t outputs_cputype; // CPU Type 0 = Normal/ 1 = V30(SUB)
#if defined(HAS_V30_SUB_CPU)
	bool use_v30;
	bool enable_v30;
	void halt_by_use_v30();
#endif
	void halt_by_value(bool val);
	
public:
	CPUREG(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		event_wait = -1;
		initialize_output_signals(&outputs_nmi);
		initialize_output_signals(&outputs_cputype);
		set_device_name(_T("CPU I/O"));
	}
	~CPUREG() {}
	
	// common functions
#if defined(HAS_V30_SUB_CPU)
	void initialize();
#endif
	void reset();
	void  __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t  __FASTCALL read_io8(uint32_t addr);
	// NOTE: NMI must route CPUREG::, should not connect directly.20190502 K.O 
	void  __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	void event_callback(int id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_intr_line(bool line, bool pending, uint32_t bit);
	uint32_t get_intr_ack();

#if defined(UPPER_I386)
	void set_context_cpu(I386* device)
#elif defined(HAS_I86) || defined(HAS_I186) || defined(HAS_I88)
	void set_context_cpu(I86* device)
#elif defined(HAS_V30)
	void set_context_cpu(I86* device)
#else
	void set_context_cpu(I286* device)
#endif
	{
		d_cpu = device;
		register_output_signal(&outputs_nmi, device, SIG_CPU_NMI, 0xffffffff, 0);
	}
	// This will be feature developing, still not implement V30 feature.20190502 K.O
#if defined(HAS_V30_SUB_CPU)
	void set_context_v30(I86* device)
	{
		d_v30cpu = device;
		register_output_signal(&outputs_nmi, device, SIG_CPU_NMI, 0xffffffff, 0);
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
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_cputype(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs_cputype, device, id, mask, shift);
	}
};


}
#endif

