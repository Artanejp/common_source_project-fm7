/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ cmos ]
*/

#ifndef _CMOS_H_
#define _CMOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace FMR30 {

class CMOS : public DEVICE
{
private:
	uint8_t cmos[0x2000];
	bool modified;
	
public:
	CMOS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CMOS RAM"));
	}
	~CMOS() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t  __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique function
	uint8_t* get_cmos()
	{
		return cmos;
	}
};

}
#endif

