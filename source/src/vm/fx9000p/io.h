/*
	CASIO FX-9000P Emulator 'eFX-9000P'

	Author : Takeda.Toshiya
	Date   : 2022.03.25-

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class HD46505;
class FIFO;

#define SIG_IO_DISP	0
#define SIG_IO_EAR	1

class IO : public DEVICE
{
private:
	DEVICE *d_cpu;
	HD46505 *d_crtc;
	DEVICE *d_drec;
	
	// keyboard
	FIFO *key_fifo;
	uint8_t key_data;
	
	// display
	uint8_t crtc_blink, crtc_disp;
	
	// OP-1 rtc
	dll_cur_time_t cur_time;
	uint8_t cmt_ear;
	uint32_t op1_addr, op1_data;
	
	uint32_t get_rtc(uint32_t addr);
	void set_rtc(uint32_t addr, uint32_t data);
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_crtc(HD46505* device)
	{
		d_crtc = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void key_down(int code);
	void draw_screen(uint8_t *vram);
};

#endif
