/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2021.02.06-

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm_template.h"
#include "../../emu_template.h"
#include "../device.h"

class FILEIO;

namespace BX1 {
	
class PRINTER : public DEVICE
{
private:
	uint8_t *ram;
	FILEIO *fio;
	uint8_t column, htab;
	uint8_t strobe, outdata;
	
	void output(uint8_t);
	
public:
	PRINTER(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Printer"));
	}
	~PRINTER() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_vline(int v, int clock);
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_ram(uint8_t* ptr)
	{
		ram = ptr;
	}
	void __FASTCALL key_down(int code);
	void __FASTCALL key_up(int code);
};
}

#endif

