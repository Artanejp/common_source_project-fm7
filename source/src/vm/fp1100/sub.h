/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.09-

	[ sub pcb ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SUB_INT2	0
#define SIG_SUB_COMM	1
#define SIG_SUB_HSYNC	2
#define SIG_SUB_SO	3
#define SIG_SUB_EAR	4

class HD46505;

namespace FP1100 {
	
class SUB : public DEVICE
{
private:
	// to sub cpu
	DEVICE *d_cpu;
	// to main pcb
	DEVICE *d_main;
	// to beep
	DEVICE *d_beep;
	// to data recorder
	DEVICE *d_drec;
	// from/to crtc
	HD46505 *d_crtc;
	uint8_t *regs;
	
	uint8_t *wbank[0x200];
	uint8_t *rbank[0x200];
	uint8_t wdmy[0x80];
	uint8_t rdmy[0x80];
	
	uint8_t ram[0x80];
	uint8_t vram_b[0x4000];
	uint8_t vram_r[0x4000];
	uint8_t vram_g[0x4000];
	uint8_t sub1[0x1000];
	uint8_t sub2[0x1000];
	uint8_t sub3[0x1000];
	
	uint8_t pa, pb, pc;
	uint8_t comm_data;
	
	bool so;
	uint8_t clock;
	
	ls74_t b16_1, b16_2, g21_1, g21_2;
	ls151_t c15;
	ls93_t c16;
	tc4024bp_t f21;
	
	void update_cmt();
	
	const uint8_t *key_stat;
	uint8_t key_sel, key_data;
	
	uint8_t color_reg;
	bool hsync, wait;
	uint8_t cblink;
	uint8_t screen[400][640];
	scrntype_t palette_pc[8];
	
	void key_update();
	
public:
	SUB(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Sub CPU Bus"));
	}
	~SUB() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE *device)
	{
		d_cpu = device;
	}
	void set_context_main(DEVICE *device)
	{
		d_main = device;
	}
	void set_context_beep(DEVICE *device)
	{
		d_beep = device;
	}
	void set_context_drec(DEVICE *device)
	{
		d_drec = device;
	}
	void set_context_crtc(HD46505 *device, uint8_t *ptr)
	{
		d_crtc = device;
		regs = ptr;
	}
	void key_down(int code);
	void key_up(int code);
	void draw_screen();
};

}
#endif
