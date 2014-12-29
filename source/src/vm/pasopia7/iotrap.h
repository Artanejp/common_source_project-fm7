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
	IOTRAP(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IOTRAP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
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

