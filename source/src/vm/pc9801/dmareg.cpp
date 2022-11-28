/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2017.06.25-

	[ dma regs ]
*/

#include "dmareg.h"
#include "../i8237.h"

static const int bank_lo_id[] = {
	SIG_I8237_BANK1, SIG_I8237_BANK2, SIG_I8237_BANK3, SIG_I8237_BANK0,
};
static const int bank_hi_id[] = {
	SIG_I8237_BANK0, SIG_I8237_BANK1, SIG_I8237_BANK2, SIG_I8237_BANK3,
};
static const int mask_id[] = {
	SIG_I8237_MASK0, SIG_I8237_MASK1, SIG_I8237_MASK2, SIG_I8237_MASK3,
};

void DMAREG::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0021:
	case 0x0023:
	case 0x0025:
	case 0x0027:
#if defined(SUPPORT_32BIT_ADDRESS)
		d_dma->write_signal(bank_lo_id[((addr - 0x0021) >> 1) & 3], data & 0xff, 0x00ff);
#elif defined(_PC98XA)
		d_dma->write_signal(bank_lo_id[((addr - 0x0021) >> 1) & 3], data & 0x7f, 0x00ff);
#elif defined(SUPPORT_24BIT_ADDRESS)
		d_dma->write_signal(bank_lo_id[((addr - 0x0021) >> 1) & 3], data & 0xff, 0x00ff);
#else
		d_dma->write_signal(bank_lo_id[((addr - 0x0021) >> 1) & 3], data & 0x0f, 0x00ff);
#endif
		break;
#if defined(SUPPORT_24BIT_ADDRESS) || defined(SUPPORT_32BIT_ADDRESS)
	case 0x0029:
		switch(data & 0x0c) {
		case 0x00:
			d_dma->write_signal(mask_id[data & 3], 0x0000, 0xffff); // 64KB
			break;
		case 0x04:
			d_dma->write_signal(mask_id[data & 3], 0x000f, 0xffff); // 1MB
			break;
		case 0x0c:
			d_dma->write_signal(mask_id[data & 3], 0x00ff, 0xffff); // 16MB
			break;
		}
		break;
#endif
#if defined(SUPPORT_32BIT_ADDRESS)
	case 0x0e05:
	case 0x0e07:
	case 0x0e09:
	case 0x0e0b:
		d_dma->write_signal(bank_hi_id[((addr - 0xe05) >> 1) & 3], (data & 0xff) << 8, 0xff00);
		break;
#endif
	}
}

uint32_t DMAREG::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0021:
	case 0x0023:
	case 0x0025:
	case 0x0027:
		return d_dma->read_signal(bank_lo_id[((addr - 0x0021) >> 1) & 3]);
	}
	return 0xff;
}

/*
#define STATE_VERSION	1

bool DMAREG::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return true;
}
*/
