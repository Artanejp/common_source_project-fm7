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
	
	scrntype_t palette_pc[17];	// 8cols * 2 + bg
	uint8_t screen0[184][192];
	uint8_t screen1[184][192];
	
	uint8_t* vram[6];
	uint8_t reg[6], bg, cmask, pmask;
	
	void create_pal();
	void create_bg();
	
public:
	VDP(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("VDP"));
	}
	~VDP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	void event_vline(int v, int clock);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		for(int i = 0; i < 6; i++) {
			vram[i] = ptr + 0x2000 * i;
		}
	}
	void draw_screen();
};

#endif
