/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date   : 2020.01.25-

	[ i386/i486/Pentium ]
*/

#include "i386_np21.h"
//#ifdef USE_DEBUGGER
#include "debugger.h"
#include "i386_dasm.h"
//#endif
#include "np21/i386c/cpucore.h"
#include "np21/i386c/ia32/instructions/fpu/fp.h"

void I386::initialize()
{
	DEVICE::initialize();
	_I86_PSEUDO_BIOS = osd->check_feature("I86_PSEUDO_BIOS");
	_SINGLE_MODE_DMA  = osd->check_feature("SINGLE_MODE_DMA");
//	realclock = get_cpu_clocks(this);
	device_cpu = this;

//#ifdef USE_DEBUGGER
	if(__USE_DEBUGGER) {
		device_mem_stored = device_mem;
		device_io_stored = device_io;
		device_debugger->set_context_mem(device_mem);
		device_debugger->set_context_io(device_io);
	} else {
		device_debugger = NULL;
	}
//#endif
	if(!(_I86_PSEUDO_BIOS)) {
		device_bios = NULL;
	}
	if(!(_SINGLE_MODE_DMA)) {
		device_dma = NULL;
	}
	waitfactor = 65536;
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
//#if defined(HAS_I386)
	out_debug_log(_T("RESET"));
	switch(device_model) {
	case INTEL_80386:
		i386cpuid.cpu_family = CPU_80386_FAMILY;
		i386cpuid.cpu_model = CPU_80386_MODEL;
		i386cpuid.cpu_stepping = CPU_80386_STEPPING;
		i386cpuid.cpu_feature = CPU_FEATURES_80386;
//		i386cpuid.cpu_feature = CPU_FEATURES_80386 | CPU_FEATURE_FPU; // 20200501 TMP K.O
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
//		i386cpuid.cpu_feature = CPU_FEATURES_I486SX;
		i386cpuid.cpu_feature = CPU_FEATURES_I486SX | CPU_FEATURE_FPU;
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
	osd->set_vm_node(this_device_id, (char *)this_device_name);

//	i386cpuid.fpu_type = FPU_TYPE_SOFTFLOAT;
//	i386cpuid.fpu_type = FPU_TYPE_DOSBOX;
	i386cpuid.fpu_type = FPU_TYPE_DOSBOX2;
	fpu_initialize();

	UINT32 PREV_CPU_ADRSMASK = CPU_ADRSMASK;

	realclock = get_cpu_clocks(this);

	CPU_RESET();
//	CPU_ADRSMASK = address_mask;
	CPU_TYPE = 0;
	CS_BASE = PREV_CS_BASE = 0xf0000;
	CPU_CS = 0xf000;
	CPU_IP = 0xfff0;
	CPU_ADRSMASK = PREV_CPU_ADRSMASK;
	CPU_CLEARPREFETCH();

	remained_cycles = extra_cycles = 0;
	waitcount = 0;
	write_signals(&outputs_extreset, 0xffffffff);
}

bool I386::check_interrupts()
{
	__UNLIKELY_IF(nmi_pending) {
		return true;
	} else __LIKELY_IF(irq_pending && CPU_isEI) {
		return true;
	}
	return false;
}

int I386::run_one_opecode()
{
//#ifdef USE_DEBUGGER

	bool now_debugging = false;
	__LIKELY_IF((__USE_DEBUGGER) && (device_debugger != NULL)) {
		now_debugging = device_debugger->now_debugging;
	}

	__UNLIKELY_IF(now_debugging) {
		device_debugger->check_break_points(get_next_pc());
		__UNLIKELY_IF(device_debugger->now_suspended) {
			device_debugger->now_waiting = true;
			emu->start_waiting_in_debugger();
			while(device_debugger->now_debugging && device_debugger->now_suspended) {
				emu->process_waiting_in_debugger();
			}
			emu->finish_waiting_in_debugger();
			device_debugger->now_waiting = false;
		}
		__LIKELY_IF(device_debugger->now_debugging) {
			device_mem = device_io = device_debugger;
		} else {
			now_debugging = false;
		}
	}
	{
//#endif
		PREV_CS_BASE = CS_BASE;
		CPU_REMCLOCK = CPU_BASECLOCK = 1;
		CPU_EXEC();
		__UNLIKELY_IF(nmi_pending) {
			CPU_INTERRUPT(2, 0);
			__LIKELY_IF(device_debugger != NULL) {
				device_debugger->add_cpu_trace_irq(get_pc(), 2);
			}
			nmi_pending = false;
		} else __LIKELY_IF(irq_pending && CPU_isEI) {
			// ToDo: Multiple interrupt within rep prefix.
			uint32_t intr_level = device_pic->get_intr_ack();
			CPU_INTERRUPT(intr_level, 0);
			__LIKELY_IF(device_debugger != NULL) {
				device_debugger->add_cpu_trace_irq(get_pc(), intr_level);
			}
			irq_pending = false;
			device_pic->update_intr();
		}
		//check_interrupts();  // OK?This may be enable only debugging;
		__UNLIKELY_IF(now_debugging) {
			__UNLIKELY_IF(!device_debugger->now_going) {
				device_debugger->now_suspended = true;
			}
			device_mem = device_mem_stored;
			device_io = device_io_stored;
		}
		return CPU_BASECLOCK - CPU_REMCLOCK;
//#ifdef USE_DEBUGGER
	}
//#endif
}

int I386::run(int cycles)
{
	// Prefer to run as one step (cycles == -1). 20220210 K.O
	__LIKELY_IF(cycles == -1) {
		if(_SINGLE_MODE_DMA) {
			__LIKELY_IF(device_dma != NULL) {
				device_dma->do_dma();
			}
		}
		int __cycles;
		__UNLIKELY_IF(busreq) {
			// don't run cpu!
			// 80386 CPI: 4.9, minimum clocks may be 5.
			__cycles = 5;
		} else {
			// run only one opcode
			__cycles = run_one_opecode();
			__UNLIKELY_IF(__cycles < 1) {
				__cycles = 1;
			}
		}
		int tmp_extra_cycles = 0;
		__UNLIKELY_IF(extra_cycles > 0) {
			tmp_extra_cycles = extra_cycles;
			extra_cycles = 0;
		}
		__UNLIKELY_IF(__USE_DEBUGGER) {
			total_cycles += __cycles;
		}
//		int64_t _dummy = __cycles;
		cpu_wait(__cycles); // OK?
		//cpu_wait(__cycles, CPU_CLOCKCNT); // OK?
		return __cycles + tmp_extra_cycles;
//#ifdef USE_DEBUGGER
//#endif
	} else {
		// Secondary
		remained_cycles += cycles;
		int tmp_extra_cycles = 0;
		__UNLIKELY_IF(extra_cycles > 0) {
			tmp_extra_cycles = extra_cycles;
			extra_cycles = 0;
		}
		int first_cycles = remained_cycles;

		// run cpu while given clocks
		__LIKELY_IF(!(busreq)) {
			while(remained_cycles > 0 && !busreq) {
				// ToDo: for _SINGLE_MODE_DMA 20230305 K.O
				remained_cycles -= run_one_opecode();
				//			remained_cycles -= run(-1);
				__UNLIKELY_IF(busreq) break;
			}
		}
		// if busreq is raised, spin cpu while remained clock
		__UNLIKELY_IF(remained_cycles > 0 && busreq) {
//#ifdef USE_DEBUGGER
			__LIKELY_IF(__USE_DEBUGGER) {
				total_cycles += remained_cycles;
			}
//#endif
			remained_cycles = 0;
		}
		int passed_cycles = first_cycles - remained_cycles;
		//cpu_wait(passed_cycles, CPU_CLOCKCNT);
		return passed_cycles + tmp_extra_cycles;
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
	irq_pending = line;
}

void I386::set_extra_clock(int cycles)
{
	extra_cycles += cycles;
	cpu_wait(cycles);
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
	} else if(_tcsicmp(reg, _T("ES")) == 0) {
		return CPU_REGS_SREG(CPU_ES_INDEX);
	} else if(_tcsicmp(reg, _T("CS")) == 0) {
		return CPU_REGS_SREG(CPU_CS_INDEX);
	} else if(_tcsicmp(reg, _T("SS")) == 0) {
		return CPU_REGS_SREG(CPU_SS_INDEX);
	} else if(_tcsicmp(reg, _T("DS")) == 0) {
		return CPU_REGS_SREG(CPU_DS_INDEX);
	} else if(_tcsicmp(reg, _T("FS")) == 0) {
		return CPU_REGS_SREG(CPU_FS_INDEX);
	} else if(_tcsicmp(reg, _T("GS")) == 0) {
		return CPU_REGS_SREG(CPU_GS_INDEX);
	} else if(_tcsicmp(reg, _T("ES.LIMIT")) == 0) {
		return CPU_STAT_SREGLIMIT(CPU_ES_INDEX);
	} else if(_tcsicmp(reg, _T("CS.LIMIT")) == 0) {
		return CPU_STAT_SREGLIMIT(CPU_CS_INDEX);
	} else if(_tcsicmp(reg, _T("SS.LIMIT")) == 0) {
		return CPU_STAT_SREGLIMIT(CPU_SS_INDEX);
	} else if(_tcsicmp(reg, _T("DS.LIMIT")) == 0) {
		return CPU_STAT_SREGLIMIT(CPU_DS_INDEX);
	} else if(_tcsicmp(reg, _T("FS.LIMIT")) == 0) {
		return CPU_STAT_SREGLIMIT(CPU_FS_INDEX);
	} else if(_tcsicmp(reg, _T("GS.LIMIT")) == 0) {
		return CPU_STAT_SREGLIMIT(CPU_GS_INDEX);
	} else if(_tcsicmp(reg, _T("ES.BASE")) == 0) {
		return CPU_STAT_SREGBASE(CPU_ES_INDEX);
	} else if(_tcsicmp(reg, _T("CS.BASE")) == 0) {
		return CPU_STAT_SREGBASE(CPU_CS_INDEX);
	} else if(_tcsicmp(reg, _T("SS.BASE")) == 0) {
		return CPU_STAT_SREGBASE(CPU_SS_INDEX);
	} else if(_tcsicmp(reg, _T("DS.BASE")) == 0) {
		return CPU_STAT_SREGBASE(CPU_DS_INDEX);
	} else if(_tcsicmp(reg, _T("FS.BASE")) == 0) {
		return CPU_STAT_SREGBASE(CPU_FS_INDEX);
	} else if(_tcsicmp(reg, _T("GS.BASE")) == 0) {
		return CPU_STAT_SREGBASE(CPU_GS_INDEX);
	}
	return 0;
}

bool I386::get_debug_regs_description(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len,
				  _T("(E)IP       : Instruction pointer\n")
				  _T("(E)FLAGS    : FLAGs\n")
				  _T("CS          : Code  SEGMENT\n")
				  _T("SS          : Stack SEGMENT\n")
				  _T("DS ES FS GS : Data  SEGMENT\n")
				  _T("(E)SP       : Stack pointer\n")
				  _T("(E)BP       : Base pointer (sometimes using local stack pointer)\n")
				  _T("(E)SI       : SOURCE INDEX\n")
				  _T("(E)DI       : DESTINATION INDEX\n")
				  _T("EAX EBX ECX EDX         : 32bit ACCUMERATORS\n")
				  _T("AX  BX  CX  DX          : 16bit ACCUMERATORS\n")
				  _T("AH AL BH BL CH CL DH DL : 8bit  ACCUMERATORS\n")
				  _T("*** ExX is same as xX, split into xH and xL\n")
				  _T("LDTR        : LOCAL     SEGMENT ADDRESS TABLE\n")
				  _T("GDTR        : GLOBAL    SEGMENT ADDRESS TABLE\n")
				  _T("IDTR        : INTERRUPT SEGMENT ADDRESS TABLE\n")
				  _T("TRx         : TEST  REGISTERs\n")
				  _T("DRx         : DEBUG REGISTERs\n")
				  _T("CR0-CR4     : SYSTEM REGISTERs\n")
				  _T("MXCSR       : \n")
				  _T("\n")
		);
	return true;
}
bool I386::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	const _TCHAR sregname[][8] =
		{ _T("ES"), _T("CS"),
		_T("SS"), _T("DS"),
		_T("FS"), _T("GS")}
		;
	_TCHAR sregstr[512] = {0};
	_TCHAR dbgregstr[512] = {0};
	_TCHAR testregstr[512] = {0};
