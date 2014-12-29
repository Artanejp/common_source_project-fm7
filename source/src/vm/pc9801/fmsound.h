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
#ifdef HAS_YM2608
	uint8 mask;
#endif
	
public:
	FMSOUND(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~FMSOUND() {}
	
	// common functions
#ifdef HAS_YM2608
	void reset();
#endif
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
#ifdef HAS_YM2608
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

