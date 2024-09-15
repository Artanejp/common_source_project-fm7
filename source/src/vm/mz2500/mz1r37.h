/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ MZ-1R37 (640KB EMM) ]
*/

#ifndef _MZ1R37_H_
#define _MZ1R37_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MZ2500 {

class MZ1R37 : public DEVICE
{
private:
	uint8_t* buffer;
	uint32_t address;
	int tmp_buffer_size;
public:
	MZ1R37(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-1R37 (640KB EMM)"));
	}
	~MZ1R37() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};

}
#endif

