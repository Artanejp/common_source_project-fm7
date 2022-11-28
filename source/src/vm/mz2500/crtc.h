/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.03 -

	[ crtc ]
*/

#ifndef _CRTC_H_
#define _CRTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CRTC_COLUMN_SIZE	0
#define SIG_CRTC_PALLETE	1
#define SIG_CRTC_REVERSE	2
#define SIG_CRTC_MASK		3

class CRTC : public DEVICE
{
private:
	DEVICE *d_mem, *d_int, *d_pio;
	
	// config
	bool scan_line, scan_tmp;
	bool monitor_200line;
	bool monitor_digital, monitor_tmp;
	int boot_mode;
	
	double frames_per_sec;
	int lines_per_frame, chars_per_line;
	
	// vram
	uint8_t *vram_b, *vram_r, *vram_g, *vram_i;
	uint8_t *tvram1, *attrib, *tvram2;
	// kanji rom, pcg
	uint8_t *kanji1, *kanji2;
	uint8_t *pcg0, *pcg1, *pcg2, *pcg3;
	
	// crtc
	void set_hsync(int h);
	uint8_t textreg_num, textreg[16];
	uint8_t cgreg_num, cgreg[32];
	uint8_t scrn_size, cg_mask, cg_mask256;
	bool cg_mask256_init;
	bool font_size, column_size;
	uint8_t latch[4];
	uint16_t GDEVS, GDEVE;
	uint8_t GDEHS, GDEHE;
	int GDEHSC, GDEHEC;
	bool hblank, vblank, blink;
	uint8_t clear_flag;
	uint8_t palette_reg[16];
	bool pal_select;
	bool screen_reverse;
	bool screen_mask;
	
	// priority and palette
	uint8_t priority16[16][9];
	scrntype_t palette16[16+8], palette4096[16];
	uint8_t palette4096r[16], palette4096g[16], palette4096b[16];
	scrntype_t palette16txt[9], palette4096txt[9];
	scrntype_t palette16pri[16][9], palette4096pri[16][9];
	uint8_t prev16;
	bool update16;
	
	uint16_t priority256[256][16+64];
	scrntype_t palette256[256+16+64];
	scrntype_t palette256txt[16+64];
	scrntype_t palette256pri[256][16+64];
	scrntype_t prev256;
	bool update256;
	
	// draw text
	void draw_text();
	void draw_80column_screen();
	void draw_40column_screen();
	void draw_80column_font(uint16_t src, int dest, int y);
	void draw_40column_font(uint16_t src, int dest, int y);
	uint8_t text[640*480*2];
	
	// draw cg
	void draw_cg();
	void draw_320x200x16screen(uint8_t pl);
	void draw_320x200x256screen(int ymax);
	void draw_640x200x16screen(uint8_t pl);
	void draw_640x400x4screen();
	void draw_640x400x16screen();
	void create_addr_map(int xmax, int ymax);
	uint8_t cg[640*400*2];
	uint16_t map_addr[400][80];
	uint8_t map_hdsc[400][80];
	
	// speed optimization
	uint8_t cg_matrix0[256][256][8];
	uint8_t cg_matrix1[256][256][8];
	uint8_t cg_matrix2[256][256][8];
	uint8_t cg_matrix3[256][256][8];
	uint8_t text_matrix[256][8][8];
	uint8_t text_matrixw[256][8][16];
	uint8_t trans_color;
	
	bool map_init, trans_init;
	
	// MZ-2000/80B
	uint8_t vram_page, vram_mask;
	uint8_t back_color, text_color;
	
	uint8_t font[0x800];
	uint8_t screen_txt[200][640];
	uint8_t screen_gra[200][640];
	
	void draw_screen_80b();
	void draw_screen_2000();
	
public:
	CRTC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CRTC"));
	}
	~CRTC() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void event_vline(int v, int clock);
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_int(DEVICE* device)
	{
		d_int = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram_b = ptr + 0x00000;
		vram_r = ptr + 0x08000;
		vram_g = ptr + 0x10000;
		vram_i = ptr + 0x18000;
	}
	void set_tvram_ptr(uint8_t* ptr)
	{
		tvram1 = ptr + 0x0000;
		attrib = ptr + 0x0800;
		tvram2 = ptr + 0x1000;
	}
	void set_kanji_ptr(uint8_t* ptr)
	{
		kanji1 = ptr + 0x00000;
		kanji2 = ptr + 0x20000;
	}
	void set_pcg_ptr(uint8_t* ptr)
	{
		pcg0 = ptr + 0x0000;
		pcg1 = ptr + 0x0800;
		pcg2 = ptr + 0x1000;
		pcg3 = ptr + 0x1800;
	}
	void draw_screen();
};

#endif

