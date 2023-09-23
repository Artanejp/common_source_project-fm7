/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.30 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../device.h"

#define SIG_FLOPPY_IRQ	0

class MB8877;

namespace FMTOWNS {
class FLOPPY : public DEVICE
{
private:
	MB8877 *d_fdc;
	outputs_t output_intr_line;
	int drvreg, drvsel;
	bool irq, irqmsk;
	bool drive_swapped;
	uint16_t machine_id;
	uint8_t cpu_id;
	bool is_removed;
	bool is_inserted[4];

public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&output_intr_line);
		machine_id = 0x0100;
		cpu_id = 0x01;
	}
	~FLOPPY() {}

	// common functions
	void initialize() override;
	void reset() override;

	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;

	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void update_intr() override;

	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_context_fdc(MB8877* device)
	{
		d_fdc = device;
	}
	void set_context_intr_line(DEVICE* dev, int id, uint32_t mask)
	{
		register_output_signal(&output_intr_line, dev, id, mask);
	}
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
	void change_disk(int drv)
	{
		is_inserted[drv] = true;
	}
};
}

#endif
