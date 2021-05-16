/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

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
#define SIG_DISPLAY_DISP		3

class HD46505;

namespace X1 {

class DISPLAY : public DEVICE
{
private:
#ifdef _X1TURBO_FEATURE
	DEVICE *d_cpu;
#endif
	HD46505 *d_crtc;
	uint8_t* regs;
	__DECL_ALIGNED(16) uint8_t vram_t[0x800];
	__DECL_ALIGNED(16) uint8_t vram_a[0x800];
#ifdef _X1TURBO_FEATURE
	__DECL_ALIGNED(16) uint8_t vram_k[0x800];
#endif
	uint8_t* vram_ptr;
	__DECL_ALIGNED(8) uint8_t pcg_b[256][8];
	__DECL_ALIGNED(8) uint8_t pcg_r[256][8];
	__DECL_ALIGNED(8) uint8_t pcg_g[256][8];
#ifdef _X1TURBO_FEATURE
	__DECL_ALIGNED(16) uint8_t gaiji_b[128][16];
	__DECL_ALIGNED(16) uint8_t gaiji_r[128][16];
	__DECL_ALIGNED(16) uint8_t gaiji_g[128][16];
#endif
	__DECL_ALIGNED(16) uint8_t font[0x800];
	__DECL_ALIGNED(16) uint8_t kanji[0x4bc00];
	
	uint8_t cur_code, cur_line;
	
	int kaddr, kofs, kflag;
	uint8_t* kanji_ptr;
	
	uint8_t pal[3];
	uint8_t priority;
	__DECL_ALIGNED(32) uint8_t pri[8][8];	// pri[cg][txt]
	uint8_t dr_priority;
	
	bool column40;
#ifdef _X1TURBO_FEATURE
	uint8_t mode1, mode2;
	bool hireso;
#endif
#ifdef _X1TURBOZ
	uint8_t zmode1;
	uint8_t zpriority;
	uint8_t zadjust;
	uint8_t zmosaic;
	uint8_t zchromakey;
	uint8_t zscroll;
	uint8_t zmode2;
	uint8_t ztpal[8];
	uint8_t dr_zpriority;

	__DECL_ALIGNED(32) struct {
		uint8_t b, r, g;
	} zpal[4096];
	int zpal_num;
#endif
	
#ifdef _X1TURBO_FEATURE
	__DECL_ALIGNED(32) uint8_t text[400][640];
	__DECL_ALIGNED(32) uint8_t cg[400][640];
	__DECL_ALIGNED(32) uint8_t pri_line[400][8][8];

	__DECL_ALIGNED(32) uint8_t dr_text[400][640];
	__DECL_ALIGNED(32) uint8_t dr_cg[400][640];
	__DECL_ALIGNED(32) uint8_t dr_pri_line[400][8][8];
#else
	__DECL_ALIGNED(32) uint8_t text[200][640+8];
	__DECL_ALIGNED(32) uint8_t cg[200][640];
	__DECL_ALIGNED(32) uint8_t pri_line[200][8][8];

	__DECL_ALIGNED(32) uint8_t dr_text[200][640+8];
	__DECL_ALIGNED(32) uint8_t dr_cg[200][640];
	__DECL_ALIGNED(32) uint8_t dr_pri_line[200][8][8];
#endif
#ifdef _X1TURBOZ
	bool zpalette_changed;
	__DECL_ALIGNED(32) uint16_t zcg[2][400][640];
	__DECL_ALIGNED(16) bool aen_line[400];
	__DECL_ALIGNED(32) scrntype_t zpalette_tmp[8+8+4096];
	__DECL_ALIGNED(32) scrntype_t zpalette_pc[8+8+4096];	// 0-7:text, 8-15:cg, 16-:4096cg

	__DECL_ALIGNED(32) uint16_t dr_zcg[2][400][640];
	__DECL_ALIGNED(16) bool dr_aen_line[400];
	__DECL_ALIGNED(32) scrntype_t dr_zpalette_pc[8+8+4096];	// 0-7:text, 8-15:cg, 16-:4096cg
#endif
	__DECL_ALIGNED(16) scrntype_t palette_pc[8+8];		// 0-7:text, 8-15:cg
	__DECL_ALIGNED(16) scrntype_t dr_palette_pc[8+8];		// 0-7:text, 8-15:cg
	bool prev_vert_double;
	int raster, cblink;
	
	int ch_height; // HD46505
	int hz_total, hz_disp, vt_disp, vt_ofs;
	int st_addr;
	uint32_t vblank_clock;
	int cur_vline;
	bool cur_blank;
	
	void update_crtc();
	void update_pal();
	uint8_t __FASTCALL get_cur_font(uint32_t addr);
	void __FASTCALL get_cur_pcg(uint32_t addr);
	void __FASTCALL get_cur_code_line();
	
	void __FASTCALL draw_line(int v);
	void __FASTCALL draw_text(int y);
	void __FASTCALL draw_cg(int line, int plane);
	
#ifdef _X1TURBOZ
	int __FASTCALL get_zpal_num(uint32_t addr, uint32_t data);
	void update_zpalette();
	scrntype_t __FASTCALL get_zpriority(uint8_t text, uint16_t cg0, uint16_t cg1);
#endif
	
	// kanji rom (from X1EMU by KM)
	void __FASTCALL write_kanji(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_kanji(uint32_t addr);
	
	uint16_t __FASTCALL jis2adr_x1(uint16_t jis);
	uint32_t __FASTCALL adr2knj_x1(uint16_t adr);
#ifdef _X1TURBO_FEATURE
	uint32_t __FASTCALL adr2knj_x1t(uint16_t adr);
#endif
	uint32_t __FASTCALL jis2knj(uint16_t jis);
	uint16_t __FASTCALL jis2sjis(uint16_t jis);

	int tmp_kanji_ptr;

	__DECL_ALIGNED(16) _bit_trans_table_t bit_trans_table_b0;
	__DECL_ALIGNED(16) _bit_trans_table_t bit_trans_table_r0;
	__DECL_ALIGNED(16) _bit_trans_table_t bit_trans_table_g0;
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_vline(int v, int clock);
#ifdef _X1TURBO_FEATURE
	void __FASTCALL event_callback(int event_id, int err);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
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

}
#endif

