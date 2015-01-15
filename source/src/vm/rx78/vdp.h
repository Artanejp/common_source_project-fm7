/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ vdp ]
*/

#ifndef _VDP_H_
#define _VDP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class VDP : public DEVICE
{
private:
	DEVICE* d_cpu;
	
	scrntype palette_pc[17];	// 8cols * 2 + bg
	uint8 screen0[184][192];
	uint8 screen1[184][192];
	
	uint8* vram[6];
	uint8 reg[6], bg, cmask, pmask;
	
	void create_pal();
	void create_bg();
	
public:
	VDP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~VDP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	void event_vline(int v, int clock);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_vram_ptr(uint8* ptr)
	{
		for(int i = 0; i < 6; i++) {
			vram[i] = ptr + 0x2000 * i;
		}
	}
	void draw_screen();
};

#endif
