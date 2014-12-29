/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#ifndef _I8237_H_
#define _I8237_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8237_CH0	0
#define SIG_I8237_CH1	1
#define SIG_I8237_CH2	2
#define SIG_I8237_CH3	3
#define SIG_I8237_BANK0	4
#define SIG_I8237_BANK1	5
#define SIG_I8237_BANK2	6
#define SIG_I8237_BANK3	7
#define SIG_I8237_MASK0	8
#define SIG_I8237_MASK1	9
#define SIG_I8237_MASK2	10
#define SIG_I8237_MASK3	11

class I8237 : public DEVICE
{
private:
	DEVICE* d_mem;
#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
#endif
	
	struct {
		DEVICE* dev;
		uint16 areg;
		uint16 creg;
		uint16 bareg;
		uint16 bcreg;
		uint8 mode;
		// external bank
		uint16 bankreg;
		uint16 incmask;
	} dma[4];
	
	bool low_high;
	uint8 cmd;
	uint8 req;
	uint8 mask;
	uint8 tc;
	uint32 tmp;
	bool mode_word;
	
	void write_mem(uint32 addr, uint32 data);
	uint32 read_mem(uint32 addr);
	void write_io(int ch, uint32 data);
	uint32 read_io(int ch);
	
public:
	I8237(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			dma[i].dev = vm->dummy;
			dma[i].bankreg = dma[i].incmask = 0;
		}
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
		mode_word = false;
	}
	~I8237() {}
	
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
	void set_mode_word(bool val)
	{
		mode_word = val;
	}
};

#endif

