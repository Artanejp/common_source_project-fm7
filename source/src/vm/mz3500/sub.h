/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ sub ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SUB_RTC_DOUT	0
#define SIG_SUB_PIO_OBF		1
#define SIG_SUB_PIO_PM		2
#define SIG_SUB_KEYBOARD_DC	3
#define SIG_SUB_KEYBOARD_STC	4
#define SIG_SUB_KEYBOARD_ACKC	5

class SUB : public DEVICE
{
private:
	DEVICE *d_main;
	
	// memory
	uint8* rbank[32];	// 64KB / 2KB
	uint8* wbank[32];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ram[0x2000];
	uint8* ipl;
	uint8* common;
	
	// display
	uint8 vram_chr[0x1000];
	uint8 *sync_chr, *ra_chr, *cs_chr;
	int* ead_chr;
	uint8 vram_gfx[0x20000];
	uint8 *sync_gfx, *ra_gfx, *cs_gfx;
	int* ead_gfx;
	uint8 disp[16];
	bool pm;
	
	uint8 screen_chr[400][640];
	uint8 screen_gfx[400][640];
	uint8 font[0x2000];
	scrntype palette_pc[8];
	int blink;
	
	void draw_chr();
	void draw_gfx();
	
	// rtc, pio
	bool dout, obf;
	
	// keyboard
	bool dk, stk, hlt;	// to cpu
	bool dc, stc, ackc;	// from cpu
	int key_phase;
	int key_send_data, key_send_bit;
	int key_recv_data, key_recv_bit;
	void key_send(int data, bool command);
	void key_recv(int data);
	void key_drive();
	
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
	
	// unique functions
	void set_context_main(DEVICE* device)
	{
		d_main = device;
	}
	void set_ipl(uint8* ptr)
	{
		ipl = ptr;
	}
	void set_common(uint8* ptr)
	{
		common = ptr;
	}
	uint8* get_vram_chr()
	{
		return vram_chr;
	}
	void set_sync_ptr_chr(uint8* ptr)
	{
		sync_chr = ptr;
	}
	void set_ra_ptr_chr(uint8* ptr)
	{
		ra_chr = ptr;
	}
	void set_cs_ptr_chr(uint8* ptr)
	{
		cs_chr = ptr;
	}
	void set_ead_ptr_chr(int* ptr)
	{
		ead_chr = ptr;
	}
	uint8* get_vram_gfx()
	{
		return vram_gfx;
	}
	void set_sync_ptr_gfx(uint8* ptr)
	{
		sync_gfx = ptr;
	}
	void set_ra_ptr_gfx(uint8* ptr)
	{
		ra_gfx = ptr;
	}
	void set_cs_ptr_gfx(uint8* ptr)
	{
		cs_gfx = ptr;
	}
	void set_ead_ptr_gfx(int* ptr)
	{
		ead_gfx = ptr;
	}
	void key_down(int code);
	void key_up(int code);
	void draw_screen();
};

#endif

