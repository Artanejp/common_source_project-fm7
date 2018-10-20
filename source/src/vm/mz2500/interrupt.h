/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2007.02.11 -

	[ interrupt ]
*/

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_INTERRUPT_CRTC	0
#define SIG_INTERRUPT_I8253	1
#define SIG_INTERRUPT_PRINTER	2
#define SIG_INTERRUPT_RP5C15	3

class INTERRUPT : public DEVICE
{
private:
	uint8_t select;
	
	// interrupt
	struct {
		uint8_t vector;
		bool enb_intr;
		bool req_intr;
		bool in_service;
	} irq[4];
	int req_intr_ch;
	
	// z80 daisy chain
	DEVICE *d_cpu, *d_child;
	bool iei, oei;
	uint32_t intr_bit;
	void update_intr();
	
public:
	INTERRUPT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_cpu = d_child = NULL;
		set_device_name(_T("Interrupt"));
	}
	~INTERRUPT() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32_t bit)
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void set_context_child(DEVICE* device)
	{
		d_child = device;
	}
	void set_intr_iei(bool val);
	uint32_t get_intr_ack();
	void notify_intr_reti();
};

#endif

