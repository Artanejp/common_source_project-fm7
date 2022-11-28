/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
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
	uint8_t *ra_chr;
	uint8_t *ra_gfx, *cs_gfx;
	
	uint8_t tvram[0x4000];
#if !defined(SUPPORT_HIRESO)
#if !defined(SUPPORT_2ND_VRAM)
	__DECL_ALIGNED(4) uint8_t vram[0x20000];
#else
	__DECL_ALIGNED(4) uint8_t vram[0x40000];
#endif
#else
	__DECL_ALIGNED(4) uint8_t vram[0x80000];
#endif
	
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	uint8_t vram_disp_sel;
	uint8_t vram_draw_sel;
#endif
	uint8_t *vram_disp_b;
	uint8_t *vram_disp_r;
	uint8_t *vram_disp_g;
#if defined(SUPPORT_16_COLORS)
	uint8_t *vram_disp_e;
#endif
	uint8_t *vram_draw;
	
	scrntype_t palette_chr[8];
	scrntype_t palette_gfx8[8];
	uint8_t digipal[4];
#if defined(SUPPORT_16_COLORS)
	scrntype_t palette_gfx16[16];
	uint8_t anapal[16][3], anapal_sel;
#endif
	
	uint8_t crtv;
	uint8_t scroll[6];
	uint8_t modereg1[8];
#if defined(SUPPORT_16_COLORS)
	uint8_t modereg2[128];
#endif
#if defined(SUPPORT_GRCG)
	uint8_t grcg_mode, grcg_tile_ptr, grcg_tile[4];
	__DECL_ALIGNED(16) uint16_t grcg_tile_word[4];
#endif
#if defined(SUPPORT_EGC)
	typedef union {
		uint8_t b[2];
		uint16_t w;
	} egcword_t;
	typedef union {
		uint8_t b[4][2];
		uint16_t w[4];
		uint32_t d[2];
		uint64_t q;
	} egcquad_t;
	
	uint16_t egc_access;
	uint16_t egc_fgbg;
	uint16_t egc_ope;
	uint16_t egc_fg;
	egcword_t egc_mask;
	uint16_t egc_bg;
	uint16_t egc_sft;
	uint16_t egc_leng;
	egcquad_t egc_lastvram;
	egcquad_t egc_patreg;
	egcquad_t egc_fgc;
	egcquad_t egc_bgc;
	int egc_func;
	uint32_t egc_remain;
	uint32_t egc_stack;
	uint8_t* egc_inptr;
	uint8_t* egc_outptr;
	egcword_t egc_mask2;
	egcword_t egc_srcmask;
	uint8_t egc_srcbit;
	uint8_t egc_dstbit;
	uint8_t egc_sft8bitl;
	uint8_t egc_sft8bitr;
	uint8_t egc_buf[528];	/* 4096/8 + 4*4 */
	egcquad_t egc_vram_src;
	egcquad_t egc_vram_data;
#endif
	
#if !defined(SUPPORT_HIRESO)
	#define FONT_SIZE	16
	#define FONT_WIDTH	8
	#define FONT_HEIGHT	16
#else
	#define FONT_SIZE	48
	#define FONT_WIDTH	14
	#define FONT_HEIGHT	24
#endif
	#define KANJI_2ND_OFS	(FONT_SIZE * 0x80)
	#define KANJI_FONT_SIZE	(FONT_SIZE * 2)
	#define ANK_FONT_OFS	(KANJI_FONT_SIZE * 0x4000)
	
	uint8_t font[ANK_FONT_OFS + FONT_SIZE * 0x400];
	uint16_t font_code;
	uint8_t font_line;
//	uint16_t font_lr;
	
	uint8_t screen_chr[SCREEN_HEIGHT][SCREEN_WIDTH + 1];
	uint8_t screen_gfx[SCREEN_HEIGHT][SCREEN_WIDTH];
	
#if !defined(SUPPORT_HIRESO)
	void kanji_copy(uint8_t *dst, uint8_t *src, int from, int to);
