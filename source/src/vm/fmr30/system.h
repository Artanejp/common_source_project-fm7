/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ system ]
*/

#ifndef _FMR30_SYSTEM_H_
#define _FMR30_SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace FMR30 {

class SYSTEM : public DEVICE
{
private:
	uint8_t arr;
	uint8_t nmistat, nmimask;
	
public:
	SYSTEM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSTEM() {}
	
	// common functions
	void initialize() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};

}
#endif

