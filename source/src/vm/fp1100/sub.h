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
	uint8 *regs;
	
	uint8 *wbank[0x200];
	uint8 *rbank[0x200];
	uint8 wdmy[0x80];
	uint8 rdmy[0x80];
	
	uint8 ram[0x80];
	uint8 vram_b[0x4000];
	uint8 vram_r[0x4000];
	uint8 vram_g[0x4000];
	uint8 sub1[0x1000];
	uint8 sub2[0x1000];
	uint8 sub3[0x1000];
	
	uint8 pa, pb, pc;
	uint8 comm_data;
	
	bool so;
	uint8 clock;
	
	// 74LS74
	struct {
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
	} b16_1, b16_2, g21_1, g21_2;
	
	// 74LS151
	struct {
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
	} c15;
	
	// 74LS93
	struct {
		bool in_a, in_b, in_rc1, in_rc2;
		bool out_qa, out_qb, out_qc;
		bool tmp_a, tmp_b;
		uint8 counter_a, counter_b;
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
	} c16;
	
	// TC4024BP
	struct {
		bool in_ck, in_clr;
		bool out_q5, out_q6;
		bool tmp_ck;
		uint8 counter;
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
	} f21;
	
	void update_cmt();
	
	const uint8 *key_stat;
	uint8 key_sel, key_data;
	
	uint8 color_reg;
	bool hsync, wait;
	uint8 cblink;
	uint8 screen[400][640];
	scrntype palette_pc[8];
	
	void key_update();
	
public:
	SUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SUB() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
	void set_context_crtc(HD46505 *device, uint8 *ptr)
	{
		d_crtc = device;
		regs = ptr;
	}
	void key_down(int code);
	void key_up(int code);
	void draw_screen();
};

#endif
