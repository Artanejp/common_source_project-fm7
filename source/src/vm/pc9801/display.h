/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class UPD7220;

class DISPLAY : public DEVICE
{
private:
	DEVICE *d_pic;
	UPD7220 *d_gdc_chr, *d_gdc_gfx;
	uint8 *ra_chr;
	uint8 *ra_gfx, *cs_gfx;
	
	uint8 tvram[0x4000];
#if defined(SUPPORT_2ND_VRAM)
	uint8 vram[0x40000];
#else
	uint8 vram[0x20000];
#endif
	uint8 *vram_disp_b;
	uint8 *vram_disp_r;
	uint8 *vram_disp_g;
#if defined(SUPPORT_16_COLORS)
	uint8 *vram_disp_e;
#endif
	uint8 *vram_draw;
	
	scrntype palette_chr[8];
	scrntype palette_gfx8[8];
	uint8 digipal[4];
#if defined(SUPPORT_16_COLORS)
	scrntype palette_gfx16[8];
	uint8 anapal[16][3], anapal_sel;
#endif
	
	uint8 crtv;
	uint8 scroll[6];
	uint8 modereg1[8];
#if defined(SUPPORT_16_COLORS)
	uint8 modereg2[128];
	uint8 grcg_mode, grcg_tile_ptr, grcg_tile[4];
#endif
	
	uint8 font[0x84000];
	uint16 font_code;
	uint8 font_line;
	uint16 font_lr;
	
	uint8 screen_chr[400][641];
	uint8 screen_gfx[400][640];
	uint32 gdc_addr[480][80];
	
	void kanji_copy(uint8 *dst, uint8 *src, int from, int to);
#if defined(SUPPORT_16_COLORS)
	void write_grcg(uint32 addr, uint32 data);
	uint32 read_grcg(uint32 addr);
#endif
	void draw_chr_screen();
	void draw_gfx_screen();
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void event_frame();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_memory_mapped_io8(uint32 addr, uint32 data);
	uint32 read_memory_mapped_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_pic(DEVICE *device)
	{
		d_pic = device;
	}
	void set_context_gdc_chr(UPD7220 *device, uint8 *ra)
	{
		d_gdc_chr = device;
		ra_chr = ra;
	}
	void set_context_gdc_gfx(UPD7220 *device, uint8 *ra, uint8 *cs)
	{
		d_gdc_gfx = device;
		ra_gfx = ra; cs_gfx = cs;
	}
	void draw_screen();
	
	bool sound_bios_ok;
};

#endif

