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
	return MEMORY::fetch_op(addr, wait);
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

#include "../../statesub.h"

void MEMBUS::decl_state()
{
	enter_decl_state(STATE_VERSION);

#if defined(_TK85)
	DECL_STATE_ENTRY_UINT32(pc7);
	DECL_STATE_ENTRY_UINT32(count);
#endif
	MEMORY::decl_state();
	
	leave_decl_state();
}

void MEMBUS::save_state(FILEIO* state_fio)
{
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//#if defined(_TK85)
//	state_fio->FputUint32(pc7);
//	state_fio->FputUint32(count);
//#endif
	MEMORY::save_state(state_fio);
}

bool MEMBUS::load_state(FILEIO* state_fio)
{
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//#if defined(_TK85)
//	pc7 = state_fio->FgetUint32();
//	count = state_fio->FgetUint32();
//#endif
	return MEMORY::load_state(state_fio);
}
bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(_TK85)
	state_fio->StateUint32(pc7);
	state_fio->StateUint32(count);
#endif
	return MEMORY::process_state(state_fio, loading);
}
