/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.14 -

	[ crtc ]
*/

#ifndef _CRTC_H_
#define _CRTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CRTC_BITMASK_LOW	0
#define SIG_CRTC_BITMASK_HIGH	1
#define SIG_CRTC_VRAM_PLANE	2

class CRTC : public DEVICE
{
private:
	DEVICE *d_pic;
	
	scrntype palette_pc[16];
	uint16 palette[16];
	uint8 sel, regs[8];
	uint16 vs, cmd;
	
	uint8 vram[0x80000];	// VRAM 128KB * 4planes
	uint32 shift, maskl, maskh, busl, bush;
	uint32 write_plane, read_plane;
	
	void update_palette(int num);
	
public:
	CRTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CRTC() {}
	
	// common functions
	void initialize();
	void event_vline(int v, int clock);
	
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_memory_mapped_io8(uint32 addr, uint32 data);
	uint32 read_memory_mapped_io8(uint32 addr);
	void write_memory_mapped_io16(uint32 addr, uint32 data);
	uint32 read_memory_mapped_io16(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void draw_screen();
};

#endif

