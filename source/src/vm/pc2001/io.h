/*
	CANON X-07 Emulator 'eX-07'

	Origin : J.Brigaud
	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

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
	
	uint8_t port_a;
	bool drec_in;
	bool rtc_in;
	
	uint8_t key_stat[256];
	uint16_t key_strobe;
	
	uint8_t get_key();
	bool key_hit(int code);
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io16(uint32_t addr, uint32_t data);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
	void key_down(int code);
	void key_up(int code);
};

#endif
