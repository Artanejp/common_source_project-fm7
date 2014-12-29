/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.03.31-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cmt, *d_drec, *d_rtc;
	
	uint8 ipl[0x8000];	// rom #0
	uint8 ext[0x8000];	// rom #1
	uint8 ram[0x8000*3];	// standard and optional ram
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	uint8* wbank[16];
	uint8* rbank[16];
	
	uint8 sio, bank;
	void update_bank();
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique functions
	void set_context_cmt(DEVICE* device)
	{
		d_cmt = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

