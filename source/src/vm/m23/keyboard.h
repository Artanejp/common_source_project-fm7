/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace M23 {
class KEYBOARD : public DEVICE
{
private:
	bool key_locked[4];
	bool led_status[4];
	uint8_t buffer[0x200];
	uint16_t buffer_ptr;

public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}

	// common functions
	void initialize() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	bool get_caps_locked()
	{
		return !key_locked[1];
	}
	bool get_kana_locked()
	{
		return key_locked[3];
	}
	uint32_t get_led_status()
	{
		return (led_status[0] ? 1 : 0) | (led_status[1] ? 2 : 0) | (led_status[2] ? 4 : 0) | (led_status[3] ? 8 : 0);
	}
	void __FASTCALL key_down(int code);
};
}

#endif
