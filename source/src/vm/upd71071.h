/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#ifndef _UPD71071_H_
#define _UPD71071_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_UPD71071_CH0	0
#define SIG_UPD71071_CH1	1
#define SIG_UPD71071_CH2	2
#define SIG_UPD71071_CH3	3

class UPD71071 : public DEVICE
{
private:
	DEVICE* d_mem;
#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
#endif
	outputs_t outputs_tc;
	
	struct {
		DEVICE* dev;
		uint32 areg, bareg;
		uint16 creg, bcreg;
		uint8 mode;
	} dma[4];
	
	uint8 b16, selch, base;
	uint16 cmd, tmp;
	uint8 req, sreq, mask, tc;
	
public:
	UPD71071(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			dma[i].dev = vm->dummy;
		}
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
		init_output_signals(&outputs_tc);
	}
	~UPD71071() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void do_dma();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_ch0(DEVICE* device)
	{
		dma[0].dev = device;
	}
	void set_context_ch1(DEVICE* device)
	{
		dma[1].dev = device;
	}
	void set_context_ch2(DEVICE* device)
	{
		dma[2].dev = device;
	}
	void set_context_ch3(DEVICE* device)
	{
		dma[3].dev = device;
	}
#ifdef SINGLE_MODE_DMA
	void set_context_child_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
	void set_context_tc(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_tc, device, id, mask);
	}
};

#endif

