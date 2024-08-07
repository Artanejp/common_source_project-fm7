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

namespace FMR50 {
	
class TIMER : public DEVICE
{
private:
	DEVICE *d_pcm, *d_pic, *d_rtc;
	
	uint16_t free_run_counter;
	uint8_t intr_reg, rtc_data;
	bool tmout0, tmout1;
public:
	TIMER(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Timer I/F"));
	}
	~TIMER() {}
	
	// common functions
	void initialize() override;
	
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;

	void update_intr() override;
	
	bool process_state(FILEIO* state_fio, bool loading) override;
	
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

}
#endif

