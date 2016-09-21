/*
	NEC PC-9801VM Emulator 'ePC-9801VM'

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2010.09.16-

	[ ITF ROM ]
*/

#include "itf.h"

ITF::ITF(VM *parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	itf = NULL;
	ipl = NULL;
	itf_bank = PC98_ITF_ITF;
}

ITF::~ITF()
{
}

void ITF::reset(void)
{
	itf_bank = PC98_ITF_ITF;
}

// See: http://www.webtech.co.jp/company/doc/undocumented_mem/io_mem.txt .
void ITF::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x1) {
	case 0x1:
		printf("SET %05x %02x\n", addr, data);
		if((data & 0xff) == 0x10) {
#if defined(_PCH98S)
			itf_bank = PC98_ITF_MENU;
#else
			itf_bank = PC98_ITF_ITF;
#endif			
		} else if((data & 0xff) == 0x12) {
			itf_bank = PC98_ITF_IPL;
		}
#if defined(_PCH98S)
		else if((data & 0xff) == 0x18) {
			itf_bank = PC98_ITF_ITF;
		}
#endif
		break;
	default:
		break;
	}
}
		
uint32_t ITF::read_io8(uint32_t addr)
{
	// Now, Dummy value.
	switch(addr & 0x1) {
	case 0x1:
		// bit2: Whether cache was hit is "1".
		return 0xffffffff;
		break;
	default:
		break;
	}
	return 0xffffffff;
}

void ITF::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	return; // NOOP (only ROM)
}

uint32_t ITF::read_memory_mapped_io8(uint32_t addr)
{
	uint32_t raddr = addr & 0xfffff;
	// ToDo: Address with i286/i386/i486.
	//if(raddr < 0xe8000) return 0xff; // OK?
	switch(itf_bank) {
	case PC98_ITF_IPL:
		if(ipl == NULL) return 0xff;
		return ipl[raddr - 0xe8000];
		break;
	case PC98_ITF_ITF:
		if(itf == NULL) return 0xff;
		raddr = raddr & 0x7fff;
		printf("DATA: %05x %02x\n", addr, itf[raddr]);
		return itf[raddr];
		break;
#if defined(_PCH98S)
	case PC98_ITF_ITF:
		return 0xff;
		break;
#endif		
	}
	return 0xff;
}

#define STATE_VERSION	1

void ITF::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	state_fio->FputUint8(itf_bank);
}

bool ITF::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	itf_bank = state_fio->FgetUint8();
	return true;
}
