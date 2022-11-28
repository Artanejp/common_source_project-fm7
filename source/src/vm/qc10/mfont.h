/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.03.24 -

	[ multifont rom card ]
*/

#ifndef _MFONT_H_
#define _MFONT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FIFO;

class MFONT : public DEVICE
{
private:
	DEVICE *d_pic;
	
	uint8_t mfont[36 * 0x60 * 2 * 17];
	uint8_t status;
	FIFO *cmd;
	FIFO *res;
	
public:
	MFONT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Multi FONT ROM Card"));
	}
	~MFONT() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

