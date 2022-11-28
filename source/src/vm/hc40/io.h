/*
	EPSON HC-40 Emulator 'eHC-40'

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_DREC	0
#define SIG_IO_ART	1

class FIFO;

class IO : public DEVICE
{
private:
	DEVICE *d_cpu, *d_mem, *d_tf20, *d_beep, *d_drec;
	uint8_t* ram;
	
	// registers
	void update_intr();
	uint32_t cur_clock;
	uint8_t bcr, slbcr;
	uint8_t isr, ier;
	uint8_t bankr;
	uint8_t ioctlr;
	uint32_t icrc, icrb;
	bool ear;
	uint8_t vadr, yoff;
	
	// sub cpu
	void send_to_7508(uint8_t val);
	uint8_t rec_from_7508();
	FIFO *cmd_buf, *rsp_buf;
	// rtc
	dll_cur_time_t cur_time;
	int register_id_1sec;
	bool onesec_intr, onesec_intr_enb;
	bool alarm_intr, alarm_intr_enb;
	uint8_t alarm[6];
	// keyboard
	void update_key(int code);
	FIFO *key_buf;
	bool kb_intr_enb;
	bool kb_rep_enb, kb_caps;
	uint8_t kb_rep_spd1, kb_rep_spd2;
	// art
	FIFO *art_buf;
	uint8_t artsr, artdir;
	bool txen, rxen, dsr;
	int register_id_art;
	// beep
	bool beep;
	// reset
	bool res_z80, res_7508;
	
	// externam ram disk
	uint8_t ext[0x40000];
	uint32_t extar;
	uint8_t extcr;
	
	// display
	scrntype_t pd, pb;
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void sysreset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_callback(int event_id, int err);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t get_intr_ack();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_mem(DEVICE* device, uint8_t* ptr)
	{
		d_mem = device;
		ram = ptr;
	}
	void set_context_tf20(DEVICE* device)
	{
		d_tf20 = device;
	}
	void set_context_beep(DEVICE* device)
	{
		d_beep = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void draw_screen();
	void key_down(int code);
	void key_up(int code);
};

#endif
