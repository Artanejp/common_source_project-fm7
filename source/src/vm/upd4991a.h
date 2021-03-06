/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ uPD4991A ]
*/

#ifndef _UPD4991A_H_
#define _UPD4991A_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

class  DLL_PREFIX UPD4991A : public DEVICE
{
private:
	dll_cur_time_t cur_time;
	int register_id;
	
	uint8_t regs[3][13];
	uint8_t ctrl1, ctrl2, mode;
	
	void __FASTCALL read_from_cur_time();
	void __FASTCALL write_to_cur_time();
	
public:
	UPD4991A(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("uPD4991A RTC"));
	}
	~UPD4991A() {}
	
	// common functions
	void initialize();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

