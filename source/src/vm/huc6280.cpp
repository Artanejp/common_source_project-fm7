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
#define PAIR pair32_t
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

bool HUC6280::process_state(FILEIO* state_fio, bool loading)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(cpustate->ICount);
	state_fio->StateValue(cpustate->ppc);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->sp);
	state_fio->StateValue(cpustate->zp);
	state_fio->StateValue(cpustate->ea);
	state_fio->StateValue(cpustate->a);
	state_fio->StateValue(cpustate->x);
	state_fio->StateValue(cpustate->y);
	state_fio->StateValue(cpustate->p);
	state_fio->StateArray(cpustate->mmr, sizeof(cpustate->mmr), 1);
	state_fio->StateValue(cpustate->irq_mask);
	state_fio->StateValue(cpustate->timer_status);
	state_fio->StateValue(cpustate->timer_ack);
	state_fio->StateValue(cpustate->clocks_per_cycle);
	state_fio->StateValue(cpustate->timer_value);
	state_fio->StateValue(cpustate->timer_load);
	state_fio->StateValue(cpustate->nmi_state);
	state_fio->StateArray(cpustate->irq_state, sizeof(cpustate->irq_state), 1);
	state_fio->StateValue(cpustate->irq_pending);
#if LAZY_FLAGS
	state_fio->StateValue(cpustate->NZ);
#endif
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_icount);
#endif
	state_fio->StateValue(icount);
	state_fio->StateValue(busreq);
	
#ifdef USE_DEBUGGER
	if(loading) {
		prev_total_icount = total_icount;
	}
#endif
 	return true;
}


