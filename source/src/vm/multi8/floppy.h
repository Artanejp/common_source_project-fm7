/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.17 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_DRQ	0

namespace MULTI8 {

class FLOPPY : public DEVICE
{
private:
	DEVICE *d_fdc;
	bool drq;

public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}

	// common functions
	void __FASTCALL write_io8(uint32_t addr, uint32_t data)  override;
	uint32_t __FASTCALL read_io8(uint32_t addr)  override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask)  override;
	bool process_state(FILEIO* state_fio, bool loading)  override;

	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

}
#endif
