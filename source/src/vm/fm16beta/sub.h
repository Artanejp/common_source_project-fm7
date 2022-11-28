/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.28-

	[ sub system ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../memory.h"

#define SIG_SUB_DISP	0
#define SIG_SUB_VSYNC	1
#define SIG_SUB_CANCEL	2
#define SIG_SUB_KEY	3
#define SIG_SUB_HALT	4
#define SIG_SUB_MAINACK	5

class SUB : public MEMORY
{
private:
	DEVICE *d_crtc, *d_pcm, *d_main, *d_subcpu, *d_keyboard;
	
	uint8_t gvram[0x30000];
	uint8_t dummy[0x8000];	// dummy plane
	uint8_t cvram[0x1000];
	uint8_t kvram[0x1000];
	
	uint8_t wram[0x1f80];
	uint8_t sram[0x0080];
	uint8_t rom[0x5000];
	
	uint8_t ank8[0x800];
	uint8_t ank16[0x1000];
	uint8_t kanji16[0x40000];
	
	uint8_t mix;
	uint8_t update, dispctrl, pagesel;
	uint8_t ankcg;
	uint16_t accaddr, dispaddr;
	
	// main-sub
	uint8_t attention[3];
	uint8_t mainack;
	
	// interrupts
	bool irq_cancel, irq_vsync, firq_key, firq_pen;
	void update_irq();
	void update_firq();
	
	// crtc
	uint8_t* chreg;
	bool disp, vsync;
	int blink;
	
	// video
	uint8_t dpal[8];
	uint8_t outctrl;
	
	void update_cvram_bank();
	void update_kvram_bank();
	
	// kanji
	int kj_h, kj_l, kj_ofs, kj_row;
	
	// logical operation
	uint8_t cmdreg, imgcol, maskreg, compreg[8], compbit, bankdis, tilereg[3];
	uint16_t lofs, lsty, lsx, lsy, lex, ley;
	void point(int x, int y, int col);
	void line();
	
	uint8_t screen_txt[SCREEN_HEIGHT][SCREEN_WIDTH + 14];
	uint8_t screen_cg[SCREEN_HEIGHT][SCREEN_WIDTH];
//	uint8_t screen_txt[400][648];
//	uint8_t screen_cg[400][640];
	scrntype_t palette_txt[8];
	scrntype_t palette_cg[8];
	
	void draw_text40();
	void draw_text80();
	void draw_cg();
	
	void write_memory(uint32_t addr, uint32_t data);
	uint32_t read_memory(uint32_t addr);
	
public:
	SUB(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Sub System"));
	}
	~SUB() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_crtc(DEVICE* device)
	{
		d_crtc = device;
	}
	void set_chregs_ptr(uint8_t* ptr)
	{
		chreg = ptr;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_main(DEVICE* device)
	{
		d_main = device;
	}
	void set_context_subcpu(DEVICE* device)
	{
		d_subcpu = device;
	}
	void set_context_keyboard(DEVICE* device)
	{
		d_keyboard = device;
	}
	void key_down(int code);
	void key_up(int code);
	void draw_screen();
};

#endif
