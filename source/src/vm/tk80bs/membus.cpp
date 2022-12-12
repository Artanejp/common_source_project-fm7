/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

	Author : Takeda.Toshiya
	Date   : 2017.01.13-

	[ memory bus ]
*/

#include "membus.h"
#include "../i8080.h"

void MEMBUS::reset()
{
#if defined(_TK85)
	pc7 = count = 0;
#endif
	MEMORY::reset();
}

uint32_t MEMBUS::fetch_op(uint32_t addr, int *wait)
{
#if defined(_TK80BS) || defined(_TK80)
	if(d_cpu->read_signal(SIG_I8080_INTE) != 0) {
		if(config.dipswitch & 1) {
			d_cpu->write_signal(SIG_I8080_INTR, 1, 1);
		}
	}
#elif defined(_TK85)
	if(pc7 != 0 && ++count == 4) {
		if(config.dipswitch & 1) {
			d_cpu->write_signal(SIG_I8085_RST7, 1, 1);
		}
	}
#endif
	return MEMORY::read_data8w(addr, wait);
}

#if defined(_TK85)
void MEMBUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMBUS_PC7) {
		if((pc7 = data & mask) == 0) {
			count = 0;
		}
	}
}
#endif

#define STATE_VERSION	1

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(_TK85)
	state_fio->StateValue(pc7);
	state_fio->StateValue(count);
#endif
	return MEMORY::process_state(state_fio, loading);
}

