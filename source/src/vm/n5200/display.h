/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

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
	DEVICE *d_pic;
	uint8_t *vram, *tvram;
	
	bool vsync_enb;
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	void event_vline(int v, int clock);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
	void set_tvram_ptr(uint8_t* ptr)
	{
		tvram = ptr;
	}
	void draw_screen();
};

#endif

