/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date   : 2020.01.25-

	[ i386/i486/Pentium ]
*/

#include "i386_np21.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#include "i386_dasm.h"
#endif
#include "np21/i386c/cpucore.h"
#include "np21/i386c/ia32/instructions/fpu/fp.h"

void I386::initialize()
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
	realclock = get_cpu_clocks(this);
	nmi_pending = irq_pending = false;
}

void I386::release()
{
	CPU_DEINITIALIZE();
}

void I386::reset()
{
	switch(device_model) {
	case INTEL_80386:
		i386cpuid.cpu_family = CPU_80386_FAMILY;
		i386cpuid.cpu_model = CPU_80386_MODEL;
		i386cpuid.cpu_stepping = CPU_80386_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_80386;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_80386;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_80386;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_80386;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_80386;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_80386);
		set_device_name(_T("80386 CPU"));
		break;
	case INTEL_I486SX:
		i386cpuid.cpu_family = CPU_I486SX_FAMILY;
		i386cpuid.cpu_model = CPU_I486SX_MODEL;
		i386cpuid.cpu_stepping = CPU_I486SX_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_I486SX;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_I486SX;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_I486SX;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_I486SX;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_I486SX;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_I486SX);
		set_device_name(_T("80486SX CPU"));
		break;
	case INTEL_I486DX:
		i386cpuid.cpu_family = CPU_I486DX_FAMILY;
		i386cpuid.cpu_model = CPU_I486DX_MODEL;
		i386cpuid.cpu_stepping = CPU_I486DX_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_I486DX;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_I486DX;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_I486DX;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_I486DX;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_I486DX;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_I486DX);
		set_device_name(_T("80486DX CPU"));
		break;
	case INTEL_PENTIUM:
		i386cpuid.cpu_family = CPU_PENTIUM_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_PENTIUM;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM);
		set_device_name(_T("Pentium CPU"));
		break;
	case INTEL_MMX_PENTIUM:
		i386cpuid.cpu_family = CPU_MMX_PENTIUM_FAMILY;
		i386cpuid.cpu_model = CPU_MMX_PENTIUM_MODEL;
		i386cpuid.cpu_stepping = CPU_MMX_PENTIUM_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_MMX_PENTIUM;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_MMX_PENTIUM;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_MMX_PENTIUM;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_MMX_PENTIUM;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_MMX_PENTIUM;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_MMX_PENTIUM);
		set_device_name(_T("MMX Pentium CPU"));
		break;
	case INTEL_PENTIUM_PRO:
		i386cpuid.cpu_family = CPU_PENTIUM_PRO_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_PRO_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_PRO_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_PRO;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_PRO;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_PRO;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_PENTIUM_PRO;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_PRO;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_PRO);
		set_device_name(_T("Pentium Pro CPU"));
		break;
	case INTEL_PENTIUM_II:
		i386cpuid.cpu_family = CPU_PENTIUM_II_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_II_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_II_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_II;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_II;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_II;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_PENTIUM_II;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_II;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_II);
		set_device_name(_T("Pentium II CPU"));
		break;
	case INTEL_PENTIUM_III:
		i386cpuid.cpu_family = CPU_PENTIUM_III_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_III_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_III_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_III;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_III;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_III;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_PENTIUM_III;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_III;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_III);
		set_device_name(_T("Pentium III CPU"));
		break;
	case INTEL_PENTIUM_M:
		i386cpuid.cpu_family = CPU_PENTIUM_M_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_M_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_M_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_M;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_M;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_M;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_PENTIUM_M;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_M;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_M);
		set_device_name(_T("Pentium M CPU"));
		break;
	case INTEL_PENTIUM_4:
		i386cpuid.cpu_family = CPU_PENTIUM_4_FAMILY;
		i386cpuid.cpu_model = CPU_PENTIUM_4_MODEL;
		i386cpuid.cpu_stepping = CPU_PENTIUM_4_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_PENTIUM_4;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_PENTIUM_4;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_PENTIUM_4;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_PENTIUM_4;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_PENTIUM_4;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_INTEL);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_PENTIUM_4);
		set_device_name(_T("Pentium 4 CPU"));
		break;
	case AMD_K6_2:
		i386cpuid.cpu_family = CPU_AMD_K6_2_FAMILY;
		i386cpuid.cpu_model = CPU_AMD_K6_2_MODEL;
		i386cpuid.cpu_stepping = CPU_AMD_K6_2_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_AMD_K6_2;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_AMD_K6_2;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_AMD_K6_2;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_AMD_K6_2;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_AMD_K6_2;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_AMD);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_AMD_K6_2);
		set_device_name(_T("AMD-K6 3D CPU"));
		break;
	case AMD_K6_III:
		i386cpuid.cpu_family = CPU_AMD_K6_III_FAMILY;
		i386cpuid.cpu_model = CPU_AMD_K6_III_MODEL;
		i386cpuid.cpu_stepping = CPU_AMD_K6_III_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_AMD_K6_III;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_AMD_K6_III;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_AMD_K6_III;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_AMD_K6_III;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_AMD_K6_III;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_AMD);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_AMD_K6_III);
		set_device_name(_T("AMD-K6 3D+ CPU"));
		break;
	case AMD_K7_ATHLON:
		i386cpuid.cpu_family = CPU_AMD_K7_ATHLON_FAMILY;
		i386cpuid.cpu_model = CPU_AMD_K7_ATHLON_MODEL;
		i386cpuid.cpu_stepping = CPU_AMD_K7_ATHLON_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_AMD_K7_ATHLON;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_AMD_K7_ATHLON;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_AMD_K7_ATHLON;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_AMD_K7_ATHLON;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_AMD_K7_ATHLON;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_AMD);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_AMD_K7_ATHLON);
		set_device_name(_T("AMD-K7 CPU"));
		break;
	case AMD_K7_ATHLON_XP:
		i386cpuid.cpu_family = CPU_AMD_K7_ATHLON_XP_FAMILY;
		i386cpuid.cpu_model = CPU_AMD_K7_ATHLON_XP_MODEL;
		i386cpuid.cpu_stepping = CPU_AMD_K7_ATHLON_XP_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_AMD_K7_ATHLON_XP;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_AMD_K7_ATHLON_XP;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_AMD_K7_ATHLON_XP;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK_AMD_K7_ATHLON_XP;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_AMD_K7_ATHLON_XP;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_AMD);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_AMD_K7_ATHLON_XP);
		set_device_name(_T("AMD Athlon XP CPU"));
		break;
	default:
		i386cpuid.cpu_family = CPU_FAMILY;
		i386cpuid.cpu_model = CPU_MODEL;
		i386cpuid.cpu_stepping = CPU_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_ALL;
		i386cpuid.cpu_feature_ex = CPU_FEATURES_EX_ALL;
		i386cpuid.cpu_feature_ecx = CPU_FEATURES_ECX_ALL;
		i386cpuid.cpu_eflags_mask = CPU_EFLAGS_MASK;
		i386cpuid.cpu_brandid = CPU_BRAND_ID_NEKOPRO2;
		strcpy(i386cpuid.cpu_vendor, CPU_VENDOR_NEKOPRO);
		strcpy(i386cpuid.cpu_brandstring, CPU_BRAND_STRING_NEKOPRO2);
		set_device_name(_T("Neko Processor II CPU"));
		break;
	}
