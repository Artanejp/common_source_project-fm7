/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2018.05.15-

	[ SASI I/F ]
*/

#ifndef _SASI_H_
#define _SASI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SASI_IRQ	0
#define SIG_SASI_DRQ	1

class SASI : public DEVICE
{
private:
	DEVICE *d_host;
#ifdef _X1TURBO_FEATURE
	DEVICE *d_dma;
#endif
	
	bool irq_status;
	bool drq_status;
	
public:
	SASI(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("SASI I/F"));
	}
	~SASI() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_host(DEVICE* device)
	{
		d_host = device;
	}
#ifdef _X1TURBO_FEATURE
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
};

#endif

