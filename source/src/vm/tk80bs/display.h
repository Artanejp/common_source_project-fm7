/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

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
#if defined(_TK80BS)
	uint8_t font[0x1000];
	uint8_t *vram;
	int mode;
#endif
	uint8_t *led;
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
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#if defined(_TK80BS)
	void set_vram_ptr(uint8_t* ptr)
	{
		vram = ptr;
	}
#endif
	void set_led_ptr(uint8_t* ptr)
	{
		led = ptr;
	}
	void draw_screen();
};

#endif

