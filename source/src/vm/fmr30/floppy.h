/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_IRQ	0

class FLOPPY : public DEVICE
{
private:
	DEVICE *d_fdc, *d_pic;
	
	uint8 fdcr, fdsl, fdst;
	int drvsel;
	bool irq, changed[4];
	void update_intr();
	
public:
	FLOPPY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~FLOPPY() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void change_disk(int drv)
	{
		changed[drv] = true;
	}
};

#endif

