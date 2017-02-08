/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

#ifndef __LIBNEWDEV_MC6809_H_
#define __LIBNEWDEV_MC6809_H_

#include "../../vm.h"
#include "../../../emu.h"
#include "./mc6809_base.h"
#include "../device.h"

class VM;
class EMU;
class DEBUGGER;
class MC6809 : public MC6809_BASE
{
	// opcodes
protected:
	void run_one_opecode();
public:
	MC6809(VM* parent_vm, EMU* parent_emu) : MC6809_BASE(parent_vm, parent_emu) 
	{
		set_device_name(_T("MC6809 MPU"));
	}
	~MC6809() {}
	
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	uint32_t cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram);

	// common functions
	void initialize();
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}

};

#endif

