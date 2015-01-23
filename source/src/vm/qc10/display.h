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
	uint8 vram_r[VRAM_SIZE];
	uint8 vram_g[VRAM_SIZE];
	uint8 vram_b[VRAM_SIZE];
	scrntype palette_pc[8];
#else
	uint8 vram[VRAM_SIZE];
	uint8 font[0x10000];	// 16bytes * 256chars
	scrntype palette_pc[16];	// normal, intensify
#endif
	uint8 screen[400][640];
	scrntype tmp[640];
	
	uint8 *sync, *zoom, *ra, *cs;
	int* ead;
	uint8 bank;
	int blink;
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_gdc(UPD7220* device)
	{
		d_gdc = device;
	}
	uint8* get_vram()
	{
#ifdef _COLOR_MONITOR
		return vram_b;
#else
		return vram;
#endif
	}
	void set_sync_ptr(uint8* ptr)
	{
		sync = ptr;
	}
	void set_zoom_ptr(uint8* ptr)
	{
		zoom = ptr;
	}
	void set_ra_ptr(uint8* ptr)
	{
		ra = ptr;
	}
	void set_cs_ptr(uint8* ptr)
	{
		cs = ptr;
	}
	void set_ead_ptr(int* ptr)
	{
		ead = ptr;
	}
	void draw_screen();
};

#endif

