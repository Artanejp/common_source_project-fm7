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

#ifdef _X1TURBO_FEATURE
class HD46505;
#endif

class DISPLAY : public DEVICE
{
private:
#ifdef _X1TURBO_FEATURE
	DEVICE *d_cpu;
	HD46505 *d_crtc;
#endif
	
	uint8* regs;
	uint8 vram_t[0x800];
	uint8 vram_a[0x800];
#ifdef _X1TURBO_FEATURE
	uint8 vram_k[0x800];
#endif
	uint8* vram_ptr;
	uint8 pcg_b[256][8];
	uint8 pcg_r[256][8];
	uint8 pcg_g[256][8];
#ifdef _X1TURBO_FEATURE
	uint8 gaiji_b[128][16];
	uint8 gaiji_r[128][16];
	uint8 gaiji_g[128][16];
#endif
	uint8 font[0x800];
	uint8 kanji[0x4bc00];
	
	uint8 cur_code, cur_line;
	
	int kaddr, kofs, kflag;
	uint8* kanji_ptr;
	
	uint8 pal[3];
	uint8 priority, pri[8][8];	// pri[cg][txt]
	
	bool column40;
#ifdef _X1TURBO_FEATURE
	uint8 mode1, mode2;
	bool hireso;
#endif
#ifdef _X1TURBOZ
	uint8 zmode1, zpriority, zscroll, zmode2, ztpal[8];
	struct {
		uint8 b, r, g;
	} zpal[4096];
	int zpal_num;
#endif
	
#ifdef _X1TURBO_FEATURE
	uint8 text[400][640];
	uint8 cg[400][640];
	uint8 pri_line[400][8][8];
#else
	uint8 text[200][640+8];
	uint8 cg[200][640];
	uint8 pri_line[200][8][8];
#endif
#ifdef _X1TURBOZ
	scrntype palette_pc[8+8+4096];	// 0-7:text, 8-15:cg, 16-:4096cg
#else
	scrntype palette_pc[8+8];	// 0-7:text, 8-15:cg
#endif
	bool prev_vert_double;
	int raster, cblink;
	
	int ch_height; // HD46505
	int hz_total, hz_disp, vt_disp;
	int st_addr;
	uint32 vblank_clock;
	
	void update_pal();
	uint8 get_cur_font(uint32 addr);
	void get_cur_pcg(uint32 addr);
	void get_cur_code_line();
	
	void draw_line(int v);
	void draw_text(int y);
	void draw_cg(int line);
	
	// kanji rom (from X1EMU by KM)
	void write_kanji(uint32 addr, uint32 data);
	uint32 read_kanji(uint32 addr);
	
	uint16 jis2adr_x1(uint16 jis);
	uint32 adr2knj_x1(uint16 adr);
#ifdef _X1TURBO_FEATURE
	uint32 adr2knj_x1t(uint16 adr);
#endif
	uint32 jis2knj(uint16 jis);
	uint16 jis2sjis(uint16 jis);
	
public:
	DISPLAY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
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
	void set_context_crtc(HD46505* device)
	{
		d_crtc = device;
	}
#endif
	void set_vram_ptr(uint8* ptr)
	{
		vram_ptr = ptr;
	}
	void set_regs_ptr(uint8* ptr)
	{
		regs = ptr;
	}
	void draw_screen();
};

#endif

