/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ beep ]
*/

#ifndef _BEEP_H_
#define _BEEP_H_

#include "../pcm1bit.h"

namespace M23 {
class BEEP: public PCM1BIT
{
private:
	uint8_t reg;

public:
	BEEP(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : PCM1BIT(parent_vm, parent_emu)
	{
		set_device_name(_T("Beep"));
	}
	~BEEP() {}

	// common functions
	void initialize() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};
}

#endif
