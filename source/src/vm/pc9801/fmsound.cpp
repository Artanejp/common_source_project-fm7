/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2012.02.03-

	[ PC-9801-26 ]
*/

#include "fmsound.h"

// PC-98DO+
#define BOARD_ID	0

#ifdef SUPPORT_PC98_OPNA
void FMSOUND::reset()
{
	mask = 0;
}
#endif

void FMSOUND::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0188:
		d_opn->write_io8(0, data);
		break;
	case 0x018a:
		d_opn->write_io8(1, data);
		break;
#ifdef SUPPORT_PC98_OPNA
	case 0x018c:
		if(mask & 1) {
			d_opn->write_io8(2, data);
		}
		break;
	case 0x018e:
		if(mask & 1) {
			d_opn->write_io8(3, data);
		}
		break;
	case 0xa460:
		mask = data;
		break;
#endif
	}
}

uint32_t FMSOUND::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0188:
		return d_opn->read_io8(0);
	case 0x018a:
		return d_opn->read_io8(1);
#ifdef SUPPORT_PC98_OPNA
	case 0x018c:
		if(mask & 1) {
			return d_opn->read_io8(2);
		}
		break;
	case 0x018e:
		if(mask & 1) {
			return d_opn->read_io8(3);
		}
		break;
	case 0xa460:
		return BOARD_ID | (mask & 3);
#endif
	}
	return 0xff;
}

#ifdef SUPPORT_PC98_OPNA
#define STATE_VERSION	1

bool FMSOUND::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(mask);
	return true;
}
#endif

