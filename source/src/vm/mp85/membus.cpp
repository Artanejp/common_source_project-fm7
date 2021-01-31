/*
	MITEC MP-85 Emulator 'eMP-85'

	Author : Takeda.Toshiya
	Date   : 2021.01.19-

	[ memory bus ]
*/

#include "membus.h"
#include "../i8255.h"

void MEMBUS::reset()
{
	d_pio->write_signal(SIG_I8255_PORT_B, 0xff, 0x80 | 0x20); // PB5 = PB7 = 1
	pc = 0xff;
	MEMORY::reset();
}

uint32_t MEMBUS::fetch_op(uint32_t addr, int *wait)
{
	if((addr & 0xf800) && !(pc & 0x40)) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
	return MEMORY::fetch_op(addr, wait);
}

void MEMBUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMBUS_PC) {
		pc &= ~mask;
		pc |= data & mask;
	}
}

void MEMBUS::key_down(int code)
{
	if(code == 0x72) {
		// MONI-IN (F3) is pressed
		if(!(pc & 0x80)) {
			d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
		}
		d_pio->write_signal(SIG_I8255_PORT_B, 0x00, 0x80); // PB7 = 0
	}
}

void MEMBUS::key_up(int code)
{
	if(code == 0x72) {
		// MONI-IN is released
		d_pio->write_signal(SIG_I8255_PORT_B, 0xff, 0x80); // PB7 = 1
	}
}

#define STATE_VERSION	1

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(pc);
	return MEMORY::process_state(state_fio, loading);
}