#else
	void ank_copy(int code, uint8_t *pattern);
	void kanji_copy(int first, int second, uint8_t *pattern);
#endif
#ifdef __BIG_ENDIAN__
	inline void vram_draw_writew(uint32_t addr, uint32_t data);
	inline uint32_t vram_draw_readw(uint32_t addr);
#endif
#if defined(SUPPORT_GRCG)
	void grcg_writeb(uint32_t addr1, uint32_t data);
	void grcg_writew(uint32_t addr1, uint32_t data);
	uint32_t grcg_readb(uint32_t addr1);
	uint32_t grcg_readw(uint32_t addr1);
#endif
#if defined(SUPPORT_EGC)
	void egc_shift();
	void egc_sftb_upn_sub(uint32_t ext);
	void egc_sftb_dnn_sub(uint32_t ext);
	void egc_sftb_upr_sub(uint32_t ext);
	void egc_sftb_dnr_sub(uint32_t ext);
	void egc_sftb_upl_sub(uint32_t ext);
	void egc_sftb_dnl_sub(uint32_t ext);
	void egc_sftb_upn0(uint32_t ext);
	void egc_sftw_upn0();
	void egc_sftb_dnn0(uint32_t ext);
	void egc_sftw_dnn0();
	void egc_sftb_upr0(uint32_t ext);
	void egc_sftw_upr0();
	void egc_sftb_dnr0(uint32_t ext);
	void egc_sftw_dnr0();
	void egc_sftb_upl0(uint32_t ext);
	void egc_sftw_upl0();
	void egc_sftb_dnl0(uint32_t ext);
	void egc_sftw_dnl0();
	void egc_sftb(int func, uint32_t ext);
	void egc_sftw(int func);
	void egc_shiftinput_byte(uint32_t ext);
	void egc_shiftinput_incw();
	void egc_shiftinput_decw();
	uint64_t egc_ope_00(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_0f(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_c0(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_f0(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_fc(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_ff(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_nd(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_np(uint8_t ope, uint32_t addr);
	uint64_t egc_ope_xx(uint8_t ope, uint32_t addr);
	uint64_t egc_opefn(uint32_t func, uint8_t ope, uint32_t addr);
	uint64_t egc_opeb(uint32_t addr, uint8_t value);
	uint64_t egc_opew(uint32_t addr, uint16_t value);
	uint32_t egc_readb(uint32_t addr1);
	uint32_t egc_readw(uint32_t addr1);
	void egc_writeb(uint32_t addr1, uint8_t value);
	void egc_writew(uint32_t addr1, uint16_t value);
#endif
	void draw_chr_screen();
	void draw_gfx_screen();
	
public:
	DISPLAY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(tvram, 0, sizeof(tvram));
		set_device_name(_T("Display"));
	}
	~DISPLAY() {}
	
	// common functions
	void initialize();
	void reset();
	void event_frame();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_memory_mapped_io8(uint32_t addr, uint32_t data);
	void write_memory_mapped_io16(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io8(uint32_t addr);
	uint32_t read_memory_mapped_io16(uint32_t addr);
	void write_dma_io8(uint32_t addr, uint32_t data);
	void write_dma_io16(uint32_t addr, uint32_t data);
	uint32_t read_dma_io8(uint32_t addr);
	uint32_t read_dma_io16(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_pic(DEVICE *device)
	{
		d_pic = device;
	}
	void set_context_gdc_chr(UPD7220 *device, uint8_t *ra)
	{
		d_gdc_chr = device;
		ra_chr = ra;
	}
	void set_context_gdc_gfx(UPD7220 *device, uint8_t *ra, uint8_t *cs)
	{
		d_gdc_gfx = device;
		ra_gfx = ra; cs_gfx = cs;
	}
	void sound_bios_ok()
	{
		tvram[0x3fee] = 8;
	}
	void draw_screen();
};

#endif

