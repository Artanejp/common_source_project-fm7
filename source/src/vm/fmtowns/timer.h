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

#define SIG_TIMER_CH0	    0
#define SIG_TIMER_CH1	    1
#define SIG_TIMER_CH2	    2
#define SIG_TIMER_RTC	    3
#define SIG_TIMER_RTC_BUSY  4

namespace FMTOWNS {
class TIMER : public DEVICE
{
private:
	DEVICE *d_pcm, *d_rtc;
	outputs_t outputs_intr_line;
	outputs_t outputs_halt_line;
	
	uint16_t free_run_counter;
	uint8_t intr_reg, rtc_data;
	bool rtc_busy;
	bool tmout0, tmout1;

	bool interval_enabled;
	pair16_t interval_us;
	bool intv_i;
	bool intv_ov;

	int event_wait_1us;
	int event_interval_us;
	
	uint16_t machine_id;
	uint8_t cpu_id;
	
	virtual void update_intr();
	virtual void do_interval(void);
public:
	TIMER(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		machine_id = 0x0100;   // FM-Towns 1,2
		initialize_output_signals(&outputs_intr_line);
		initialize_output_signals(&outputs_halt_line);
	}
	~TIMER() {}
	
	// common functions
	virtual void initialize();
	virtual void reset();
	virtual void event_callback(int id, int err);

	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	void __FASTCALL write_io16(uint32_t addr, uint32_t data);
	
	uint32_t __FASTCALL read_io8(uint32_t addr);
	uint32_t __FASTCALL read_io16(uint32_t addr);
	
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_intr_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_intr_line, dev, id, mask);
	}
	void set_context_halt_line(DEVICE *dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_halt_line, dev, id, mask);
	}
};
}

#endif

