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
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2022.07.06-

	[ serial ]
*/

#include "serial.h"
#include "../i8259.h"

void SERIAL::reset()
{
	pc = 0;
	rxr = txe = txr = false;
}

void SERIAL::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SERIAL_PORT_C) {
		pc = (pc & ~mask) | (data & mask);
	} else if(id == SIG_SERIAL_RXR) {
		rxr = ((data & mask) != 0);
	} else if(id == SIG_SERIAL_TXE) {
		txe = ((data & mask) != 0);
	} else if(id == SIG_SERIAL_TXR) {
		txr = ((data & mask) != 0);
	}
	update_irq();
}

void SERIAL::update_irq()
{
	bool value = false;
	
	if((pc & 0x01) != 0 && rxr) value = true;
	if((pc & 0x02) != 0 && txe) value = true;
	if((pc & 0x04) != 0 && txr) value = true;
	
	d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR4, value ? 1 : 0, 1);
}

#define STATE_VERSION	1

bool SERIAL::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(pc);
	state_fio->StateValue(rxr);
	state_fio->StateValue(txe);
	state_fio->StateValue(txr);
	return true;
}

