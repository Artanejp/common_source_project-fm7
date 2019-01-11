/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2018.01.09 -

	[ msdos rom ]
*/
#pragma once

#include "../device.h"

namespace FMTOWNS {

class MSDOSROM : public DEVICE
{
private:
	uint8_t rom[0x80000];   // 0xc2000000 - 0xc207ffff
public:
	MSDOSROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("MSDOS ROM"));
	}
	~MSDOSROM() {}

	void initialize();
	uint32_t read_data8(uint32_t addr);	
};

}
	
