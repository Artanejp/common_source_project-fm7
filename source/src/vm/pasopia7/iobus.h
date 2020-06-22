/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ 8bit i/o bus ]
*/

#ifndef _IOBUS_H_
#define _IOBUS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IOBUS_MIO	0

namespace PASOPIA7 {

class IOBUS : public DEVICE
{
private:
	DEVICE* d_io;
	uint8_t* ram;
	bool mio;
	
public:
	IOBUS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IOBUS() {}
	
	// common functions
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_ram_ptr(uint8_t* ptr)
	{
		ram = ptr;
	}
};

}

#endif

