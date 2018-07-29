/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

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
	uint8_t data_buffer[EMM_BUFFER_SIZE];
	uint32_t data_addr;
	
public:
	EMM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("EMM"));
	}
	~EMM() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void decl_state();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

