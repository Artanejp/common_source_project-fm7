/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ rtc ]
*/

#ifndef _RTC_H_
#define _RTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class RTC : public DEVICE
{
private:
	DEVICE* d_pic;
	
	cur_time_t cur_time;
	int register_id;
	
	uint16 rtcmr, rtdsr, rtadr, rtobr, rtibr;
	uint8 regs[40];
	
	void read_from_cur_time();
	void write_to_cur_time();
	void update_checksum();
	void update_intr();
public:
	RTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~RTC() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data)
	{
		write_io16(addr, data);
	}
	uint32 read_io8(uint32 addr)
	{
		return (uint8)read_io16(addr);
	}
	void write_io16(uint32 addr, uint32 data);
	uint32 read_io16(uint32 addr);
	void event_callback(int event_id, int err);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

