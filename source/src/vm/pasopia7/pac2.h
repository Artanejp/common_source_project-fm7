/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ pac slot 2 ]
*/

#ifndef _PAC2_H_
#define _PAC2_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PAC2DEV;
class RAMPAC2;
class KANJIPAC2;
class JOYPAC2;

class PAC2 : public DEVICE
{
private:
	PAC2DEV* dev[8];
	int sel;
	
	RAMPAC2* rampac2[2];
	KANJIPAC2* kanji;
	JOYPAC2* joy;
	PAC2DEV* dummy;
	
public:
	PAC2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("PAC2 Slot"));
	}
	~PAC2() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void open_rampac2(int drv, const _TCHAR* file_path);
};

#endif

