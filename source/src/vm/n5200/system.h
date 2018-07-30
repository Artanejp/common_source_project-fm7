/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2009.06.04-

	[ system i/o ]
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SYSTEM : public DEVICE
{
private:
	DEVICE *d_dma;
	
	uint8_t mode;
	bool nmi_enb;
	
public:
	SYSTEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSTEM() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
};

#endif

