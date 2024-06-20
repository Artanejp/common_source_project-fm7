/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MB8877;
#ifdef _X1TURBO_FEATURE
class Z80DMA;
#endif

namespace X1 {

class FLOPPY : public DEVICE
{
private:
	MB8877 *d_fdc;
	int prev;
	bool motor_on;
	int register_id;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		prev = 0;
		motor_on = false;
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
#ifdef _X1TURBO_FEATURE
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
#endif
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_fdc(MB8877* device)
	{
		d_fdc = device;
	}
	bool get_motor_on()
	{
		return motor_on;
	}
};

}
#endif

