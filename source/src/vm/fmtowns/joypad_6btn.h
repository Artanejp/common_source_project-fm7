
#pragma once

#include "./joypad_2btn.h"

namespace FMTOWNS {

class JOYPAD_6BTN : public JOYPAD_2BTN {
public:
	JOYPAD_6BTN(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : JOYPAD_2BTN(parent_vm, parent_emu)
	{
		set_device_name(_T("6 buttons JOY PAD"));
	}
	~JOYPAD_6BTN() {}

	virtual void initialize();
	virtual uint8_t __FASTCALL query(bool& status);
};
}
