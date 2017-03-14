/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2012.02.03-

	[ PC-9801-26 ]
*/

#ifndef _FMSOUND_H_
#define _FMSOUND_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FMSOUND : public DEVICE
{
private:
	DEVICE* d_opn;
#ifdef SUPPORT_PC98_OPNA
	uint8_t mask;
#endif
	
public:
	FMSOUND(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef SUPPORT_PC98_OPNA
		set_device_name(_T("PC-9801-86 (FM Sound)"));
#else
		set_device_name(_T("PC-9801-26 (FM Sound)"));
#endif
	}
	~FMSOUND() {}
	
	// common functions
#ifdef SUPPORT_PC98_OPNA
	void reset();
#endif
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#ifdef SUPPORT_PC98_OPNA
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
#endif
	
	// unique function
	void set_context_opn(DEVICE* device)
	{
		d_opn = device;
	}
};

#endif

