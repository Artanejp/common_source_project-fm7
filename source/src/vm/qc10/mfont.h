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
	
	uint8 mfont[36 * 0x60 * 2 * 17];
	uint8 status;
	FIFO *cmd;
	FIFO *res;
	
public:
	MFONT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MFONT() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

