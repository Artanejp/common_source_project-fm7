/*
	NEC PC-9801VM Emulator 'ePC-9801VM'

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2010.09.16-

	[ ITF ROM ]
*/

#ifndef _PC98_ITF_H_
#define _PC98_ITF_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

enum {
	PC98_ITF_IPL = 0,
	PC98_ITF_ITF,
	PC98_ITF_MENU, // PC98S etc.
};

class ITF : public DEVICE
{
private:
	uint8_t *itf;
	uint8_t *ipl;
	uint8_t itf_bank;

public:
	ITF(VM *parent_vm, EMU* parent_emu);
	~ITF();

	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_memory_mapped_io8(uint32_t addr, uint32_t data);
	uint32_t read_memory_mapped_io8(uint32_t addr);
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);

	void set_context_itf(uint8_t *p)
	{
		itf = p;
	}
	void set_context_ipl(uint8_t *p)
	{
		ipl = p;
	}

};
#endif
