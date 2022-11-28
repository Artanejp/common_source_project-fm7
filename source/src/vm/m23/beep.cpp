/*
	SORD M23 Emulator 'Emu23'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ beep ]
*/

#include "beep.h"

void BEEP::initialize()
{
	PCM1BIT::initialize();
	reg = 0;
}

void BEEP::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xd3:
		reg ^= 1;
		PCM1BIT::write_signal(SIG_PCM1BIT_SIGNAL, reg, 1);
		break;
	}
}

uint32_t BEEP::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xd3:
		reg ^= 1;
		PCM1BIT::write_signal(SIG_PCM1BIT_SIGNAL, reg, 1);
		return reg;
	}
	return 0xff;
}

#define STATE_VERSION	1

bool BEEP::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!PCM1BIT::process_state(state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(reg);
	return true;
}

