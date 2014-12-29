/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2010.08.10-

	[ M6502 ]
*/

#ifndef _M6502_H_ 
#define _M6502_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_M6502_OVERFLOW	0

class M6502 : public DEVICE
{
private:
	DEVICE *d_mem, *d_pic;
	
	pair pc, sp, zp, ea;
	uint16 prev_pc;
	uint8 a, x, y, p;
	bool pending_irq, after_cli;
	bool nmi_state, irq_state, so_state;
	int icount;
	bool busreq;
	
	void run_one_opecode();
	void OP(uint8 code);
	void update_irq();
	
public:
	M6502(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		busreq = false;
	}
	~M6502() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	void set_intr_line(bool line, bool pending, uint32 bit)
	{
		write_signal(SIG_CPU_IRQ, line ? 1 : 0, 1);
	}
	uint32 get_pc()
	{
		return prev_pc;
	}
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

