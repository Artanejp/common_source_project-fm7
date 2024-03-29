/*
	MICOM MAHJONG Emulator 'eMuCom Mahjong'

	Author : Hiroaki GOTO as GORRY
	Date   : 2020.07.20 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_STROBE	0

namespace MICOM_MAHJONG {
class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_memory;

	const uint8_t* key_stat;
	uint8_t column;
	void update_key();

public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}

	// common functions
	void initialize() override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void event_frame() override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique function
	void set_context_memory(DEVICE* device)
	{
		d_memory = device;
	}
};
}
#endif
