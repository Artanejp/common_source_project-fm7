
#pragma once

#include "./js_template.h"
namespace FMTOWNS {

class JOYPAD_2BTN : public JSDEV_TEMPLATE {
protected:
	virtual uint8_t __FASTCALL hook_changed_com(bool changed);

public:
	JOYPAD_2BTN(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : JSDEV_TEMPLATE(parent_vm, parent_emu)
	{
		set_device_name(_T("2 buttons JOY PAD"));
	}
	~JOYPAD_2BTN() {}

	virtual void initialize();
	virtual uint8_t __FASTCALL query(bool& status);

};
}
