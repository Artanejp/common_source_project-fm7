/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ timer ]
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_TIMER_CH0	0
#define SIG_TIMER_CH1	1
#define SIG_TIMER_RTC	2

class TIMER : public DEVICE
{
private:
	DEVICE *d_pcm, *d_pic, *d_rtc;
	
	uint16 free_run_counter;
	uint8 intr_reg, rtc_data;
	bool tmout0, tmout1;
	void update_intr();
	
public:
	TIMER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~TIMER() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

