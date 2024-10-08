/*
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2011.05.16-

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_DRQ	0

namespace MZ700 {
	
class FLOPPY : public DEVICE
{
private:
	DEVICE* d_cpu;
	DEVICE* d_fdc;
	
	uint32_t prev_dc;
	int register_id;
	bool motor_on;
	bool irq_enabled;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

}
#endif

