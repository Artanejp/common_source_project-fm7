/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

namespace M23 {
class MEMBUS : public MEMORY
{
private:
	uint8_t ram[0x20000];
#if defined(_M68)
	uint8_t rom[0x1000];
#else
	uint8_t rom[0x800];
#endif

	bool rom_selected;
	bool page;
	bool page_after_jump, after_jump;
	bool page_exchange;
	bool dma_bank;

	void update_bank();

public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}

	// common functions
	void initialize() override;
	void reset() override;
	uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait) override;
	uint32_t __FASTCALL read_data8w(uint32_t addr, int *wait) override;
	void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int *wait) override;
	uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	void __FASTCALL write_dma_data8w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique function
	uint8_t* get_vram()
	{
		return ram + 0x1f000;
	}
};
}

#endif
