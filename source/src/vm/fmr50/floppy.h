/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_IRQ	0

class MB8877;

class FLOPPY : public DEVICE
{
private:
	MB8877 *d_fdc;
	DEVICE *d_pic;
	
	int drvreg, drvsel;
	bool irq, irqmsk, changed[4];
	void update_intr();
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_fdc(MB8877* device)
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

