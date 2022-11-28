// from "v99x8.h" of Zodiac

#ifndef _V99X8_H_
#define _V99X8_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifndef VAR
#	define VAR extern
#endif

//#include "../misc/ut.h"
//#include <md.h>


#define V99X8_NREG 48
#define V99X8_NSTAT 10

enum
{
	V99X8_SCREEN_IGN = -1,
	V99X8_SCREEN_0, V99X8_SCREEN_1, V99X8_SCREEN_2, V99X8_SCREEN_3,
	V99X8_SCREEN_4, V99X8_SCREEN_5, V99X8_SCREEN_6, V99X8_SCREEN_7,
	V99X8_SCREEN_8, V99X8_SCREEN_X, V99X8_SCREEN_A, V99X8_SCREEN_C
};

typedef struct
{
	bool f_tms;
	bool f_interleave; /* sc7/8 における特別なマッピングモード */

	int xsize;
	int xshift;
} v99x8_screen_mode_t;

typedef struct
{
	uint8_t ctrl[V99X8_NREG], status[V99X8_NSTAT];

	int scr;
	v99x8_screen_mode_t mode;

	uint8_t col_fg, col_bg;

	uint8_t *tbl_pg, *tbl_pn, *tbl_cl;

	int pages;	/* VRAM memory size */
	bool f_zoom;

	uint8_t *vram;

	int scanline, n_scanlines; /* 処理中の scanline と scanline 数。
	                               ??? もっとよいネーミング？ */


/* private */


} v99x8_t;

VAR v99x8_t v99x8;


extern void v99x8_init(void);
extern void v99x8_ctrl(int n, uint8_t m);
extern int v99x8_hsync(void);

extern uint8_t v99x8_in_0(void);	/* VRAM read */
extern uint8_t v99x8_in_1(void);	/* status in */

extern void v99x8_out_0(uint8_t n);	/* VRAM write */
extern void v99x8_out_1(uint8_t n);	/* ctrl out */
extern void v99x8_out_2(uint8_t n);	/* palette out */
extern void v99x8_out_3(uint8_t n);	/* ctrl out */


extern void v99x8_pallete_set(uint8_t n, uint8_t r, uint8_t g, uint8_t b);
extern void v99x8_refresh_init(void);
extern void v99x8_refresh_screen(void);
extern void v99x8_refresh_clear(void);
extern void v99x8_refresh_sc0(int y, int h);
extern void v99x8_refresh_sc1(int y, int h);
extern void v99x8_refresh_sc2(int y, int h);
extern void v99x8_refresh_sc3(int y, int h);
extern void v99x8_refresh_sc4(int y, int h);
extern void v99x8_refresh_sc5(int y, int h);
extern void v99x8_refresh_sc6(int y, int h);
extern void v99x8_refresh_sc7(int y, int h);
extern void v99x8_refresh_sc8(int y, int h);
extern void v99x8_refresh_sca(int y, int h);
extern void v99x8_refresh_scc(int y, int h);
extern void v99x8_refresh_scx(int y, int h);

extern uint8_t vram_read(int addr);
extern void vram_write(int addr, uint8_t n);

/*
#define VRAM_ADDR(addr) (v99x8.f_interleave ? \
                          (addr >> 1) | ((addr & 1) << 16) : \
                          addr)
*/





/*
	Skelton for retropc emulator

	Origin : "tms9918a.h"
	Author : umaiboux
	Date   : 2014.12.XX -

	[ V99x8 ]
*/


#include "vm.h"
#include "../emu.h"
#include "device.h"


class V99X8 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_irq;
	
	uint8_t vram[1024*128];
	scrntype_t screen[SCREEN_WIDTH*SCREEN_HEIGHT];
	bool intstat;
	
	void set_intstat(bool val);

	int hsync(int v/*void*/);
	void z80_intreq(int a);
	int md_video_pitch(void);
	uint8_t *md_video_lockline(int x, int y, int w, int h);
	void md_video_update(int n, /*md_video_rect_t*/void *rp);
	void md_video_fill(int x, int y, int w, int h, uint32_t c);
	void v99x8_refresh_screen(void);
	void v99x8_refresh_clear(void);
	uint8_t *v99x8_refresh_start(int x, int w, int h);
	void v99x8_refresh_sc0(int y, int h);
	void v99x8_refresh_sc1(int y, int h);
	void v99x8_refresh_sc2(int y, int h);
	void v99x8_refresh_sc3(int y, int h);
	void v99x8_refresh_sc4(int y, int h);
	void v99x8_refresh_sc5(int y, int h);
	void v99x8_refresh_sc6(int y, int h);
	void v99x8_refresh_sc7(int y, int h);
	void v99x8_refresh_sc8(int y, int h);
	void v99x8_refresh_sca(int y, int h);
	void v99x8_refresh_scc(int y, int h);
	void v99x8_refresh_scx(int y, int h);
public:
	V99X8(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		set_device_name(_T("V99x8 VDP"));
	}
	~V99X8() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_vline(int v, int clock);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void draw_screen();
};

#endif
