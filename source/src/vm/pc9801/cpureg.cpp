/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2017.06.25-

	[ cpu regs ]
*/

#include "cpureg.h"
#if defined(SUPPORT_32BIT_ADDRESS)
#include "../i386.h"
#else
#include "../i286.h"
#endif

void CPUREG::reset()
{
	d_cpu->set_address_mask(0x000fffff);
}

void CPUREG::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x00f0:
		d_cpu->reset();
		d_cpu->set_address_mask(0x000fffff);
		break;
	case 0x00f2:
#if defined(SUPPORT_32BIT_ADDRESS)
		d_cpu->set_address_mask(0xffffffff);
#else
		d_cpu->set_address_mask(0x00ffffff);
#endif
		break;
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x00f6:
		switch(data) {
		case 0x02:
			d_cpu->set_address_mask(0xffffffff);
			break;
		case 0x03:
			d_cpu->set_address_mask(0x000fffff);
			break;
		}
		break;
#endif
	}
}

uint32_t CPUREG::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x00f0:
		return 0xe9;
	case 0x00f2:
		return ((d_cpu->get_address_mask() & (1 << 20)) ? 0 : 1) | 0x2e;
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x00f6:
		return ((d_cpu->get_address_mask() & (1 << 20)) ? 0 : 1) | 0x5e;
#endif
	}
	return 0xff;
}

/*
#define STATE_VERSION	1

void CPUREG::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
}

bool CPUREG::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	return true;
}
*/
