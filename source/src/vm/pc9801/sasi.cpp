/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

	Author : Takeda.Toshiya
	Date   : 2018.04.01-

	[ sasi i/f ]
*/

#include "sasi.h"

#define OCR_CHEN	0x80
#define OCR_NRDSW	0x40
#define OCR_SEL		0x20
#define OCR_RST		0x08
#define OCR_DMAE	0x02
#define OCR_INTE	0x01

#define ISR_REQ		0x80
#define ISR_ACK		0x40
#define ISR_BSY		0x20
#define ISR_MSG		0x10
#define ISR_CXD		0x08
#define ISR_IXO		0x04
#define ISR_INT		0x01

void SASI::initialize()
{
	
}

void SASI::reset()
{
	
}

void SASI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x0080:
		break;
	case 0x0082:
		ocr = data;
		break;
	}
}

uint32_t SASI::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x0080:
		break;
	case 0x0082:
		if(ocr & OCR_NRDSW) {
			return isr;
		} else {
			
		}
		break;
	}
	return 0xff;
}

void SASI::write_dma_io8(uint32_t addr, uint32_t data)
{
	write_io8(0x0080, data);
}

uint32_t SASI::read_dma_io8(uint32_t addr)
{
	return read_io8(0x0080);
}

#define STATE_VERSION	1

void SASI::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
}

bool SASI::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	return true;
}

