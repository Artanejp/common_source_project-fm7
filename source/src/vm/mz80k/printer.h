/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'
	SHARP MZ-80A Emulator 'EmuZ-80A'

	Author : Hideki Suga
	Date   : 2016.03.18-

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PRINTER : public DEVICE
{
private:
	DEVICE* d_prn;
#if defined(_MZ1200) || defined(_MZ80K)
	uint8_t out_ch;
#endif
	
public:
	PRINTER(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Printer I/F"));
	}
	~PRINTER() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#if defined(_MZ1200) || defined(_MZ80K)
	bool process_state(FILEIO* state_fio, bool loading);
#endif
	
	// unique function
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
};

#endif

