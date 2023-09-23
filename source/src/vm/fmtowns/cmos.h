/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ CMOS RAM area 0x000D8000h - 0x000D9FFFh , also C2140000h - C2141FFFh ]
	* MEMORY :
	*   0x000d8000 - 0x000d9fff : DICTIONARY RAM / GAIJI RAM
	*   0xc2140000 - 0xc2141fff : DICTIONARY RAM
	* I/O :
	*   0x3000 - 0x3ffe (even address) : DICTIONARY RAM
*/

#pragma once

#include "../../common.h"
#include "../device.h"

class DEBUGGER;
namespace FMTOWNS {

class CMOS : public DEVICE
{
protected:
	DEBUGGER* d_debugger;
	uint8_t ram[0x2000];  // 2 + 6KB
	bool cmos_dirty;

	virtual void save_ram();
public:
	CMOS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_debugger = NULL;
		set_device_name("FM-Towns CMOS RAM 8K Bytes");
	}
	~CMOS() {}
	void initialize() override;
	void release() override;

	void reset() override;

	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr) override;
	void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data) override;

	uint32_t __FASTCALL read_memory_mapped_io8w(uint32_t addr, int* wait) override;
	void __FASTCALL write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait) override;

	uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	void __FASTCALL write_dma_data8w(uint32_t addr,  uint32_t data, int* wait) override;

	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;

	bool is_debugger_available() override
	{
		return (d_debugger != NULL) ? true : false;
	}
	uint64_t get_debug_data_addr_space() override
	{
		return 0x2000;
	}
	void *get_debugger() override
	{
		return d_debugger;
	}

	uint32_t __FASTCALL read_debug_data8(uint32_t addr) override;
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data) override;

	uint32_t __FASTCALL read_via_debugger_data8(uint32_t addr) override;
	void __FASTCALL write_via_debugger_data8(uint32_t addr, uint32_t data) override;

	uint32_t __FASTCALL read_via_debugger_data8w(uint32_t addr, int* wait) override;
	void __FASTCALL write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait) override;
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len) override;

	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_context_debugger(DEBUGGER *device)
	{
		d_debugger = device;
	}

};
}
