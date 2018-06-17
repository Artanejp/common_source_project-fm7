/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#include "huc6280.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

/* ----------------------------------------------------------------------------
	MAME h6280
---------------------------------------------------------------------------- */

#define INLINE inline
#define PAIR pair_t
#define offs_t UINT16

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define READ8_HANDLER(name) 			UINT8 name(h6280_Regs *cpustate, offs_t offset)
#define WRITE8_HANDLER(name)			void name(h6280_Regs *cpustate, offs_t offset, UINT8 data)

#include "mame/emu/cpu/h6280/h6280.h"
//#include "mame/emu/cpu/h6280/h6280.c"
//#ifdef USE_DEBUGGER
//#include "mame/emu/cpu/h6280/6280dasm.c"
//#endif


void HUC6280::initialize()
{
	HUC6280_BASE::initialize();
	
#ifdef USE_DEBUGGER
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void HUC6280::release()
{
	free(opaque);
}

void HUC6280::reset()
{
	HUC6280_BASE::reset();
#ifdef USE_DEBUGGER
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
}

int HUC6280::run(int clock)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	if(clock == -1) {
		if(busreq) {
			// don't run cpu!
#ifdef USE_DEBUGGER
			total_icount += 1;
#endif
			return 1;
		} else {
			// run only one opcode
			return run_one_opecode();
		}
	} else {
		icount += clock;
		int first_icount = icount;
		
		// run cpu while given clocks
		while(icount > 0 && !busreq) {
			icount -= run_one_opecode();
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && busreq) {
#ifdef USE_DEBUGGER
			total_icount += icount;
#endif
			icount = 0;
		}
		return first_icount - icount;
	}
}

int HUC6280::run_one_opecode()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
#ifdef USE_DEBUGGER
	d_debugger->add_cpu_trace(cpustate->pc.w.l);
#endif
	int passed_icount = HUC6280_BASE::run_one_opecode();
#ifdef USE_DEBUGGER
	total_icount += passed_icount;
#endif
	return passed_icount;
}

#define STATE_VERSION	5

#include "../statesub.h"

void HUC6280::decl_state()
{
	// You should call this after initialize().
	enter_decl_state(STATE_VERSION);
	
	decl_state_registers();
	
#ifdef USE_DEBUGGER
	DECL_STATE_ENTRY_UINT64(total_icount);
#endif
	DECL_STATE_ENTRY_INT32(icount);
	DECL_STATE_ENTRY_BOOL(busreq);

	leave_decl_state();
}
void HUC6280::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	//state_fio->FputUint32(STATE_VERSION);
	//state_fio->FputInt32(this_device_id);

	//save_state_registers(state_fio);
//#ifdef USE_DEBUGGER
	//state_fio->FputUint64(total_icount);
//#endif
	//state_fio->FputInt32(icount);
	//state_fio->FputBool(busreq);
}

bool HUC6280::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
	//if(state_fio->FgetUint32() != STATE_VERSION) {
	//	return false;
	//}
	//if(state_fio->FgetInt32() != this_device_id) {
	//	return false;
	//}
//#ifdef USE_DEBUGGER
	//total_icount = prev_total_icount = state_fio->FgetUint64();
//#endif
	//load_state_registers(state_fio);
	//icount = state_fio->FgetInt32();
	//busreq = state_fio->FgetBool();

	// post process   
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	prev_total_icount = total_icount;
#endif
	return true;
}
