/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2018.04.01-

	[ sasi i/f ]
*/

#ifndef _SASI_BIOS_H_
#define _SASI_BIOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"


class SASI_HDD;
class HARDDISK;
class I386;
class I8086;
class I80286;

namespace PC9801 {
	class MEMBUS;
	class SASI;
}

namespace PC9801 {
class BIOS : public DEVICE
{
protected:
	MEMBUS *d_mem;
	SASI   *d_sasi;
	DEVICE *d_cpu;
	DEVICE *d_pic;
	DEVICE *d_cpureg;
	
	int event_halt;
	int event_irq;
	
	void __FASTCALL halt_host_cpu(double usec);
	void __FASTCALL interrupt_to_host(double usec);
	
	int __FASTCALL sxsi_get_drive(uint8_t al);
	long __FASTCALL sasi_get_position(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	
	void __FASTCALL sasi_command_verify(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_retract(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_illegal(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_initialize(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_sense(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_read(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_write(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void __FASTCALL sasi_command_format(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	
	bool sasi_bios(uint32_t PC, uint32_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);

public:
	BIOS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = NULL;
		d_sasi = NULL;
		d_cpu = NULL;
		d_pic = NULL;
		d_cpureg = NULL;
		set_device_name(_T("SASI PSEUDO BIOS"));
	}
	~BIOS() {}
	
	// common functions
	void reset();
	void initialize();
	bool bios_int_i86(int intnum, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
	bool bios_call_far_i86(uint32_t PC, uint16_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
	bool bios_int_ia32(int intnum, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
	bool bios_call_far_ia32(uint32_t PC, uint32_t regs[], const uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag, int* cycles, uint64_t* total_cycles);
	void __FASTCALL event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_sasi(SASI* device)
	{
		d_sasi = device;
	}
	void set_context_memory(MEMBUS* device)
	{
		d_mem = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_cpureg(DEVICE* device)
	{
		d_cpureg = device;
	}
};

}
#endif

