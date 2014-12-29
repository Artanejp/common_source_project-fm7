/*
	SORD m5 Emulator 'Emu5'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt/printer ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_IN		0
#define SIG_CMT_EOT		1
#define SIG_PRINTER_BUSY	2

class CMT : public DEVICE
{
private:
	DEVICE* d_drec;
	
	// data recorder
	bool in, out, remote, eot;
	
	// printer
	uint8 pout;
	bool strobe, busy;
	
	// reset/halt key
	uint8* key_stat;
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMT() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
};

#endif
