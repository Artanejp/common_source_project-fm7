/*
	FUJITSU FM16pi Emulator 'eFM16pi'

	Author : Takeda.Toshiya
	Date   : 2010.12.25-

	[ sub system ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SUB_RTC	0

class FIFO;

class SUB : public DEVICE
{
private:
	DEVICE *d_cpu, *d_fdc, *d_pcm, *d_pic, *d_pio, *d_rtc;
	
	FIFO *key_buffer;
	uint8 key_data;
	bool key_irq;
	uint8 fdc_drive, fdc_side;
	uint8 rtc_data;
	uint8 *vram;
	
public:
	SUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SUB() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_vram_ptr(uint8 *ptr)
	{
		vram = ptr;
	}
	void key_down(int code);
	void key_up(int code);
	void notify_power_off();
	void draw_screen();
};

#endif
