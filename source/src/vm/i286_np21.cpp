/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date   : 2020.02.02-

	[ i286 ]
*/

#include "i286_np21.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#include "i386_dasm.h"
#include "v30_dasm.h"
#endif
#include "np21/i286c/cpucore.h"
#include "np21/i286c/v30patch.h"

void I286::initialize()
{
	device_cpu = this;
#ifdef USE_DEBUGGER
	device_mem_stored = device_mem;
	device_io_stored = device_io;
	device_debugger->set_context_mem(device_mem);
	device_debugger->set_context_io(device_io);
#endif
	CPU_INITIALIZE();
	CPU_ADRSMASK = 0x000fffff;
	nmi_pending = irq_pending = false;
}

void I286::release()
{
	CPU_DEINITIALIZE();
}

void I286::reset()
{
	UINT32 PREV_CPU_ADRSMASK = CPU_ADRSMASK;
	CPU_RESET();
	if(device_model == NEC_V30) {
		CPU_TYPE = CPUTYPE_V30;
		v30cinit();
		set_device_name(_T("V30 CPU"));
	} else if(device_model == INTEL_8086) {
		CPU_TYPE = CPUTYPE_I8086;
		i86cinit();
		set_device_name(_T("8086 CPU"));
	} else if(device_model == INTEL_80186) {
		CPU_TYPE = CPUTYPE_I80186;
		i186cinit();
		set_device_name(_T("80186 CPU"));
	} else {
		CPU_TYPE = 0;
		device_model = INTEL_80286;
		set_device_name(_T("80286 CPU"));
	}
	CS_BASE = PREV_CS_BASE = 0xf0000;
	CPU_CS = 0xf000;
	CPU_IP = CPU_PREV_IP = 0xfff0;
	CPU_ADRSMASK = PREV_CPU_ADRSMASK;
	CPU_CLEARPREFETCH();
	
	remained_cycles = extra_cycles = 0;
}

int I286::run_one_opecode()
{
#ifdef USE_DEBUGGER
	bool now_debugging = device_debugger->now_debugging;
	if(now_debugging) {
		device_debugger->check_break_points(get_next_pc());
		if(device_debugger->now_suspended) {
			device_debugger->now_waiting = true;
			emu->start_waiting_in_debugger();
			while(device_debugger->now_debugging && device_debugger->now_suspended) {
				emu->process_waiting_in_debugger();
			}
			emu->finish_waiting_in_debugger();
			device_debugger->now_waiting = false;
		}
		if(device_debugger->now_debugging) {
			device_mem = device_io = device_debugger;
		} else {
			now_debugging = false;
		}
		
		PREV_CS_BASE = CS_BASE;
		CPU_PREV_IP = CPU_IP;
		CPU_REMCLOCK = CPU_BASECLOCK = 1;
		if(CPU_TYPE) {
			CPU_EXECV30();
		} else {
			CPU_EXEC();
		}
		if(nmi_pending) {
			CPU_INTERRUPT(2, 0);
			nmi_pending = false;
		} else if(irq_pending && CPU_isEI) {
			CPU_INTERRUPT(device_pic->get_intr_ack(), 0);
			irq_pending = false;
			device_pic->update_intr();
		}
		if(now_debugging) {
			if(!device_debugger->now_going) {
				device_debugger->now_suspended = true;
			}
			device_mem = device_mem_stored;
			device_io = device_io_stored;
		}
		return CPU_BASECLOCK - CPU_REMCLOCK;
	} else {
#endif
		PREV_CS_BASE = CS_BASE;
		CPU_PREV_IP = CPU_IP;
		CPU_REMCLOCK = CPU_BASECLOCK = 1;
		if(CPU_TYPE) {
			CPU_EXECV30();
		} else {
			CPU_EXEC();
		}
		if(nmi_pending) {
			CPU_INTERRUPT(2, 0);
			nmi_pending = false;
		} else if(irq_pending && CPU_isEI) {
			CPU_INTERRUPT(device_pic->get_intr_ack(), 0);
			irq_pending = false;
			device_pic->update_intr();
		}
		return CPU_BASECLOCK - CPU_REMCLOCK;
#ifdef USE_DEBUGGER
	}
#endif
}

