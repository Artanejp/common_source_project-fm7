/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ CZ-8RB (ROM BASIC board) ]
*/

#ifndef _CZ8RB_H_
#define _CZ8RB_H_

#include "../vm_template.h"
#include "../../emu_template.h"
#include "../device.h"

#define CZ8RB_BUFFER_SIZE	0x50000
namespace X1 {

class CZ8RB : public DEVICE
{
private:
	uint8_t data_buffer[CZ8RB_BUFFER_SIZE];
	uint32_t data_addr;
	
public:
	CZ8RB(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CZ-8RB"));
	}
	~CZ8RB() {}
	
	// common functions
	virtual void initialize();
	virtual void reset();
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
//#ifdef USE_DEBUGGER
	virtual bool is_debugger_available()
	{
		return true;
	}
	virtual uint64_t get_debug_data_addr_space()
	{
		return CZ8RB_BUFFER_SIZE;
	}
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < CZ8RB_BUFFER_SIZE) {
			data_buffer[addr] = data;
		}
	}
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr)
	{
		if(addr < CZ8RB_BUFFER_SIZE) {
			return data_buffer[addr];
		}
		return 0;
	}
//#endif
	virtual bool process_state(FILEIO* state_fio, bool loading);
};
}
#endif

