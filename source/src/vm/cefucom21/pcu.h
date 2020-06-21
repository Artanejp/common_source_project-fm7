/*
	Hino Electronics CEFUCOM-21 Emulator 'eCEFUCOM-21'

	Author : Takeda.Toshiya
	Date   : 2019.03.28-

	[ PCU-I (peripheral control unit ???) ]
*/

#ifndef _PCU_H_
#define _PCU_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PCU : public DEVICE
{
private:
	
public:
	PCU(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("PCU-I"));
	}
	~PCU() {}
	
	// common functions
	void initialize();
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void event_frame();
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

