/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ system ]
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SYSTEM_TC2O	0

class SYSTEM : public DEVICE
{
private:
	DEVICE *d_pcm, *d_pit;
	
	uint8_t status;
	
public:
	SYSTEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSTEM() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	
	// unique functions
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
};

#endif

