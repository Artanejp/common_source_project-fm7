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
	
	uint8_t* rbank[8192];	// 16MB / 2KB
	uint8_t* wbank[8192];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	
	uint8_t ram[0x400000];	// RAM 1+3MB
#ifdef _FMR60
	uint8_t vram[0x80000];	// VRAM 512KB
	uint8_t cvram[0x2000];
	uint8_t avram[0x2000];
#else
	uint8_t vram[0x40000];	// VRAM 256KB
	uint8_t cvram[0x1000];
	uint8_t kvram[0x1000];
	uint8_t dummy[0x8000];	// dummy plane
#endif
	uint8_t ipl[0x4000];	// IPL 16KB
#ifdef _FMR60
	uint8_t ank24[0x3000];		// ANK(14x24)
	uint8_t kanji24[0x240000];	// KANJI(24x24)
#else
	uint8_t ank8[0x800];	// ANK(8x8) 2KB
	uint8_t ank16[0x1000];	// ANK(8x16) 4KB
	uint8_t kanji16[0x40000];	// KANJI(16x16) 256KB
#endif
	uint8_t machine_id;	// MACHINE ID
	
	// memory
	uint8_t protect, rst;
	uint8_t mainmem, rplane, wplane;
	uint8_t dma_addr_reg, dma_wrap_reg;
	uint32_t dma_addr_mask;
	
	// crtc
	uint8_t* chreg;
	bool disp, vsync;
	int blink;
	
	// video
	uint8_t apal[16][3], apalsel, dpal[8];
	uint8_t outctrl;
	
#ifndef _FMR60
	// 16bit card
	uint8_t pagesel, ankcg;
	uint8_t dispctrl;
	uint8_t mix;
	uint16_t accaddr, dispaddr;
	
	// kanji
	int kj_h, kj_l, kj_ofs, kj_row;
	
	// logical operation
	uint8_t cmdreg, imgcol, maskreg, compreg[8], compbit, bankdis, tilereg[3];
	uint16_t lofs, lsty, lsx, lsy, lex, ley;
	void point(int x, int y, int col);
	void line();
#endif
	
	uint8_t screen_txt[SCREEN_HEIGHT][SCREEN_WIDTH + 14];
	uint8_t screen_cg[SCREEN_HEIGHT][SCREEN_WIDTH];
//	uint8_t screen_txt[400][648];
//	uint8_t screen_cg[400][640];
	scrntype_t palette_txt[16];
	scrntype_t palette_cg[16];
	
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
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#if defined(HAS_I286)
	void set_context_cpu(I286* device)
#else
	void set_context_cpu(I386* device)
#endif
	{
		d_cpu = device;
	}
	void set_machine_id(uint8_t id)
	{
		machine_id = id;
	}
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_chregs_ptr(uint8_t* ptr)
	{
		chreg = ptr;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
	uint8_t* get_cvram()
	{
		return cvram;
	}
#ifdef _FMR60
	uint8_t* get_avram()
	{
		return avram;
	}
#else
	uint8_t* get_kvram()
	{
		return kvram;
	}
#endif
	void draw_screen();
};

#endif

