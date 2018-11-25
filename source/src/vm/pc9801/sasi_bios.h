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
	
	bool sasi_bios(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	int sxsi_get_drive(uint8_t al);
	void sasi_command_verify(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_retract(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_illegal(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_initialize(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_sense(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_read(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_write(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	void sasi_command_format(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	long sasi_get_position(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);

public:
	BIOS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("SASI PSEUDO BIOS"));
	}
	~BIOS() {}
	
	// common functions
	void reset();
	void initialize();
	bool bios_int_i86(int intnum, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	bool bios_call_far_i86(uint32_t PC, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	
	// unique functions
	void set_context_sasi(SASI* device)
	{
		d_sasi = device;
	}
	void set_context_memory(MEMBUS* device)
	{
		d_mem = device;
	}
};

}
#endif