int I286::run(int cycles)
{
	if(cycles == -1) {
		int passed_cycles;
		if(busreq) {
			// don't run cpu!
#ifdef SINGLE_MODE_DMA
			if(device_dma != NULL) device_dma->do_dma();
#endif
			passed_cycles = max(5, extra_cycles); // 80286 CPI: 4.8
			extra_cycles = 0;
		} else {
			// run only one opcode
			passed_cycles = extra_cycles;
			extra_cycles = 0;
			passed_cycles += run_one_opecode();
		}
#ifdef USE_DEBUGGER
		total_cycles += passed_cycles;
#endif
		return passed_cycles;
	} else {
		remained_cycles += cycles + extra_cycles;
		extra_cycles = 0;
		int first_cycles = remained_cycles;
		
		// run cpu while given clocks
		while(remained_cycles > 0 && !busreq) {
			remained_cycles -= run(-1);
		}
		// if busreq is raised, spin cpu while remained clock
		if(remained_cycles > 0 && busreq) {
#ifdef USE_DEBUGGER
			total_cycles += remained_cycles;
#endif
			remained_cycles = 0;
		}
		return first_cycles - remained_cycles;
	}
}

void I286::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		nmi_pending = ((data & mask) != 0);
	} else if(id == SIG_CPU_IRQ) {
		irq_pending = ((data & mask) != 0);
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else if(id == SIG_I286_A20) {
		CPU_ADRSMASK = (data & mask) ? 0x00ffffff : 0x000fffff;
	}
}

void I286::set_intr_line(bool line, bool pending, uint32_t bit)
{
	irq_pending = line;
}

void I286::set_extra_clock(int cycles)
{
	extra_cycles += cycles;
}

int I286::get_extra_clock()
{
	return extra_cycles;
}

uint32_t I286::get_pc()
{
	return PREV_CS_BASE + CPU_PREV_IP;
}

uint32_t I286::get_next_pc()
{
	return CS_BASE + CPU_IP;
}

#ifdef USE_DEBUGGER
void I286::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	device_mem->write_data8w(addr, data, &wait);
}

uint32_t I286::read_debug_data8(uint32_t addr)
{
	int wait;
	return device_mem->read_data8w(addr, &wait);
}

void I286::write_debug_data16(uint32_t addr, uint32_t data)
{
	int wait;
	device_mem->write_data16w(addr, data, &wait);
}

uint32_t I286::read_debug_data16(uint32_t addr)
{
	int wait;
	return device_mem->read_data16w(addr, &wait);
}

void I286::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	device_io->write_io8w(addr, data, &wait);
}

uint32_t I286::read_debug_io8(uint32_t addr)
{
	int wait;
	return device_io->read_io8w(addr, &wait);
}

void I286::write_debug_io16(uint32_t addr, uint32_t data)
{
	int wait;
	device_io->write_io16w(addr, data, &wait);
}

uint32_t I286::read_debug_io16(uint32_t addr)
{
	int wait;
	return device_io->read_io16w(addr, &wait);
}

bool I286::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("IP")) == 0) {
		CPU_IP = data;
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		CPU_AX = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		CPU_BX = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		CPU_CX = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		CPU_DX = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		CPU_SP = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		CPU_BP = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		CPU_SI = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		CPU_DI = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		CPU_AL = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		CPU_AH = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		CPU_BL = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		CPU_BH = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		CPU_CL = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		CPU_CH = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		CPU_DL = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		CPU_DH = data;
	} else {
		return false;
	}
	return true;
}

