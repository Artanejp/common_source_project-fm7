/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

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
#if defined(_TK80BS)
	DEVICE *d_pio_b, *d_cpu;
	
	uint8_t prev_type, prev_brk, prev_kana;
	bool kana_lock;
	uint32_t kb_type;
#endif
	
	DEVICE *d_pio_t;
	const uint8_t* key_stat;
	
	uint32_t column;
	
	void update_tk80();
	
public:
	KEYBOARD(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Keyboard"));
	}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_intr_ack();
#if defined(_TK80BS)
	uint32_t read_signal(int ch)
	{
		return kb_type & 3;
	}
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#if defined(_TK80BS)
	void set_context_pio_b(DEVICE* device)
	{
		d_pio_b = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
#endif
	void set_context_pio_t(DEVICE* device)
	{
		d_pio_t = device;
	}
	void key_down(int code);
	void key_up(int code);
	bool get_caps_locked()
	{
//		return caps_lock;
		return true;
	}
	bool get_kana_locked()
	{
#if defined(_TK80BS)
		return kana_lock;
#else
		return false;
#endif
	}
};

#endif

