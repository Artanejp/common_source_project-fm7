/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2018.04.01-

	[ sasi i/f ]
*/

#ifndef _SASI_H_
#define _SASI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SASI_IRQ	0
#define SIG_SASI_DRQ	1
#define SIG_SASI_TC	2

class SASI_HDD;

class SASI : public DEVICE
{
private:
	DEVICE *d_host;
	SASI_HDD *d_hdd;
	DEVICE *d_dma, *d_pic;
	
	uint8_t ocr;
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
//	void write_dma_io8(uint32_t addr, uint32_t data);
//	uint32_t read_dma_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_host(DEVICE* device)
	{
		d_host = device;
	}
	void set_context_hdd(SASI_HDD* device)
	{
		d_hdd = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

