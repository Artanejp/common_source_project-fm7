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

#if defined(SUPPORT_32BIT_ADDRESS)
class I386;
#else
class I286;
#endif
#if !defined(SUPPORT_HIRESO)
class I86;
#endif

namespace PC9801 {

class CPUREG : public DEVICE
{
private:
	outputs_t outputs_nmi;
	outputs_t outputs_cputype;
	uint8_t reg_0f0;
	
#if defined(SUPPORT_32BIT_ADDRESS) || defined(UPPER_I386)
	I386 *d_cpu;
#else
	I286 *d_cpu;
#endif
#if !defined(SUPPORT_HIRESO)
	I86 *d_v30;
	DEVICE *d_pio;
#endif
	bool nmi_enabled;
	uint64_t init_clock;
	int event_wait;
public:
	CPUREG(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_nmi);
		initialize_output_signals(&outputs_cputype);
		set_device_name(_T("CPU I/O"));
	}
	~CPUREG() {}
	
	// common functions
	void reset();
	void initialize();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
#if !defined(SUPPORT_HIRESO)
	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit);
#endif
	void __FASTCALL event_callback(int id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
#if defined(SUPPORT_32BIT_ADDRESS) || defined(UPPER_I386)
	void set_context_cpu(I386* device)
#else
	void set_context_cpu(I286* device)
#endif
	{
		d_cpu = device;
	}
#if !defined(SUPPORT_HIRESO)
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


}
#endif

