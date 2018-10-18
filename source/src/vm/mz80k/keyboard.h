/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_COLUMN	0

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_pio;
	
#if defined(_MZ80K) || defined(_MZ1200)
	uint8_t key_buf[256];
#endif
	const uint8_t* key_stat;
	uint8_t column;
	bool kana;
	void update_key();
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void key_down(int code);
	bool get_caps_locked()
	{
//		return caps;
		return true;
	}
	bool get_kana_locked()
	{
		return kana;
	}
};

#endif
