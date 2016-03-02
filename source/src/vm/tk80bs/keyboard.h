/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'

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
	DEVICE *d_pio_b, *d_pio_t, *d_cpu;
	
	const uint8_t* key_stat;
	uint8_t prev_type, prev_brk, prev_kana;
	bool kana_lock;
	uint32_t column, kb_type;
	
	void update_tk80();
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_intr_ack();
	uint32_t read_signal(int ch)
	{
		return kb_type & 3;
	}
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_pio_b(DEVICE* device)
	{
		d_pio_b = device;
	}
	void set_context_pio_t(DEVICE* device)
	{
		d_pio_t = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif

