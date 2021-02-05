/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2020.08.22-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm_template.h"
#include "../../emu_template.h"
#include "../device.h"

namespace BX1 {
class DISPLAY : public DEVICE
{
private:
	DEVICE *d_cpu;
	DEVICE *d_dma;
	uint8_t *ram;
	uint8_t font[8*256];
	int ptr;
	uint8_t buffer[16];
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	virtual void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	virtual void event_frame();
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_ram(uint8_t* ptr)
	{
		ram = ptr;
	}
	void draw_screen();
};
}
#endif
