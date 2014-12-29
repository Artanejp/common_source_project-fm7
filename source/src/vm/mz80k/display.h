/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_VGATE	0
#if defined(_MZ1200) || defined(_MZ80A)
#define SIG_DISPLAY_REVERSE	1
#endif

class DISPLAY : public DEVICE
{
private:
	uint8 screen[200][320];
	uint8 font[0x800];
	uint8 *vram_ptr;
#if defined(_MZ80A)
	uint8 *e200_ptr;
#endif
	scrntype palette_pc[2];
	bool vgate;
#if defined(_MZ1200) || defined(_MZ80A)
	bool reverse;
#endif
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_vline(int v, int clock);
	
	// unique function
	void set_vram_ptr(uint8* ptr)
	{
		vram_ptr = ptr;
	}
	void draw_screen();
#if defined(_MZ80A)
	void set_e200_ptr(uint8* ptr)
	{
		e200_ptr = ptr;
	}
	
#endif
};

#endif

