/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ display ]
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_DISPLAY_VBLANK		0
#define SIG_DISPLAY_COLUMN40		1
#define SIG_DISPLAY_DETECT_VBLANK	2

class HD46505;

class DISPLAY : public DEVICE
{
private:
#ifdef _X1TURBO_FEATURE
	DEVICE *d_cpu;
#endif
	HD46505 *d_crtc;
	uint8_t* regs;
	uint8_t vram_t[0x800];
	uint8_t vram_a[0x800];
#ifdef _X1TURBO_FEATURE
	uint8_t vram_k[0x800];
#endif
	uint8_t* vram_ptr;
	uint8_t pcg_b[256][8];
	uint8_t pcg_r[256][8];
	uint8_t pcg_g[256][8];
#ifdef _X1TURBO_FEATURE
	uint8_t gaiji_b[128][16];
	uint8_t gaiji_r[128][16];
	uint8_t gaiji_g[128][16];
#endif
	uint8_t font[0x800];
	uint8_t kanji[0x4bc00];
	
	uint8_t cur_code, cur_line;
	
	int kaddr, kofs, kflag;
	uint8_t* kanji_ptr;
	
	uint8_t pal[3];
	uint8_t priority, pri[8][8];	// pri[cg][txt]
	
	bool column40;
#ifdef _X1TURBO_FEATURE
	uint8_t mode1, mode2;
	bool hireso;
#endif
#ifdef _X1TURBOZ
	uint8_t zmode1, zpriority, zscroll, zmode2, ztpal[8];
	struct {
		uint8_t b, r, g;
	} zpal[4096];
	int zpal_num;
#endif
	
#ifdef _X1TURBO_FEATURE
	uint8_t text[400][640];
	uint8_t cg[400][640];
	uint8_t pri_line[400][8][8];
#else
	uint8_t text[200][640+8];
	uint8_t cg[200][640];
	uint8_t pri_line[200][8][8];
#endif
#ifdef _X1TURBOZ
	scrntype_t palette_pc[8+8+4096];	// 0-7:text, 8-15:cg, 16-:4096cg
#else
	scrntype_t palette_pc[8+8];	// 0-7:text, 8-15:cg
#endif
	bool prev_vert_double;
	int raster, cblink;
	
	int ch_height; // HD46505
	int hz_total, hz_disp, vt_disp;
	int st_addr;
	uint32_t vblank_clock;
	
	void update_crtc();
	void update_pal();
	uint8_t get_cur_font(uint32_t addr);
	void get_cur_pcg(uint32_t addr);
	void get_cur_code_line();
	
	void draw_line(int v);
	void draw_text(int y);
	void draw_cg(int line);
	
	// kanji rom (from X1EMU by KM)
	void write_kanji(uint32_t addr, uint32_t data);
	uint32_t read_kanji(uint32_t addr);
	
	uint16_t jis2adr_x1(uint16_t jis);
	uint32_t adr2knj_x1(uint16_t adr);
#ifdef _X1TURBO_FEATURE
	uint32_t adr2knj_x1t(uint16_t adr);
#endif
	uint32_t jis2knj(uint16_t jis);
	uint16_t jis2sjis(uint16_t jis);
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_vline(int v, int clock);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
#ifdef _X1TURBO_FEATURE
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
#endif
	void set_context_crtc(HD46505* device)
	{
		d_crtc = device;
	}
	void set_vram_ptr(uint8_t* ptr)
	{
		vram_ptr = ptr;
	}
	void set_regs_ptr(uint8_t* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

