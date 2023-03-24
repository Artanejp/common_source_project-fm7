/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ i/o bus ]
*/

#ifndef _IOBUS_H_
#define _IOBUS_H_

#include "../io.h"

class MB8877;

namespace M23 {
class IOBUS : public IO
{
private:
	DEVICE *d_apu;
	MB8877 *d_fdc;

	void __FASTCALL write_port(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_port(uint32_t addr);

public:
	IOBUS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : IO(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IOBUS() {}

	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_context_apu(DEVICE* device)
	{
		d_apu = device;
	}
	void set_context_fdc(MB8877* device)
	{
		d_fdc = device;
	}
};
}
#endif
