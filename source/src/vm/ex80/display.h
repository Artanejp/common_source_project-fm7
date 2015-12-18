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
	
	uint8 font[0x400];
	uint8 screen[8 * 29 * 2][8 * 12];
	
	uint8 *ram;
	int odd_even;
	bool dma;
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_ram_ptr(uint8* ptr)
	{
		ram = ptr;
	}
	void draw_screen();
};

#endif

