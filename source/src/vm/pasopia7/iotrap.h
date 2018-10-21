/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ i/o trap ]
*/

#ifndef _IOTRAP_H_
#define _IOTRAP_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IOTRAP_I8255_2_A	0
#define SIG_IOTRAP_I8255_2_C	1

class IOTRAP : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pio2;
	bool nmi_mask, pasopia;
	
public:
	IOTRAP(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Trap"));
	}
	~IOTRAP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pio2(DEVICE* device)
	{
		d_pio2 = device;
	}
	void do_reset();
};

#endif

