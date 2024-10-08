/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2014.12.26-

	[ kanji/dictionary rom ]
*/

#ifndef _KANJI_H_
#define _KANJI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MZ700 {
	
class KANJI : public DEVICE
{
private:
	uint8_t kanji[0x20000];
	uint8_t dic[0x40000];
	uint32_t control_reg;
	uint32_t kanji_addr, dic_addr;
	
public:
	KANJI(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Kanji ROM"));
	}
	~KANJI() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};

}
#endif

