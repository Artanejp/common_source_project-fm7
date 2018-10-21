/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.08.14 -

	[ note i/o ]
*/

#ifndef _NOTE_H_
#define _NOTE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class NOTE : public DEVICE
{
private:
	DEVICE *d_pic;
	uint8_t ch, regs[16];
	
public:
	NOTE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("98NOTE I/O"));
	}
	~NOTE() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