//	i386cpuid.fpu_type = FPU_TYPE_SOFTFLOAT;
//	i386cpuid.fpu_type = FPU_TYPE_DOSBOX;
	i386cpuid.fpu_type = FPU_TYPE_DOSBOX2;
	fpu_initialize();
	
	UINT32 PREV_CPU_ADRSMASK = CPU_ADRSMASK;
	CPU_RESET();
	CPU_TYPE = 0;
	CS_BASE = PREV_CS_BASE = 0xf0000;
	CPU_CS = 0xf000;
	CPU_IP = 0xfff0;
	CPU_ADRSMASK = PREV_CPU_ADRSMASK;
	CPU_CLEARPREFETCH();
	
	remained_cycles = extra_cycles = 0;
}

int I386::run_one_opecode()
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
		CPU_REMCLOCK = CPU_BASECLOCK = 1;
		CPU_EXEC();
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
		CPU_REMCLOCK = CPU_BASECLOCK = 1;
		CPU_EXEC();
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

int I386::run(int cycles)
{
	if(cycles == -1) {
		int passed_cycles;
		if(busreq) {
			// don't run cpu!
#ifdef SINGLE_MODE_DMA
			if(device_dma != NULL) device_dma->do_dma();
#endif
			passed_cycles = max(5, extra_cycles); // 80386 CPI: 4.9
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

void I386::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		nmi_pending = ((data & mask) != 0);
	} else if(id == SIG_CPU_IRQ) {
		irq_pending = ((data & mask) != 0);
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else if(id == SIG_I386_A20) {
		CPU_ADRSMASK = (data & mask) ? ~0 : ~(1 << 20);
	}
}

void I386::set_intr_line(bool line, bool pending, uint32_t bit)
{
	irq_pending = line;
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

uint32_t I386::convert_address(uint32_t cs_base, uint32_t eip)
{
	if(CPU_STAT_PM) {
		uint32_t addr = cs_base + eip;
		
		if(CPU_STAT_PAGING) {
			uint32_t pde_addr = CPU_STAT_PDE_BASE + ((addr >> 20) & 0xffc);
			uint32_t pde = device_mem->read_data32(pde_addr);
			/* XXX: check */
			uint32_t pte_addr = (pde & CPU_PDE_BASEADDR_MASK) + ((addr >> 10) & 0xffc);
			uint32_t pte = device_mem->read_data32(pte_addr);
			/* XXX: check */
			addr = (pte & CPU_PTE_BASEADDR_MASK) + (addr & CPU_PAGE_MASK);
		}
		return addr;
	}
	return cs_base + (eip & 0xffff);
}

uint32_t I386::get_pc()
{
	return convert_address(PREV_CS_BASE, CPU_PREV_EIP);
}

uint32_t I386::get_next_pc()
{
	return convert_address(CS_BASE, CPU_EIP);
}

#ifdef USE_DEBUGGER
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
	if(_tcsicmp(reg, _T("EIP")) == 0) {
		CPU_EIP = data;
	} else if(_tcsicmp(reg, _T("EAX")) == 0) {
		CPU_EAX = data;
	} else if(_tcsicmp(reg, _T("EBX")) == 0) {
		CPU_EBX = data;
	} else if(_tcsicmp(reg, _T("ECX")) == 0) {
		CPU_ECX = data;
	} else if(_tcsicmp(reg, _T("EDX")) == 0) {
		CPU_EDX = data;
	} else if(_tcsicmp(reg, _T("ESP")) == 0) {
		CPU_ESP = data;
	} else if(_tcsicmp(reg, _T("EBP")) == 0) {
		CPU_EBP = data;
	} else if(_tcsicmp(reg, _T("ESI")) == 0) {
		CPU_ESI = data;
	} else if(_tcsicmp(reg, _T("EDI")) == 0) {
		CPU_EDI = data;
	} else if(_tcsicmp(reg, _T("IP")) == 0) {
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
	if(_tcsicmp(reg, _T("EIP")) == 0) {
		return CPU_EIP;
	} else if(_tcsicmp(reg, _T("EAX")) == 0) {
		return CPU_EAX;
	} else if(_tcsicmp(reg, _T("EBX")) == 0) {
		return CPU_EBX;
	} else if(_tcsicmp(reg, _T("ECX")) == 0) {
		return CPU_ECX;
	} else if(_tcsicmp(reg, _T("EDX")) == 0) {
		return CPU_EDX;
	} else if(_tcsicmp(reg, _T("ESP")) == 0) {
		return CPU_ESP;
	} else if(_tcsicmp(reg, _T("EBP")) == 0) {
		return CPU_EBP;
	} else if(_tcsicmp(reg, _T("ESI")) == 0) {
		return CPU_ESI;
	} else if(_tcsicmp(reg, _T("EDI")) == 0) {
		return CPU_EDI;
	} else if(_tcsicmp(reg, _T("IP")) == 0) {
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
	if(CPU_STAT_PM) {
		my_stprintf_s(buffer, buffer_len,
		_T("EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X\n")
		_T("ESP=%08X  EBP=%08X  ESI=%08X  EDI=%08X\n")
		_T("DS=%04X  ES=%04X  SS=%04X  CS=%04X  EIP=%08X  FLAG=[%c%c%c%c%c%c%c%c%c%s]\n")
		_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
		CPU_EAX, CPU_EBX, CPU_ECX, CPU_EDX, CPU_ESP, CPU_EBP, CPU_ESI, CPU_EDI,
		CPU_DS, CPU_ES, CPU_SS, CPU_CS, CPU_EIP,
		(CPU_FLAG & O_FLAG) ? _T('O') : _T('-'), (CPU_FLAG & D_FLAG) ? _T('D') : _T('-'), (CPU_FLAG & I_FLAG) ? _T('I') : _T('-'), (CPU_FLAG & T_FLAG) ? _T('T') : _T('-'), (CPU_FLAG & S_FLAG) ? _T('S') : _T('-'),
		(CPU_FLAG & Z_FLAG) ? _T('Z') : _T('-'), (CPU_FLAG & A_FLAG) ? _T('A') : _T('-'), (CPU_FLAG & P_FLAG) ? _T('P') : _T('-'), (CPU_FLAG & C_FLAG) ? _T('C') : _T('-'), (CPU_STAT_VM86) ? _T(":VM86") : _T(""),
		total_cycles, total_cycles - prev_total_cycles,
		get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	} else {
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
	}
	prev_total_cycles = total_cycles;
	return true;
}

int I386::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint32_t eip = pc - (CPU_CS << 4);
	uint8_t oprom[16];
	
	for(int i = 0; i < 16; i++) {
		int wait;
		oprom[i] = device_mem->read_data8w((pc + i) & CPU_ADRSMASK, &wait);
	}
	if(CPU_INST_OP32) {
		return i386_dasm(oprom, eip, true,  buffer, buffer_len);
	} else {
		return i386_dasm(oprom, eip, false, buffer, buffer_len);
	}
}
#endif

void I386::set_address_mask(uint32_t mask)
{
	CPU_ADRSMASK = mask;
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

#ifdef I86_PSEUDO_BIOS
void I386::set_context_bios(DEVICE* device)
{
	device_bios = device;
}
#endif

#ifdef SINGLE_MODE_DMA
void I386::set_context_dma(DEVICE* device)
{
	device_dma = device;
}
#endif

#ifdef USE_DEBUGGER
void I386::set_context_debugger(DEBUGGER* device)
{
	device_debugger = device;
}

void *I386::get_debugger()
{
	return device_debugger;
}
#endif

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
	return true;
}

