/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2010.09.02 -

	[ cmos memory ]
*/

#ifndef _CMOS_H_
#define _CMOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MZ700 {
	
class CMOS : public DEVICE
{
private:
	uint8_t *data_buffer;
	uint32_t data_addr;
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
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};

}
#endif

