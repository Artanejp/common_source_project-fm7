/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MULTI8 {

class KEYBOARD : public DEVICE
{
private:
	int init;
	uint8_t code, code_prev, stat;
	bool caps, caps_prev;
	bool graph, graph_prev;
	bool kana, kana_prev;
	const uint8_t* key_stat;

public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}

	// common functions
	void initialize()  override;
	void reset()  override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data)  override;
	uint32_t __FASTCALL read_io8(uint32_t addr)  override;
	void event_frame()  override;
	bool process_state(FILEIO* state_fio, bool loading)  override;

	// unique functions
	bool get_caps_locked()
	{
		return caps;
	}
	bool get_kana_locked()
	{
		return kana;
	}
};

}
#endif
