/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2011.02.17-

	[ emm ]
*/

#ifndef _EMM_H_
#define _EMM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define EMM_BUFFER_SIZE	0x50000

class EMM : public DEVICE
{
private:
	uint8 data_buffer[EMM_BUFFER_SIZE];
	uint32 data_addr;
	
public:
	EMM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~EMM() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

