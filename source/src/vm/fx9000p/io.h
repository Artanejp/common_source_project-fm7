/*
	CASIO FX-9000P Emulator 'eFX-9000P'

	Author : Takeda.Toshiya
	Date   : 2022.03.25-

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu_template.h"
#include "../device.h"

class HD46505;
class FIFO;

#define SIG_IO_DISP	0
#define SIG_IO_EAR	1

namespace FX9000P {
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

	uint32_t __FASTCALL get_rtc(uint32_t addr);
	void set_rtc(uint32_t addr, uint32_t data);

public:
	IO(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O"));
	}
	~IO() {}

	// common functions
	void initialize()  override;
	void release()  override;
	void reset()  override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data)  override;
	uint32_t __FASTCALL read_io8(uint32_t addr)  override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask)  override;
	void event_frame()  override;
	void __FASTCALL event_callback(int event_id, int err)  override;
	bool process_state(FILEIO* state_fio, bool loading)  override;

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
	void __FASTCALL key_down(int code);
	void __FASTCALL draw_screen(uint8_t *vram);
};
}

#endif
