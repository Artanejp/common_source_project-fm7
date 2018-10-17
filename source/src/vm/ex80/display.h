/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_DMA		0

class DISPLAY : public DEVICE
{
private:
	DEVICE *d_cpu;
	
	uint8_t font[0x400];
	uint8_t screen[8 * 29 * 2][8 * 12];
	
	uint8_t *ram;
	int odd_even;
	bool dma;
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_ram_ptr(uint8_t* ptr)
	{
		ram = ptr;
	}
	void draw_screen();
};

#endif

