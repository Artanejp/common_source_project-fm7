/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.17-

	[ MZ-1R12 (32KB SRAM) ]
*/

#ifndef _MZ1R12_H_
#define _MZ1R12_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MZ2500 {

class MZ1R12 : public DEVICE
{
private:
	uint8_t sram[0x8000];
	bool read_only;
	uint16_t address;
	uint32_t crc32;
	
public:
	MZ1R12(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-1R12 (32KB SRAM)"));
	}
	~MZ1R12() {}
	
	// common functions
	void initialize() override;
	void release() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};

}
#endif

