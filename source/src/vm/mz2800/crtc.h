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
	uint8_t *vram_b, *vram_r, *vram_g, *vram_i;
	uint8_t *tvram;
	// kanji rom, pcg
	uint8_t *kanji1, *kanji2;
	uint8_t *pcg0, *pcg1, *pcg2, *pcg3;
	
	// crtc
	void set_hsync(int h);
	uint8_t textreg_num, textreg[16];
	uint8_t rmwreg_num[2], rmwreg[2][32];
	uint8_t cgreg_num, cgreg[32];
	uint8_t scrn_size, cg_mask;
	bool font_size, column_size;
	uint8_t latch[2][4];
	uint16_t GDEVS, GDEVE;
	uint8_t GDEHS, GDEHE;
	int GDEHSC, GDEHEC;
	bool blank, hblank, vblank, blink;
	uint8_t clear_flag;
	uint8_t palette_reg[16];
	bool pal_select;
	bool screen_mask;
	
	// priority and palette
	uint8_t priority16[16][9];
	scrntype_t palette16[16+8], palette4096[16];
	uint8_t palette4096r[16], palette4096g[16], palette4096b[16];
	scrntype_t palette16txt[9], palette4096txt[9];
	scrntype_t palette16pri[16][9], palette4096pri[16][9];
	scrntype_t palette65536[0x10000];	// BRGI
	uint8_t prev16;
	bool update16;
	
	// draw text
	void draw_text();
	void draw_80column_screen();
	void draw_40column_screen();
	void draw_80column_font(uint16_t src, int dest, int y);
	void draw_40column_font(uint16_t src, int dest, int y);
	uint8_t text[640*480*2];
	
	// draw cg
	void draw_cg();
	uint16_t cg[640*400*2];
	uint32_t map_addr[400][80];
	uint8_t map_hdsc[400][80];
	
	// speed optimize
	uint8_t cg_matrix0[256][256][8];
	uint8_t cg_matrix1[256][256][8];
	uint8_t cg_matrix2[256][256][8];
	uint8_t cg_matrix3[256][256][8];
	uint8_t text_matrix[256][8][8];
	uint8_t text_matrixw[256][8][16];
	uint8_t trans_color;
	bool map_init, trans_init;
	
public:
	CRTC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CRTC"));
	}
	~CRTC() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void event_vline(int v, int clock);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram_b = ptr + 0x00000;
		vram_r = ptr + 0x20000;
		vram_g = ptr + 0x40000;
		vram_i = ptr + 0x60000;
	}
	void set_tvram_ptr(uint8_t* ptr)
	{
		tvram = ptr;
	}
	void set_kanji_ptr(uint8_t* ptr)
	{
		kanji1 = ptr + 0x00000;
		kanji2 = ptr + 0x40000;
	}
	void set_pcg_ptr(uint8_t* ptr)
	{
		pcg0 = ptr + 0x0000;
		pcg1 = ptr + 0x1000;
		pcg2 = ptr + 0x2000;
		pcg3 = ptr + 0x3000;
	}
	void draw_screen();
};

#endif

