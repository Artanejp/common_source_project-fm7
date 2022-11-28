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

// 74LS74
typedef struct ls74_s {
	bool in_d, in_ck, in_s, in_r;
	bool out_q, out_nq;
	bool tmp_ck;
	void update()
	{
		if(!in_s && in_r) {
			out_q = true;
			out_nq = false;
		} else if(in_s && !in_r) {
			out_q = false;
			out_nq = true;
		} else if(!in_s && !in_r) {
			out_q = out_nq = true; // undetermined
		} else if(!tmp_ck && in_ck) {
			out_q = in_d;
			out_nq = !in_d;
		}
		tmp_ck = in_ck;
	}
} ls74_t;

// 74LS151
typedef struct ls151_s {
	bool in_d0, in_d1, in_d2, in_d3, in_d4, in_d5, in_d6, in_d7;
	bool in_a, in_b, in_c, in_s;
	bool out_y, out_ny;
	void update()
	{
		if(in_s) {
			out_y = false;
		} else if(!in_a && !in_b && !in_c) {
			out_y = in_d0;
		} else if( in_a && !in_b && !in_c) {
			out_y = in_d1;
		} else if(!in_a &&  in_b && !in_c) {
			out_y = in_d2;
		} else if( in_a &&  in_b && !in_c) {
			out_y = in_d3;
		} else if(!in_a && !in_b &&  in_c) {
			out_y = in_d4;
		} else if( in_a && !in_b &&  in_c) {
			out_y = in_d5;
		} else if(!in_a &&  in_b &&  in_c) {
			out_y = in_d6;
		} else if( in_a &&  in_b &&  in_c) {
			out_y = in_d7;
		}
		out_ny = !out_y;
	}
} ls151_t;

// 74LS93
typedef struct ls93_s {
	bool in_a, in_b, in_rc1, in_rc2;
	bool out_qa, out_qb, out_qc;
	bool tmp_a, tmp_b;
	uint8_t counter_a, counter_b;
	void update()
	{
		if(in_rc1 && in_rc2) {
			counter_a = counter_b = 0;
		} else {
			if(tmp_a && !in_a) {
				counter_a++;
			}
			if(tmp_b && !in_b) {
				counter_b++;
			}
		}
		tmp_a = in_a;
		tmp_b = in_b;
		out_qa = ((counter_a & 1) != 0);
		out_qb = ((counter_b & 1) != 0);
		out_qc = ((counter_b & 2) != 0);
	}
} ls93_t;

// TC4024BP
typedef struct tc4024bp_s {
	bool in_ck, in_clr;
	bool out_q5, out_q6;
	bool tmp_ck;
	uint8_t counter;
	void update()
	{
		if(in_clr) {
			counter = 0;
		} else if(tmp_ck && !in_ck) {
			counter++;
		}
		tmp_ck = in_ck;
		out_q5 = ((counter & 0x10) != 0);
		out_q6 = ((counter & 0x20) != 0);
	}
} tc4024bp_t;

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

#endif
