/*
	NEC PC-2001 Emulator 'ePC-2001'

	Origin : PockEmul
	Author : Takeda.Toshiya
	Date   : 2016.03.18-

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_DREC_IN	0
#define SIG_IO_RTC_IN	1

class UPD16434;

class IO : public DEVICE
{
private:
	UPD16434 *d_lcd[4];
	DEVICE *d_drec;
	DEVICE *d_rtc;
	DEVICE *d_cpu;
	
	uint8_t port_a, port_b, port_s;
	bool drec_in, rtc_in;
	
	const uint8_t *key_stat;
	uint16_t key_strobe;
	
	uint8_t get_key();
	bool key_hit(int code);
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io16(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_lcd(int index, UPD16434* device)
	{
		d_lcd[index] = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
};

#endif
