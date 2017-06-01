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

// main
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
			return 1;
		} else {
			// run only one opcode
#ifdef USE_DEBUGGER
			return exec_call_debug();
#else
			return exec_call();
#endif
		}
	} else {
		icount += clock;
		int first_icount = icount;
		
		// run cpu while given clocks
		while(icount > 0 && !busreq) {
#ifdef USE_DEBUGGER
			icount -= exec_call_debug();
#else
			icount -= exec_call();
#endif
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && busreq) {
			icount = 0;
		}
		return first_icount - icount;
	}
}

#define STATE_VERSION	4

void HUC6280::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	//state_fio->Fwrite(opaque, sizeof(h6280_Regs), 1);
	save_state_registers(state_fio);
	state_fio->FputInt32(icount);
	state_fio->FputBool(busreq);
}

bool HUC6280::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	//state_fio->Fread(opaque, sizeof(h6280_Regs), 1);
	load_state_registers(state_fio);
	icount = state_fio->FgetInt32();
	busreq = state_fio->FgetBool();

	// post process   
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
	return true;
}
