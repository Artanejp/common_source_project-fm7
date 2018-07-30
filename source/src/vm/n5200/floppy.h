/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2008.06.11 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_DRQ	0

class FLOPPY : public DEVICE
{
private:
	DEVICE *d_fdc, *d_dma;
	
	uint8_t chgreg, ctrlreg;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	
	// unique functions
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
};

#endif

