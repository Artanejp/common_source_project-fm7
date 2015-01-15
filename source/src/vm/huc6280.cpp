/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#include "huc6280.h"
#include "../fileio.h"

/* ----------------------------------------------------------------------------
	MAME h6280
---------------------------------------------------------------------------- */

#define INLINE inline
#define PAIR pair
#define offs_t UINT16

enum line_state
{
	CLEAR_LINE = 0,				// clear (a fired or held) line
	ASSERT_LINE,				// assert an interrupt immediately
	HOLD_LINE,				// hold interrupt line until acknowledged
	PULSE_LINE				// pulse interrupt line instantaneously (only for NMI, RESET)
};

enum
{
	INPUT_LINE_IRQ1 = 0,
	INPUT_LINE_IRQ2 = 1,
	INPUT_LINE_TIRQ = 2,
	INPUT_LINE_NMI
};

#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)()

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(h6280_Regs *cpustate)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(h6280_Regs *cpustate, int ICount)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate, icount)

#define READ8_HANDLER(name) 			UINT8 name(h6280_Regs *cpustate, offs_t offset)
#define WRITE8_HANDLER(name)			void name(h6280_Regs *cpustate, offs_t offset, UINT8 data)

#define logerror(...)

#include "mame/emu/cpu/h6280/h6280.c"

// main

void HUC6280::initialize()
{
	opaque = CPU_INIT_CALL(h6280);
	
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;
}

void HUC6280::release()
{
	free(opaque);
}

void HUC6280::reset()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	CPU_RESET_CALL(h6280);
	
	cpustate->program = d_mem;
	cpustate->io = d_io;
}

int HUC6280::run(int icount)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return CPU_EXECUTE_CALL(h6280);
}

void HUC6280::write_signal(int id, uint32 data, uint32 mask)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	set_irq_line(cpustate, id, data);
}

uint32 HUC6280::get_pc()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return cpustate->ppc.w.l;
}

uint8 HUC6280::irq_status_r(uint16 offset)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return h6280_irq_status_r(cpustate, offset);
}

void HUC6280::irq_status_w(uint16 offset, uint8 data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	h6280_irq_status_w(cpustate, offset, data);
}

uint8 HUC6280::timer_r(uint16 offset)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return h6280_timer_r(cpustate, offset);
}

void HUC6280::timer_w(uint16 offset, uint8 data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	h6280_timer_w(cpustate, offset, data);
}

#define STATE_VERSION	1

void HUC6280::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(opaque, sizeof(h6280_Regs), 1);

}

bool HUC6280::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(opaque, sizeof(h6280_Regs), 1);
	
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;
	
	return true;
}
