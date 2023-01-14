/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.16 -

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define VRAM_SIZE	0x20000

class UPD7220;

class DISPLAY : public DEVICE
{
private:
	UPD7220 *d_gdc;
	
#ifdef _COLOR_MONITOR
	uint8_t vram_r[VRAM_SIZE];
	uint8_t vram_g[VRAM_SIZE];
	uint8_t vram_b[VRAM_SIZE];
	scrntype_t palette_pc[8];
#else
	uint8_t vram[VRAM_SIZE];
	uint8_t font[0x10000];	// 16bytes * 256chars
	scrntype_t palette_pc[16];	// normal, intensify
#endif
	uint8_t screen[400][640];
	scrntype_t tmp[640];
	
	uint8_t bank;
	int blink;
	
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
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_gdc(UPD7220* device)
	{
		d_gdc = device;
	}
	uint8_t* get_vram()
	{
#ifdef _COLOR_MONITOR
		return vram_b;
#else
		return vram;
#endif
	}
	void draw_screen();
};

#endif

