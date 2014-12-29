/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.29 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_DISP		0
#define SIG_MEMORY_VSYNC	1

#if defined(HAS_I286)
class I286;
#else
class I386;
#endif

class MEMORY : public DEVICE
{
private:
#if defined(HAS_I286)
	I286 *d_cpu;
#else
	I386 *d_cpu;
#endif
	DEVICE *d_crtc;
	
	uint8* rbank[8192];	// 16MB / 2KB
	uint8* wbank[8192];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	
	uint8 ram[0x400000];	// RAM 1+3MB
#ifdef _FMR60
	uint8 vram[0x80000];	// VRAM 512KB
	uint8 cvram[0x2000];
	uint8 avram[0x2000];
#else
	uint8 vram[0x40000];	// VRAM 256KB
	uint8 cvram[0x1000];
	uint8 kvram[0x1000];
	uint8 dummy[0x8000];	// dummy plane
#endif
	uint8 ipl[0x4000];	// IPL 16KB
#ifdef _FMR60
	uint8 ank24[0x3000];		// ANK(14x24)
	uint8 kanji24[0x240000];	// KANJI(24x24)
#else
	uint8 ank8[0x800];	// ANK(8x8) 2KB
	uint8 ank16[0x1000];	// ANK(8x16) 4KB
	uint8 kanji16[0x40000];	// KANJI(16x16) 256KB
#endif
	uint8 machine_id;	// MACHINE ID
	
	// memory
	uint8 protect, rst;
	uint8 mainmem, rplane, wplane;
	uint8 dma_addr_reg, dma_wrap_reg;
	uint32 dma_addr_mask;
	
	// crtc
	uint8* chreg;
	bool disp, vsync;
	int blink;
	
	// video
	uint8 apal[16][3], apalsel, dpal[8];
	uint8 outctrl;
	
#ifndef _FMR60
	// 16bit card
	uint8 pagesel, ankcg;
	uint8 dispctrl;
	uint8 mix;
	uint16 accaddr, dispaddr;
	
	// kanji
	int kj_h, kj_l, kj_ofs, kj_row;
	
	// logical operation
	uint8 cmdreg, imgcol, maskreg, compreg[8], compbit, bankdis, tilereg[3];
	uint16 lofs, lsty, lsx, lsy, lex, ley;
	void point(int x, int y, int col);
	void line();
#endif
	
	uint8 screen_txt[SCREEN_HEIGHT][SCREEN_WIDTH + 14];
	uint8 screen_cg[SCREEN_HEIGHT][SCREEN_WIDTH];
//	uint8 screen_txt[400][648];
//	uint8 screen_cg[400][640];
	scrntype palette_txt[16];
	scrntype palette_cg[16];
	
	void update_bank();
	void update_dma_addr_mask();
#ifdef _FMR60
	void draw_text();
#else
	void draw_text40();
	void draw_text80();
#endif
	void draw_cg();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_dma_data8(uint32 addr, uint32 data);
	uint32 read_dma_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	
	// unitque function
#if defined(HAS_I286)
	void set_context_cpu(I286* device)
#else
	void set_context_cpu(I386* device)
#endif
	{
		d_cpu = device;
	}
	void set_machine_id(uint8 id)
	{
		machine_id = id;
	}
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_chregs_ptr(uint8* ptr)
	{
		chreg = ptr;
	}
	uint8* get_vram()
	{
		return vram;
	}
	uint8* get_cvram()
	{
		return cvram;
	}
#ifdef _FMR60
	uint8* get_avram()
	{
		return avram;
	}
#else
	uint8* get_kvram()
	{
		return kvram;
	}
#endif
	void draw_screen();
};

#endif

