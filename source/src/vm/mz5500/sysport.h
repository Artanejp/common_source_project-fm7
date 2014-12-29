/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ system port ]
*/

#ifndef _SYSPORT_H_
#define _SYSPORT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SYSPORT : public DEVICE
{
private:
	DEVICE *d_fdc, *d_ctc, *d_sio;
	int rst, highden;
	
public:
	SYSPORT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SYSPORT() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_frame()
	{
		if(rst) rst--;
	}
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_ctc(DEVICE* device)
	{
		d_ctc = device;
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void nmi_reset()
	{
		rst = 20;
	}
};

#endif

