/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : Takeda.Toshiya
	Date   : 2014.05.22-

	[ timer ]
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_TIMER_IRQ_SUB_CPU	0
#define SIG_TIMER_IRQ_JOYSTICK	1
#define SIG_TIMER_IRQ_TIMER	2
#define SIG_TIMER_IRQ_VOICE	3
#define SIG_TIMER_IRQ_VRTC	4
#define SIG_TIMER_IRQ_RS_232C	5
#define SIG_TIMER_IRQ_PRINTER	6
#define SIG_TIMER_IRQ_EXT_INT	7

#ifndef _PC6001
class MEMORY;
#endif

class TIMER : public DEVICE
{
private:
	DEVICE *d_cpu, *d_sub;
#ifndef _PC6001
	MEMORY *d_mem;
#endif
	
	uint8 IRQ, NewIRQ;
	int timer_id;
	
#ifndef _PC6001
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	uint8 sr_vectors[8];
	uint8 portFA;
	uint8 portFB;
#endif
	uint8 portF3;
	uint8 portF4;
	uint8 portF5;
	uint8 portF6;
	uint8 portF7;
#endif
	void update_intr();
	
public:
	TIMER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~TIMER() {}
	
	// common functions
	void initialize();
	void reset();
#ifndef _PC6001
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
#endif
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 intr_ack();
	void intr_reti();
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_sub(DEVICE* device)
	{
		d_sub = device;
	}
#ifndef _PC6001
	void set_context_memory(MEMORY* device)
	{
		d_mem = device;
	}
#endif
	void set_portB0(uint32 data);
};
#endif
