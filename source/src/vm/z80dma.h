/*
	Skelton for retropc emulator

	Origin : MAME Z80DMA / Xmillenium
	Author : Takeda.Toshiya
	Date   : 2011.04.96-

	[ Z80DMA ]
*/

#ifndef _Z80DMA_H_
#define _Z80DMA_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_Z80DMA_READY	0

class Z80DMA : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
	
	typedef union {
		uint16 m[7][8];
		uint16 t[6*8+1+1];
	} regs_t;
	regs_t regs;
	uint8 status;
	
	uint16 wr_tmp[4];
	int wr_num, wr_ptr;
	uint16 rr_tmp[7];
	int rr_num, rr_ptr;
	
	bool enabled;
	uint32 ready;
	bool force_ready;
	
	uint16 addr_a;
	uint16 addr_b;
	int upcount;
	int blocklen;
	bool dma_stop;
	bool bus_master;
	
	// interrupt
	bool req_intr;
	bool in_service;
	uint8 vector;
	
	bool now_ready();
	void request_bus();
	void release_bus();
	void update_read_buffer();
	void request_intr(int level);
	
	// daisy chain
	DEVICE *d_cpu, *d_child;
	bool iei, oei;
	uint32 intr_bit;
	void update_intr();
	
public:
	Z80DMA(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 6 * 8 + 1 + 1; i++) {
			regs.t[i] = 0;
		}
		d_cpu = d_child = NULL;
	}
	~Z80DMA() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void do_dma();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32 bit)
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void set_context_child(DEVICE* device)
	{
		d_child = device;
	}
	void set_intr_iei(bool val);
	uint32 intr_ack();
	void intr_reti();
	
	// unique function
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
};

#endif

