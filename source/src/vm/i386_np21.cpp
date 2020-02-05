/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date  : 2020.01.25-

	[ i386/i486/Pentium ]
*/

#include "i386_np21.h"
//#ifdef USE_DEBUGGER
#include "debugger.h"
//#endif
#include "i8259.h"
#include "np21/i386c/ia32/cpu.h"
#include "np21/i386c/ia32/instructions/fpu/fp.h"

void I386::initialize()
{
	DEVICE::initialize();
	_I386_PSEUDO_BIOS = osd->check_feature("I386_PSEUDO_BIOS");
	_SINGLE_MODE_DMA  = osd->check_feature("SINGLE_MODE_DMA");
	_USE_DEBUGGER     = osd->check_feature("USE_DEBUGGER");
	
	n_cpu_type = I386_NP21::N_CPU_TYPE_I386DX;
	if(osd->check_feature("HAS_I386")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_I386DX;
	} else if(osd->check_feature("HAS_I386DX")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_I386DX;
	} else if(osd->check_feature("HAS_I386SX")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_I386SX;
	} else if(osd->check_feature("HAS_I486DX")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_I486DX;
	} else if(osd->check_feature("HAS_I486")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_I486DX;
	} else if(osd->check_feature("HAS_I486SX")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_I486SX;
	} else if(osd->check_feature("HAS_PENTIUM")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_PENTIUM;
	} else if(osd->check_feature("HAS_PENTIUM_PRO")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_PENTIUM_PRO;
	} else if(osd->check_feature("HAS_PENTIUM_MMX")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_PENTIUM_MMX;
	} else if(osd->check_feature("HAS_PENTIUM2")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_PENTIUM2;
	} else if(osd->check_feature("HAS_PENTIUM3")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_PENTIUM3;
	} else if(osd->check_feature("HAS_PENTIUM4")) {
		n_cpu_type = I386_NP21::N_CPU_TYPE_PENTIUM4;
	}
	switch(n_cpu_type) {
	case I386_NP21::N_CPU_TYPE_I386DX:
		set_device_name(_T("i80386DX CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_I386SX:
		set_device_name(_T("i80386SX CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_I486DX:
		set_device_name(_T("i486DX CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_I486SX:
		set_device_name(_T("i486SX CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_PENTIUM:
		set_device_name(_T("Pentium CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_PENTIUM_PRO:
		set_device_name(_T("Pentium PRO CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_PENTIUM_MMX:
		set_device_name(_T("Pentium MMX CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_PENTIUM2:
		set_device_name(_T("Pentium2 CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_PENTIUM3:
		set_device_name(_T("Pentium3 CPU"));
		break;
	case I386_NP21::N_CPU_TYPE_PENTIUM4:
		set_device_name(_T("Pentium4 CPU"));
		break;
	default: // ???
		set_device_name(_T("i80386 CPU"));
		break;
	}
	
	realclock = get_cpu_clocks(this);
	device_cpu = this;
//#ifdef USE_DEBUGGER
	if(_USE_DEBUGGER) {
		device_mem_stored = device_mem;
		device_io_stored = device_io;
		device_debugger->set_context_mem(device_mem);
		device_debugger->set_context_io(device_io);
	}
//#endif
	if(!(_I386_PSEUDO_BIOS)) {
		device_bios = NULL;
	}
	if(!(_SINGLE_MODE_DMA)) {
		device_dma = NULL;
	}
	waitfactor = 65536;
	CPU_INITIALIZE();
}

void I386::release()
{
	CPU_DEINITIALIZE();
}

void I386::reset()
{
//#if defined(HAS_I386)
	out_debug_log(_T("RESET"));
	switch(n_cpu_type) {
	default:
	case I386_NP21::N_CPU_TYPE_I386DX:
	case I386_NP21::N_CPU_TYPE_I386SX:
		i386cpuid.cpu_family = CPU_80386_FAMILY;
		i386cpuid.cpu_model = CPU_80386_MODEL;
		i386cpuid.cpu_stepping = CPU_80386_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_80386;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_80386;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_80386;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_80386;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_80386);
		break;
//#elif defined(HAS_I486SX)
	case I386_NP21::N_CPU_TYPE_I486SX:
		i386cpuid.cpu_family = CPU_I486SX_FAMILY;
		i386cpuid.cpu_model = CPU_I486SX_MODEL;
		i386cpuid.cpu_stepping = CPU_I486SX_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_I486SX;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_I486SX;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_I486SX;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_I486SX;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_I486SX);
		break;
//#elif defined(HAS_I486DX)
	case I386_NP21::N_CPU_TYPE_I486DX:
		i386cpuid.cpu_family = CPU_I486DX_FAMILY;
		i386cpuid.cpu_model = CPU_I486DX_MODEL;
		i386cpuid.cpu_stepping = CPU_I486DX_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_I486DX;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_I486DX;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_I486DX;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_I486DX;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_I486DX);
		break;
//#elif defined(HAS_PENTIUM)
	case I386_NP21::N_CPU_TYPE_PENTIUM:
		i386cpuid.cpu_family = CPU_PENTIUM_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM);
		break;
//#elif defined(HAS_PENTIUM_PRO)
	case I386_NP21::N_CPU_TYPE_PENTIUM_PRO:
		i386cpuid.cpu_family = CPU_PENTIUM_PRO_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_PRO_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_PRO_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_PRO;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_PRO;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_PRO;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_PRO;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_PRO);
		break;
//#elif defined(HAS_PENTIUM_MMX)
	case I386_NP21::N_CPU_TYPE_PENTIUM_MMX:
		i386cpuid.cpu_family = CPU_MMX_PENTIUM_FAMILY;
		i386cpuid.cpu_model = CPU_MMX_PENTIUM_MODEL;
		i386cpuid.cpu_stepping = CPU_MMX_PENTIUM_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_MMX_PENTIUM;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_MMX_PENTIUM;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_MMX_PENTIUM;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_MMX_PENTIUM;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_MMX_PENTIUM);
		break;
//#elif defined(HAS_PENTIUM2)
	case I386_NP21::N_CPU_TYPE_PENTIUM2:
		i386cpuid.cpu_family = CPU_PENTIUM_II_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_II_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_II_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_II;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_II;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_II;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_II;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_II);
		break;
//#elif defined(HAS_PENTIUM3)
	case I386_NP21::N_CPU_TYPE_PENTIUM3:

		i386cpuid.cpu_family = CPU_PENTIUM_III_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_III_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_III_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_III;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_III;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_III;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_III;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_III);
		break;
//#elif defined(HAS_PENTIUM4)
	case I386_NP21::N_CPU_TYPE_PENTIUM4:
		i386cpuid.cpu_family = CPU_PENTIUM_4_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_4_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_4_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_4;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_4;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_4;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_4;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_4);
		break;
	}
//#endif
	i386cpuid.fpu_type = FPU_TYPE_SOFTFLOAT;
//	i386cpuid.fpu_type = FPU_TYPE_DOSBOX;
//	i386cpuid.fpu_type = FPU_TYPE_DOSBOX2;
	FPU_INITIALIZE();
	
	CPU_RESET();
	CPU_ADRSMASK = address_mask;
	CPU_TYPE = 0;
	CS_BASE = 0xf0000;
	CPU_CS = CPU_PREV_CS = 0xf000;
	CPU_IP = 0xfff0;
	CPU_CLEARPREFETCH();
	
	remained_cycles = extra_cycles = 0;
	i386_memory_wait = 0;
	waitcount = 0;
	write_signals(&outputs_extreset, 0xffffffff);
}

void I386::cpu_wait(int clocks)
{
	if(clocks <= 0) clocks = 1;
	int64_t wfactor = waitfactor;
	int64_t wcount = waitcount;
	int64_t mwait = i386_memory_wait;
	int64_t ncount;
	if(wfactor > 65536) {
		wcount += ((wfactor - 65536) * clocks); // Append wait due to be slower clock.
	}
	wcount += (wfactor * mwait);  // memory wait
	if(wcount >= 65536) {
		ncount = wcount >> 16;
		wcount = wcount - (ncount << 16);
		extra_cycles += (int)ncount;
	} else if(wcount < 0) {
		wcount = 0;
	}
	waitcount = wcount;
	i386_memory_wait = 0;
}
int I386::run_one_opecode()
{
//#ifdef USE_DEBUGGER
	
	bool now_debugging = false;
	if((_USE_DEBUGGER) && (device_debugger != NULL)) {
		now_debugging = device_debugger->now_debugging;
	}
	if((now_debugging)) {
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
		
		CPU_PREV_CS = CPU_CS;
		CPU_REMCLOCK = 0;
		UINT16 CPU_PREV_isEI = CPU_isEI;
		CPU_EXEC();
		if(!CPU_PREV_isEI && CPU_isEI) device_pic->update_intr();
		
		if(now_debugging) {
			if(!device_debugger->now_going) {
				device_debugger->now_suspended = true;
			}
			device_mem = device_mem_stored;
			device_io = device_io_stored;
		}
		return -CPU_REMCLOCK;
	} else {
//#endif
		CPU_PREV_CS = CPU_CS;
		CPU_REMCLOCK = 0;
		UINT16 CPU_PREV_isEI = CPU_isEI;
		CPU_EXEC();
		if(!CPU_PREV_isEI && CPU_isEI) device_pic->update_intr();
		return -CPU_REMCLOCK;
//#ifdef USE_DEBUGGER
	}
//#endif
}

int I386::run(int cycles)
{
	if(cycles == -1) {
		int passed_cycles;
		if(busreq) {
			// don't run cpu!
//#ifdef SINGLE_MODE_DMA
			if(_SINGLE_MODE_DMA) {
				if(device_dma != NULL) device_dma->do_dma();
			}
//#endif
			passed_cycles = max(1, extra_cycles);
			extra_cycles = 0;
		} else {
			// run only one opcode
			passed_cycles = extra_cycles;
			extra_cycles = 0;
			passed_cycles += run_one_opecode();
		}
//#ifdef USE_DEBUGGER
		if(_USE_DEBUGGER) {
			total_cycles += passed_cycles;
		}
//#endif
		cpu_wait(passed_cycles);
		return passed_cycles;
	} else {
		remained_cycles += cycles;
		int first_cycles = remained_cycles;
		
		// run cpu while given clocks
		while(remained_cycles > 0 && !busreq) {
			remained_cycles -= run(-1);
		}
		// if busreq is raised, spin cpu while remained clock
		if(remained_cycles > 0 && busreq) {
//#ifdef USE_DEBUGGER
			if(_USE_DEBUGGER) {
				total_cycles += remained_cycles;
			}
//#endif
			remained_cycles = 0;
		}
		int passed_cycles = first_cycles - remained_cycles;
		cpu_wait(passed_cycles);
		return passed_cycles;
	}
}

void I386::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		if(data & mask) {
			CPU_INTERRUPT(2, 0);
		}
	} else if(id == SIG_CPU_IRQ) {
		if(data & mask) {
			if(CPU_isEI) {
				CPU_INTERRUPT(device_pic->get_intr_ack(), 0);
			}
		}
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else if(id == SIG_I386_A20) {
		//CPU_ADRSMASK = data & mask;
		ia32a20enable((data & mask) != 0);
	} else if(id == SIG_I386_NOTIFY_RESET) {
		write_signals(&outputs_extreset, (((data & mask) == 0) ? 0x00000000 : 0xffffffff));
	} else if(id == SIG_CPU_WAIT_FACTOR) {
		waitfactor = data; // 65536.
		waitcount = 0; // 65536.
	}
}

void I386::set_intr_line(bool line, bool pending, uint32_t bit)
{
	if(line) {
		if(CPU_isEI) {
			CPU_INTERRUPT(device_pic->get_intr_ack(), 0);
		}
	}
}

void I386::set_extra_clock(int cycles)
{
	extra_cycles += cycles;
}

int I386::get_extra_clock()
{
	return extra_cycles;
}

// from convert_address() in np21/i386c/ia32/disasm.cpp

uint32_t I386::convert_address(uint32_t cs, uint32_t eip)
{
	uint32_t addr = (cs << 4) + eip;
	
	if (CPU_STAT_PM && CPU_STAT_PAGING) {
		uint32_t pde_addr = CPU_STAT_PDE_BASE + ((addr >> 20) & 0xffc);
		uint32_t pde = device_mem->read_data32(pde_addr);
		/* XXX: check */
		uint32_t pte_addr = (pde & CPU_PDE_BASEADDR_MASK) + ((addr >> 10) & 0xffc);
		uint32_t pte = device_mem->read_data32(pte_addr);
		/* XXX: check */
		addr = (pte & CPU_PTE_BASEADDR_MASK) + (addr & 0x00000fff);
	}
	return addr;
}

uint32_t I386::get_pc()
{
	return convert_address(CPU_PREV_CS, CPU_PREV_EIP);
}

uint32_t I386::get_next_pc()
{
	return convert_address(CPU_CS, CPU_EIP);
}

//#ifdef USE_DEBUGGER
void I386::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	device_mem->write_data8w(addr, data, &wait);
}

uint32_t I386::read_debug_data8(uint32_t addr)
{
	int wait;
	return device_mem->read_data8w(addr, &wait);
}

void I386::write_debug_data16(uint32_t addr, uint32_t data)
{
	int wait;
	device_mem->write_data16w(addr, data, &wait);
}

uint32_t I386::read_debug_data16(uint32_t addr)
{
	int wait;
	return device_mem->read_data16w(addr, &wait);
}

void I386::write_debug_data32(uint32_t addr, uint32_t data)
{
	int wait;
	device_mem->write_data32w(addr, data, &wait);
}

uint32_t I386::read_debug_data32(uint32_t addr)
{
	int wait;
	return device_mem->read_data32w(addr, &wait);
}

void I386::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	device_io->write_io8w(addr, data, &wait);
}

uint32_t I386::read_debug_io8(uint32_t addr)
{
	int wait;
	return device_io->read_io8w(addr, &wait);
}

void I386::write_debug_io16(uint32_t addr, uint32_t data)
{
	int wait;
	device_io->write_io16w(addr, data, &wait);
}

uint32_t I386::read_debug_io16(uint32_t addr)
{
	int wait;
	return device_io->read_io16w(addr, &wait);
}

void I386::write_debug_io32(uint32_t addr, uint32_t data)
{
	int wait;
	device_io->write_io32w(addr, data, &wait);
}

uint32_t I386::read_debug_io32(uint32_t addr)
{
	int wait;
	return device_io->read_io32w(addr, &wait);
}

bool I386::write_debug_reg(const _TCHAR *reg, uint32_t data)
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

uint32_t I386::read_debug_reg(const _TCHAR *reg)
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

bool I386::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len,
	_T("AX=%04X  BX=%04X  CX=%04X  DX=%04X  SP=%04X  BP=%04X  SI=%04X  DI=%04X\n")
	_T("DS=%04X  ES=%04X  SS=%04X  CS=%04X  IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]\n")
	_T("EIP=%08X  PC=%08X  PM=%d  VM86=%d\n")
	_T("ADDRESS_MASK=%08X\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	CPU_AX, CPU_BX, CPU_CX, CPU_DX, CPU_SP, CPU_BP, CPU_SI, CPU_DI,
	CPU_DS, CPU_ES, CPU_SS, CPU_CS, CPU_IP,
	(CPU_FLAG & O_FLAG) ? _T('O') : _T('-'), (CPU_FLAG & D_FLAG) ? _T('D') : _T('-'), (CPU_FLAG & I_FLAG) ? _T('I') : _T('-'), (CPU_FLAG & T_FLAG) ? _T('T') : _T('-'),
	(CPU_FLAG & S_FLAG) ? _T('S') : _T('-'), (CPU_FLAG & Z_FLAG) ? _T('Z') : _T('-'), (CPU_FLAG & A_FLAG) ? _T('A') : _T('-'), (CPU_FLAG & P_FLAG) ? _T('P') : _T('-'), (CPU_FLAG & C_FLAG) ? _T('C') : _T('-'),
	CPU_EIP, convert_address(CPU_CS, CPU_EIP), CPU_STAT_PM ? 1 : 0, CPU_STAT_VM86 ? 1 : 0,
    CPU_ADRSMASK,
	total_cycles, total_cycles - prev_total_cycles,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_cycles = total_cycles;
	return true;
}

#define USE_MAME_I386_DASM

#ifdef USE_MAME_I386_DASM
/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			int CPU_DISASSEMBLE_NAME(name)(_TCHAR *buffer, offs_t eip, const UINT8 *oprom)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, eip, oprom)

/*****************************************************************************/
/* src/emu/didisasm.h */

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

/*****************************************************************************/
/* src/osd/osdcomm.h */

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

#include "mame/emu/cpu/i386/i386dasm.c"
#endif

int I386::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
#ifdef USE_MAME_I386_DASM
	UINT64 eip = pc - (CPU_CS << 4);
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = device_mem->read_data8w((pc + i) & CPU_ADRSMASK, &wait);
	}
	UINT8 *oprom = ops;
	
	if (!CPU_STAT_PM || CPU_STAT_VM86) {
		return CPU_DISASSEMBLE_CALL(x86_16) & DASMFLAG_LENGTHMASK;
	} else {
		return CPU_DISASSEMBLE_CALL(x86_32) & DASMFLAG_LENGTHMASK;
	}
#else
	disasm_context_t ctx;
	uint32_t eip = pc - (CPU_CS << 4);
	uint32_t prev_eip = eip;
	
	if(disasm(&eip, &ctx) == 0) {
		my_strcpy_s(buffer, buffer_len, char_to_tchar(ctx.str));
	} else {
		buffer[0] = _T('\0');
	}
	return eip - prev_eip;
#endif
}
//#endif

void I386::set_address_mask(uint32_t mask)
{
	CPU_ADRSMASK = mask;
	address_mask = mask;
}

uint32_t I386::get_address_mask()
{
	return CPU_ADRSMASK;
}

void I386::set_shutdown_flag(int shutdown)
{
	// FIXME: shutdown just now
	if(shutdown) CPU_SHUT();
}

int I386::get_shutdown_flag()
{
	// FIXME: shutdown already done
	return 0;
}

void I386::set_context_mem(DEVICE* device)
{
	device_mem = device;
}

void I386::set_context_io(DEVICE* device)
{
	device_io = device;
}

//#ifdef I386_PSEUDO_BIOS
void I386::set_context_bios(DEVICE* device)
{
	device_bios = device;
}
//#endif

//#ifdef SINGLE_MODE_DMA
void I386::set_context_dma(DEVICE* device)
{
	device_dma = device;
}
//#endif

#define STATE_VERSION	2

bool I386::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// FIXME
	state_fio->StateBuffer(&CPU_STATSAVE, sizeof(CPU_STATSAVE), 1);
	state_fio->StateBuffer(&i386cpuid, sizeof(i386cpuid), 1);
	state_fio->StateBuffer(&i386msr, sizeof(i386msr), 1);
//#ifdef USE_DEBUGGER
	state_fio->StateValue(total_cycles);
	state_fio->StateValue(prev_total_cycles);
//#endif
	state_fio->StateValue(remained_cycles);
	state_fio->StateValue(extra_cycles);
	state_fio->StateValue(busreq);
	state_fio->StateValue(CPU_PREV_CS);
	state_fio->StateValue(waitfactor);
	state_fio->StateValue(waitcount);
	state_fio->StateValue(i386_memory_wait);
	state_fio->StateValue(address_mask);
	
	return true;
}

