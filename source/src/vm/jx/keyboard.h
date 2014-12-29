/*
	IBM Japan Ltd PC/JX Emulator 'eJX'

	Author : Takeda.Toshiya
	Date   : 2011.05.10-

	[ keyboard ]
*/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_KEYBOARD_TIMER	0

class FIFO;

class KEYBOARD : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pio, *d_pit;
	
	FIFO* key_buf;
#ifndef KEYBOARD_HACK
	uint64 send_data;
	int send_count;
#endif
	bool key_latched;
	
	uint8 nmi_reg;
	
public:
	KEYBOARD(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KEYBOARD() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
	void key_down(int code);
	void key_up(int code);
};

#endif

