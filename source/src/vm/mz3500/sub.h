/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ sub pcb ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SUB : public DEVICE
{
private:
	DEVICE *d_main;
	
	// memory
	uint8_t* rbank[32];	// 64KB / 2KB
	uint8_t* wbank[32];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ram[0x4000];
	uint8_t kanji[0x20000];
	uint8_t* ipl;
	uint8_t* common;
	
	// display
	uint8_t vram_chr[0x2000];
	uint8_t *sync_chr, *ra_chr, *cs_chr;
	int* ead_chr;
	uint8_t vram_gfx[0x18000];
	uint8_t *sync_gfx, *ra_gfx, *cs_gfx;
	int* ead_gfx;
	uint8_t disp[16];
	
	uint8_t screen_chr[400][640];
	uint8_t screen_gfx[400][640];
	uint8_t font[0x2000];
	int cblink;
	bool crt_400line;
	
	void draw_chr_400line();
	void draw_chr_200line();
	void draw_gfx_400line();
	void draw_gfx_200line_16bit();
	void draw_gfx_200line_8bit();
	
public:
	SUB(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus (Sub)"));
	}
	~SUB() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_main(DEVICE* device)
	{
		d_main = device;
	}
	void set_ipl(uint8_t* ptr)
	{
		ipl = ptr;
	}
	void set_common(uint8_t* ptr)
	{
		common = ptr;
	}
	uint8_t* get_vram_chr()
	{
		return vram_chr;
	}
	void set_sync_ptr_chr(uint8_t* ptr)
	{
		sync_chr = ptr;
	}
	void set_ra_ptr_chr(uint8_t* ptr)
	{
		ra_chr = ptr;
	}
	void set_cs_ptr_chr(uint8_t* ptr)
	{
		cs_chr = ptr;
	}
	void set_ead_ptr_chr(int* ptr)
	{
		ead_chr = ptr;
	}
	uint8_t* get_vram_gfx()
	{
		return vram_gfx;
	}
	void set_sync_ptr_gfx(uint8_t* ptr)
	{
		sync_gfx = ptr;
	}
	void set_ra_ptr_gfx(uint8_t* ptr)
	{
		ra_gfx = ptr;
	}
	void set_cs_ptr_gfx(uint8_t* ptr)
	{
		cs_gfx = ptr;
	}
	void set_ead_ptr_gfx(int* ptr)
	{
		ead_gfx = ptr;
	}
	void draw_screen();
};

#endif

