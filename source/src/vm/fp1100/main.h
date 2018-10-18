/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.17-

	[ main pcb ]
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MAIN_INTS	0
#define SIG_MAIN_INTA	1
#define SIG_MAIN_INTB	2
#define SIG_MAIN_INTC	3
#define SIG_MAIN_INTD	4
#define SIG_MAIN_COMM	5

class MAIN : public DEVICE
{
private:
	// to main cpu
	DEVICE *d_cpu;
	// to sub pcb
	DEVICE *d_sub;
	// to slots
	DEVICE *d_slot[2][4];
	
	uint8_t *wbank[16];
	uint8_t *rbank[16];
	int wait[16];
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t ram[0x10000];
	uint8_t rom[0x9000];
	
	uint8_t comm_data;
	bool rom_sel;
	uint8_t slot_sel;
	uint8_t slot_exp[2];
	
	uint8_t intr_mask;
	uint8_t intr_request;
	uint8_t intr_in_service;
	
	void update_memory_map();
	void update_intr();
	
public:
	MAIN(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		intr_mask = intr_request = intr_in_service = 0;
		set_device_name(_T("Main CPU Bus"));
	}
	~MAIN() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
#ifdef Z80_MEMORY_WAIT
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
#endif
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#ifdef Z80_IO_WAIT
	void write_io8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_io8w(uint32_t addr, int *wait);
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_intr_ack();
	void notify_intr_reti();
	void notify_intr_ei();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE *device)
	{
		d_cpu = device;
	}
	void set_context_sub(DEVICE *device)
	{
		d_sub = device;
	}
	void set_context_slot(int slot, DEVICE *device)
	{
		slot &= 7;
		d_slot[slot >> 2][slot & 3] = device;
	}
};

#endif
