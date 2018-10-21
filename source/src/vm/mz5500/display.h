/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

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
	uint8_t screen[400][640];
	uint16_t tmp[640];
	scrntype_t palette_pc_base[8];
	scrntype_t palette_pc[8];
	
	uint8_t *vram_b, *vram_r, *vram_g, *mapram;
	uint8_t *sync, *ra, *cs;
	int* ead;
	
	uint8_t palette[8];
	uint8_t back[5], reverse[5];
	uint8_t rno, wregs[16];
	int pri[16];
	int vma[5];
	uint8_t vds[5];
	uint8_t mode_r, mode_c, mode_p;
	
	void draw_640dot_screen(int ymax);
	void draw_320dot_screen(int ymax);
	void update_palette();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_vram_ptr(uint8_t* ptr)
	{
		vram_b = ptr + 0x00000;
		vram_r = ptr + 0x10000;
		vram_g = ptr + 0x20000;
		mapram = ptr + 0x60000;
	}
	void set_sync_ptr(uint8_t* ptr)
	{
		sync = ptr;
	}
	void set_ra_ptr(uint8_t* ptr)
	{
		ra = ptr;
	}
	void set_cs_ptr(uint8_t* ptr)
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

