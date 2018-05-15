/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
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

class SASI : public DEVICE
{
private:
	DEVICE *d_dma, *d_pic;
	
	uint8_t ocr, isr, isr_dev;
	
public:
	SASI(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("SASI I/F"));
	}
	~SASI() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
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

