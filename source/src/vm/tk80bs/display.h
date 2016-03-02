/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_MODE	0
#define SIG_DISPLAY_DMA		1

class DISPLAY : public DEVICE
{
private:
	DEVICE* d_key;
	
	uint8_t font[0x1000];
	scrntype_t screen[36][256];
	
	uint8_t *vram, *led;
	int mode;
	bool dma;
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_key(DEVICE* device)
	{
		d_key = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
	void set_led_ptr(uint8_t* ptr)
	{
		led = ptr;
	}
	void draw_screen();
};

#endif

