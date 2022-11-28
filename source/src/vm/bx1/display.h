/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class DISPLAY : public DEVICE
{
private:
	DEVICE *d_dma;
	
	int buffer_ptr;
	uint8_t buffer[16];
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_dma_io8(uint32_t addr, uint32_t data);
	void event_frame();
	
	// unique functions
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void draw_screen();
};

#endif
