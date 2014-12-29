/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ crtc ]
*/

#ifndef _CRTC_H_
#define _CRTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CRTC_COLUMN_SIZE	0
#define SIG_CRTC_PALLETE	1
#define SIG_CRTC_MASK		2

class CRTC : public DEVICE
{
private:
	DEVICE *d_pic, *d_pio;
	
	// vram
	uint8 *vram_b, *vram_r, *vram_g, *vram_i;
	uint8 *tvram;
	// kanji rom, pcg
	uint8 *kanji1, *kanji2;
	uint8 *pcg0, *pcg1, *pcg2, *pcg3;
	
	// crtc
	void set_hsync(int h);
	uint8 textreg_num, textreg[16];
	uint8 rmwreg_num[2], rmwreg[2][32];
	uint8 cgreg_num, cgreg[32];
	uint8 scrn_size, cg_mask;
	bool font_size, column_size;
	uint8 latch[2][4];
	uint16 GDEVS, GDEVE;
	uint8 GDEHS, GDEHE;
	int GDEHSC, GDEHEC;
	bool blank, hblank, vblank, blink;
	uint8 clear_flag;
	uint8 palette_reg[16];
	bool pal_select;
	bool screen_mask;
	
	// priority and palette
	uint8 priority16[16][9];
	scrntype palette16[16+8], palette4096[16];
	uint8 palette4096r[16], palette4096g[16], palette4096b[16];
	scrntype palette16txt[9], palette4096txt[9];
	scrntype palette16pri[16][9], palette4096pri[16][9];
	scrntype palette65536[0x10000];	// BRGI
	uint8 prev16;
	bool update16;
	
	// draw text
	void draw_text();
	void draw_80column_screen();
	void draw_40column_screen();
	void draw_80column_font(uint16 src, int dest, int y);
	void draw_40column_font(uint16 src, int dest, int y);
	uint8 text[640*480*2];
	
	// draw cg
	void draw_cg();
	uint16 cg[640*400*2];
	uint32 map_addr[400][80];
	uint8 map_hdsc[400][80];
	
	// speed optimize
	uint8 cg_matrix0[256][256][8];
	uint8 cg_matrix1[256][256][8];
	uint8 cg_matrix2[256][256][8];
	uint8 cg_matrix3[256][256][8];
	uint8 text_matrix[256][8][8];
	uint8 text_matrixw[256][8][16];
	uint8 trans_color;
	bool map_init, trans_init;
	
public:
	CRTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CRTC() {}
	
	// common functions
	void initialize();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void event_vline(int v, int clock);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_vram_ptr(uint8* ptr)
	{
		vram_b = ptr + 0x00000;
		vram_r = ptr + 0x20000;
		vram_g = ptr + 0x40000;
		vram_i = ptr + 0x60000;
	}
	void set_tvram_ptr(uint8* ptr)
	{
		tvram = ptr;
	}
	void set_kanji_ptr(uint8* ptr)
	{
		kanji1 = ptr + 0x00000;
		kanji2 = ptr + 0x40000;
	}
	void set_pcg_ptr(uint8* ptr)
	{
		pcg0 = ptr + 0x0000;
		pcg1 = ptr + 0x1000;
		pcg2 = ptr + 0x2000;
		pcg3 = ptr + 0x3000;
	}
	void draw_screen();
};

#endif

