/*
	SORD M23 Emulator 'Emu23'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ beep ]
*/

#ifndef _BEEP_H_
#define _BEEP_H_

#include "../pcm1bit.h"

class BEEP: public PCM1BIT
{
private:
	uint8_t reg;
	
public:
	BEEP(VM_TEMPLATE* parent_vm, EMU* parent_emu) : PCM1BIT(parent_vm, parent_emu)
	{
		set_device_name(_T("Beep"));
	}
	~BEEP() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif
