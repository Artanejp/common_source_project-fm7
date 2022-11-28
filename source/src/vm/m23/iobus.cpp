/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ i/o bus ]
*/

#include "iobus.h"
#include "../mb8877.h"

void IOBUS::initialize()
{
	IO::initialize();
}

void IOBUS::reset()
{
	IO::reset();
}

void IOBUS::write_port(uint32_t addr, uint32_t data)
{
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%06x\tOUT8\t%04x,%02x\n"), get_cpu_pc(0), addr, data & 0xff);
#endif
	switch(addr & 0xff) {
	case 0xc5:
		// 10, 11, 00
/*
・第6ビット：記録密度切り替え(単密度は1、倍密度は0)
・第5ビット：読込面(Side)切り替え(Side0は0、Side1は1)
・第3ビット：Not Readyへの遷移。In USE?
*/
		break;
	case 0xd4:
		d_apu->reset();
		break;
	}
}

uint32_t IOBUS::read_port(uint32_t addr)
{
	uint32_t val = 0xff;
	
//	switch(addr & 0xff) {
//	case 0xd0:
//		val = config.dipswitch;
//		break;
//	}
#ifdef _IO_DEBUG_LOG
	this->out_debug_log(_T("%06x\tIN8\t%04x = %02x\n"), get_cpu_pc(0), addr, val & 0xff);
#endif
	return val;
}

void IOBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xc5:
	case 0xd4:
		write_port(addr, data);
		return;
	}
	IO::write_io8(addr, data);
}

uint32_t IOBUS::read_io8(uint32_t addr)
{
//	switch(addr) {
//	case 0xd0:
//		return read_port(addr);
//	}
	return IO::read_io8(addr);
}

#define STATE_VERSION	1

bool IOBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!IO::process_state(state_fio, loading)) {
		return false;
	}
	return true;
}

