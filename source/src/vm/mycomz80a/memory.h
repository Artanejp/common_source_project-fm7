/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.13-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MYCOMZ80A {

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu;
	DEVICE *d_fdc;
	DEVICE *d_pio;
	
	uint8_t* rbank[16];
	uint8_t* wbank[16];
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t ram[0x10000];	// Main RAM 64KB
	uint8_t bios[0x3000];	// IPL 12KB
	uint8_t basic[0x1000];	// BASIC 4KB
	
	uint32_t addr_mask;
	bool rom_sel;
	uint8_t drv_sel, nmi_req;
	
	void update_memory_map();
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_cpu = nullptr;
		d_fdc = nullptr;
		d_pio = nullptr;
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait) override;
	void __FASTCALL write_data8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_data8(uint32_t addr) override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}

};

}
#endif