uint32_t I286::read_debug_reg(const _TCHAR *reg)
{
	if(_tcsicmp(reg, _T("IP")) == 0) {
		return CPU_IP;
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		return CPU_AX;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		return CPU_BX;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		return CPU_CX;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		return CPU_DX;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		return CPU_SP;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		return CPU_BP;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		return CPU_SI;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		return CPU_DI;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		return CPU_AL;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		return CPU_AH;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		return CPU_BL;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		return CPU_BH;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		return CPU_CL;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		return CPU_CH;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		return CPU_DL;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		return CPU_DH;
	}
	return 0;
}

bool I286::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len,
	_T("AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n")
	_T("DS=%04X  ES=%04X  SS=%04X  CS=%04X  IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	CPU_AX, CPU_BX, CPU_CX, CPU_DX, CPU_SP, CPU_BP, CPU_SI, CPU_DI,
	CPU_DS, CPU_ES, CPU_SS, CPU_CS, CPU_IP,
	(CPU_FLAG & O_FLAG) ? _T('O') : _T('-'), (CPU_FLAG & D_FLAG) ? _T('D') : _T('-'), (CPU_FLAG & I_FLAG) ? _T('I') : _T('-'), (CPU_FLAG & T_FLAG) ? _T('T') : _T('-'), (CPU_FLAG & S_FLAG) ? _T('S') : _T('-'),
	(CPU_FLAG & Z_FLAG) ? _T('Z') : _T('-'), (CPU_FLAG & A_FLAG) ? _T('A') : _T('-'), (CPU_FLAG & P_FLAG) ? _T('P') : _T('-'), (CPU_FLAG & C_FLAG) ? _T('C') : _T('-'),
	total_cycles, total_cycles - prev_total_cycles,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_cycles = total_cycles;
	return true;
}

int I286::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint32_t eip = pc - (CPU_CS << 4);
	uint8_t oprom[16];
	
	for(int i = 0; i < 16; i++) {
		int wait;
		oprom[i] = device_mem->read_data8w((pc + i) & CPU_ADRSMASK, &wait);
	}
	if(device_model == NEC_V30) {
		return v30_dasm(oprom, eip, buffer, buffer_len);
	} else {
		return i386_dasm(oprom, eip, false, buffer, buffer_len);
	}
}
#endif

void I286::set_address_mask(uint32_t mask)
{
	CPU_ADRSMASK = mask;
}

uint32_t I286::get_address_mask()
{
	return CPU_ADRSMASK;
}

void I286::set_shutdown_flag(int shutdown)
{
	// FIXME: shutdown just now
	if(shutdown) CPU_SHUT();
}

int I286::get_shutdown_flag()
{
	// FIXME: shutdown already done
	return 0;
}

void I286::set_context_mem(DEVICE* device)
{
	device_mem = device;
}

void I286::set_context_io(DEVICE* device)
{
	device_io = device;
}

#ifdef I86_PSEUDO_BIOS
void I286::set_context_bios(DEVICE* device)
{
	device_bios = device;
}
#endif

#ifdef SINGLE_MODE_DMA
void I286::set_context_dma(DEVICE* device)
{
	device_dma = device;
}
#endif

#ifdef USE_DEBUGGER
void I286::set_context_debugger(DEBUGGER* device)
{
	device_debugger = device;
}

void *I286::get_debugger()
{
	return device_debugger;
}
#endif

#define STATE_VERSION	1

bool I286::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// FIXME
	state_fio->StateBuffer(&CPU_STATSAVE, sizeof(CPU_STATSAVE), 1);
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_cycles);
	state_fio->StateValue(prev_total_cycles);
#endif
	state_fio->StateValue(remained_cycles);
	state_fio->StateValue(extra_cycles);
	state_fio->StateValue(busreq);
	state_fio->StateValue(nmi_pending);
	state_fio->StateValue(irq_pending);
	state_fio->StateValue(PREV_CS_BASE);
	state_fio->StateValue(CPU_PREV_IP);
	return true;
}