#if 1
	for(int i = 0; i < 6; i++) {
		_TCHAR segdesc[128] = {0};
		my_stprintf_s(segdesc, 127, _T("%s: %04X BASE=%08X LIMIT=%08X (PAGED BASE=%08X) \n"),
					  sregname[i],
					  CPU_REGS_SREG(i),
					  (CPU_STAT_PM) ? CPU_STAT_SREGBASE(i) : (CPU_STAT_SREGBASE(i) & 0xfffff),
					  (CPU_STAT_PM) ? CPU_STAT_SREGLIMIT(i) : (CPU_STAT_SREGLIMIT(i) & 0xffff),
					  convert_address(CPU_STAT_SREGBASE(i), 0)
			);
		my_tcscat_s(sregstr, 511, segdesc);
	}
	my_stprintf_s(dbgregstr, 511, _T("DEBUG REG:"));
	for(int i = 0; i < CPU_DEBUG_REG_NUM; i++) {
		_TCHAR ddesc[128] = {0};
		my_stprintf_s(ddesc, 31, _T(" %08X"), CPU_DR(i));
		my_tcscat_s(dbgregstr, 511, ddesc);
	}
	my_stprintf_s(testregstr, 511, _T("TEST  REG:"));
	for(int i = 0; i < CPU_TEST_REG_NUM; i++) {
		_TCHAR tdesc[32] = {0};
		my_stprintf_s(tdesc, 31, _T(" %08X"), CPU_STATSAVE.cpu_regs.tr[i]);
		my_tcscat_s(testregstr, 511, tdesc);
	}

	if(CPU_STAT_PM) {
		// ToDo: Dump/Convert PDE/PTE TABLE.
		_TCHAR pde_desc[256] = {0};
		if(CPU_STAT_PAGING) {
			my_stprintf_s(pde_desc, sizeof(pde_desc) -1,
						  _T("PAGING TABLE: PDE BASE=%08X\n"),
						  CPU_STAT_PDE_BASE);
		}
		my_stprintf_s(buffer, buffer_len,
		_T("PM %s %s %s %s MODE=%01X CPL=%02X\n")
		_T("EFLAGS=%08X FLAG=[%s%s%s%s][%c%c%c%c%c%c%c%c%c]\n")
		_T("EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X\n")
		_T("ESP=%08X  EBP=%08X  ESI=%08X  EDI=%08X\n")
	    _T("%s%s")
	    _T("PC=%08X ")
		_T("EIP=%08X PREV_EIP=%08X PREV_ESP=%08X\n")
		_T("CRx=%08X %08X %08X %08X %08X MXCSR=%08X\n")
		_T("GDTR: BASE=%08X LIMIT=%08X / LDTR: BASE=%08X LIMIT=%08X\n")
		_T("IDTR: BASE=%08X LIMIT=%08X / TR:   BASE=%08X LIMIT=%08X\n")
	    _T("%s\n%s\n")
		_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
		(CPU_STAT_VM86) ? _T("VM86") : _T("    "),
		(CPU_STAT_PAGING) ? _T("PAGE") : _T("    "),
		(CPU_STAT_SS32) ? _T("SS32") : _T("    "),
		(CPU_STAT_WP) ? _T("WP") : _T("  "),
		CPU_STAT_USER_MODE, CPU_STAT_CPL,
		CPU_EFLAG,
		(CPU_EFLAG & RF_FLAG)  ? _T("RF  ") : _T("    "),
		(CPU_EFLAG & VM_FLAG)  ? _T("VM  ") : _T("    "),
		(CPU_EFLAG & VIF_FLAG) ? _T("VIF ") : _T("    "),
		(CPU_EFLAG & VIP_FLAG) ? _T("VIP ") : _T("    "),
		(CPU_FLAG & O_FLAG) ? _T('O') : _T('-'),
		(CPU_FLAG & D_FLAG) ? _T('D') : _T('-'),
		(CPU_FLAG & I_FLAG) ? _T('I') : _T('-'),
		(CPU_FLAG & T_FLAG) ? _T('T') : _T('-'),
		(CPU_FLAG & S_FLAG) ? _T('S') : _T('-'),
		(CPU_FLAG & Z_FLAG) ? _T('Z') : _T('-'),
		(CPU_FLAG & A_FLAG) ? _T('A') : _T('-'),
		(CPU_FLAG & P_FLAG) ? _T('P') : _T('-'),
		(CPU_FLAG & C_FLAG) ? _T('C') : _T('-'),

		CPU_EAX, CPU_EBX, CPU_ECX, CPU_EDX, CPU_ESP, CPU_EBP, CPU_ESI, CPU_EDI,
	    sregstr,
	    pde_desc,
	    get_pc(),
 	    CPU_EIP, CPU_PREV_EIP, CPU_PREV_ESP,
		CPU_CR0, CPU_CR1, CPU_CR2, CPU_CR3, CPU_CR4, CPU_MXCSR,
	    CPU_GDTR_BASE, CPU_GDTR_LIMIT, CPU_LDTR_BASE, CPU_LDTR_LIMIT,
	    CPU_IDTR_BASE, CPU_IDTR_LIMIT, CPU_TR_BASE, CPU_TR_LIMIT,
		dbgregstr, testregstr,
		total_cycles, total_cycles - prev_total_cycles,
		get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	} else {
		my_stprintf_s(buffer, buffer_len,
		_T("-- %s %s %s %s MODE=%01X CPL=%02X\n")
		_T("EFLAGS=%08X FLAG=[%s%s%s%s][%c%c%c%c%c%c%c%c%c]\n")
		_T("EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X  \nESP=%08X  EBP=%08X  ESI=%08X  EDI=%08X\n")
	    _T("%s")
	    _T("PC=%08X ")
		_T("IP=%04X  PREV_IP=%04X PREV_SP=%04X\n")
		_T("CRx=%08X %08X %08X %08X %08X MXCSR=%08X\n")
	    _T("%s\n%s\n")
		_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
		(CPU_STAT_VM86) ? _T("VM86 ") : _T("     "),
		(CPU_STAT_PAGING) ? _T("PAGE ") : _T("     "),
		(CPU_STAT_SS32) ? _T("SS32 ") : _T("     "),
		(CPU_STAT_WP) ? _T("WP ") : _T("   "),
		CPU_STAT_USER_MODE, CPU_STAT_CPL,
		CPU_EFLAG,
		(CPU_EFLAG & RF_FLAG) ? _T("RF  ") : _T("    "), (CPU_EFLAG & VM_FLAG) ? _T("VM  ") : _T("    "), (CPU_EFLAG & VIF_FLAG) ? _T("VIF ") : _T("    "), (CPU_EFLAG & VIP_FLAG) ? _T("VIP ") : _T("    "),
		(CPU_FLAG & O_FLAG) ? _T('O') : _T('-'), (CPU_FLAG & D_FLAG) ? _T('D') : _T('-'), (CPU_FLAG & I_FLAG) ? _T('I') : _T('-'), (CPU_FLAG & T_FLAG) ? _T('T') : _T('-'), (CPU_FLAG & S_FLAG) ? _T('S') : _T('-'),
		(CPU_FLAG & Z_FLAG) ? _T('Z') : _T('-'), (CPU_FLAG & A_FLAG) ? _T('A') : _T('-'), (CPU_FLAG & P_FLAG) ? _T('P') : _T('-'), (CPU_FLAG & C_FLAG) ? _T('C') : _T('-'),
		CPU_EAX, CPU_EBX, CPU_ECX, CPU_EDX, CPU_ESP, CPU_EBP, CPU_ESI, CPU_EDI,
		sregstr,
		(((CPU_STAT_SREGBASE(CPU_CS_INDEX) & 0xfffff ) + CPU_IP) & 0x000fffff),
		CPU_IP, CPU_STATSAVE.cpu_regs.prev_eip.w.w, CPU_STATSAVE.cpu_regs.prev_esp.w.w,
		CPU_CR0, CPU_CR1, CPU_CR2, CPU_CR3, CPU_CR4, CPU_MXCSR,
		dbgregstr, testregstr,
		total_cycles, total_cycles - prev_total_cycles,
		get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	}
#else
	my_stprintf_s(buffer, buffer_len,
				  _T("%s\n"), cpu_reg2str());
#endif
	prev_total_cycles = total_cycles;
	return true;
}

bool I386::debug_rewind_call_trace(uint32_t pc, int &size, _TCHAR* buffer, size_t buffer_len, uint64_t userdata)
{
	size = 0;
	_TCHAR prefix[128] = {0};
	if((userdata & ((uint64_t)0xffffffff << 32)) == ((uint64_t)0x80000000 << 32)) {
		my_stprintf_s(prefix, 127, _T("*RETURN*"));
	} else if((userdata & (uint64_t)0xffffffff00000000) != 0) {
		my_stprintf_s(prefix, 127, _T("*IRQ %X HAPPENED*"), (uint32_t)(userdata >> 32));
	}  else {
		my_stprintf_s(prefix, 127, _T("*CALL TO %08X*"), (uint32_t)userdata);
	}
	_TCHAR dasmbuf[1024] = {0};
	size = debug_dasm_with_userdata(pc, dasmbuf, 1023, userdata);
	if(size <= 0) {
		my_tcscpy_s(dasmbuf, 1023, _T("**UNDEFINED BEHAVIOR**"));
	}
	my_stprintf_s(buffer, buffer_len, _T("HIT %s	@%08X	%s\n"),
				  prefix, pc,  dasmbuf);
	return true;
}
int I386::debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata)
{
	uint32_t eip = pc - (CPU_CS << 4);
	uint8_t oprom[16];

	for(int i = 0; i < 16; i++) {
		int wait;
		oprom[i] = device_mem->read_data8w((pc + i) & CPU_ADRSMASK, &wait);
	}
	bool __op32 =  (userdata & I386_TRACE_DATA_BIT_USERDATA_SET) ? ((userdata & I386_TRACE_DATA_BIT_OP32) ? true : false) : ((CPU_INST_OP32 != 0) ? true : false);
	return i386_dasm(oprom, eip, __op32,  buffer, buffer_len);
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

//#ifdef I86_PSEUDO_BIOS
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
void I386::set_context_intr(DEVICE* device, uint32_t bit)
{
	device_pic = device;
}

void I386::set_context_debugger(DEBUGGER* deb)
{
	device_debugger = deb;
}

void *I386::get_debugger()
{
	return device_debugger;
}

#define STATE_VERSION	3

bool I386::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	// FIXME : This should dedicate ENDIAN.
	state_fio->StateBuffer(&CPU_STATSAVE, sizeof(CPU_STATSAVE), 1);
	state_fio->StateBuffer(&i386cpuid, sizeof(i386cpuid), 1);
	state_fio->StateBuffer(&i386msr, sizeof(i386msr), 1);
//#ifdef USE_DEBUGGER
	if(__USE_DEBUGGER) {
		state_fio->StateValue(total_cycles);
		state_fio->StateValue(prev_total_cycles);
	}
//#endif
	state_fio->StateValue(remained_cycles);
	state_fio->StateValue(extra_cycles);
	state_fio->StateValue(busreq);
	state_fio->StateValue(nmi_pending);
	state_fio->StateValue(irq_pending);
	state_fio->StateValue(PREV_CS_BASE);

	state_fio->StateValue(waitfactor);
	state_fio->StateValue(waitcount);
	state_fio->StateValue(address_mask);

	return true;
}
