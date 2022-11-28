/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ emm ]
*/

#ifndef _EMM_H_
#define _EMM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define EMM_BUFFER_SIZE	0x50000

class EMM : public DEVICE
{
private:
	uint8_t data_buffer[EMM_BUFFER_SIZE];
	uint32_t data_addr;
	
public:
	EMM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("EMM"));
	}
	~EMM() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#ifdef USE_DEBUGGER
	bool is_debugger_available()
	{
		return true;
	}
	uint64_t get_debug_data_addr_space()
	{
		return EMM_BUFFER_SIZE;
	}
	void write_debug_data8(uint32_t addr, uint32_t data)
	{
		if(addr < EMM_BUFFER_SIZE) {
			data_buffer[addr] = data;
		}
	}
	uint32_t read_debug_data8(uint32_t addr)
	{
		if(addr < EMM_BUFFER_SIZE) {
			return data_buffer[addr];
		}
		return 0;
	}
#endif
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

