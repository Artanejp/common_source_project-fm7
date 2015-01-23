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

class SUB : public DEVICE
{
private:
	// to sub cpu
	DEVICE *d_cpu;
	// to main pcb
	DEVICE *d_main;
	// to beep
	DEVICE *d_beep;
	// from/to crtc
	DEVICE *d_crtc;
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
	uint8 *key_stat;
	uint8 key_sel, key_data;
	uint8 color;
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
	void set_context_crtc(DEVICE *device, uint8* ptr)
	{
		d_crtc = device;
		regs = ptr;
	}
	void key_down(int code);
	void key_up(int code);
	void draw_screen();
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	bool tape_inserted()
	{
		return false;
	}
	void close_tape();
};

#endif
