/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.08.14 -

	[ note i/o ]
*/

#include "note.h"
#include "../i8259.h"

void NOTE::initialize()
{
	ch = 0;
	memset(regs, 0, sizeof(regs));
}

void NOTE::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x810:
		regs[ch & 0x0f] = data;
		break;
	case 0x812:
		ch = data;
		break;
	case 0x4810:
		// unknown
		break;
	case 0x6e8e:
		// modem control 1
		break;
	case 0x7e8e:
		// modem control 2
		break;
	case 0x8810:
		// power status
		// bit5 = 1: unknown
		// bit2 = 1: stanby
		// bit0 = 1: power off
//		d_pic->write_signal(SIG_I8259_IR5, data, 2);
		break;
	case 0xc810:
		// unknown
		break;
	}
}

uint32_t NOTE::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x810:
		return regs[ch & 0x0f];
	case 0x812:
		return ch;
	case 0xf8e:
		// pc card slot
		return 0xe;	// 4 if no memcard
	case 0x5e8e:
		// pc card slot ???
		return 0x46;	// 0x40 if no memcard
	case 0x8810:
		// bit7 = 1: docking station
		// bit6 = 1: ac power supply
		// bit4 = 1: alarm
		// bit3 = 1: unknown
		// bit2 = 1: li.battery low
		// bit1 = 1: battery low
		// bit0 = 1: power off
#ifdef DOCKING_STATION
		return 0xc0;
#else
		return 0x40;
#endif
	}
	return 0xff;
}

#define STATE_VERSION	1

bool NOTE::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ch);
	state_fio->StateArray(regs, sizeof(regs), 1);
	return true;
}

