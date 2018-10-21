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
	
	scrntype_t palette_pc[16];
	uint16_t palette[16];
	uint8_t sel, regs[8];
	uint16_t vs, cmd;
	
	uint8_t vram[0x80000];	// VRAM 128KB * 4planes
	uint32_t shift, maskl, maskh, busl, bush;
	uint32_t write_plane, read_plane;
	
	void update_palette(int num);
	
public:
	CRTC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CRTC"));
	}
	~CRTC() {}
	
	// common functions
	void initialize();
	void event_vline(int v, int clock);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io8(uint32_t addr);
	void write_memory_mapped_io16(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io16(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void draw_screen();
};

#endif

